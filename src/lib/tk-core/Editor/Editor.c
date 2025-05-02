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

#include "nh-core/System/Memory.h"
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

static tk_core_EditorView tk_core_initEditorView()
{
    tk_core_EditorView View;

    memset(&View, 0, sizeof(tk_core_EditorView));
    View.FileEditor.FileViews = nh_core_initList(8);

    return View;
}

static void tk_core_freeEditorView(
    tk_core_EditorView *View_p)
{
    nh_core_freeList(&View_p->FileEditor.FileViews, true);
}

// UPDATE ==========================================================================================

static void tk_core_closeTreeListingNode(
    tk_core_FileEditor *Editor_p, tk_core_TreeListingNode *Node_p)
{
//    for (int i = 0; i < Node_p->Children.size; ++i) {
//        tk_core_TreeListingNode *Child_p = Node_p->Children.pp[i];
//        nh_encoding_freeUTF32(&Child_p->Path);
//        tk_core_closeTreeListingNode(Editor_p, Child_p);
//    }
//
//    if (Node_p->File_p) {tk_core_closeFile(Editor_p, Node_p->File_p);}
//
//    nh_core_freeList(&Node_p->Children, true);
}

static TK_CORE_RESULT tk_core_handleDeletedNodes(
    tk_core_FileEditor *Editor_p, tk_core_TreeListingNode *Node_p)
{
//#ifdef __unix__
//
//    struct dirent **namelist_pp = NULL;
//    nh_encoding_UTF8String CurrentPath = nh_encoding_encodeUTF8(Node_p->Path.p, Node_p->Path.length);
//
//    int n = scandir(CurrentPath.p, &namelist_pp, 0, alphasort);
//    if (n < 0) {TK_CORE_DIAGNOSTIC_END(TK_CORE_ERROR_BAD_STATE)}
//
//    nh_encoding_freeUTF8(&CurrentPath);
//
//    for (int i = 0; i < Node_p->Children.size; ++i)
//    {
//        tk_core_TreeListingNode *Child_p = Node_p->Children.pp[i];
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
//            tk_core_closeTreeListingNode(Editor_p, Child_p);
//            nh_core_removeFromList(&Node_p->Children, true, i);
//            i--;
//            *updated_p = true;
//            continue;
//        }
//
//        if (Child_p->Children.size > 0) {
//            TK_CHECK(tk_core_handleDeletedNodes(Editor_p, Child_p, updated_p))
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

    return TK_CORE_SUCCESS;
}

TK_CORE_RESULT tk_core_updateEditor(
    tk_core_Program *Program_p)
{
    tk_core_Editor *Editor_p = Program_p->handle_p;

    if (!Editor_p->treeListing) {
        return TK_CORE_SUCCESS;
    }

    nh_core_SystemTime Now = nh_core_getSystemTime();
    if (nh_core_getSystemTimeDiffInSeconds(Editor_p->LastUpdate, Now) < Editor_p->updateIntervalInSeconds) {
        return TK_CORE_SUCCESS;
    }

    TK_CHECK(tk_core_handleDeletedNodes(&Editor_p->FileEditor, Editor_p->TreeListing.Root_p))

    Editor_p->LastUpdate = Now;
    Editor_p->TreeListing.dirty = true;
    Program_p->refresh = true;
    Editor_p->LastUpdate = Now;

    return TK_CORE_SUCCESS;
}

// INPUT ===========================================================================================

static TK_CORE_RESULT tk_core_handleEditorInput(
    tk_core_Program *Program_p, nh_api_WSIEvent Event)
{
    if (Event.type != NH_API_WSI_EVENT_KEYBOARD) {return TK_CORE_SUCCESS;}
    if (Event.Keyboard.trigger != NH_API_TRIGGER_PRESS) {return TK_CORE_SUCCESS;}

    NH_API_UTF32 c = Event.Keyboard.codepoint;
    tk_core_Editor *Editor_p = Program_p->handle_p;

    switch (c) 
    {
        case 'i' : if (Editor_p->insertMode) {goto FILE_EDITOR_INPUT;}
        case CTRL_KEY('i') :
        case 27 :
            Editor_p->insertMode = c == 27 ? false : !Editor_p->insertMode; 
            TK_CHECK(tk_core_setDefaultMessage(
                NULL, Editor_p->insertMode ? TK_CORE_MESSAGE_EDITOR_INSERT_ACTIVATED : TK_CORE_MESSAGE_EDITOR_INSERT_DEACTIVATED 
            ))
            Program_p->refresh = true;
            break;

        case 'f' :
        case 'g' : if (Editor_p->insertMode) {goto FILE_EDITOR_INPUT;}

            // Switching file focus is mostly done in the file editor using the next function.
            TK_CHECK(tk_core_cycleThroughFiles(Program_p, c))
            Program_p->refresh = true;
            break;

        case 'w' :
        case 'a' :
        case 's' :
        case 'd' : if (Editor_p->insertMode) {goto FILE_EDITOR_INPUT;}
            if  (Editor_p->treeListing) {
                TK_CHECK(tk_core_handleTreeListingInput(Program_p, c))
            }
            Editor_p->treeListing = true;
            Program_p->refresh = true;
            break;

        default :
            goto FILE_EDITOR_INPUT;
    }
 
    return TK_CORE_SUCCESS;

FILE_EDITOR_INPUT :

    TK_CHECK(tk_core_handleFileEditorInput(Program_p, c))
    return TK_CORE_SUCCESS;
}

// DRAW ============================================================================================

static TK_CORE_RESULT tk_core_drawEditor(
    tk_core_Program *Program_p, tk_core_Glyph *Glyphs_p, int width, int height, int row)
{
    tk_core_Editor *Editor_p = Program_p->handle_p;
    Editor_p->View.height = height;

    // First, normalize glyphs.
    for (int i = 0; i < width; ++i) {
        Glyphs_p[i].codepoint = ' ';
    }

    // Draw tree listing row if necessary.
    if (Editor_p->treeListing) {
        TK_CHECK(tk_core_drawTreeListingRow(Program_p, Glyphs_p, width, height, row))
        Glyphs_p += Editor_p->View.TreeListing.width;
    }
    
    // Draw file editor row.
    TK_CHECK(tk_core_drawFileEditorRow(
        Program_p, Glyphs_p, Editor_p->treeListing ? width - Editor_p->View.TreeListing.width : width, 
        height, row
    ))

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_drawEditorTopbar(
    tk_core_Program *Program_p, tk_core_Glyph *Glyphs_p, int width)
{
    tk_core_Editor *Editor_p = Program_p->handle_p;
    tk_core_File *File_p = nh_core_getFromLinkedList(&Editor_p->FileEditor.Files, Editor_p->FileEditor.current);

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

    return TK_CORE_SUCCESS;
}

// CURSOR ==========================================================================================

static TK_CORE_RESULT tk_core_getEditorCursor(
    tk_core_Program *Program_p, int *x_p, int *y_p)
{
    tk_core_Editor *Editor_p = Program_p->handle_p;
    tk_core_FileView *FileView_p = NULL;

    int x = Editor_p->treeListing ? Editor_p->View.TreeListing.width : 0;

    tk_core_File *File_p = NULL;
    for (int i = Editor_p->View.FileEditor.currentOffset; i < Editor_p->FileEditor.Files.count; ++i) {
        File_p = nh_core_getFromLinkedList(&Editor_p->FileEditor.Files, i);
        FileView_p = tk_core_getFileView(&Editor_p->View, File_p);
        if (i == Editor_p->FileEditor.current) {break;}
        x += FileView_p->width;
    }

    if (File_p != NULL && File_p->type == TK_CORE_FILE_TEXT) 
    {
        tk_core_TextFile *TextFile_p = File_p->handle_p;
        *x_p = FileView_p->TextFile.screenCursorX + TextFile_p->lineNumberOffset + x;
        *y_p = FileView_p->TextFile.screenCursorY;
    }
    else {
        *x_p = -1;
        *y_p = -1;
    }

    return TK_CORE_SUCCESS;
}

// COMMANDS ========================================================================================

static TK_CORE_RESULT tk_core_executeEditorCommand(
    tk_core_Program *Program_p)
{
    tk_core_Editor *Editor_p = Program_p->handle_p;

//    switch (Program_p->command)
//    {
//        case TK_CORE_EDITOR_COMMAND_PREVIEW:
//
//            Editor_p->TreeListing.preview = !Editor_p->TreeListing.preview; 
//            TK_CHECK(tk_core_setDefaultMessage(
//               &tk_core_getTTY()->Tab_p->Tile_p->Status,
//               Editor_p->TreeListing.preview ? TK_CORE_MESSAGE_EDITOR_PREVIEW_ENABLED : TK_CORE_MESSAGE_EDITOR_PREVIEW_DISABLED
//            ))
//            Program_p->refresh = true;
//            break;
//
//        case TK_CORE_EDITOR_COMMAND_TREE:
//
//            Editor_p->treeListing = !Editor_p->treeListing; 
//            TK_CHECK(tk_core_setDefaultMessage(
//               &tk_core_getTTY()->Tab_p->Tile_p->Status,
//               Editor_p->treeListing ? TK_CORE_MESSAGE_EDITOR_SHOW_TREE : TK_CORE_MESSAGE_EDITOR_HIDE_TREE 
//            ))
//            Program_p->refresh = true;
//            break;
//
//        case TK_CORE_EDITOR_COMMAND_NEW:
//        {
//            if (Arguments_p->size != 1) {TK_CORE_END(TK_CORE_ERROR_INVALID_ARGUMENT)}
//            nh_encoding_UTF32String *Argument_p = Arguments_p->pp[0];
//
//            tk_core_TreeListingNode *Node_p = 
//                tk_core_insertTreeListingNode(&Editor_p->TreeListing, Argument_p->p, Argument_p->length);
//            TK_CHECK_NULL(Node_p)
//
//            tk_core_File *File_p = tk_core_openFile(Program_p, Node_p, false);
//            TK_CHECK_NULL(File_p)
//
//            TK_CHECK(tk_core_setDefaultMessage(
//                &tk_core_getTTY()->Tab_p->Tile_p->Status, TK_CORE_MESSAGE_EDITOR_NEW_FILE
//            ))
//
//            Program_p->refresh = true;
//            TK_CHECK(tk_core_setTreeListingCursor(Program_p, File_p))
//            break;
//        }
//
//        case TK_CORE_EDITOR_COMMAND_SEARCH:
//        {
//            if (Editor_p->FileEditor.Files.count > 0) 
//            {
//                tk_core_File *File_p = 
//                    nh_core_getFromLinkedList(&Editor_p->FileEditor.Files, Editor_p->FileEditor.current);
//
//                if (Arguments_p->size == 0) {
//                    TK_CHECK(tk_core_clearFileSearch(File_p))
//                }
//                else {
//                    nh_encoding_UTF32String *Argument_p = Arguments_p->pp[0];
//                    TK_CHECK(tk_core_searchFile(File_p, Argument_p->p, Argument_p->length))
//                }
//            }
//            break;
//        }
//
//        case TK_CORE_EDITOR_COMMAND_TAB_SPACES:
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
//            TK_CHECK(tk_core_setCustomMessage(
//               &tk_core_getTTY()->Tab_p->Tile_p->Status, TK_CORE_MESSAGE_EDITOR_NUMBER_OF_TAB_SPACES,
//               TabSpaces.p, TabSpaces.length
//            ))
//
//            nh_encoding_freeUTF32(&TabSpaces);
//            break;
//
//        case TK_CORE_EDITOR_COMMAND_TAB_TO_SPACES:
//
//            Editor_p->FileEditor.tabToSpaces = !Editor_p->FileEditor.tabToSpaces;
//            TK_CHECK(tk_core_setDefaultMessage(
//               &tk_core_getTTY()->Tab_p->Tile_p->Status, Editor_p->FileEditor.tabToSpaces ? 
//               TK_CORE_MESSAGE_EDITOR_TAB_TO_SPACES_ENABLED : TK_CORE_MESSAGE_EDITOR_TAB_TO_SPACES_DISABLED
//            ))
//            break;
//
//        default : TK_CORE_DIAGNOSTIC_END(TK_CORE_ERROR_UNKNOWN_COMMAND)
//    }

    return TK_CORE_SUCCESS;
}

// INIT/DESTROY ====================================================================================

static void *tk_core_initEditor(
    void *arg_p)
{
    tk_core_Editor *Editor_p = (tk_core_Editor*)nh_core_allocate(sizeof(tk_core_Editor));
    TK_CHECK_MEM_2(NULL, Editor_p)

    Editor_p->View        = tk_core_initEditorView();
    Editor_p->focus       = 0;
    Editor_p->insertMode  = false;
    Editor_p->treeListing = true;
    Editor_p->TreeListing = tk_core_initTreeListing();
    Editor_p->FileEditor  = tk_core_initFileEditor();

    Editor_p->LastUpdate = nh_core_getSystemTime();
    Editor_p->updateIntervalInSeconds = 0.2;

    return Editor_p;
}

static void tk_core_destroyEditor(
    void *handle_p)
{
    tk_core_Editor *Editor_p = handle_p;

    tk_core_freeEditorView(&Editor_p->View);
    tk_core_freeFileEditor(&Editor_p->FileEditor);
    tk_core_freeTreeListing(&Editor_p->TreeListing);

    nh_core_free(Editor_p);
}

// PROTOTYPE =======================================================================================

typedef enum TK_CORE_EDITOR_COMMAND_E {
    TK_CORE_EDITOR_COMMAND_PREVIEW = 0,
    TK_CORE_EDITOR_COMMAND_TREE,
    TK_CORE_EDITOR_COMMAND_NEW,
    TK_CORE_EDITOR_COMMAND_E_COUNT,
} TK_CORE_EDITOR_COMMAND_E;

static void tk_core_destroyEditorPrototype(
    tk_core_Interface *Prototype_p)
{
    nh_core_free(Prototype_p);
}

tk_core_Interface *tk_core_createEditorPrototype()
{
    tk_core_Interface *Prototype_p = (tk_core_Interface*)nh_core_allocate(sizeof(tk_core_Interface));
    TK_CHECK_MEM_2(NULL, Prototype_p)

    memset(Prototype_p, 0, sizeof(tk_core_Interface));

    Prototype_p->Callbacks.init_f = tk_core_initEditor;
    Prototype_p->Callbacks.draw_f = tk_core_drawEditor;
    Prototype_p->Callbacks.drawTopbar_f = tk_core_drawEditorTopbar;
    Prototype_p->Callbacks.handleInput_f = tk_core_handleEditorInput;
    Prototype_p->Callbacks.getCursorPosition_f = tk_core_getEditorCursor;
    Prototype_p->Callbacks.update_f = tk_core_updateEditor;
    Prototype_p->Callbacks.handleCommand_f = tk_core_executeEditorCommand;
    Prototype_p->Callbacks.destroyPrototype_f = tk_core_destroyEditorPrototype;
    Prototype_p->Callbacks.destroy_f = tk_core_destroyEditor;

    NH_API_UTF32 name_p[7] = {'e', 'd', 'i', 't', 'o', 'r', 0};
    memcpy(Prototype_p->name_p, name_p, sizeof(name_p));

//    nh_encoding_UTF32String *CommandNames_p =
//        nh_core_allocate(sizeof(nh_encoding_UTF32String)*TK_CORE_EDITOR_COMMAND_E_COUNT);
//    TK_CHECK_MEM_2(NULL, CommandNames_p)
//
//    NH_API_UTF32 command1_p[7] = {'p', 'r', 'e', 'v', 'i', 'e', 'w'};
//    NH_API_UTF32 command2_p[4] = {'t', 'r', 'e', 'e'};
//    NH_API_UTF32 command3_p[3] = {'n', 'e', 'w'};
//
//    CommandNames_p[TK_CORE_EDITOR_COMMAND_PREVIEW] = nh_encoding_initUTF32(7);
//    CommandNames_p[TK_CORE_EDITOR_COMMAND_TREE] = nh_encoding_initUTF32(4);
//    CommandNames_p[TK_CORE_EDITOR_COMMAND_NEW] = nh_encoding_initUTF32(3);
//
//    NH_ENCODING_CHECK_2(NULL, nh_encoding_appendUTF32(&CommandNames_p[TK_CORE_EDITOR_COMMAND_PREVIEW], command1_p, 7))
//    NH_ENCODING_CHECK_2(NULL, nh_encoding_appendUTF32(&CommandNames_p[TK_CORE_EDITOR_COMMAND_TREE], command2_p, 4))
//    NH_ENCODING_CHECK_2(NULL, nh_encoding_appendUTF32(&CommandNames_p[TK_CORE_EDITOR_COMMAND_NEW], command3_p, 3))

    Prototype_p->commands = 0;

    return Prototype_p;
}

