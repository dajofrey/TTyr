// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "TextFileInput.h"
#include "TextFile.h"
#include "Editor.h"
#include "TreeListing.h"

#include "../TTY/TTY.h"
#include "../Common/Macros.h"

#include "nh-core/System/Process.h"
#include "nh-core/System/Memory.h"
#include "nh-core/Util/File.h"
#include "nh-core/Common/Macros.h"

#include "nh-css/Parser/Parser.h"
#include "nh-html/Parser/Parser.h"
#include "nh-webidl/Runtime/Parser.h"

#include "nh-encoding/Encodings/UTF32.h"
#include "nh-encoding/Encodings/UTF8.h"
#include "nh-encoding/Common/Macros.h"

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

// CLIPBOARD =======================================================================================

static TTYR_TTY_RESULT ttyr_tty_setClipboard(
    ttyr_tty_TextFile *TextFile_p, bool append, bool *refresh_p)
{
    if (!append) {ttyr_tty_resetClipboard();}

    for (int i = 0; i < TextFile_p->Lines.size; ++i) 
    {
        ttyr_tty_TextFileLine *Line_p = TextFile_p->Lines.pp[i];
        nh_encoding_UTF32String *Copy_p = NULL;

	if (!Line_p->Copy.length && Line_p->copy) {
            Copy_p = ttyr_tty_newClipboardLine();
            TTYR_CHECK_MEM(Copy_p)
	    Line_p->copy = false;
            continue;
	}

        for (int j = 0; j < Line_p->Copy.length; ++j) {
            if (((bool*)Line_p->Copy.p)[j]) {
                if (!Copy_p) {
                    Copy_p = ttyr_tty_newClipboardLine();
                    TTYR_CHECK_MEM(Copy_p)
                }
                nh_encoding_appendUTF32(Copy_p, &Line_p->Codepoints.p[j], 1);
                ((bool*)Line_p->Copy.p)[j] = false;
            }
        }

	Line_p->copy = false;

        if (Copy_p) {
            TTYR_CHECK(ttyr_tty_renderTextFileLine(TextFile_p, i))
            *refresh_p = true;
        }
    }

    return TTYR_TTY_SUCCESS;
}

static TTYR_TTY_RESULT ttyr_tty_insertClipboard(
    nh_core_List *Views_p, ttyr_tty_File *File_p, bool *refresh_p)
{
    ttyr_tty_TextFile *TextFile_p = File_p->handle_p;
    ttyr_tty_Clipboard *Clipboard_p = ttyr_tty_getClipboard();

    // Move cursor.
    if (Clipboard_p->Lines.length > 1) 
    {
        int fileCursorX = TextFile_p->fileCursorX;
        TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, 13, false, refresh_p))
        TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, 'k', false, refresh_p))

        while (TextFile_p->fileCursorX < fileCursorX) {
            TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, 'l', false, refresh_p))
        }
    }

    for (int i = 0; i < Clipboard_p->Lines.length; ++i) 
    {
        nh_encoding_UTF32String *ClipboardLine_p = &((nh_encoding_UTF32String*)Clipboard_p->Lines.p)[i];

	if (ClipboardLine_p->length == 0) {
	    TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, 'o', false, refresh_p))
	    continue;
	}
        for (int j = 0; j < ClipboardLine_p->length; ++j) {
            TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, ClipboardLine_p->p[j], true, refresh_p))
        }
        if (i + 1 < Clipboard_p->Lines.length) {
            TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, 'o', false, refresh_p))
        }
    }

    *refresh_p = true;

    return TTYR_TTY_SUCCESS;
}

// INPUT ===========================================================================================

static TTYR_TTY_RESULT ttyr_tty_handleFileCursorXTarget(
    nh_core_List *Views_p, ttyr_tty_File *File_p, bool *refresh_p)
{
    ttyr_tty_TextFile *TextFile_p = File_p->handle_p;
    ttyr_tty_TextFileLine *Line_p = nh_core_getFromList(&TextFile_p->Lines, TextFile_p->fileCursorY);
    TTYR_CHECK_NULL(Line_p)

    int *colOffsets_p = nh_core_allocate(sizeof(int) * Views_p->size);
    TTYR_CHECK_MEM(colOffsets_p)

    for (int i = 0; i < Views_p->size; ++i) {
        colOffsets_p[i] = ((ttyr_tty_FileView*)Views_p->pp[i])->TextFile.colOffset;
        ((ttyr_tty_FileView*)Views_p->pp[i])->TextFile.colOffset = 0;
        ((ttyr_tty_FileView*)Views_p->pp[i])->TextFile.screenCursorX = 0;
    }

    int xTarget = TextFile_p->fileCursorXTarget;
    TextFile_p->fileCursorX = 0;

    if (xTarget > Line_p->Codepoints.length) {
        while (TextFile_p->fileCursorX < Line_p->Codepoints.length) {
            TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, 'l', false, refresh_p))
        }
        TextFile_p->fileCursorXTarget = xTarget;
    }
    else {
        while (xTarget > TextFile_p->fileCursorX) {
            TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, 'l', false, refresh_p))
        }
        TextFile_p->fileCursorXTarget = xTarget;
    }

    for (int i = 0; i < Views_p->size; ++i) {
        if (((ttyr_tty_FileView*)Views_p->pp[i])->TextFile.colOffset != colOffsets_p[i]) {
            *refresh_p = true;
        }
    }

    nh_core_free(colOffsets_p);

    return TTYR_TTY_SUCCESS;
}

static TTYR_TTY_RESULT ttyr_tty_handleCopySelection(
    ttyr_tty_TextFile *TextFile_p, ttyr_tty_TextFileLine *Line_p, NH_API_UTF32 c, bool *refresh_p)
{
    if (TextFile_p->select < 0) {return TTYR_TTY_SUCCESS;}

    switch (c)
    {
        case 'j' :
            for (int i = 0; i < Line_p->Copy.length; ++i) { 
                ((bool*)Line_p->Copy.p)[i] = TextFile_p->select < TextFile_p->fileCursorY;
            }
	    Line_p->copy = true;
            TTYR_CHECK(ttyr_tty_renderTextFileLine(TextFile_p, TextFile_p->fileCursorY - 1))
            break;

        case 'k' :
            for (int i = 0; i < Line_p->Copy.length; ++i) { 
                ((bool*)Line_p->Copy.p)[i] = TextFile_p->select > TextFile_p->fileCursorY + 1;
            }
	    Line_p->copy = false;
            TTYR_CHECK(ttyr_tty_renderTextFileLine(TextFile_p, TextFile_p->fileCursorY + 1))
            break;

        case 'l' :
            ((bool*)Line_p->Copy.p)[TextFile_p->fileCursorX - 1] = true;
            TTYR_CHECK(ttyr_tty_renderTextFileLine(TextFile_p, TextFile_p->fileCursorY))
            break;

        case 'h' :
            ((bool*)Line_p->Copy.p)[TextFile_p->fileCursorX + 1] = false;
            TTYR_CHECK(ttyr_tty_renderTextFileLine(TextFile_p, TextFile_p->fileCursorY))
            break;
    }

    *refresh_p = true;

    return TTYR_TTY_SUCCESS;
}

// READ ============================================================================================

static TTYR_TTY_RESULT ttyr_tty_handleTextFileViews(
    nh_core_List *Views_p, ttyr_tty_TextFile *TextFile_p, NH_API_UTF32 c, bool *refresh_p)
{
    switch (c)
    {
        case 'j' : 
            for (int i = 0; i < Views_p->size; ++i) {
                ttyr_tty_FileView *FileView_p = Views_p->pp[i];
                if (FileView_p->TextFile.screenCursorY < FileView_p->height-1) {
                    FileView_p->TextFile.screenCursorY++;
                } else {
                    FileView_p->TextFile.rowOffset++;
                    *refresh_p = true;
                }
            }
            break; 

        case 'k' :
            for (int i = 0; i < Views_p->size; ++i) {
                ttyr_tty_FileView *FileView_p = Views_p->pp[i];
                if (FileView_p->TextFile.screenCursorY > 0) {FileView_p->TextFile.screenCursorY--;}
                else if (FileView_p->TextFile.rowOffset > 0) {
                    FileView_p->TextFile.rowOffset--;
                    *refresh_p = true;
                }
            }
            break; 

        case 'l' :
            for (int i = 0; i < Views_p->size; ++i) {
                ttyr_tty_FileView *FileView_p = Views_p->pp[i];
                if (FileView_p->TextFile.screenCursorX < FileView_p->width - TextFile_p->lineNumberOffset - 1) {FileView_p->TextFile.screenCursorX++;}
                else {
                    FileView_p->TextFile.colOffset++;
                    *refresh_p = true;
                }
            }
            break;

        case 'h' :
            for (int i = 0; i < Views_p->size; ++i) {
                ttyr_tty_FileView *FileView_p = Views_p->pp[i];
                if (FileView_p->TextFile.screenCursorX > 0) {FileView_p->TextFile.screenCursorX--;}
                else if (FileView_p->TextFile.colOffset > 0) {
                    FileView_p->TextFile.colOffset--;
                    *refresh_p = true;
                }
            }
            break;
    }

    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_handleReadOperation(
    nh_core_List *Views_p, ttyr_tty_File *File_p, NH_API_UTF32 c, bool insertMode, bool *refresh_p,
    bool *read_p)
{
    ttyr_tty_TextFile *TextFile_p = File_p->handle_p;
    ttyr_tty_TextFileLine *Line_p = nh_core_getFromList(&TextFile_p->Lines, TextFile_p->fileCursorY);
    TTYR_CHECK_NULL(Line_p)

    if (insertMode) {
        switch (c) {
            case 'j' :
            case 'k' :
            case 'l' :
            case 'h' :
            case 'c' : return TTYR_TTY_SUCCESS;
        }
    }

    switch (c)
    {
        case 'j' : 
        case 'k' :
        case 'l' :
        case 'h' :
        case 'c' :
        case CTRL_KEY('j') : 
        case CTRL_KEY('k') : 
        case CTRL_KEY('l') : 
        case CTRL_KEY('h') : 
        case CTRL_KEY('c') : *read_p = true;
    }

    switch (c)
    {
        case 'j' : 
        case CTRL_KEY('j') : 

            if (TextFile_p->fileCursorY < TextFile_p->Lines.size - 1)
            {
                TextFile_p->fileCursorY++;

                TTYR_CHECK(ttyr_tty_handleTextFileViews(Views_p, TextFile_p, 'j', refresh_p))
                TTYR_CHECK(ttyr_tty_handleFileCursorXTarget(Views_p, File_p, refresh_p))
                TTYR_CHECK(ttyr_tty_handleCopySelection(TextFile_p, Line_p, 'j', refresh_p))

                // Force refresh.
                *refresh_p = true;
            }
            break; 

        case 'k' :
        case CTRL_KEY('k') : 

            if (TextFile_p->fileCursorY > 0)
            {
                TextFile_p->fileCursorY--;

                TTYR_CHECK(ttyr_tty_handleTextFileViews(Views_p, TextFile_p, 'k', refresh_p))
                TTYR_CHECK(ttyr_tty_handleFileCursorXTarget(Views_p, File_p, refresh_p))
                TTYR_CHECK(ttyr_tty_handleCopySelection(TextFile_p, Line_p, 'k', refresh_p))

                // Force refresh.
                *refresh_p = true;
            }
            break; 

        case 'l' :
        case CTRL_KEY('l') : 

            if (TextFile_p->fileCursorX < Line_p->Codepoints.length) 
            {
                TextFile_p->fileCursorX++;
                TextFile_p->fileCursorXTarget = TextFile_p->fileCursorX;

                TTYR_CHECK(ttyr_tty_handleTextFileViews(Views_p, TextFile_p, 'l', refresh_p))
                TTYR_CHECK(ttyr_tty_handleCopySelection(TextFile_p, Line_p, 'l', refresh_p))

                // Force refresh.
                *refresh_p = true;
            }
            break;

        case 'h' :
        case CTRL_KEY('h') : 

            if (TextFile_p->fileCursorX > 0) 
            {
                TextFile_p->fileCursorX--;
                TextFile_p->fileCursorXTarget = TextFile_p->fileCursorX;

                TTYR_CHECK(ttyr_tty_handleTextFileViews(Views_p, TextFile_p, 'h', refresh_p))
                TTYR_CHECK(ttyr_tty_handleCopySelection(TextFile_p, Line_p, 'h', refresh_p))

                // Force refresh.
                *refresh_p = true;
            }
            break;

         case 'c' :
         case CTRL_KEY('c') :

             if (TextFile_p->select >= 0) {
                 TTYR_CHECK(ttyr_tty_setClipboard(TextFile_p, false, refresh_p))
                 TextFile_p->select = -1;
             }
             else {TextFile_p->select = TextFile_p->fileCursorY;}
             break;
    }

    return TTYR_TTY_SUCCESS;
}

// WRITE ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_handleWriteOperation(
    nh_core_List *Views_p, ttyr_tty_File *File_p, NH_API_UTF32 c, bool insertMode, bool *refresh_p, 
    bool *write_p)
{
    ttyr_tty_Editor *Editor_p = ttyr_tty_getCurrentProgram(&TTYR_TTY_MACRO_TAB(((ttyr_tty_TTY*)nh_core_getWorkloadArg())->Window_p->Tile_p)->MicroWindow)->handle_p;
    ttyr_tty_TextFile *TextFile_p = File_p->handle_p;
    ttyr_tty_TextFileLine *Line_p = nh_core_getFromList(&TextFile_p->Lines, TextFile_p->fileCursorY);
    TTYR_CHECK_NULL(Line_p)

    switch (c)
    {
        case 9 : // tab
        case 13  : // enter
        case 127 : // delete
        case CTRL_KEY('o') : // new line
        case CTRL_KEY('x') : 
        case CTRL_KEY('u') : // delete
        case CTRL_KEY('p') : // forward delete
        case CTRL_KEY('v') : // paste 
            *write_p = true;
    }

    if (*write_p) {*refresh_p = true;}

    switch (c)
    {
        case CTRL_KEY('o') :

            TTYR_CHECK_MEM(ttyr_tty_newTextFileLine(TextFile_p, TextFile_p->fileCursorY + 1))
            ttyr_tty_renderTextFileLine(TextFile_p, TextFile_p->fileCursorY + 1);
            TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, 'j', false, refresh_p))
            break;
 
        case CTRL_KEY('p') :

            TTYR_CHECK(ttyr_tty_removeFromTextFileLine(Line_p, TextFile_p->fileCursorX, 1))
            TTYR_CHECK(ttyr_tty_renderTextFileLine(TextFile_p, TextFile_p->fileCursorY))
            break;

        case CTRL_KEY('x') :

            if (TextFile_p->fileCursorY >= TextFile_p->Lines.size - 1) {
                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('k'), false, refresh_p))
                NH_CORE_CHECK_2(TTYR_TTY_ERROR_BAD_STATE, nh_core_removeFromList(&TextFile_p->Lines, true, TextFile_p->fileCursorY + 1))
            }
            else {
                NH_CORE_CHECK_2(TTYR_TTY_ERROR_BAD_STATE, nh_core_removeFromList(&TextFile_p->Lines, true, TextFile_p->fileCursorY))
            }
            break;

        case 127 : // delete
        case CTRL_KEY('u') :

            if (TextFile_p->fileCursorX > 0) {
                TTYR_CHECK(ttyr_tty_removeFromTextFileLine(Line_p, TextFile_p->fileCursorX - 1, 1))
                TTYR_CHECK(ttyr_tty_renderTextFileLine(TextFile_p, TextFile_p->fileCursorY))
                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('h'), false, refresh_p))
            }
            else {
                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('k'), false, refresh_p))

                ttyr_tty_TextFileLine *PreviousLine_p = 
                    nh_core_getFromList(&TextFile_p->Lines, TextFile_p->fileCursorY);
                int length = PreviousLine_p->Codepoints.length;

                while (TextFile_p->fileCursorX < length) {
                    TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('l'), false, refresh_p))
                }

                for (int i = 0; i < Line_p->Codepoints.length; ++i) {
                    TTYR_CHECK(ttyr_tty_handleTextFileInput(
                        Views_p, File_p, Line_p->Codepoints.p[i], true, refresh_p
                    ))
                }

                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('j'), false, refresh_p))
                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('x'), false, refresh_p))
                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('k'), false, refresh_p))

                for (int i = 0; i < length; ++i) {
                    TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('l'), false, refresh_p))
                }
            }
            break;

        case CTRL_KEY('v') : // insert

            TTYR_CHECK(ttyr_tty_insertClipboard(Views_p, File_p, refresh_p))
            break;

        case 9 : // tab

            if (Editor_p->FileEditor.tabToSpaces) {
                for (int i = 0; i < Editor_p->FileEditor.tabSpaces; ++i) {
                    TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, 32, true, refresh_p))
                }
            } else {*write_p = false;}
            break;

        case 13 : // carriage-return aka enter
        {
            int fileCursorX = TextFile_p->fileCursorX;
            TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('o'), false, refresh_p))

            for (int i = fileCursorX; i < Line_p->Codepoints.length; ++i) {
                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, Line_p->Codepoints.p[i], true, refresh_p))
            }
            while (TextFile_p->fileCursorX > 0) {
                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('h'), false, refresh_p))
            }

            TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('k'), false, refresh_p))

            while (TextFile_p->fileCursorX < fileCursorX) {
                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('l'), false, refresh_p))
            }
            while (TextFile_p->fileCursorX < Line_p->Codepoints.length) {
                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('p'), false, refresh_p))
            }

            TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('j'), false, refresh_p))
            while (TextFile_p->fileCursorX > 0) {
                TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('h'), false, refresh_p))
            }

            break;
        }
    }
 
    return TTYR_TTY_SUCCESS;
}

// HANDLE INPUT ====================================================================================

TTYR_TTY_RESULT ttyr_tty_handleTextFileInput(
    nh_core_List *Views_p, ttyr_tty_File *File_p, NH_API_UTF32 c, bool insertMode, bool *refresh_p)
{
    bool read = false;
    TTYR_CHECK(ttyr_tty_handleReadOperation(Views_p, File_p, c, insertMode, refresh_p, &read))
    if (read || File_p->readOnly) {return TTYR_TTY_SUCCESS;}

    bool write = false;
    TTYR_CHECK(ttyr_tty_handleWriteOperation(Views_p, File_p, c, insertMode, refresh_p, &write))
    if (write) {return TTYR_TTY_SUCCESS;}

    if (insertMode) {
        ttyr_tty_TextFile *TextFile_p = File_p->handle_p;
        ttyr_tty_TextFileLine *Line_p = nh_core_getFromList(&TextFile_p->Lines, TextFile_p->fileCursorY);
        TTYR_CHECK(ttyr_tty_insertIntoTextFileLine(Line_p, TextFile_p->fileCursorX, c))
        TTYR_CHECK(ttyr_tty_handleTextFileInput(Views_p, File_p, CTRL_KEY('l'), false, refresh_p))
        TTYR_CHECK(ttyr_tty_renderTextFileLine(TextFile_p, TextFile_p->fileCursorY))
        *refresh_p = true;
    }
 
    if (!insertMode) {
        ttyr_tty_setCustomSuffixMessage(NULL, TTYR_TTY_MESSAGE_EDITOR_IGNORE_INPUT, &c, 1);
        *refresh_p = true;
    }

    return TTYR_TTY_SUCCESS;
}

