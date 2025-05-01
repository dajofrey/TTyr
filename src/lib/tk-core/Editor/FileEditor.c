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

#include "nh-core/System/Process.h"
#include "nh-core/System/Memory.h"
#include "nh-core/Util/LinkedList.h"

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

// INIT/FREE =======================================================================================

tk_core_FileEditor tk_core_initFileEditor()
{
    tk_core_FileEditor Editor;
    Editor.Files = nh_core_initLinkedList();
    Editor.current = 0;
    Editor.minCols = 60;
    Editor.tabSpaces = 4;
    Editor.tabToSpaces = true;

    return Editor;
}

TTYR_CORE_RESULT tk_core_freeFileEditor(
     tk_core_FileEditor *FileEditor_p)
{
    while (1) {
        tk_core_File *File_p = nh_core_getFromLinkedList(&FileEditor_p->Files, 0);
        if (!File_p) {break;}
        tk_core_closeFile(FileEditor_p, File_p);
    }

    nh_core_destroyLinkedList(&FileEditor_p->Files, true);

    return TTYR_CORE_SUCCESS;
}

// RENDER ==========================================================================================

static TTYR_CORE_RESULT tk_core_renderFileEditor(
    tk_core_FileEditor *FileEditor_p)
{
    for (int i = 0; i < FileEditor_p->Files.count; ++i) {
        tk_core_File *File_p = nh_core_getFromLinkedList(&FileEditor_p->Files, i);
        TTYR_CHECK_NULL(File_p)
        TTYR_CHECK(tk_core_renderFile(File_p))
    }

    return TTYR_CORE_SUCCESS;
}

// FILE ============================================================================================

static bool tk_core_hasFile(
    tk_core_FileEditor *Editor_p, tk_core_File *File_p)
{
    bool hasFile = false;

    for (int i = 0; i < Editor_p->Files.count; ++i) {
        tk_core_File *Compare_p = nh_core_getFromLinkedList(&Editor_p->Files, i);
        if (File_p == Compare_p) {
            hasFile = true;
            break;
        }
    }

    return hasFile;
}

// TODO Fix possible memory leaks.
tk_core_File *tk_core_openFile(
    tk_core_Program *Program_p, tk_core_TreeListingNode *Node_p, bool readOnly)
{
    tk_core_FileEditor *FileEditor_p = &((tk_core_Editor*)Program_p->handle_p)->FileEditor;
    tk_core_FileEditorView *FileEditorView_p = &((tk_core_Editor*)Program_p->handle_p)->View.FileEditor;

    tk_core_File *File_p = (tk_core_File*)nh_core_allocate(sizeof(tk_core_File));
    TTYR_CHECK_MEM_2(NULL, File_p)

    File_p->type      = tk_core_getFileType(&Node_p->Path);
    File_p->Node_p    = Node_p;
    File_p->handle_p  = NULL;
    File_p->readOnly  = readOnly;

    switch (File_p->type) 
    {
        case TTYR_CORE_FILE_TEXT :
            File_p->handle_p = tk_core_openTextFile(&Node_p->Path); 
            break;
        case TTYR_CORE_FILE_CHANGES :
            break;
    }

    TTYR_CHECK_NULL_2(NULL, File_p->handle_p)
    TTYR_CHECK_2(NULL, tk_core_createFileViews(FileEditorView_p, File_p))

    nh_core_prependToLinkedList(&FileEditor_p->Files, File_p);
    FileEditor_p->current = 0;
    Node_p->File_p = File_p;

    TTYR_CHECK_2(NULL, tk_core_renderFileEditor(FileEditor_p))

    TTYR_CHECK_2(NULL, tk_core_setCustomSuffixMessage(
        NULL, TTYR_CORE_MESSAGE_EDITOR_FILE_OPENED, Node_p->Path.p, Node_p->Path.length
    ))

    return File_p;
}

TTYR_CORE_RESULT tk_core_closeFile(
    tk_core_FileEditor *Editor_p, tk_core_File *File_p)
{
    if (File_p == NULL || !tk_core_hasFile(Editor_p, File_p)) {
        return TTYR_CORE_ERROR_BAD_STATE;
    }

    switch (File_p->type)
    {
        case TTYR_CORE_FILE_TEXT :
            tk_core_closeTextFile(File_p->handle_p);
            break;
    }

    nh_core_removeFromLinkedList2(&Editor_p->Files, File_p, true);

    return TTYR_CORE_SUCCESS;
}

// HELP TEXT =======================================================================================

static const char *help_pp[] =
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

TTYR_CORE_RESULT tk_core_cycleThroughFiles(
    tk_core_Program *Program_p, NH_API_UTF32 c)
{
    tk_core_FileEditor *FileEditor_p = &((tk_core_Editor*)Program_p->handle_p)->FileEditor;

    switch (c)
    {
        case 'f' :
        {
            // Handle model.
            if (FileEditor_p->current > 0) {
                FileEditor_p->current--;
            }

            // Handle view.
            tk_core_EditorView *View_p = &((tk_core_Editor*)Program_p->handle_p)->View;
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
            tk_core_EditorView *View_p = &((tk_core_Editor*)Program_p->handle_p)->View;
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

    tk_core_File *File_p =
        nh_core_getFromLinkedList(&FileEditor_p->Files, FileEditor_p->current);

    if (File_p != NULL) {
        // Move tree listing cursor to focused file.
        TTYR_CHECK(tk_core_setTreeListingCursor(Program_p, File_p))
        // Update message.
        tk_core_setCustomSuffixMessage(NULL, TTYR_CORE_MESSAGE_EDITOR_FILE_EDIT, File_p->Node_p->Path.p, File_p->Node_p->Path.length);
    }

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT tk_core_handleFileEditorInput(
    tk_core_Program *Program_p, NH_API_UTF32 c)
{
    tk_core_Editor *Editor_p = Program_p->handle_p;
    tk_core_FileEditor *FileEditor_p = &((tk_core_Editor*)Editor_p)->FileEditor;
    tk_core_File *File_p = nh_core_getFromLinkedList(&FileEditor_p->Files, FileEditor_p->current);

    if (File_p != NULL) {
        TTYR_CHECK(tk_core_handleFileInput(Program_p, File_p, c, Editor_p->insertMode, &Program_p->refresh))

        switch (c) {
            case 'l' :
            case 'h' : Editor_p->treeListing = false; break;
        }
    }

    return TTYR_CORE_SUCCESS;
}

// DRAW ============================================================================================

static TTYR_CORE_RESULT tk_core_drawHelp(
    tk_core_Glyph *Glyphs_p, int width, int line, int lines, int scroll)
{
    line += scroll;

//    char *text_p = "No file opened.";
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

    return TTYR_CORE_SUCCESS;
}

static TTYR_CORE_RESULT tk_core_updateFileEditorView(
    tk_core_FileEditor *FileEditor_p, tk_core_EditorView *View_p, int width)
{
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

    TTYR_CHECK(tk_core_updateFileViews(View_p))

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT tk_core_drawFileEditorRow(
    tk_core_Program *Program_p, tk_core_Glyph *Glyphs_p, int width, int height, int row)
{
    tk_core_Editor *Editor_p = Program_p->handle_p;
    tk_core_FileEditor *FileEditor_p = &Editor_p->FileEditor;
    tk_core_FileEditorView *View_p = &Editor_p->View.FileEditor;

    TTYR_CHECK(tk_core_updateFileEditorView(FileEditor_p, &Editor_p->View, width))

    if (FileEditor_p->Files.count == 0) {
        TTYR_CHECK(tk_core_drawHelp(Glyphs_p, View_p->width, row, height, View_p->helpScroll))
    }

    for (int i = View_p->currentOffset, width2 = 0, count = 0; i < FileEditor_p->Files.count; ++i, ++count) {
        if (count == View_p->maxOnScreen) {break;}
        tk_core_File *File_p = nh_core_getFromLinkedList(&FileEditor_p->Files, i);
        tk_core_FileView *FileView_p = tk_core_getFileView(&Editor_p->View, File_p);
        TTYR_CHECK_NULL(File_p)
        TTYR_CHECK_NULL(FileView_p)
        TTYR_CHECK(tk_core_drawFileRow(Program_p, File_p, FileView_p, Glyphs_p+width2, row))
        width2 += FileView_p->width;
    }

    return TTYR_CORE_SUCCESS;
}

