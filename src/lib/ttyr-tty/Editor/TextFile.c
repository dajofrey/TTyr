// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "TextFile.h"
#include "Editor.h"
#include "TextFileInput.h"
#include "TreeListing.h"
#include "SyntaxHighlights.h"

#include "../TTY/TTY.h"

#include "../Common/Macros.h"

#include "nhcore/System/Process.h"
#include "nhcore/System/Memory.h"
#include "nhcore/Util/File.h"
#include "nhcore/Common/Macros.h"

#include "nhcss/Parser/Parser.h"
#include "nhhtml/Parser/Parser.h"
#include "nhwebidl/Runtime/Parser.h"

#include "nhencoding/Encodings/UTF32.h"
#include "nhencoding/Encodings/UTF8.h"
#include "nhencoding/Common/Macros.h"

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

// LINES ===========================================================================================

ttyr_tty_TextFileLine *ttyr_tty_newTextFileLine(
    ttyr_tty_TextFile *TextFile_p, int index)
{
    ttyr_tty_TextFileLine *New_p = nh_core_allocate(sizeof(ttyr_tty_TextFileLine));
    TTYR_CHECK_MEM_2(NULL, New_p)
    New_p->copy = NH_FALSE;
    New_p->Copy = nh_core_initArray(sizeof(NH_BOOL), 128);
    New_p->Unsaved = nh_core_initArray(sizeof(NH_BOOL), 128);
    New_p->Search = nh_core_initArray(sizeof(NH_BOOL), 128);
    New_p->Codepoints = nh_encoding_initUTF32(128);
    New_p->RenderCodepoints = nh_encoding_initUTF32(128);
    NH_CORE_CHECK_2(NULL, nh_core_insertIntoList(&TextFile_p->Lines, New_p, index))

    return New_p;
}

static TTYR_TTY_RESULT ttyr_tty_appendToTextFileLine(
    ttyr_tty_TextFileLine *Line_p, NH_ENCODING_UTF32 *codepoints_p, int length)
{
    NH_ENCODING_CHECK_2(TTYR_TTY_ERROR_BAD_STATE, nh_encoding_appendUTF32(&Line_p->Codepoints, codepoints_p, length))
    NH_BOOL b = NH_FALSE;
    for (int i = 0; i < length; ++i) {
        nh_core_appendToArray(&Line_p->Copy, &b, 1);
        nh_core_appendToArray(&Line_p->Unsaved, &b, 1);
        nh_core_appendToArray(&Line_p->Search, &b, 1);
    }

    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_insertIntoTextFileLine(
    ttyr_tty_TextFileLine *Line_p, int index, NH_ENCODING_UTF32 c)
{
    NH_ENCODING_CHECK_2(TTYR_TTY_ERROR_BAD_STATE, nh_encoding_insertUTF32(&Line_p->Codepoints, index, &c, 1))
    NH_BOOL b = NH_FALSE;
    nh_core_insertIntoArray(&Line_p->Copy, index, &b, 1);
    nh_core_insertIntoArray(&Line_p->Search, index, &b, 1);
    b = NH_TRUE;
    nh_core_insertIntoArray(&Line_p->Unsaved, index, &b, 1);

    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_removeFromTextFileLine(
    ttyr_tty_TextFileLine *Line_p, int index, int length)
{
    NH_ENCODING_CHECK_2(TTYR_TTY_ERROR_BAD_STATE, nh_encoding_removeUTF32(&Line_p->Codepoints, index, length))
    nh_core_removeFromArray(&Line_p->Copy, index, length);

    return TTYR_TTY_SUCCESS;
}

// OPEN ============================================================================================

static ttyr_tty_TextFile *ttyr_tty_createTextFile()
{
    ttyr_tty_TextFile *TextFile_p = malloc(sizeof(ttyr_tty_TextFile));
    TTYR_CHECK_MEM_2(NULL, TextFile_p)

    TextFile_p->textType          = 0;
    TextFile_p->fileCursorXTarget = 0;
    TextFile_p->fileCursorX       = 0;
    TextFile_p->fileCursorY       = 0;
    TextFile_p->select            = -1;
    TextFile_p->Lines             = nh_core_initList(1024);

    return TextFile_p;
}

static NH_BYTE *ttyr_tty_readFile(
    nh_encoding_UTF32String *Path_p)
{
    nh_encoding_UTF8String Path = nh_encoding_encodeUTF8(Path_p->p, Path_p->length);

    NH_BYTE *buffer_p = NULL;
    long length;
    FILE *f = fopen(Path.p, "r");

    nh_encoding_freeUTF8(&Path);

    if (f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);

        if (length > 0) {
            fseek(f, 0, SEEK_SET);
            buffer_p = malloc(length);
            if (buffer_p) {fread(buffer_p, 1, length, f);}
        }
        else { // maybe we can read it
            nh_String String = nh_core_initString(1024);
            NH_BYTE str_p[1024];
            memset(str_p, 0, 1024);
            read(fileno(f), str_p, 1023);
            nh_core_appendToString(&String, str_p, strlen(str_p));
            nh_core_appendToString(&String, "\0", 1);
            buffer_p = String.p;
        }

        fclose(f);
    }
    
    return buffer_p;
}

ttyr_tty_TextFile *ttyr_tty_openTextFile(
    nh_encoding_UTF32String *Path_p)
{
    ttyr_tty_TextFile *TextFile_p = ttyr_tty_createTextFile();
    TTYR_CHECK_MEM_2(NULL, TextFile_p)

    NH_BYTE *p = ttyr_tty_readFile(Path_p);
    if (p) 
    {
        TextFile_p->textType = ttyr_tty_getTextType(Path_p);
        nh_encoding_UTF32String Codepoints = nh_encoding_decodeUTF8(p, strlen(p), NULL);

        if (Codepoints.length == 0) {
            TTYR_CHECK_MEM_2(NULL, ttyr_tty_newTextFileLine(TextFile_p, TextFile_p->Lines.size))
        }
        else {
            for (unsigned long i = 0, lineStart = 0; i < Codepoints.length; ++i) {
                if (Codepoints.p[i] == '\n') {
                    ttyr_tty_TextFileLine *Line_p = ttyr_tty_newTextFileLine(TextFile_p, TextFile_p->Lines.size);
                    TTYR_CHECK_MEM_2(NULL, Line_p)
                    TTYR_CHECK_2(NULL, ttyr_tty_appendToTextFileLine(Line_p, &Codepoints.p[lineStart], i - lineStart))
                    lineStart = i + 1;
                }
                if (Codepoints.p[i] == '\0') {break;}
            }
        }

        nh_encoding_freeUTF32(&Codepoints);
        free(p);
    }
    else {TTYR_CHECK_MEM_2(NULL, ttyr_tty_newTextFileLine(TextFile_p, TextFile_p->Lines.size))}

    return TextFile_p;
}

// CLOSE ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_closeTextFile(
    ttyr_tty_TextFile *TextFile_p)
{
    for (int i = 0; i < TextFile_p->Lines.size; ++i) {
        ttyr_tty_TextFileLine *Line_p = TextFile_p->Lines.pp[i];
        nh_core_freeArray(&Line_p->Copy);
        nh_core_freeArray(&Line_p->Unsaved);
        nh_core_freeArray(&Line_p->Search);
        nh_encoding_freeUTF32(&Line_p->Codepoints);
        nh_encoding_freeUTF32(&Line_p->RenderCodepoints); 
    }

    nh_core_freeList(&TextFile_p->Lines, NH_TRUE);
    nh_core_free(TextFile_p);

    return TTYR_TTY_SUCCESS;
}

// SEARCH ==========================================================================================

TTYR_TTY_RESULT ttyr_tty_clearTextFileSearch(
    ttyr_tty_TextFile *TextFile_p)
{
    for (int i = 0; i < TextFile_p->Lines.size; ++i) {
        ttyr_tty_TextFileLine *Line_p = TextFile_p->Lines.pp[i];
        NH_BOOL render = NH_FALSE;
        for (int j = 0; j < Line_p->Codepoints.length; ++j) {
            if (((NH_BOOL*)Line_p->Search.p)[j]) {render = NH_TRUE;}
            ((NH_BOOL*)Line_p->Search.p)[j] = NH_FALSE;
        }
        TTYR_CHECK(ttyr_tty_renderTextFileLine(TextFile_p, i))
    }

    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_searchTextFile(
    ttyr_tty_TextFile *TextFile_p, NH_ENCODING_UTF32 *str_p, int length)
{
    for (int i = 0; i < TextFile_p->Lines.size; ++i) {
        ttyr_tty_TextFileLine *Line_p = TextFile_p->Lines.pp[i];
        for (int j = 0; j < Line_p->Codepoints.length; ++j) {
            if (Line_p->Codepoints.p[j] == str_p[0]) {
                int tmp = length;
                for (int k = j, l = 0; k < Line_p->Codepoints.length; ++k, ++l) {
                    if (tmp == 0) {break;}
                    if (Line_p->Codepoints.p[k] != str_p[l]) {break;}
                    tmp--;
                }
                if (tmp == 0) {
                    for (int k = j, l = 0; l < length; ++k, ++l) {
                        ((NH_BOOL*)Line_p->Search.p)[k] = NH_TRUE;
                    }
                    TTYR_CHECK(ttyr_tty_renderTextFileLine(TextFile_p, i))
                }
            }
        }
    }

    return TTYR_TTY_SUCCESS;
}

// WRITE ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_writeTextFile(
    ttyr_tty_TextFile *File_p, nh_encoding_UTF32String *Path_p)
{
    nh_encoding_UTF8String Result = nh_core_initString(1024);

    for (int i = 0; i < File_p->Lines.size; ++i) 
    {
        ttyr_tty_TextFileLine *Line_p = File_p->Lines.pp[i];
        NH_BOOL render = NH_FALSE;

        for (int j = 0; j < Line_p->Unsaved.length; ++j) {
            if (((NH_BOOL*)Line_p->Unsaved.p)[j]) {
                render = NH_TRUE;
                ((NH_BOOL*)Line_p->Unsaved.p)[j] = NH_FALSE;
            }
        }

        if (render) {TTYR_CHECK(ttyr_tty_renderTextFileLine(File_p, i))}

        nh_encoding_UTF8String Line = nh_encoding_encodeUTF8(Line_p->Codepoints.p, Line_p->Codepoints.length);
        nh_core_appendToString(&Result, Line.p, Line.length);
        nh_core_appendToString(&Result, "\n", 1);

        nh_encoding_freeUTF8(&Line);
    }

    nh_encoding_UTF8String Path = nh_encoding_encodeUTF8(Path_p->p, Path_p->length);
    nh_core_writeBytesToFile(Path.p, Result.p);

    nh_encoding_freeUTF8(&Result);
    nh_encoding_freeUTF8(&Path);

    return TTYR_TTY_SUCCESS;
}

// RENDER ==========================================================================================

TTYR_TTY_RESULT ttyr_tty_renderTextFileLine(
    ttyr_tty_TextFile *TextFile_p, int line)
{
    ttyr_tty_TextFileLine *Line_p = TextFile_p->Lines.pp[line];
    nh_encoding_freeUTF32(&Line_p->RenderCodepoints);
    Line_p->RenderCodepoints = nh_encoding_initUTF32(128);

    NH_BOOL selection = NH_FALSE, unsaved = NH_FALSE, search = NH_FALSE;

    for (int i = 0; i < Line_p->Codepoints.length; ++i) 
    {
        if (((NH_BOOL*)Line_p->Unsaved.p)[i] && !unsaved) {
            unsaved = NH_TRUE;
        }
        else if (!((NH_BOOL*)Line_p->Unsaved.p)[i] && unsaved) {
            unsaved = NH_FALSE;
            selection = NH_FALSE;
            search = NH_FALSE;
        }

        if (((NH_BOOL*)Line_p->Copy.p)[i] && !selection) {
            selection = NH_TRUE;
        }
        else if (!((NH_BOOL*)Line_p->Copy.p)[i] && selection) {
            selection = NH_FALSE;
            unsaved = NH_FALSE;
            search = NH_FALSE;
        }

        if (((NH_BOOL*)Line_p->Search.p)[i] && !search) {
            search = NH_TRUE;
        }
        else if (!((NH_BOOL*)Line_p->Search.p)[i] && search) {
            selection = NH_FALSE;
            unsaved = NH_FALSE;
            search = NH_FALSE;
        }

        if (Line_p->Codepoints.p[i] == 9) {
            ttyr_tty_Editor *Editor_p = ttyr_tty_getCurrentProgram(&TTYR_TTY_MACRO_TAB(((ttyr_tty_TTY*)nh_core_getWorkloadArg())->Window_p->Tile_p)->MicroWindow)->handle_p;
            for (int j = 0; j < Editor_p->FileEditor.tabSpaces; ++j) {
                nh_encoding_appendUTF32Codepoint(&Line_p->RenderCodepoints, 32);
            }
        }
        else {
            NH_ENCODING_UTF32 c = Line_p->Codepoints.p[i];
            if (Line_p->Codepoints.p[i] == '\r') {c = 0xFFFD;}
            nh_encoding_appendUTF32(&Line_p->RenderCodepoints, &c, 1);
        }
    }

    return TTYR_TTY_SUCCESS;
}

// DRAW ============================================================================================

static ttyr_tty_Glyph *ttyr_tty_setNextGlyph(
    ttyr_tty_Glyph **Glyphs_pp, NH_ENCODING_UTF32 codepoint)
{
    ttyr_tty_Glyph Glyph;
    memset(&Glyph, 0, sizeof(ttyr_tty_Glyph));
    Glyph.codepoint = codepoint;
 
    ttyr_tty_Glyph *Glyph_p = *Glyphs_pp;
    (*Glyphs_pp)[0] = Glyph;
    (*Glyphs_pp) = (*Glyphs_pp)+1;

    return Glyph_p;
}

static int ttyr_tty_renderTextFileLineNumber(
    ttyr_tty_Program *Program_p, ttyr_tty_TextFile *TextFile_p, ttyr_tty_Glyph **Glyphs_pp, int line)
{
    NH_BYTE maxX_p[16] = {'\0'};
    sprintf(maxX_p, "%d", TextFile_p->fileCursorXTarget);

    NH_BYTE maxY_p[16] = {'\0'};
    sprintf(maxY_p, "%d", TextFile_p->Lines.size);

    NH_BYTE maxXY_p[16] = {'\0'};
    sprintf(maxXY_p, "%s %s", maxX_p, maxY_p);

    NH_BYTE x_p[16] = {'\0'};
    sprintf(x_p, "%d", TextFile_p->fileCursorX);

    NH_BYTE y_p[16] = {'\0'};
    sprintf(y_p, "%d", line);

    NH_BYTE xy_p[16] = {'\0'};
    memset(xy_p, 0, 16);
    memset(xy_p, ' ', strlen(maxXY_p));

    for (int i = strlen(maxXY_p)-strlen(y_p), j = 0; i < strlen(maxXY_p) && j < strlen(y_p); ++i, ++j) {
        xy_p[i] = y_p[j];
    }
    if (TextFile_p->fileCursorY == line) {
        for (int i = (strlen(maxXY_p)-strlen(y_p))-(strlen(x_p)+1), j = 0; j < strlen(x_p); ++i, ++j) {
            xy_p[i] = x_p[j];
        }
    }

    for (int i = 0; i < strlen(xy_p); ++i) {
        ttyr_tty_Glyph *Glyph_p = ttyr_tty_setNextGlyph(Glyphs_pp, xy_p[i]);
        if (xy_p[i] != ' ') {
            Glyph_p->Attributes.reverse = TextFile_p->fileCursorY == line;
            if (TextFile_p->fileCursorY != line) {
                Glyph_p->Foreground.custom = NH_TRUE;
                Glyph_p->Foreground.Color.r = 0.6f;
                Glyph_p->Foreground.Color.g = 0.3f;
                Glyph_p->Foreground.Color.b = 0.2f;
            }
        }
    }

    (*Glyphs_pp) += 1;

    return strlen(maxXY_p)+1;
}

static int ttyr_tty_computeRenderSegmentOffset(
    nh_encoding_UTF32String *RenderLine_p, int offset)
{
    NH_BOOL esc = NH_FALSE;
    int length = 0;

    for (int i = 0; i < RenderLine_p->length; ++i) {
        if (offset == 0) {break;}
        if (RenderLine_p->p[i] == '\x1b') {esc = NH_TRUE;}
        if (!esc) {--offset;}
        if (RenderLine_p->p[i] == 'm') {esc = NH_FALSE;}
        length++;
    }

    return length;
}

static int ttyr_tty_computeRenderSegment(
    nh_encoding_UTF32String *RenderLine_p, int offset, int width)
{
    NH_BOOL esc = NH_FALSE;
    int length = 0;

    for (int i = offset; i < RenderLine_p->length; ++i) {
        if (width == 0) {break;}
        if (RenderLine_p->p[i] == '\x1b') {esc = NH_TRUE;}
        if (!esc) {--width;}
        if (RenderLine_p->p[i] == 'm') {esc = NH_FALSE;}
        length++;
    }

    return length;
}

TTYR_TTY_RESULT ttyr_tty_drawTextFileLine(
    ttyr_tty_Program *Program_p, ttyr_tty_TextFile *TextFile_p, ttyr_tty_FileView *FileView_p, 
    ttyr_tty_Glyph *Glyphs_p, int line)
{
    line += FileView_p->TextFile.rowOffset;

    if (line < TextFile_p->Lines.size) 
    {
        TextFile_p->lineNumberOffset = ttyr_tty_renderTextFileLineNumber(Program_p, TextFile_p, &Glyphs_p, line);
        ttyr_tty_TextFileLine *Line_p = TextFile_p->Lines.pp[line];

        int offset = 
            ttyr_tty_computeRenderSegmentOffset(
                &Line_p->RenderCodepoints, FileView_p->TextFile.colOffset);
        int length = 
            ttyr_tty_computeRenderSegment(
                &Line_p->RenderCodepoints, offset, FileView_p->width - TextFile_p->lineNumberOffset);

        for (int i = 0; i < length; ++i) {
            ttyr_tty_setNextGlyph(&Glyphs_p, Line_p->RenderCodepoints.p[i+offset]);
        }
    }
    else {
        NH_BYTE maxX_p[16] = {'\0'};
        sprintf(maxX_p, "%d ~", TextFile_p->fileCursorXTarget);
        ttyr_tty_Glyph *Glyph_p = NULL;
        for (int i = 0; i < strlen(maxX_p); ++i) {
            Glyph_p = ttyr_tty_setNextGlyph(&Glyphs_p, ' ');
        }
        Glyph_p->codepoint = '~';
    }

    return TTYR_TTY_SUCCESS;
}

