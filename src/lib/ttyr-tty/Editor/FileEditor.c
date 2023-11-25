// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "FileEditor.h"
#include "TreeListing.h"
#include "TextFile.h"
#include "TextFileInput.h"
#include "Editor.h"
#include "SyntaxHighlights.h"

#include "../TTY/TTY.h"
#include "../Common/Macros.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Process.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/LinkedList.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Common/Macros.h"

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

// INIT/FREE =======================================================================================

ttyr_tty_FileEditor ttyr_tty_initFileEditor()
{
TTYR_TTY_BEGIN()

    ttyr_tty_FileEditor Editor;
    Editor.Files = nh_core_initLinkedList();
    Editor.current = 0;
    Editor.minCols = 60;
    Editor.tabSpaces = 4;
    Editor.tabToSpaces = NH_TRUE;

TTYR_TTY_END(Editor)
}

TTYR_TTY_RESULT ttyr_tty_freeFileEditor(
     ttyr_tty_FileEditor *FileEditor_p)
{
TTYR_TTY_BEGIN()

    while (1) {
        ttyr_tty_File *File_p = nh_core_getFromLinkedList(&FileEditor_p->Files, 0);
        if (!File_p) {break;}
        ttyr_tty_closeFile(FileEditor_p, File_p);
    }

    nh_core_destroyLinkedList(&FileEditor_p->Files, NH_TRUE);

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// RENDER ==========================================================================================

static TTYR_TTY_RESULT ttyr_tty_renderFileEditor(
    ttyr_tty_FileEditor *FileEditor_p)
{
TTYR_TTY_BEGIN()

    for (int i = 0; i < FileEditor_p->Files.count; ++i) {
        ttyr_tty_File *File_p = nh_core_getFromLinkedList(&FileEditor_p->Files, i);
        TTYR_TTY_CHECK_NULL(File_p)
        TTYR_TTY_CHECK(ttyr_tty_renderFile(File_p))
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// FILE ============================================================================================

static NH_BOOL ttyr_tty_hasFile(
    ttyr_tty_FileEditor *Editor_p, ttyr_tty_File *File_p)
{
TTYR_TTY_BEGIN()

    NH_BOOL hasFile = NH_FALSE;

    for (int i = 0; i < Editor_p->Files.count; ++i) {
        ttyr_tty_File *Compare_p = nh_core_getFromLinkedList(&Editor_p->Files, i);
        if (File_p == Compare_p) {
            hasFile = NH_TRUE;
            break;
        }
    }

TTYR_TTY_END(hasFile)
}

// TODO Fix possible memory leaks.
ttyr_tty_File *ttyr_tty_openFile(
    ttyr_tty_Program *Program_p, ttyr_tty_TreeListingNode *Node_p, NH_BOOL readOnly)
{
TTYR_TTY_BEGIN()

    ttyr_tty_FileEditor *FileEditor_p = &((ttyr_tty_Editor*)Program_p->handle_p)->FileEditor;
    ttyr_tty_FileEditorView *FileEditorView_p = &((ttyr_tty_Editor*)Program_p->handle_p)->View.FileEditor;

    ttyr_tty_File *File_p = nh_core_allocate(sizeof(ttyr_tty_File));
    TTYR_TTY_CHECK_MEM_2(NULL, File_p)

    File_p->type      = ttyr_tty_getFileType(&Node_p->Path);
    File_p->Node_p    = Node_p;
    File_p->handle_p  = NULL;
    File_p->readOnly  = readOnly;

    switch (File_p->type) 
    {
        case TTYR_TTY_FILE_TEXT :
            File_p->handle_p = ttyr_tty_openTextFile(&Node_p->Path); 
            break;
        case TTYR_TTY_FILE_CHANGES :
            break;
    }

    TTYR_TTY_CHECK_NULL_2(NULL, File_p->handle_p)
    TTYR_TTY_CHECK_2(NULL, ttyr_tty_createFileViews(FileEditorView_p, File_p))

    nh_core_prependToLinkedList(&FileEditor_p->Files, File_p);
    FileEditor_p->current = 0;
    Node_p->File_p = File_p;

    TTYR_TTY_CHECK_2(NULL, ttyr_tty_renderFileEditor(FileEditor_p))

    TTYR_TTY_CHECK_2(NULL, ttyr_tty_setCustomSuffixMessage(
        NULL, TTYR_TTY_MESSAGE_EDITOR_FILE_OPENED, Node_p->Path.p, Node_p->Path.length
    ))

TTYR_TTY_END(File_p)
}

TTYR_TTY_RESULT ttyr_tty_closeFile(
    ttyr_tty_FileEditor *Editor_p, ttyr_tty_File *File_p)
{
TTYR_TTY_BEGIN()

    if (File_p == NULL || !ttyr_tty_hasFile(Editor_p, File_p)) {
        TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_ERROR_BAD_STATE)
    }

    switch (File_p->type)
    {
        case TTYR_TTY_FILE_TEXT :
            ttyr_tty_closeTextFile(File_p->handle_p);
            break;
    }

    nh_core_removeFromLinkedList2(&Editor_p->Files, File_p, NH_TRUE);

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// HELP TEXT =======================================================================================

static const NH_BYTE *help_pp[] =
{
    "General Controls                                       ",
    "                                                       ",
    "  [w][a][s][d]                                         ",
    "                                                       ",
    "  Press [w] and [s] to navigate up and down the tree   ",
    "  listing. Press [d] to open the current node. If the  ",
    "  node is a file, the file will be opened for editing. ",
    "  Press [d] twice on a directory node to set it as the ",
    "  root node of the directory tree. Press [d] twice on a",
    "  file node to save the file. Press [a] to close the   ",
    "  current node, this will close any related files as   ",
    "  well.                                                ",
    "                                                       ",
    "  [f][g]                                               ",
    "                                                       ",
    "  Press [f] to go to the left neighbour file (if any). ",
    "  Press [g] to go to the right neighbour file (if any).",
    "                                                       ", 
    "  [i]                                                  ",
    "                                                       ",
    "  Press [i] to toggle INSERT MODE. Please note that you",
    "  need to press [ESC] or [CTRL] + [i] to escape insert ",
    "  mode. INSERT MODE allows for char insertion if the   ",
    "  file-type supports it.                               ",
    "                                                       ",
    "  [h][j][k][l]                                         ",
    "                                                       ",
    "  Press [h] and [l] to move the cursor left/right.     ",
    "  Press [k] and [j] to move the cursor up/down.        ",
    "",
    "Insert Mode Controls                                   ",
    "                                                       ",
    "  [CTRL] + [h][j][k][l]                                ",
    "                                                       ",
    "  Press [CTRL] + [h][l] to move the cursor left/right. ",
    "  Press [CTRL] + [k][j] to move the cursor up/down.    ",
    "                                                       ",
    "  [CTRL] + [u] Backward delete.                        ",
    "  [CTRL] + [x] Delete line.                            ",
    "  [CTRL] + [z] Undo.                                   ",
    "  [CTRL] + [y] Redo.                                   ",
    "  [CTRL] + [c] Select and Copy.                        ",
    "  [CTRL] + [v] Paste.                                  ",
    "",
    "General Commands                                       ",
    "",
    "\"tree\"                                               ",
    "    Toggle tree-listing.                               ",
    "\"preview\"                                            ",
    "    Toggle file preview.                               ",
    "\"new (name)\"                                         ",
    "    Create a new file in the current directory.        ",
    "",
};

// INPUT ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_cycleThroughFiles(
    ttyr_tty_Program *Program_p, NH_ENCODING_UTF32 c)
{
TTYR_TTY_BEGIN()

    ttyr_tty_FileEditor *FileEditor_p = &((ttyr_tty_Editor*)Program_p->handle_p)->FileEditor;

    switch (c)
    {
        case 'f' :
        {
            // Handle model.
            if (FileEditor_p->current > 0) {
                FileEditor_p->current--;
            }

            // Handle view.
            ttyr_tty_EditorView *View_p = &((ttyr_tty_Editor*)Program_p->handle_p)->View;
            if (View_p->FileEditor.currentOffset > 0) {
                if (View_p->FileEditor.fromLeft > 0 ) {
                    View_p->FileEditor.fromLeft--;
                }
                if (View_p->FileEditor.fromLeft == 0) {
                    View_p->FileEditor.currentOffset--;
                }
            }

            break;
        }
        case 'g' :
        {
            if (FileEditor_p->current == FileEditor_p->Files.count-1) {break;}

            // Handle model.
            FileEditor_p->current++;

            // Handle view.
            ttyr_tty_EditorView *View_p = &((ttyr_tty_Editor*)Program_p->handle_p)->View;
            if (View_p->FileEditor.currentOffset > 0) {
                View_p->FileEditor.fromLeft++;
            }
            if (FileEditor_p->current - View_p->FileEditor.currentOffset == View_p->FileEditor.maxOnScreen) {
                View_p->FileEditor.currentOffset++;
                View_p->FileEditor.fromLeft = View_p->FileEditor.maxOnScreen;
            }
        
            break;
        }
    }

    ttyr_tty_File *File_p =
        nh_core_getFromLinkedList(&FileEditor_p->Files, FileEditor_p->current);

    if (File_p != NULL) {
        // Move tree listing cursor to focused file.
        TTYR_TTY_CHECK(ttyr_tty_setTreeListingCursor(Program_p, File_p))
        // Update message.
        ttyr_tty_setCustomSuffixMessage(NULL, TTYR_TTY_MESSAGE_EDITOR_FILE_EDIT, File_p->Node_p->Path.p, File_p->Node_p->Path.length);
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_handleFileEditorInput(
    ttyr_tty_Program *Program_p, NH_ENCODING_UTF32 c)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Editor *Editor_p = Program_p->handle_p;
    ttyr_tty_FileEditor *FileEditor_p = &((ttyr_tty_Editor*)Editor_p)->FileEditor;
    ttyr_tty_File *File_p = nh_core_getFromLinkedList(&FileEditor_p->Files, FileEditor_p->current);

    if (File_p != NULL) {
        TTYR_TTY_CHECK(ttyr_tty_handleFileInput(Program_p, File_p, c, Editor_p->insertMode, &Program_p->refresh))

        switch (c) {
            case 'l' :
            case 'h' : Editor_p->treeListing = NH_FALSE; break;
        }
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// DRAW ============================================================================================

static TTYR_TTY_RESULT ttyr_tty_drawHelp(
    ttyr_tty_Glyph *Glyphs_p, int width, int line, int lines, int scroll)
{
TTYR_TTY_BEGIN()

    line += scroll;

//    NH_BYTE *text_p = "No file opened.";
//    if (lines/2 == line) {
//        for (int i = (width-strlen(text_p))/2, j = 0; j < strlen(text_p); ++i, ++j) {
//            Glyphs_p[i].codepoint = text_p[j];
//        }
//    }
//
//    if (line < sizeof(help_pp)/sizeof(help_pp[0]))
//    {
//        int len = strlen(help_pp[line]);
//        if (width-len <= 0) {
//            for (int i = 0; i < width; ++i) {
//                Glyphs_p[i].codepoint = help_pp[line][i];
//            }
//        } else {
//            for (int i = (width-len)/2, j = 0; j < strlen(help_pp[line]); ++i, ++j) {
//                Glyphs_p[i].codepoint = help_pp[line][j];
//            }
//        }
//    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

static TTYR_TTY_RESULT ttyr_tty_updateFileEditorView(
    ttyr_tty_FileEditor *FileEditor_p, ttyr_tty_EditorView *View_p, int width)
{
TTYR_TTY_BEGIN()

    View_p->FileEditor.width = width;

    int oldMaxOnScreen = View_p->FileEditor.maxOnScreen;
    View_p->FileEditor.maxOnScreen = width / FileEditor_p->minCols;
    if (View_p->FileEditor.maxOnScreen <= 0) {
        View_p->FileEditor.maxOnScreen = 1;
    }
    if (oldMaxOnScreen != View_p->FileEditor.maxOnScreen) {
        FileEditor_p->current = 0;
        View_p->FileEditor.currentOffset = 0;
    }

    TTYR_TTY_CHECK(ttyr_tty_updateFileViews(View_p))

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_drawFileEditorRow(
    ttyr_tty_Program *Program_p, ttyr_tty_Glyph *Glyphs_p, int width, int height, int row)
{
TTYR_TTY_BEGIN()

    ttyr_tty_Editor *Editor_p = Program_p->handle_p;
    ttyr_tty_FileEditor *FileEditor_p = &Editor_p->FileEditor;
    ttyr_tty_FileEditorView *View_p = &Editor_p->View.FileEditor;

    TTYR_TTY_CHECK(ttyr_tty_updateFileEditorView(FileEditor_p, &Editor_p->View, width))

    if (FileEditor_p->Files.count == 0) {
        TTYR_TTY_CHECK(ttyr_tty_drawHelp(Glyphs_p, View_p->width, row, height, View_p->helpScroll))
    }

    for (int i = View_p->currentOffset, width2 = 0, count = 0; i < FileEditor_p->Files.count; ++i, ++count) {
        if (count == View_p->maxOnScreen) {break;}
        ttyr_tty_File *File_p = nh_core_getFromLinkedList(&FileEditor_p->Files, i);
        ttyr_tty_FileView *FileView_p = ttyr_tty_getFileView(&Editor_p->View, File_p);
        TTYR_TTY_CHECK_NULL(File_p)
        TTYR_TTY_CHECK_NULL(FileView_p)
        TTYR_TTY_CHECK(ttyr_tty_drawFileRow(Program_p, File_p, FileView_p, Glyphs_p+width2, row))
        width2 += FileView_p->width;
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

