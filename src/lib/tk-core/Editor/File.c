// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "File.h"
#include "FileEditor.h"
#include "Editor.h"
#include "TreeListing.h"
#include "TextFile.h"
#include "TextFileInput.h"
#include "SyntaxHighlights.h"

#include "../TTY/TTY.h"
#include "../Common/Macros.h"

#include "nh-core/System/Process.h"
#include "nh-core/System/Memory.h"

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

tk_core_FileView tk_core_initFileView(
    tk_core_File *File_p)
{
    tk_core_FileView View;

    View.File_p = File_p;
    View.width  = 0;
    View.height = 0;    

    View.TextFile.screenCursorX = 0;
    View.TextFile.screenCursorY = 0;
    View.TextFile.rowOffset = 0;
    View.TextFile.colOffset = 0;

    return View;
}

TK_CORE_RESULT tk_core_createFileViews(
    tk_core_FileEditorView *View_p, tk_core_File *File_p)
{
    tk_core_FileView *FileView_p = (tk_core_FileView*)nh_core_allocate(sizeof(tk_core_FileView));
    TK_CHECK_MEM(FileView_p)
    *FileView_p = tk_core_initFileView(File_p);
    nh_core_appendToList(&View_p->FileViews, FileView_p);

    return TK_CORE_SUCCESS;
}

TK_CORE_RESULT tk_core_destroyFileViews(
    tk_core_FileEditorView *View_p, tk_core_File *File_p)
{
    tk_core_FileView *FileView_p = NULL;
    for (int j = 0; j < View_p->FileViews.size; ++j) {
        FileView_p = View_p->FileViews.pp[j];
        if (FileView_p->File_p == File_p) {break;}
        FileView_p = NULL;
    }
    if (FileView_p) {nh_core_removeFromList2(&View_p->FileViews, true, FileView_p);}

    return TK_CORE_SUCCESS;
}

tk_core_FileView *tk_core_getFileView(
    tk_core_EditorView *View_p, tk_core_File *File_p)
{
    for (int i = 0; i < View_p->FileEditor.FileViews.size; ++i) {
        tk_core_FileView *FileView_p = View_p->FileEditor.FileViews.pp[i];
        if (FileView_p->File_p == File_p) {return FileView_p;}
    }
    return NULL;
}

TK_CORE_RESULT tk_core_updateFileViews(
    tk_core_EditorView *View_p)
{
    int oddLeftOver, fileWidth = View_p->FileEditor.width;

    if (View_p->FileEditor.FileViews.size > 0) {
        int count = View_p->FileEditor.FileViews.size;
        if (View_p->FileEditor.FileViews.size > View_p->FileEditor.maxOnScreen) {
            count = View_p->FileEditor.maxOnScreen;
        }
        fileWidth = View_p->FileEditor.width / count;
        oddLeftOver = View_p->FileEditor.width % (count * fileWidth);
    }

    for (int j = 0; j < View_p->FileEditor.FileViews.size; ++j) {
        tk_core_FileView *FileView_p = View_p->FileEditor.FileViews.pp[j];
        if (j == View_p->FileEditor.FileViews.size - 1) {fileWidth += oddLeftOver;}
        FileView_p->width = fileWidth;
        FileView_p->height = View_p->height;
    }

    return TK_CORE_SUCCESS;
}

// TYPE ============================================================================================

TK_CORE_FILE tk_core_getFileType(
    nh_encoding_UTF32String *Path_p)
{
    TK_CORE_FILE type = TK_CORE_FILE_TEXT;
    nh_encoding_UTF8String Path = nh_encoding_encodeUTF8(Path_p->p, Path_p->length);

    if (strstr(Path.p, "netzhaut/CHANGES")) {
        type = TK_CORE_FILE_CHANGES;
    }

    nh_encoding_freeUTF8(&Path);

    return type;
}

// RENDER ==========================================================================================

TK_CORE_RESULT tk_core_renderFile(
    tk_core_File *File_p)
{
    switch (File_p->type)
    {
        case TK_CORE_FILE_TEXT :
            for (int i = 0; i < ((tk_core_TextFile*)File_p->handle_p)->Lines.size; ++i) {
                TK_CHECK(tk_core_renderTextFileLine(File_p->handle_p, i))
            }
            break;
        case TK_CORE_FILE_CHANGES :
            break;
    }

    return TK_CORE_SUCCESS;
}

// WRITE ===========================================================================================

TK_CORE_RESULT tk_core_writeFile(
    tk_core_File *File_p)
{
    switch (File_p->type)
    {
        case TK_CORE_FILE_TEXT :
            TK_CHECK(tk_core_writeTextFile(File_p->handle_p, &File_p->Node_p->Path))
            break;
        case TK_CORE_FILE_CHANGES :
            break;
    }

    TK_CHECK(tk_core_setCustomSuffixMessage(
        NULL, TK_CORE_MESSAGE_EDITOR_FILE_SAVED, File_p->Node_p->Path.p, File_p->Node_p->Path.length
    ))

    return TK_CORE_SUCCESS;
}

// SEARCH ==========================================================================================

TK_CORE_RESULT tk_core_clearFileSearch(
    tk_core_File *File_p)
{
    switch (File_p->type)
    {
        case TK_CORE_FILE_TEXT :
            tk_core_clearTextFileSearch(File_p->handle_p);
            break;
    }

    return TK_CORE_SUCCESS;
}

TK_CORE_RESULT tk_core_searchFile(
    tk_core_File *File_p, NH_API_UTF32 *str_p, int length)
{
    switch (File_p->type)
    {
        case TK_CORE_FILE_TEXT :
            TK_CHECK(tk_core_searchTextFile(File_p->handle_p, str_p, length))
            break;
    }

    return TK_CORE_SUCCESS;
}

// INPUT ===========================================================================================

TK_CORE_RESULT tk_core_handleFileInput(
    tk_core_Program *Program_p, tk_core_File *File_p, NH_API_UTF32 c, bool insertMode, 
    bool *refresh_p)
{
    if (File_p->readOnly) {
        *refresh_p = true;
        TK_CHECK(tk_core_setDefaultMessage(NULL, TK_CORE_MESSAGE_EDITOR_READ_ONLY))
    }

    nh_core_List FileViews = ((tk_core_Editor*)Program_p->handle_p)->View.FileEditor.FileViews;

    switch (File_p->type)
    {
        case TK_CORE_FILE_TEXT :
            TK_CHECK(tk_core_handleTextFileInput(&FileViews, File_p, c, insertMode, refresh_p))
            break;    
        case TK_CORE_FILE_CHANGES :
            break;    
    }

    return TK_CORE_SUCCESS;
}

// DRAW ============================================================================================

TK_CORE_RESULT tk_core_drawFileRow(
    tk_core_Program *Program_p, tk_core_File *File_p, tk_core_FileView *View_p, tk_core_Glyph *Glyphs_p, 
    int row)
{
    switch (File_p->type)
    {
        case TK_CORE_FILE_TEXT :
            tk_core_drawTextFileLine(Program_p, File_p->handle_p, View_p, Glyphs_p, row);
            break;
        case TK_CORE_FILE_CHANGES :
            break;
    }

    return TK_CORE_SUCCESS;
}

