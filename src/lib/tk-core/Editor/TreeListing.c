// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "TreeListing.h"
#include "Editor.h"

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

// HELPER ==========================================================================================

static int tk_core_isRegularFile(
    tk_core_TreeListingNode *Node_p)
{
    if (Node_p->unsaved) {return true;}

    nh_encoding_UTF8String Path = nh_encoding_encodeUTF8(Node_p->Path.p, Node_p->Path.length);
    struct stat path_stat;
    stat(Path.p, &path_stat);
    nh_encoding_freeUTF8(&Path);

    return S_ISREG(path_stat.st_mode);
}

static TK_CORE_RESULT tk_core_getNodeList(
    nh_core_List *List_p, tk_core_TreeListingNode *Node_p)
{
    nh_core_appendToList(List_p, Node_p);

    if (!Node_p->open) {return TK_CORE_SUCCESS;}

    for (int i = 0; i < Node_p->Children.size; ++i) {
        tk_core_getNodeList(List_p, Node_p->Children.pp[i]);
    }

    return TK_CORE_SUCCESS;
}

static tk_core_TreeListingNode *tk_core_getCurrentNode(
    tk_core_TreeListing *Listing_p)
{
    int current = 0; 
    nh_core_List Nodes = nh_core_initList(128);
    tk_core_getNodeList(&Nodes, Listing_p->Root_p);

    tk_core_TreeListingNode *Current_p = Nodes.pp[Listing_p->current];

    nh_core_freeList(&Nodes, false);

    return Current_p;
}

static tk_core_TreeListingNode *tk_core_getTreeListingNode(
    tk_core_TreeListing *Listing_p, nh_encoding_UTF32String *Path_p)
{
    int current = 0; 
    nh_core_List Nodes = nh_core_initList(128);
    tk_core_getNodeList(&Nodes, Listing_p->Root_p);

    tk_core_TreeListingNode *Result_p = NULL;
    for (int i = 0; i < Nodes.size; ++i) {
        if (nh_encoding_compareUTF32(((tk_core_TreeListingNode*)Nodes.pp[i])->Path.p, Path_p->p)) {
            Result_p = Nodes.pp[i];
            break;
        }
    }

    nh_core_freeList(&Nodes, false);

    return Result_p;
}

// CREATE ==========================================================================================

static tk_core_TreeListingNode *tk_core_createTreeListingNode(
    tk_core_TreeListingNode *Parent_p, nh_encoding_UTF32String Path, tk_core_File *File_p)
{
    tk_core_TreeListingNode *Node_p = (tk_core_TreeListingNode*)nh_core_allocate(sizeof(tk_core_TreeListingNode));
    TK_CHECK_MEM_2(NULL, Node_p)

    Node_p->open      = false;
    Node_p->unsaved   = false;
    Node_p->Path      = Path;
    Node_p->Children  = nh_core_initList(16);
    Node_p->Parent_p  = Parent_p;
    Node_p->File_p    = File_p;

    Node_p->Overflow.offset    = 0;
    Node_p->Overflow.LastShift = nh_core_getSystemTime();

    return Node_p;
}

static TK_CORE_RESULT tk_core_freeTreeListingNode(
    tk_core_TreeListingNode *Node_p)
{
    nh_encoding_freeUTF32(&Node_p->Path);

    for (int i = 0; i < Node_p->Children.size; ++i) {
        tk_core_freeTreeListingNode(Node_p->Children.pp[i]);
    }
    nh_core_freeList(&Node_p->Children, true);

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_openNode(
    tk_core_TreeListingNode *Node_p)
{
    TK_CHECK_NULL(Node_p)

    if (Node_p->Path.length <= 0 || Node_p->Children.size > 0 || tk_core_isRegularFile(Node_p)) {
        return TK_CORE_ERROR_BAD_STATE;
    }

#ifdef __unix__ 

    struct dirent **namelist_pp;
    nh_encoding_UTF8String CurrentPath = nh_encoding_encodeUTF8(Node_p->Path.p, Node_p->Path.length);

    int n = scandir(CurrentPath.p, &namelist_pp, 0, alphasort);
    if (n < 0) {return TK_CORE_ERROR_BAD_STATE;}

    nh_encoding_freeUTF8(&CurrentPath);

    for (int i = 0; i < n; ++i) 
    {
        if (strcmp(namelist_pp[i]->d_name, ".") && strcmp(namelist_pp[i]->d_name, "..")) 
        {
            nh_encoding_UTF32String NewPath = nh_encoding_initUTF32(128);
            nh_encoding_UTF32String FileName = nh_encoding_decodeUTF8(namelist_pp[i]->d_name, strlen(namelist_pp[i]->d_name), NULL);

            if (Node_p->Path.p[Node_p->Path.length - 1] != '/') {
                nh_encoding_appendUTF32(&NewPath, Node_p->Path.p, Node_p->Path.length);
                nh_encoding_appendUTF32Codepoint(&NewPath, '/');
                nh_encoding_appendUTF32(&NewPath, FileName.p, FileName.length);
            } else {
                nh_encoding_appendUTF32(&NewPath, Node_p->Path.p, Node_p->Path.length);
                nh_encoding_appendUTF32(&NewPath, FileName.p, FileName.length);
            }

            nh_encoding_freeUTF32(&FileName);

            tk_core_TreeListingNode *New_p = tk_core_createTreeListingNode(Node_p, NewPath, NULL);
            TK_CHECK_MEM(New_p)

            NH_CORE_CHECK_2(TK_CORE_ERROR_BAD_STATE, nh_core_appendToList(&Node_p->Children, New_p))
        }

        free(namelist_pp[i]);
    }

    if (Node_p->Children.size == 0) {
        TK_CHECK(tk_core_setCustomSuffixMessage(
            NULL, TK_CORE_MESSAGE_EDITOR_EMPTY_DIRECTORY, Node_p->Path.p, Node_p->Path.length
        ))
    }

    free(namelist_pp);

#elif defined(_WIN32) || defined(WIN32)

    printf("microsoft windows not supported\n");
    exit(0);

#endif

    Node_p->open = true;

    return TK_CORE_SUCCESS;
}

tk_core_TreeListingNode *tk_core_insertTreeListingNode(
    tk_core_TreeListing *Listing_p, NH_API_UTF32 *name_p, int length)
{
    TK_CHECK_NULL_2(NULL, name_p)
    if (length <= 0) {return NULL;}

    tk_core_TreeListingNode *Current_p = tk_core_getCurrentNode(Listing_p);

    int offset = Current_p->Path.length;
    if (tk_core_isRegularFile(Current_p)) {
        for (offset = Current_p->Path.length - 1; Current_p->Path.p[offset] != '/'; --offset);
    }

    offset = Current_p->Path.length - offset;

    nh_encoding_UTF32String Path = nh_encoding_initUTF32(128);
    nh_encoding_appendUTF32(&Path, Current_p->Path.p, Current_p->Path.length - offset);
    nh_encoding_appendUTF32Codepoint(&Path, '/');
    nh_encoding_appendUTF32(&Path, name_p, length);

    tk_core_TreeListingNode *New_p = NULL;

    if (!tk_core_getTreeListingNode(Listing_p, &Path)) {
        if (Current_p->Children.size > 0) {
            New_p = tk_core_createTreeListingNode(Current_p, Path, NULL);
            NH_CORE_CHECK_2(NULL, nh_core_appendToList(&Current_p->Children, New_p))
        }
        else if (Current_p->Parent_p != NULL) {
            New_p = tk_core_createTreeListingNode(Current_p->Parent_p, Path, NULL);
            NH_CORE_CHECK_2(NULL, nh_core_appendToList(&Current_p->Parent_p->Children, New_p))
        }
        else {return NULL;}
        TK_CHECK_MEM_2(NULL, New_p)
        New_p->unsaved = true;
        Listing_p->dirty = true;
    }
    else {
        TK_CHECK_2(NULL, tk_core_setDefaultMessage(NULL, TK_CORE_MESSAGE_EDITOR_FILE_ALREADY_EXISTS))
        nh_encoding_freeUTF32(&Path);
    }

    return New_p;
}

// INPUT ===========================================================================================

static TK_CORE_RESULT tk_core_removeFile(
    tk_core_Editor *Editor_p)
{
    tk_core_TreeListingNode *Node_p = tk_core_getCurrentNode(&Editor_p->TreeListing);
    nh_encoding_UTF8String Path = nh_encoding_encodeUTF8(Node_p->Path.p, Node_p->Path.length);

    remove(Path.p);
    nh_encoding_freeUTF8(&Path);

    TK_CHECK(tk_core_setCustomSuffixMessage(
        NULL, TK_CORE_MESSAGE_EDITOR_FILE_REMOVED, Node_p->Path.p, Node_p->Path.length
    ))

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_delete(
    nh_api_KeyboardEvent Event, bool *continue_p)
{
    if (Event.trigger != NH_API_TRIGGER_PRESS) {return TK_CORE_SUCCESS;}

    NH_API_UTF32 c = Event.codepoint;

    if (c == 'y' || c == 'n') 
    {
        if (c == 'n') {
            TK_CHECK(tk_core_setDefaultMessage(NULL, TK_CORE_MESSAGE_BINARY_QUERY_DELETE_INTERRUPTED))
        }
        if (c == 'y') {
//            tk_core_Program *Program_p = tk_core_getCurrentProgram(&tk_core_getTTY()->Tab_p->Tile_p->TopBar);
//            tk_core_removeFile(Program_p->handle_p);
//            TK_CHECK(tk_core_handleTreeListingInput(
//                Program_p, ((tk_core_Editor*)Program_p->handle_p)->height, 'w'
//            ))
        }
    }

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_setCurrentToRoot(
    tk_core_TreeListing *Listing_p)
{
    tk_core_TreeListingNode *Current_p = tk_core_getCurrentNode(Listing_p);

    if (Current_p->Children.size > 0) {
        for (tk_core_TreeListingNode *Parent_p = Current_p; Parent_p = Parent_p->Parent_p;) {
            Parent_p->open = false;
        }
        Listing_p->Root_p = Current_p;
    }

    TK_CHECK(tk_core_setCustomSuffixMessage(
        NULL, TK_CORE_MESSAGE_EDITOR_NEW_ROOT, Listing_p->Root_p->Path.p, Listing_p->Root_p->Path.length
    ))

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_setParentToRoot(
    tk_core_TreeListing *Listing_p)
{
    if (Listing_p->Root_p->Parent_p != NULL) {
        Listing_p->Root_p = Listing_p->Root_p->Parent_p;
        Listing_p->Root_p->open = true;
    }
    else {
        nh_encoding_UTF32String Path = nh_encoding_initUTF32(128);
        nh_encoding_appendUTF32(&Path, Listing_p->Root_p->Path.p, Listing_p->Root_p->Path.length);

        while (Path.p[Path.length - 1] != '/') {nh_encoding_removeUTF32Tail(&Path, 1);}
        if (Path.length > 1) {nh_encoding_removeUTF32Tail(&Path, 1);}

        tk_core_TreeListingNode *OldRoot_p = Listing_p->Root_p;

        Listing_p->Root_p = tk_core_createTreeListingNode(NULL, Path, NULL);
        TK_CHECK_MEM(Listing_p->Root_p)

        OldRoot_p->Parent_p = Listing_p->Root_p;
        TK_CHECK(tk_core_openNode(Listing_p->Root_p))

        bool isChild = false;
        for (int i = 0; i < Listing_p->Root_p->Children.size; ++i) {
            tk_core_TreeListingNode *Child_p = Listing_p->Root_p->Children.pp[i];
            if (nh_encoding_compareUTF32(Child_p->Path.p, OldRoot_p->Path.p)) {
                nh_encoding_freeUTF32(&Child_p->Path);
                nh_core_free(Listing_p->Root_p->Children.pp[i]);
                Listing_p->Root_p->Children.pp[i] = OldRoot_p;
                isChild = true;
                break;
            }
        }

        if (!isChild) {return TK_CORE_ERROR_BAD_STATE;}
    }

    TK_CHECK(tk_core_setCustomSuffixMessage(
        NULL, TK_CORE_MESSAGE_EDITOR_NEW_ROOT, Listing_p->Root_p->Path.p, Listing_p->Root_p->Path.length
    ))

    return TK_CORE_SUCCESS;
}

static void tk_core_updateTreeListingView(
    tk_core_Editor *Editor_p, int key)
{
    switch (key) 
    {
        case 'w' :
        {
            tk_core_TreeListingView *View_p = &Editor_p->View.TreeListing;
            if (View_p->current > 0) {View_p->current--;}
            else if (View_p->offset > 0) {View_p->offset--;} 
            break;
        }
        case 's' :
        {
            tk_core_TreeListingView *View_p = &Editor_p->View.TreeListing;
            if (View_p->current < Editor_p->View.height - 1) {View_p->current++;}
            else {View_p->offset++;}
            break;
        }
    }
}

static TK_CORE_RESULT tk_core_closeFileFromTreeListing(
    tk_core_Program *Program_p, tk_core_File *File_p)
{
    tk_core_Editor *Editor_p = Program_p->handle_p;

    TK_CHECK(tk_core_setCustomSuffixMessage(
        NULL, TK_CORE_MESSAGE_EDITOR_FILE_CLOSED, File_p->Node_p->Path.p, File_p->Node_p->Path.length
    ))
 
    tk_core_destroyFileViews(&Editor_p->View.FileEditor, File_p);
    TK_CHECK(tk_core_closeFile(&Editor_p->FileEditor, File_p))
 
    return TK_CORE_SUCCESS;
}

static void tk_core_moveCursorVertically(
    tk_core_Program *Program_p, int key) 
{
    tk_core_FileEditor *FileEditor_p = &((tk_core_Editor*)Program_p->handle_p)->FileEditor;
    tk_core_TreeListing *Listing_p = &((tk_core_Editor*)Program_p->handle_p)->TreeListing;

    nh_core_List Nodes = nh_core_initList(32);
    tk_core_getNodeList(&Nodes, Listing_p->Root_p);

    switch (key) 
    {
        case 'w' :
        case CTRL_KEY('w') :

            if (Listing_p->current > 0) {
                Listing_p->current--;
                tk_core_updateTreeListingView(Program_p->handle_p, 'w');
            }
            break;

        case 's' :
        case CTRL_KEY('s') :

            if (Listing_p->current < Nodes.size - 1) { 
                Listing_p->current++;
                tk_core_updateTreeListingView(Program_p->handle_p, 's');
            }
            break;
    }

    nh_core_freeList(&Nodes, false);

    if (Listing_p->preview && FileEditor_p != NULL)
    {
        if (Listing_p->Preview_p != NULL) {
            tk_core_closeFileFromTreeListing(Program_p, Listing_p->Preview_p->File_p);   
            Listing_p->Preview_p->File_p = NULL;
            Listing_p->Preview_p = NULL;
        }

        tk_core_TreeListingNode *Current_p = tk_core_getCurrentNode(Listing_p);

        if (Current_p->File_p == NULL && Current_p->Path.length > 0 && tk_core_isRegularFile(Current_p)) {
            Current_p->File_p = tk_core_openFile(Program_p, Current_p, true);
            Listing_p->Preview_p = Current_p; 
        }
    }
}

TK_CORE_RESULT tk_core_handleTreeListingInput(
    tk_core_Program *Program_p, NH_API_UTF32 c)
{
    tk_core_Editor *Editor_p = Program_p->handle_p;
    tk_core_EditorView *View_p = &Editor_p->View;
    tk_core_TreeListing *Listing_p = &Editor_p->TreeListing;
    tk_core_TreeListingNode *Current_p = tk_core_getCurrentNode(Listing_p);

    switch (c)
    {
        case 'q' :

            if (View_p->TreeListing.offset > 0) {View_p->TreeListing.offset--;}
            break;

        case 'e' :

            if (View_p->TreeListing.offset < View_p->TreeListing.maxOffset) {View_p->TreeListing.offset++;}
            break;

        case 'w' :
        case 's' :
        case CTRL_KEY('w') :
        case CTRL_KEY('s') :

            tk_core_moveCursorVertically(Program_p, c);
            break;

        case 'a' :
        case CTRL_KEY('a') :

            if (Listing_p->current == 0) {
                View_p->TreeListing.offset = 0;
                tk_core_setParentToRoot(Listing_p);
            } 
            else if (Current_p->Children.size > 0 && Current_p->open) {
                Current_p->open = false;
            }
            else if (Current_p->Children.size == 0 && Current_p->File_p != NULL && Current_p->Path.length > 0) {
                TK_CHECK(tk_core_closeFileFromTreeListing(Program_p, Current_p->File_p))   
                Current_p->File_p = NULL;
                TK_CHECK(tk_core_cycleThroughFiles(Program_p, 'f'))
            }
            else if (Current_p->Children.size == 0 && Current_p->File_p != NULL) {
                TK_CHECK(tk_core_closeFileFromTreeListing(Program_p, Current_p->File_p))
                nh_core_removeFromList2(&Current_p->Parent_p->Children, true, Current_p);
                TK_CHECK(tk_core_handleTreeListingInput(Program_p, 'w'))
            }
            else { // delete ?
                nh_encoding_UTF32String Question = nh_encoding_initUTF32(128);
                int deleteLength;
                NH_API_UTF32 *delete_p = tk_core_getMessage(TK_CORE_MESSAGE_BINARY_QUERY_DELETE, &deleteLength);
                NH_CORE_CHECK_2(TK_CORE_ERROR_BAD_STATE, nh_encoding_appendUTF32(&Question, delete_p, deleteLength))
                NH_CORE_CHECK_2(TK_CORE_ERROR_BAD_STATE, nh_encoding_appendUTF32(&Question, Current_p->Path.p, Current_p->Path.length))
                TK_CHECK(tk_core_setBinaryQueryMessage(NULL, Question.p, Question.length, NULL, tk_core_delete))
                nh_encoding_freeUTF32(&Question);
            }
            break;

        case 'd' :
        case CTRL_KEY('d') :

            if (Listing_p->Preview_p != NULL) {
                Listing_p->Preview_p->File_p->readOnly = false;
                Listing_p->Preview_p = NULL;
            }
            else if (Current_p->Children.size > 0 && !Current_p->open) {
                Current_p->open = true;
            }
            else if (Current_p->Children.size == 0 && !tk_core_isRegularFile(Current_p)) {
                TK_CHECK(tk_core_openNode(Current_p))
            }
            else if (Current_p->Children.size == 0 && tk_core_isRegularFile(Current_p)) {
                if (Current_p->File_p == NULL) {
                    Current_p->File_p = tk_core_openFile(Program_p, Current_p, false);
                }
                else {
                    TK_CHECK(tk_core_writeFile(Current_p->File_p))
                    Current_p->unsaved = false;
                }
            } else {
                tk_core_setCurrentToRoot(Listing_p);
                Listing_p->current = 0;
                View_p->TreeListing.offset = 0;
                View_p->TreeListing.current = 0;
            }
            break;
    }
 
    Listing_p->dirty = true;

    return TK_CORE_SUCCESS;
}

// RENDER ==========================================================================================

static tk_core_Glyph tk_core_glyph(
    NH_API_UTF32 codepoint, bool reverse)
{
    tk_core_Glyph Glyph;
    memset(&Glyph, 0, sizeof(tk_core_Glyph));
    Glyph.Attributes.reverse = reverse;
    Glyph.codepoint = codepoint;
    return Glyph;
}

static TK_CORE_RESULT tk_core_renderTreeListingNode(
    tk_core_TreeListingNode *Node_p, bool isCurrent, tk_core_Glyph *Row_p, int *length_p,
    bool first, tk_core_TreeListingView *View_p)
{
    tk_core_TreeListingNode *Parent_p = Node_p->Parent_p;

    int prefixLength = 0, suffixLength = 0;

    tk_core_Glyph Prefix_p[64];  // Holds stuff that comes before the file name.
    tk_core_Glyph Suffix_p[512]; // Holds the file name.

    // Render prefix.
    while (Parent_p != NULL && Parent_p->open) {
        prefixLength += 2; 
        Parent_p = Parent_p->Parent_p;
    }

    for (int i = 0; i < prefixLength; i++) {
        Prefix_p[i] = tk_core_glyph(' ', false);
        if (i == prefixLength - 2) {Prefix_p[i] = tk_core_glyph('-', false);}
        if (i == prefixLength - 1 && isCurrent) {Prefix_p[i] = tk_core_glyph('>', false);}
    }

    if (isCurrent && !first) {
        Prefix_p[prefixLength++] = tk_core_glyph(' ', false);
    }

    // Render suffix.
    if (Node_p->Path.length > 0)
    {
        int pathOffset = 0;

        for (int i = 0; i < Node_p->Path.length; ++i) {
            if (Node_p->Path.p[i] == '/') {pathOffset = i;}
        }

        pathOffset++;

        for (int i = 0, j = pathOffset; j < Node_p->Path.length; ++i, ++j) {
            Suffix_p[i] = tk_core_glyph(Node_p->Path.p[j], Node_p->File_p != NULL);
        }

        suffixLength += Node_p->Path.length - pathOffset;

        if (!tk_core_isRegularFile(Node_p)) {
            Suffix_p[suffixLength++] = tk_core_glyph('/', false); 
        }
    }

    // Render based on view with overflow handling, or just render everything
    if (View_p) {
        for (int i = 0; i < prefixLength && i < View_p->width; ++i) {
            Row_p[i] = Prefix_p[i];
        }
        if (prefixLength + suffixLength > View_p->width) {
            nh_core_SystemTime Now = nh_core_getSystemTime();
            if (nh_core_getSystemTimeDiffInSeconds(Node_p->Overflow.LastShift, Now) >= 0.2f) {
                Node_p->Overflow.LastShift = Now;
                Node_p->Overflow.offset++;
                if ((Node_p->Overflow.offset % (suffixLength+1)) == 0) {Node_p->Overflow.offset = 0;}
            }

            int count = 0;
            for (int i = prefixLength, j = Node_p->Overflow.offset; i < View_p->width && j < suffixLength; ++i, ++j) {
                count++;
                Row_p[i] = Suffix_p[j];
            }

            for (int i = prefixLength + count + 1, j = 0; i < View_p->width && j < suffixLength; ++i, ++j) {
                Row_p[i] = Suffix_p[j];
            }
        }
        else {
            for (int i = prefixLength, j = 0; j < suffixLength && i < View_p->width; ++i, ++j) {
                Row_p[i] = Suffix_p[j];
            }
        }
    }
    else {
        for (int i = 0; i < prefixLength; ++i) {
            Row_p[i] = Prefix_p[i];
        }
        for (int i = 0; i < suffixLength; ++i) {
            Row_p[i+prefixLength] = Suffix_p[i];
        }
    }

    if (length_p) {
        *length_p = prefixLength + suffixLength;
    }

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_calculateTreeListingWidth(
    tk_core_TreeListing *Listing_p, tk_core_TreeListingView *View_p, int editorWidth)
{
    nh_core_List Nodes = nh_core_initList(64);
    TK_CHECK(tk_core_getNodeList(&Nodes, Listing_p->Root_p))
    tk_core_TreeListingNode *Current_p = tk_core_getCurrentNode(Listing_p);

    View_p->width = 0;
    tk_core_Glyph Row_p[1024] = {'\0'};

    for (int i = 0; i < Nodes.size; ++i) {
        int length = 0;
        TK_CHECK(tk_core_renderTreeListingNode(Nodes.pp[i], Nodes.pp[i] == Current_p, Row_p, &length, i == 0, NULL))
        if (length > View_p->width) {View_p->width = length;}
    }

    // Restrict tree-listing width.
    if (View_p->width > editorWidth/3) {
        View_p->maxOffset = View_p->width - editorWidth/3;
        View_p->width = editorWidth/3;
    } else {
        View_p->maxOffset = 0;
    }

    nh_core_freeList(&Nodes, false);

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_renderTreeListing(
    tk_core_TreeListing *Listing_p, tk_core_TreeListingView *View_p, int editorWidth)
{
    nh_core_List Nodes = nh_core_initList(32);
    TK_CHECK(tk_core_getNodeList(&Nodes, Listing_p->Root_p))
    tk_core_TreeListingNode *Current_p = tk_core_getCurrentNode(Listing_p);

    for (int i = 0; i < Listing_p->RenderLines.length; ++i) {
        nh_core_freeArray(&((nh_core_Array*)Listing_p->RenderLines.p)[i]);
    }
    nh_core_freeArray(&Listing_p->RenderLines);
    Listing_p->RenderLines = nh_core_initArray(sizeof(nh_core_Array), Nodes.size);

    TK_CHECK(tk_core_calculateTreeListingWidth(Listing_p, View_p, editorWidth))

    for (int row = 0; row < Nodes.size; ++row) 
    {
        nh_core_Array *Line_p = (nh_core_Array*)nh_core_incrementArray(&Listing_p->RenderLines);
        *Line_p = nh_core_initArray(sizeof(tk_core_Glyph), 32);
        tk_core_TreeListingNode *Node_p = nh_core_getFromList(&Nodes, row);
        TK_CHECK_NULL(Node_p)

        tk_core_Glyph Glyphs_p[1024] = {'\0'};
        for (int col = 0; col < View_p->width; ++col) {
            Glyphs_p[col].codepoint = ' ';
        }

        TK_CHECK(tk_core_renderTreeListingNode(Node_p, Node_p == Current_p, Glyphs_p, NULL, row == 0, View_p))

        for (int col = 0; col < View_p->width; ++col) {
            *((tk_core_Glyph*)nh_core_incrementArray(Line_p)) = Glyphs_p[col];
        }
    }  

    nh_core_freeList(&Nodes, false);

    return TK_CORE_SUCCESS;
}

TK_CORE_RESULT tk_core_treeListingNeedsRefresh(
    tk_core_TreeListing *Listing_p)
{
    nh_core_List Nodes = nh_core_initList(32);
    TK_CHECK(tk_core_getNodeList(&Nodes, Listing_p->Root_p))
    nh_core_freeList(&Nodes, false);
    return TK_CORE_SUCCESS;
}

// DRAW ============================================================================================

TK_CORE_RESULT tk_core_drawTreeListingRow(
    tk_core_Program *Program_p, tk_core_Glyph *Glyphs_p, int width, int height, int row)
{
    tk_core_TreeListing *Listing_p = &((tk_core_Editor*)Program_p->handle_p)->TreeListing;
    tk_core_TreeListingView *View_p = &((tk_core_Editor*)Program_p->handle_p)->View.TreeListing;

    if (Listing_p->dirty) {
        if (strlen(Listing_p->wrkDir_p) == 0) {
            getcwd(Listing_p->wrkDir_p, 2048);
            Listing_p->Root_p->Path = nh_encoding_decodeUTF8(Listing_p->wrkDir_p, strlen(Listing_p->wrkDir_p), NULL);
            tk_core_openNode(Listing_p->Root_p);
        }
        TK_CHECK(tk_core_renderTreeListing(Listing_p, View_p, width))
        Listing_p->dirty = false;
    }

    row += View_p->offset; 

    if (row < Listing_p->RenderLines.length) {
        nh_core_Array *RenderLine_p = &((nh_core_Array*)Listing_p->RenderLines.p)[row];
        for (int i = 0; i < RenderLine_p->length; ++i) {
            Glyphs_p[i] = ((tk_core_Glyph*)RenderLine_p->p)[i];
        }
    }

    return TK_CORE_SUCCESS;
}

// CURSOR ==========================================================================================

TK_CORE_RESULT tk_core_setTreeListingCursor(
    tk_core_Program *Program_p, tk_core_File *File_p)
{
    nh_core_List Nodes = nh_core_initList(16);
    tk_core_TreeListing *Listing_p = &((tk_core_Editor*)Program_p->handle_p)->TreeListing;

    tk_core_getNodeList(&Nodes, Listing_p->Root_p);

    for (int i = 0; i < Nodes.size; ++i) {
        if (((tk_core_TreeListingNode*)Nodes.pp[i])->File_p == File_p) {
            if (Listing_p->current < i) {
                int diff = i - Listing_p->current;
                while (diff-- > 0) {
                    tk_core_moveCursorVertically(Program_p, CTRL_KEY('s'));
                }
            }
            else if (Listing_p->current > i) {
                int diff = Listing_p->current - i;
                while (diff-- > 0) {
                    tk_core_moveCursorVertically(Program_p, CTRL_KEY('w'));
                }
            }
            break;
        }
    }

    nh_core_freeList(&Nodes, false);
    Listing_p->dirty = true;

    return TK_CORE_SUCCESS;
}

// INIT/DESTROY ====================================================================================

tk_core_TreeListing tk_core_initTreeListing()
{
    tk_core_TreeListing Listing;

    Listing.Root_p      = tk_core_createTreeListingNode(NULL, nh_encoding_initUTF32(0), NULL);
    Listing.current     = 0;
    Listing.RenderLines = nh_core_initArray(sizeof(nh_core_String), 255);
    Listing.dirty       = true;
    Listing.preview     = false;
    Listing.Preview_p   = NULL;

    memset(Listing.wrkDir_p, 0, 2048);

    return Listing;
}

TK_CORE_RESULT tk_core_freeTreeListing(
    tk_core_TreeListing *TreeListing_p)
{
    tk_core_freeTreeListingNode(TreeListing_p->Root_p);
    nh_core_free(TreeListing_p->Root_p);
    for (int i = 0; i < TreeListing_p->RenderLines.length; ++i) {
        nh_core_freeString(((nh_core_String*)TreeListing_p->RenderLines.p)+i);
    }
    nh_core_freeArray(&TreeListing_p->RenderLines);
 
    return TK_CORE_SUCCESS;
}

