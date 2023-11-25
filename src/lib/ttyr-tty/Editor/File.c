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

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Process.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"

#include "../../../../external/Netzhaut/src/lib/nhencoding/Encodings/UTF8.h"

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

// VIEW ============================================================================================

ttyr_tty_FileView ttyr_tty_initFileView(
    ttyr_tty_File *File_p)
{
TTYR_TTY_BEGIN()

    ttyr_tty_FileView View;

    View.File_p = File_p;
    View.width  = 0;
    View.height = 0;    

    View.TextFile.screenCursorX = 0;
    View.TextFile.screenCursorY = 0;
    View.TextFile.rowOffset = 0;
    View.TextFile.colOffset = 0;

TTYR_TTY_END(View)
}

TTYR_TTY_RESULT ttyr_tty_createFileViews(
    ttyr_tty_FileEditorView *View_p, ttyr_tty_File *File_p)
{
TTYR_TTY_BEGIN()

    ttyr_tty_FileView *FileView_p = nh_core_allocate(sizeof(ttyr_tty_FileView));
    TTYR_TTY_CHECK_MEM(FileView_p)
    *FileView_p = ttyr_tty_initFileView(File_p);
    nh_core_appendToList(&View_p->FileViews, FileView_p);

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_destroyFileViews(
    ttyr_tty_FileEditorView *View_p, ttyr_tty_File *File_p)
{
TTYR_TTY_BEGIN()

    ttyr_tty_FileView *FileView_p = NULL;
    for (int j = 0; j < View_p->FileViews.size; ++j) {
        FileView_p = View_p->FileViews.pp[j];
        if (FileView_p->File_p == File_p) {break;}
        FileView_p = NULL;
    }
    if (FileView_p) {nh_core_removeFromList2(&View_p->FileViews, NH_TRUE, FileView_p);}

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

ttyr_tty_FileView *ttyr_tty_getFileView(
    ttyr_tty_EditorView *View_p, ttyr_tty_File *File_p)
{
TTYR_TTY_BEGIN()

    for (int i = 0; i < View_p->FileEditor.FileViews.size; ++i) {
        ttyr_tty_FileView *FileView_p = View_p->FileEditor.FileViews.pp[i];
        if (FileView_p->File_p == File_p) {TTYR_TTY_END(FileView_p)}
    }
  
TTYR_TTY_END(NULL)
}

TTYR_TTY_RESULT ttyr_tty_updateFileViews(
    ttyr_tty_EditorView *View_p)
{
TTYR_TTY_BEGIN()

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
        ttyr_tty_FileView *FileView_p = View_p->FileEditor.FileViews.pp[j];
        if (j == View_p->FileEditor.FileViews.size - 1) {fileWidth += oddLeftOver;}
        FileView_p->width = fileWidth;
        FileView_p->height = View_p->height;
    }

TTYR_TTY_END(TTYR_TTY_SUCCESS)
}

// TYPE ============================================================================================

TTYR_TTY_FILE ttyr_tty_getFileType(
    nh_encoding_UTF32String *Path_p)
{
TTYR_TTY_BEGIN()

    TTYR_TTY_FILE type = TTYR_TTY_FILE_TEXT;
    nh_encoding_UTF8String Path = nh_encoding_encodeUTF8(Path_p->p, Path_p->length);

    if (strstr(Path.p, "netzhaut/CHANGES")) {
        type = TTYR_TTY_FILE_CHANGES;
    }

    nh_encoding_freeUTF8(&Path);

TTYR_TTY_END(type)
}

// RENDER ==========================================================================================

TTYR_TTY_RESULT ttyr_tty_renderFile(
    ttyr_tty_File *File_p)
{
TTYR_TTY_BEGIN()

    switch (File_p->type)
    {
        case TTYR_TTY_FILE_TEXT :
            for (int i = 0; i < ((ttyr_tty_TextFile*)File_p->handle_p)->Lines.size; ++i) {
                TTYR_TTY_CHECK(ttyr_tty_renderTextFileLine(File_p->handle_p, i))
            }
            break;
        case TTYR_TTY_FILE_CHANGES :
            break;
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// WRITE ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_writeFile(
    ttyr_tty_File *File_p)
{
TTYR_TTY_BEGIN()

    switch (File_p->type)
    {
        case TTYR_TTY_FILE_TEXT :
            TTYR_TTY_CHECK(ttyr_tty_writeTextFile(File_p->handle_p, &File_p->Node_p->Path))
            break;
        case TTYR_TTY_FILE_CHANGES :
            break;
    }

    TTYR_TTY_CHECK(ttyr_tty_setCustomSuffixMessage(
        NULL, TTYR_TTY_MESSAGE_EDITOR_FILE_SAVED, File_p->Node_p->Path.p, File_p->Node_p->Path.length
    ))

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// SEARCH ==========================================================================================

TTYR_TTY_RESULT ttyr_tty_clearFileSearch(
    ttyr_tty_File *File_p)
{
TTYR_TTY_BEGIN()

    switch (File_p->type)
    {
        case TTYR_TTY_FILE_TEXT :
            ttyr_tty_clearTextFileSearch(File_p->handle_p);
            break;
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_searchFile(
    ttyr_tty_File *File_p, NH_ENCODING_UTF32 *str_p, int length)
{
TTYR_TTY_BEGIN()

    switch (File_p->type)
    {
        case TTYR_TTY_FILE_TEXT :
            TTYR_TTY_CHECK(ttyr_tty_searchTextFile(File_p->handle_p, str_p, length))
            break;
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// INPUT ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_handleFileInput(
    ttyr_tty_Program *Program_p, ttyr_tty_File *File_p, NH_ENCODING_UTF32 c, NH_BOOL insertMode, 
    NH_BOOL *refresh_p)
{
TTYR_TTY_BEGIN()

    if (File_p->readOnly) {
        *refresh_p = NH_TRUE;
        TTYR_TTY_CHECK(ttyr_tty_setDefaultMessage(NULL, TTYR_TTY_MESSAGE_EDITOR_READ_ONLY))
    }

    nh_List FileViews = ((ttyr_tty_Editor*)Program_p->handle_p)->View.FileEditor.FileViews;

    switch (File_p->type)
    {
        case TTYR_TTY_FILE_TEXT :
            TTYR_TTY_CHECK(ttyr_tty_handleTextFileInput(&FileViews, File_p, c, insertMode, refresh_p))
            break;    
        case TTYR_TTY_FILE_CHANGES :
            break;    
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

// DRAW ============================================================================================

TTYR_TTY_RESULT ttyr_tty_drawFileRow(
    ttyr_tty_Program *Program_p, ttyr_tty_File *File_p, ttyr_tty_FileView *View_p, ttyr_tty_Glyph *Glyphs_p, 
    int row)
{
TTYR_TTY_BEGIN()

    switch (File_p->type)
    {
        case TTYR_TTY_FILE_TEXT :
            ttyr_tty_drawTextFileLine(Program_p, File_p->handle_p, View_p, Glyphs_p, row);
            break;
        case TTYR_TTY_FILE_CHANGES :
            break;
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

