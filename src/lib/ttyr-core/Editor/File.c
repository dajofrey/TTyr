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

ttyr_core_FileView ttyr_core_initFileView(
    ttyr_core_File *File_p)
{
    ttyr_core_FileView View;

    View.File_p = File_p;
    View.width  = 0;
    View.height = 0;    

    View.TextFile.screenCursorX = 0;
    View.TextFile.screenCursorY = 0;
    View.TextFile.rowOffset = 0;
    View.TextFile.colOffset = 0;

    return View;
}

TTYR_CORE_RESULT ttyr_core_createFileViews(
    ttyr_core_FileEditorView *View_p, ttyr_core_File *File_p)
{
    ttyr_core_FileView *FileView_p = (ttyr_core_FileView*)nh_core_allocate(sizeof(ttyr_core_FileView));
    TTYR_CHECK_MEM(FileView_p)
    *FileView_p = ttyr_core_initFileView(File_p);
    nh_core_appendToList(&View_p->FileViews, FileView_p);

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_destroyFileViews(
    ttyr_core_FileEditorView *View_p, ttyr_core_File *File_p)
{
    ttyr_core_FileView *FileView_p = NULL;
    for (int j = 0; j < View_p->FileViews.size; ++j) {
        FileView_p = View_p->FileViews.pp[j];
        if (FileView_p->File_p == File_p) {break;}
        FileView_p = NULL;
    }
    if (FileView_p) {nh_core_removeFromList2(&View_p->FileViews, true, FileView_p);}

    return TTYR_CORE_SUCCESS;
}

ttyr_core_FileView *ttyr_core_getFileView(
    ttyr_core_EditorView *View_p, ttyr_core_File *File_p)
{
    for (int i = 0; i < View_p->FileEditor.FileViews.size; ++i) {
        ttyr_core_FileView *FileView_p = View_p->FileEditor.FileViews.pp[i];
        if (FileView_p->File_p == File_p) {return FileView_p;}
    }
    return NULL;
}

TTYR_CORE_RESULT ttyr_core_updateFileViews(
    ttyr_core_EditorView *View_p)
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
        ttyr_core_FileView *FileView_p = View_p->FileEditor.FileViews.pp[j];
        if (j == View_p->FileEditor.FileViews.size - 1) {fileWidth += oddLeftOver;}
        FileView_p->width = fileWidth;
        FileView_p->height = View_p->height;
    }

    return TTYR_CORE_SUCCESS;
}

// TYPE ============================================================================================

TTYR_CORE_FILE ttyr_core_getFileType(
    nh_encoding_UTF32String *Path_p)
{
    TTYR_CORE_FILE type = TTYR_CORE_FILE_TEXT;
    nh_encoding_UTF8String Path = nh_encoding_encodeUTF8(Path_p->p, Path_p->length);

    if (strstr(Path.p, "netzhaut/CHANGES")) {
        type = TTYR_CORE_FILE_CHANGES;
    }

    nh_encoding_freeUTF8(&Path);

    return type;
}

// RENDER ==========================================================================================

TTYR_CORE_RESULT ttyr_core_renderFile(
    ttyr_core_File *File_p)
{
    switch (File_p->type)
    {
        case TTYR_CORE_FILE_TEXT :
            for (int i = 0; i < ((ttyr_core_TextFile*)File_p->handle_p)->Lines.size; ++i) {
                TTYR_CHECK(ttyr_core_renderTextFileLine(File_p->handle_p, i))
            }
            break;
        case TTYR_CORE_FILE_CHANGES :
            break;
    }

    return TTYR_CORE_SUCCESS;
}

// WRITE ===========================================================================================

TTYR_CORE_RESULT ttyr_core_writeFile(
    ttyr_core_File *File_p)
{
    switch (File_p->type)
    {
        case TTYR_CORE_FILE_TEXT :
            TTYR_CHECK(ttyr_core_writeTextFile(File_p->handle_p, &File_p->Node_p->Path))
            break;
        case TTYR_CORE_FILE_CHANGES :
            break;
    }

    TTYR_CHECK(ttyr_core_setCustomSuffixMessage(
        NULL, TTYR_CORE_MESSAGE_EDITOR_FILE_SAVED, File_p->Node_p->Path.p, File_p->Node_p->Path.length
    ))

    return TTYR_CORE_SUCCESS;
}

// SEARCH ==========================================================================================

TTYR_CORE_RESULT ttyr_core_clearFileSearch(
    ttyr_core_File *File_p)
{
    switch (File_p->type)
    {
        case TTYR_CORE_FILE_TEXT :
            ttyr_core_clearTextFileSearch(File_p->handle_p);
            break;
    }

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_searchFile(
    ttyr_core_File *File_p, NH_API_UTF32 *str_p, int length)
{
    switch (File_p->type)
    {
        case TTYR_CORE_FILE_TEXT :
            TTYR_CHECK(ttyr_core_searchTextFile(File_p->handle_p, str_p, length))
            break;
    }

    return TTYR_CORE_SUCCESS;
}

// INPUT ===========================================================================================

TTYR_CORE_RESULT ttyr_core_handleFileInput(
    ttyr_core_Program *Program_p, ttyr_core_File *File_p, NH_API_UTF32 c, bool insertMode, 
    bool *refresh_p)
{
    if (File_p->readOnly) {
        *refresh_p = true;
        TTYR_CHECK(ttyr_core_setDefaultMessage(NULL, TTYR_CORE_MESSAGE_EDITOR_READ_ONLY))
    }

    nh_core_List FileViews = ((ttyr_core_Editor*)Program_p->handle_p)->View.FileEditor.FileViews;

    switch (File_p->type)
    {
        case TTYR_CORE_FILE_TEXT :
            TTYR_CHECK(ttyr_core_handleTextFileInput(&FileViews, File_p, c, insertMode, refresh_p))
            break;    
        case TTYR_CORE_FILE_CHANGES :
            break;    
    }

    return TTYR_CORE_SUCCESS;
}

// DRAW ============================================================================================

TTYR_CORE_RESULT ttyr_core_drawFileRow(
    ttyr_core_Program *Program_p, ttyr_core_File *File_p, ttyr_core_FileView *View_p, ttyr_core_Glyph *Glyphs_p, 
    int row)
{
    switch (File_p->type)
    {
        case TTYR_CORE_FILE_TEXT :
            ttyr_core_drawTextFileLine(Program_p, File_p->handle_p, View_p, Glyphs_p, row);
            break;
        case TTYR_CORE_FILE_CHANGES :
            break;
    }

    return TTYR_CORE_SUCCESS;
}

