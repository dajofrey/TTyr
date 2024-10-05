// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Editor.h"
#include "TextFile.h"

#include "../TTY/TTY.h"
#include "../Common/Macros.h"

#include "nh-core/System/Logger.h"
#include "nh-core/System/Memory.h"

#include "nh-encoding/Common/Macros.h"
#include "nh-encoding/Encodings/UTF32.h"
#include "nh-encoding/Encodings/UTF8.h"

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

// VIEW ============================================================================================

static ttyr_tty_EditorView ttyr_tty_initEditorView()
{
    ttyr_tty_EditorView View;

    memset(&View, 0, sizeof(ttyr_tty_EditorView));
    View.FileEditor.FileViews = nh_core_initList(8);

    return View;
}

static void ttyr_tty_freeEditorView(
    ttyr_tty_EditorView *View_p)
{
    nh_core_freeList(&View_p->FileEditor.FileViews, true);
}

// UPDATE ==========================================================================================

static void ttyr_tty_closeTreeListingNode(
    ttyr_tty_FileEditor *Editor_p, ttyr_tty_TreeListingNode *Node_p)
{
//    for (int i = 0; i < Node_p->Children.size; ++i) {
//        ttyr_tty_TreeListingNode *Child_p = Node_p->Children.pp[i];
//        nh_encoding_freeUTF32(&Child_p->Path);
//        ttyr_tty_closeTreeListingNode(Editor_p, Child_p);
//    }
//
//    if (Node_p->File_p) {ttyr_tty_closeFile(Editor_p, Node_p->File_p);}
//
//    nh_core_freeList(&Node_p->Children, true);
}

static TTYR_TTY_RESULT ttyr_tty_handleDeletedNodes(
    ttyr_tty_FileEditor *Editor_p, ttyr_tty_TreeListingNode *Node_p)
{
//#ifdef __unix__
//
//    struct dirent **namelist_pp = NULL;
//    nh_encoding_UTF8String CurrentPath = nh_encoding_encodeUTF8(Node_p->Path.p, Node_p->Path.length);
//
//    int n = scandir(CurrentPath.p, &namelist_pp, 0, alphasort);
//    if (n < 0) {TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_ERROR_BAD_STATE)}
//
//    nh_encoding_freeUTF8(&CurrentPath);
//
//    for (int i = 0; i < Node_p->Children.size; ++i)
//    {
//        ttyr_tty_TreeListingNode *Child_p = Node_p->Children.pp[i];
//        if (Child_p->unsaved) {continue;}
//        
//        bool match = false;
//        for (int j = 0; j < n; ++j)
//        {
//            if (!strcmp(namelist_pp[j]->d_name, ".") || !strcmp(namelist_pp[j]->d_name, "..")) {continue;}
//
//            nh_encoding_UTF32String NewPath = nh_encoding_initUTF32(128);
//            nh_encoding_UTF32String FileName = nh_encoding_decodeUTF8(namelist_pp[j]->d_name, strlen(namelist_pp[j]->d_name), NULL);
//
//            if (Node_p->Path.p[Node_p->Path.length - 1] != '/') {
//                nh_encoding_appendUTF32(&NewPath, Node_p->Path.p, Node_p->Path.length);
//                nh_encoding_appendUTF32Codepoint(&NewPath, '/');
//                nh_encoding_appendUTF32(&NewPath, FileName.p, FileName.length);
//            } else {
//                nh_encoding_appendUTF32(&NewPath, Node_p->Path.p, Node_p->Path.length);
//                nh_encoding_appendUTF32(&NewPath, FileName.p, FileName.length);
//            }
//
//            if (nh_encoding_compareUTF32(&Child_p->Path, &NewPath)) {match = true;}
//
//            nh_encoding_freeUTF32(&NewPath);
//            nh_encoding_freeUTF32(&FileName);
//        }
//
//        if (!match) {
//            ttyr_tty_closeTreeListingNode(Editor_p, Child_p);
//            nh_core_removeFromList(&Node_p->Children, true, i);
//            i--;
//            *updated_p = true;
//            continue;
//        }
//
//        if (Child_p->Children.size > 0) {
//            TTYR_CHECK(ttyr_tty_handleDeletedNodes(Editor_p, Child_p, updated_p))
//        }
//    }
//
//    if (namelist_pp) {
//        for (int i = 0; i < n; ++i) {
//            free(namelist_pp[i]);
//        }
//        free(namelist_pp);
//    }
//
//#elif defined(_WIN32) || defined(WIN32)
//
//    printf("microsoft windows not supported\n");
//    exit(0);
//
//#endif

    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_updateEditor(
    ttyr_tty_Program *Program_p)
{
    ttyr_tty_Editor *Editor_p = Program_p->handle_p;

    if (!Editor_p->treeListing) {
        return TTYR_TTY_SUCCESS;
    }

    nh_core_SystemTime Now = nh_core_getSystemTime();
    if (nh_core_getSystemTimeDiffInSeconds(Editor_p->LastUpdate, Now) < Editor_p->updateIntervalInSeconds) {
        return TTYR_TTY_SUCCESS;
    }

    TTYR_CHECK(ttyr_tty_handleDeletedNodes(&Editor_p->FileEditor, Editor_p->TreeListing.Root_p))

    Editor_p->LastUpdate = Now;
    Editor_p->TreeListing.dirty = true;
    Program_p->refresh = true;
    Editor_p->LastUpdate = Now;

    return TTYR_TTY_SUCCESS;
}

// INPUT ===========================================================================================

static TTYR_TTY_RESULT ttyr_tty_handleEditorInput(
    ttyr_tty_Program *Program_p, nh_api_WSIEvent Event)
{
    if (Event.type != NH_API_WSI_EVENT_KEYBOARD) {return TTYR_TTY_SUCCESS;}
    if (Event.Keyboard.trigger != NH_API_TRIGGER_PRESS) {return TTYR_TTY_SUCCESS;}

    NH_API_UTF32 c = Event.Keyboard.codepoint;
    ttyr_tty_Editor *Editor_p = Program_p->handle_p;

    switch (c) 
    {
        case 'i' : if (Editor_p->insertMode) {goto FILE_EDITOR_INPUT;}
        case CTRL_KEY('i') :
        case 27 :
            Editor_p->insertMode = c == 27 ? false : !Editor_p->insertMode; 
            TTYR_CHECK(ttyr_tty_setDefaultMessage(
                NULL, Editor_p->insertMode ? TTYR_TTY_MESSAGE_EDITOR_INSERT_ACTIVATED : TTYR_TTY_MESSAGE_EDITOR_INSERT_DEACTIVATED 
            ))
            Program_p->refresh = true;
            break;

        case 'f' :
        case 'g' : if (Editor_p->insertMode) {goto FILE_EDITOR_INPUT;}

            // Switching file focus is mostly done in the file editor using the next function.
            TTYR_CHECK(ttyr_tty_cycleThroughFiles(Program_p, c))
            Program_p->refresh = true;
            break;

        case 'w' :
        case 'a' :
        case 's' :
        case 'd' : if (Editor_p->insertMode) {goto FILE_EDITOR_INPUT;}
            if  (Editor_p->treeListing) {
                TTYR_CHECK(ttyr_tty_handleTreeListingInput(Program_p, c))
            }
            Editor_p->treeListing = true;
            Program_p->refresh = true;
            break;

        default :
            goto FILE_EDITOR_INPUT;
    }
 
    return TTYR_TTY_SUCCESS;

FILE_EDITOR_INPUT :

    TTYR_CHECK(ttyr_tty_handleFileEditorInput(Program_p, c))
    return TTYR_TTY_SUCCESS;
}

// DRAW ============================================================================================

static TTYR_TTY_RESULT ttyr_tty_drawEditor(
    ttyr_tty_Program *Program_p, ttyr_tty_Glyph *Glyphs_p, int width, int height, int row)
{
    ttyr_tty_Editor *Editor_p = Program_p->handle_p;
    Editor_p->View.height = height;

    // First, normalize glyphs.
    for (int i = 0; i < width; ++i) {
        Glyphs_p[i].codepoint = ' ';
    }

    // Draw tree listing row if necessary.
    if (Editor_p->treeListing) {
        TTYR_CHECK(ttyr_tty_drawTreeListingRow(Program_p, Glyphs_p, width, height, row))
        Glyphs_p += Editor_p->View.TreeListing.width;
    }
    
    // Draw file editor row.
    TTYR_CHECK(ttyr_tty_drawFileEditorRow(
        Program_p, Glyphs_p, Editor_p->treeListing ? width - Editor_p->View.TreeListing.width : width, 
        height, row
    ))

    return TTYR_TTY_SUCCESS;
}

static TTYR_TTY_RESULT ttyr_tty_drawEditorTopbar(
    ttyr_tty_Program *Program_p, ttyr_tty_Glyph *Glyphs_p, int width)
{
    ttyr_tty_Editor *Editor_p = Program_p->handle_p;
    ttyr_tty_File *File_p = nh_core_getFromLinkedList(&Editor_p->FileEditor.Files, Editor_p->FileEditor.current);

    char topbar_p[1024] = {0};
    memset(topbar_p, ' ', 1024);

    if (File_p) {
        nh_encoding_UTF8String Path = nh_encoding_encodeUTF8(File_p->Node_p->Path.p, File_p->Node_p->Path.length);
        int offset = (width / 2) - (strlen(Path.p) / 2);
        if (offset >= 0) {
            sprintf(topbar_p+offset, "%s", Path.p);
        }
        nh_encoding_freeUTF8(&Path);
    } else {
        char buf_p[] = "No file open";
        int offset = (width / 2) - (strlen(buf_p) / 2);
        if (offset >= 0) {
            sprintf(topbar_p+offset, "%s", buf_p);
        }
    }

    for (int i = 0; i < width; ++i) {
        Glyphs_p[i].codepoint = topbar_p[i];
    }

    return TTYR_TTY_SUCCESS;
}

// CURSOR ==========================================================================================

static TTYR_TTY_RESULT ttyr_tty_getEditorCursor(
    ttyr_tty_Program *Program_p, int *x_p, int *y_p)
{
    ttyr_tty_Editor *Editor_p = Program_p->handle_p;
    ttyr_tty_FileView *FileView_p = NULL;

    int x = Editor_p->treeListing ? Editor_p->View.TreeListing.width : 0;

    ttyr_tty_File *File_p = NULL;
    for (int i = Editor_p->View.FileEditor.currentOffset; i < Editor_p->FileEditor.Files.count; ++i) {
        File_p = nh_core_getFromLinkedList(&Editor_p->FileEditor.Files, i);
        FileView_p = ttyr_tty_getFileView(&Editor_p->View, File_p);
        if (i == Editor_p->FileEditor.current) {break;}
        x += FileView_p->width;
    }

    if (File_p != NULL && File_p->type == TTYR_TTY_FILE_TEXT) 
    {
        ttyr_tty_TextFile *TextFile_p = File_p->handle_p;
        *x_p = FileView_p->TextFile.screenCursorX + TextFile_p->lineNumberOffset + x;
        *y_p = FileView_p->TextFile.screenCursorY;
    }
    else {
        *x_p = -1;
        *y_p = -1;
    }

    return TTYR_TTY_SUCCESS;
}

// COMMANDS ========================================================================================

static TTYR_TTY_RESULT ttyr_tty_executeEditorCommand(
    ttyr_tty_Program *Program_p)
{
    ttyr_tty_Editor *Editor_p = Program_p->handle_p;

//    switch (Program_p->command)
//    {
//        case TTYR_TTY_EDITOR_COMMAND_PREVIEW:
//
//            Editor_p->TreeListing.preview = !Editor_p->TreeListing.preview; 
//            TTYR_CHECK(ttyr_tty_setDefaultMessage(
//               &ttyr_tty_getTTY()->Tab_p->Tile_p->Status,
//               Editor_p->TreeListing.preview ? TTYR_TTY_MESSAGE_EDITOR_PREVIEW_ENABLED : TTYR_TTY_MESSAGE_EDITOR_PREVIEW_DISABLED
//            ))
//            Program_p->refresh = true;
//            break;
//
//        case TTYR_TTY_EDITOR_COMMAND_TREE:
//
//            Editor_p->treeListing = !Editor_p->treeListing; 
//            TTYR_CHECK(ttyr_tty_setDefaultMessage(
//               &ttyr_tty_getTTY()->Tab_p->Tile_p->Status,
//               Editor_p->treeListing ? TTYR_TTY_MESSAGE_EDITOR_SHOW_TREE : TTYR_TTY_MESSAGE_EDITOR_HIDE_TREE 
//            ))
//            Program_p->refresh = true;
//            break;
//
//        case TTYR_TTY_EDITOR_COMMAND_NEW:
//        {
//            if (Arguments_p->size != 1) {TTYR_TTY_END(TTYR_TTY_ERROR_INVALID_ARGUMENT)}
//            nh_encoding_UTF32String *Argument_p = Arguments_p->pp[0];
//
//            ttyr_tty_TreeListingNode *Node_p = 
//                ttyr_tty_insertTreeListingNode(&Editor_p->TreeListing, Argument_p->p, Argument_p->length);
//            TTYR_CHECK_NULL(Node_p)
//
//            ttyr_tty_File *File_p = ttyr_tty_openFile(Program_p, Node_p, false);
//            TTYR_CHECK_NULL(File_p)
//
//            TTYR_CHECK(ttyr_tty_setDefaultMessage(
//                &ttyr_tty_getTTY()->Tab_p->Tile_p->Status, TTYR_TTY_MESSAGE_EDITOR_NEW_FILE
//            ))
//
//            Program_p->refresh = true;
//            TTYR_CHECK(ttyr_tty_setTreeListingCursor(Program_p, File_p))
//            break;
//        }
//
//        case TTYR_TTY_EDITOR_COMMAND_SEARCH:
//        {
//            if (Editor_p->FileEditor.Files.count > 0) 
//            {
//                ttyr_tty_File *File_p = 
//                    nh_core_getFromLinkedList(&Editor_p->FileEditor.Files, Editor_p->FileEditor.current);
//
//                if (Arguments_p->size == 0) {
//                    TTYR_CHECK(ttyr_tty_clearFileSearch(File_p))
//                }
//                else {
//                    nh_encoding_UTF32String *Argument_p = Arguments_p->pp[0];
//                    TTYR_CHECK(ttyr_tty_searchFile(File_p, Argument_p->p, Argument_p->length))
//                }
//            }
//            break;
//        }
//
//        case TTYR_TTY_EDITOR_COMMAND_TAB_SPACES:
//
//            if (Arguments_p->size == 1) {
//                nh_encoding_UTF32String *Argument_p = Arguments_p->pp[0];
//                nh_encoding_UTF8String Argument = nh_encoding_encodeUTF8(Argument_p->p, Argument_p->length);
//                Editor_p->FileEditor.tabSpaces = strtol(Argument.p, NULL, 10);
//                nh_encoding_freeUTF8(&Argument);
//            }
//
//            char tabSpaces_p[16];
//            sprintf(tabSpaces_p, "%d", Editor_p->FileEditor.tabSpaces);
//            nh_encoding_UTF32String TabSpaces = nh_encoding_decodeUTF8(tabSpaces_p, strlen(tabSpaces_p), NULL);
//
//            TTYR_CHECK(ttyr_tty_setCustomMessage(
//               &ttyr_tty_getTTY()->Tab_p->Tile_p->Status, TTYR_TTY_MESSAGE_EDITOR_NUMBER_OF_TAB_SPACES,
//               TabSpaces.p, TabSpaces.length
//            ))
//
//            nh_encoding_freeUTF32(&TabSpaces);
//            break;
//
//        case TTYR_TTY_EDITOR_COMMAND_TAB_TO_SPACES:
//
//            Editor_p->FileEditor.tabToSpaces = !Editor_p->FileEditor.tabToSpaces;
//            TTYR_CHECK(ttyr_tty_setDefaultMessage(
//               &ttyr_tty_getTTY()->Tab_p->Tile_p->Status, Editor_p->FileEditor.tabToSpaces ? 
//               TTYR_TTY_MESSAGE_EDITOR_TAB_TO_SPACES_ENABLED : TTYR_TTY_MESSAGE_EDITOR_TAB_TO_SPACES_DISABLED
//            ))
//            break;
//
//        default : TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_ERROR_UNKNOWN_COMMAND)
//    }

    return TTYR_TTY_SUCCESS;
}

// INIT/DESTROY ====================================================================================

static void *ttyr_tty_initEditor(
    void *arg_p)
{
    ttyr_tty_Editor *Editor_p = nh_core_allocate(sizeof(ttyr_tty_Editor));
    TTYR_CHECK_MEM_2(NULL, Editor_p)

    Editor_p->View        = ttyr_tty_initEditorView();
    Editor_p->focus       = 0;
    Editor_p->insertMode  = false;
    Editor_p->treeListing = true;
    Editor_p->TreeListing = ttyr_tty_initTreeListing();
    Editor_p->FileEditor  = ttyr_tty_initFileEditor();

    Editor_p->LastUpdate = nh_core_getSystemTime();
    Editor_p->updateIntervalInSeconds = 0.2;

    return Editor_p;
}

static void ttyr_tty_destroyEditor(
    void *handle_p)
{
    ttyr_tty_Editor *Editor_p = handle_p;

    ttyr_tty_freeEditorView(&Editor_p->View);
    ttyr_tty_freeFileEditor(&Editor_p->FileEditor);
    ttyr_tty_freeTreeListing(&Editor_p->TreeListing);

    nh_core_free(Editor_p);
}

// PROTOTYPE =======================================================================================

typedef enum TTYR_TTY_EDITOR_COMMAND_E {
    TTYR_TTY_EDITOR_COMMAND_PREVIEW = 0,
    TTYR_TTY_EDITOR_COMMAND_TREE,
    TTYR_TTY_EDITOR_COMMAND_NEW,
    TTYR_TTY_EDITOR_COMMAND_E_COUNT,
} TTYR_TTY_EDITOR_COMMAND_E;

static void ttyr_tty_destroyEditorPrototype(
    ttyr_tty_Interface *Prototype_p)
{
    nh_core_free(Prototype_p);
}

ttyr_tty_Interface *ttyr_tty_createEditorPrototype()
{
    ttyr_tty_Interface *Prototype_p = nh_core_allocate(sizeof(ttyr_tty_Interface));
    TTYR_CHECK_MEM_2(NULL, Prototype_p)

    memset(Prototype_p, 0, sizeof(ttyr_tty_Interface));

    Prototype_p->Callbacks.init_f = ttyr_tty_initEditor;
    Prototype_p->Callbacks.draw_f = ttyr_tty_drawEditor;
    Prototype_p->Callbacks.drawTopbar_f = ttyr_tty_drawEditorTopbar;
    Prototype_p->Callbacks.handleInput_f = ttyr_tty_handleEditorInput;
    Prototype_p->Callbacks.getCursorPosition_f = ttyr_tty_getEditorCursor;
    Prototype_p->Callbacks.update_f = ttyr_tty_updateEditor;
    Prototype_p->Callbacks.handleCommand_f = ttyr_tty_executeEditorCommand;
    Prototype_p->Callbacks.destroyPrototype_f = ttyr_tty_destroyEditorPrototype;
    Prototype_p->Callbacks.destroy_f = ttyr_tty_destroyEditor;

    NH_API_UTF32 name_p[7] = {'e', 'd', 'i', 't', 'o', 'r', 0};
    memcpy(Prototype_p->name_p, name_p, sizeof(name_p));

//    nh_encoding_UTF32String *CommandNames_p =
//        nh_core_allocate(sizeof(nh_encoding_UTF32String)*TTYR_TTY_EDITOR_COMMAND_E_COUNT);
//    TTYR_CHECK_MEM_2(NULL, CommandNames_p)
//
//    NH_API_UTF32 command1_p[7] = {'p', 'r', 'e', 'v', 'i', 'e', 'w'};
//    NH_API_UTF32 command2_p[4] = {'t', 'r', 'e', 'e'};
//    NH_API_UTF32 command3_p[3] = {'n', 'e', 'w'};
//
//    CommandNames_p[TTYR_TTY_EDITOR_COMMAND_PREVIEW] = nh_encoding_initUTF32(7);
//    CommandNames_p[TTYR_TTY_EDITOR_COMMAND_TREE] = nh_encoding_initUTF32(4);
//    CommandNames_p[TTYR_TTY_EDITOR_COMMAND_NEW] = nh_encoding_initUTF32(3);
//
//    NH_ENCODING_CHECK_2(NULL, nh_encoding_appendUTF32(&CommandNames_p[TTYR_TTY_EDITOR_COMMAND_PREVIEW], command1_p, 7))
//    NH_ENCODING_CHECK_2(NULL, nh_encoding_appendUTF32(&CommandNames_p[TTYR_TTY_EDITOR_COMMAND_TREE], command2_p, 4))
//    NH_ENCODING_CHECK_2(NULL, nh_encoding_appendUTF32(&CommandNames_p[TTYR_TTY_EDITOR_COMMAND_NEW], command3_p, 3))

    Prototype_p->commands = 0;

    return Prototype_p;
}

