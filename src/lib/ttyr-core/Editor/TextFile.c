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

#include "nh-core/System/Process.h"
#include "nh-core/System/Memory.h"
#include "nh-core/Util/File.h"
#include "nh-css/Parser/Parser.h"
#include "nh-html/Parser/Parser.h"
#include "nh-webidl/Runtime/Parser.h"
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

// LINES ===========================================================================================

ttyr_core_TextFileLine *ttyr_core_newTextFileLine(
    ttyr_core_TextFile *TextFile_p, int index)
{
    ttyr_core_TextFileLine *New_p = (ttyr_core_TextFileLine*)nh_core_allocate(sizeof(ttyr_core_TextFileLine));
    TTYR_CHECK_MEM_2(NULL, New_p)
    New_p->copy = false;
    New_p->Copy = nh_core_initArray(sizeof(bool), 128);
    New_p->Unsaved = nh_core_initArray(sizeof(bool), 128);
    New_p->Search = nh_core_initArray(sizeof(bool), 128);
    New_p->Codepoints = nh_encoding_initUTF32(128);
    New_p->RenderCodepoints = nh_encoding_initUTF32(128);
    NH_CORE_CHECK_2(NULL, nh_core_insertIntoList(&TextFile_p->Lines, New_p, index))

    return New_p;
}

static TTYR_CORE_RESULT ttyr_core_appendToTextFileLine(
    ttyr_core_TextFileLine *Line_p, NH_API_UTF32 *codepoints_p, int length)
{
    NH_CORE_CHECK_2(TTYR_CORE_ERROR_BAD_STATE, nh_encoding_appendUTF32(&Line_p->Codepoints, codepoints_p, length))
    bool b = false;
    for (int i = 0; i < length; ++i) {
        nh_core_appendToArray(&Line_p->Copy, &b, 1);
        nh_core_appendToArray(&Line_p->Unsaved, &b, 1);
        nh_core_appendToArray(&Line_p->Search, &b, 1);
    }

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_insertIntoTextFileLine(
    ttyr_core_TextFileLine *Line_p, int index, NH_API_UTF32 c)
{
    NH_CORE_CHECK_2(TTYR_CORE_ERROR_BAD_STATE, nh_encoding_insertUTF32(&Line_p->Codepoints, index, &c, 1))
    bool b = false;
    nh_core_insertIntoArray(&Line_p->Copy, index, &b, 1);
    nh_core_insertIntoArray(&Line_p->Search, index, &b, 1);
    b = true;
    nh_core_insertIntoArray(&Line_p->Unsaved, index, &b, 1);

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_removeFromTextFileLine(
    ttyr_core_TextFileLine *Line_p, int index, int length)
{
    NH_CORE_CHECK_2(TTYR_CORE_ERROR_BAD_STATE, nh_encoding_removeUTF32(&Line_p->Codepoints, index, length))
    nh_core_removeFromArray(&Line_p->Copy, index, length);

    return TTYR_CORE_SUCCESS;
}

// OPEN ============================================================================================

static ttyr_core_TextFile *ttyr_core_createTextFile()
{
    ttyr_core_TextFile *TextFile_p = (ttyr_core_TextFile*)malloc(sizeof(ttyr_core_TextFile));
    TTYR_CHECK_MEM_2(NULL, TextFile_p)

    TextFile_p->textType          = 0;
    TextFile_p->fileCursorXTarget = 0;
    TextFile_p->fileCursorX       = 0;
    TextFile_p->fileCursorY       = 0;
    TextFile_p->select            = -1;
    TextFile_p->Lines             = nh_core_initList(1024);

    return TextFile_p;
}

static char *ttyr_core_readFile(
    nh_encoding_UTF32String *Path_p)
{
    nh_encoding_UTF8String Path = nh_encoding_encodeUTF8(Path_p->p, Path_p->length);

    char *buffer_p = NULL;
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
            nh_core_String String = nh_core_initString(1024);
            char str_p[1024];
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

ttyr_core_TextFile *ttyr_core_openTextFile(
    nh_encoding_UTF32String *Path_p)
{
    ttyr_core_TextFile *TextFile_p = ttyr_core_createTextFile();
    TTYR_CHECK_MEM_2(NULL, TextFile_p)

    char *p = ttyr_core_readFile(Path_p);
    if (p) 
    {
        TextFile_p->textType = ttyr_core_getTextType(Path_p);
        nh_encoding_UTF32String Codepoints = nh_encoding_decodeUTF8(p, strlen(p), NULL);

        if (Codepoints.length == 0) {
            TTYR_CHECK_MEM_2(NULL, ttyr_core_newTextFileLine(TextFile_p, TextFile_p->Lines.size))
        }
        else {
            for (unsigned long i = 0, lineStart = 0; i < Codepoints.length; ++i) {
                if (Codepoints.p[i] == '\n') {
                    ttyr_core_TextFileLine *Line_p = ttyr_core_newTextFileLine(TextFile_p, TextFile_p->Lines.size);
                    TTYR_CHECK_MEM_2(NULL, Line_p)
                    TTYR_CHECK_2(NULL, ttyr_core_appendToTextFileLine(Line_p, &Codepoints.p[lineStart], i - lineStart))
                    lineStart = i + 1;
                }
                if (Codepoints.p[i] == '\0') {break;}
            }
        }

        nh_encoding_freeUTF32(&Codepoints);
        free(p);
    }
    else {TTYR_CHECK_MEM_2(NULL, ttyr_core_newTextFileLine(TextFile_p, TextFile_p->Lines.size))}

    return TextFile_p;
}

// CLOSE ===========================================================================================

TTYR_CORE_RESULT ttyr_core_closeTextFile(
    ttyr_core_TextFile *TextFile_p)
{
    for (int i = 0; i < TextFile_p->Lines.size; ++i) {
        ttyr_core_TextFileLine *Line_p = TextFile_p->Lines.pp[i];
        nh_core_freeArray(&Line_p->Copy);
        nh_core_freeArray(&Line_p->Unsaved);
        nh_core_freeArray(&Line_p->Search);
        nh_encoding_freeUTF32(&Line_p->Codepoints);
        nh_encoding_freeUTF32(&Line_p->RenderCodepoints); 
    }

    nh_core_freeList(&TextFile_p->Lines, true);
    nh_core_free(TextFile_p);

    return TTYR_CORE_SUCCESS;
}

// SEARCH ==========================================================================================

TTYR_CORE_RESULT ttyr_core_clearTextFileSearch(
    ttyr_core_TextFile *TextFile_p)
{
    for (int i = 0; i < TextFile_p->Lines.size; ++i) {
        ttyr_core_TextFileLine *Line_p = TextFile_p->Lines.pp[i];
        bool render = false;
        for (int j = 0; j < Line_p->Codepoints.length; ++j) {
            if (((bool*)Line_p->Search.p)[j]) {render = true;}
            ((bool*)Line_p->Search.p)[j] = false;
        }
        TTYR_CHECK(ttyr_core_renderTextFileLine(TextFile_p, i))
    }

    return TTYR_CORE_SUCCESS;
}

TTYR_CORE_RESULT ttyr_core_searchTextFile(
    ttyr_core_TextFile *TextFile_p, NH_API_UTF32 *str_p, int length)
{
    for (int i = 0; i < TextFile_p->Lines.size; ++i) {
        ttyr_core_TextFileLine *Line_p = TextFile_p->Lines.pp[i];
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
                        ((bool*)Line_p->Search.p)[k] = true;
                    }
                    TTYR_CHECK(ttyr_core_renderTextFileLine(TextFile_p, i))
                }
            }
        }
    }

    return TTYR_CORE_SUCCESS;
}

// WRITE ===========================================================================================

TTYR_CORE_RESULT ttyr_core_writeTextFile(
    ttyr_core_TextFile *File_p, nh_encoding_UTF32String *Path_p)
{
    nh_encoding_UTF8String Result = nh_core_initString(1024);

    for (int i = 0; i < File_p->Lines.size; ++i) 
    {
        ttyr_core_TextFileLine *Line_p = File_p->Lines.pp[i];
        bool render = false;

        for (int j = 0; j < Line_p->Unsaved.length; ++j) {
            if (((bool*)Line_p->Unsaved.p)[j]) {
                render = true;
                ((bool*)Line_p->Unsaved.p)[j] = false;
            }
        }

        if (render) {TTYR_CHECK(ttyr_core_renderTextFileLine(File_p, i))}

        nh_encoding_UTF8String Line = nh_encoding_encodeUTF8(Line_p->Codepoints.p, Line_p->Codepoints.length);
        nh_core_appendToString(&Result, Line.p, Line.length);
        nh_core_appendToString(&Result, "\n", 1);

        nh_encoding_freeUTF8(&Line);
    }

    nh_encoding_UTF8String Path = nh_encoding_encodeUTF8(Path_p->p, Path_p->length);
    nh_core_writeBytesToFile(Path.p, Result.p);

    nh_encoding_freeUTF8(&Result);
    nh_encoding_freeUTF8(&Path);

    return TTYR_CORE_SUCCESS;
}

// RENDER ==========================================================================================

TTYR_CORE_RESULT ttyr_core_renderTextFileLine(
    ttyr_core_TextFile *TextFile_p, int line)
{
    ttyr_core_TextFileLine *Line_p = TextFile_p->Lines.pp[line];
    nh_encoding_freeUTF32(&Line_p->RenderCodepoints);
    Line_p->RenderCodepoints = nh_encoding_initUTF32(128);

    bool selection = false, unsaved = false, search = false;

    for (int i = 0; i < Line_p->Codepoints.length; ++i) 
    {
        if (((bool*)Line_p->Unsaved.p)[i] && !unsaved) {
            unsaved = true;
        }
        else if (!((bool*)Line_p->Unsaved.p)[i] && unsaved) {
            unsaved = false;
            selection = false;
            search = false;
        }

        if (((bool*)Line_p->Copy.p)[i] && !selection) {
            selection = true;
        }
        else if (!((bool*)Line_p->Copy.p)[i] && selection) {
            selection = false;
            unsaved = false;
            search = false;
        }

        if (((bool*)Line_p->Search.p)[i] && !search) {
            search = true;
        }
        else if (!((bool*)Line_p->Search.p)[i] && search) {
            selection = false;
            unsaved = false;
            search = false;
        }

        if (Line_p->Codepoints.p[i] == 9) {
            ttyr_core_Editor *Editor_p = ttyr_core_getCurrentProgram(&TTYR_CORE_MACRO_TAB(((ttyr_core_TTY*)nh_core_getWorkloadArg())->Window_p->Tile_p)->MicroWindow)->handle_p;
            for (int j = 0; j < Editor_p->FileEditor.tabSpaces; ++j) {
                nh_encoding_appendUTF32Codepoint(&Line_p->RenderCodepoints, 32);
            }
        }
        else {
            NH_API_UTF32 c = Line_p->Codepoints.p[i];
            if (Line_p->Codepoints.p[i] == '\r') {c = 0xFFFD;}
            nh_encoding_appendUTF32(&Line_p->RenderCodepoints, &c, 1);
        }
    }

    return TTYR_CORE_SUCCESS;
}

// DRAW ============================================================================================

static ttyr_core_Glyph *ttyr_core_setNextGlyph(
    ttyr_core_Glyph **Glyphs_pp, NH_API_UTF32 codepoint)
{
    ttyr_core_Glyph Glyph;
    memset(&Glyph, 0, sizeof(ttyr_core_Glyph));
    Glyph.codepoint = codepoint;
 
    ttyr_core_Glyph *Glyph_p = *Glyphs_pp;
    (*Glyphs_pp)[0] = Glyph;
    (*Glyphs_pp) = (*Glyphs_pp)+1;

    return Glyph_p;
}

static int ttyr_core_renderTextFileLineNumber(
    ttyr_core_Program *Program_p, ttyr_core_TextFile *TextFile_p, ttyr_core_Glyph **Glyphs_pp, int line)
{
    char maxX_p[16] = {'\0'};
    sprintf(maxX_p, "%d", TextFile_p->fileCursorXTarget);

    char maxY_p[16] = {'\0'};
    sprintf(maxY_p, "%d", TextFile_p->Lines.size);

    char maxXY_p[16] = {'\0'};
    sprintf(maxXY_p, "%s %s", maxX_p, maxY_p);

    char x_p[16] = {'\0'};
    sprintf(x_p, "%d", TextFile_p->fileCursorX);

    char y_p[16] = {'\0'};
    sprintf(y_p, "%d", line);

    char xy_p[16] = {'\0'};
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
        ttyr_core_Glyph *Glyph_p = ttyr_core_setNextGlyph(Glyphs_pp, xy_p[i]);
        if (xy_p[i] != ' ') {
            Glyph_p->Attributes.reverse = TextFile_p->fileCursorY == line;
            if (TextFile_p->fileCursorY != line) {
                Glyph_p->Foreground.custom = true;
                Glyph_p->Foreground.Color.r = 0.6f;
                Glyph_p->Foreground.Color.g = 0.3f;
                Glyph_p->Foreground.Color.b = 0.2f;
            }
        }
    }

    (*Glyphs_pp) += 1;

    return strlen(maxXY_p)+1;
}

static int ttyr_core_computeRenderSegmentOffset(
    nh_encoding_UTF32String *RenderLine_p, int offset)
{
    bool esc = false;
    int length = 0;

    for (int i = 0; i < RenderLine_p->length; ++i) {
        if (offset == 0) {break;}
        if (RenderLine_p->p[i] == '\x1b') {esc = true;}
        if (!esc) {--offset;}
        if (RenderLine_p->p[i] == 'm') {esc = false;}
        length++;
    }

    return length;
}

static int ttyr_core_computeRenderSegment(
    nh_encoding_UTF32String *RenderLine_p, int offset, int width)
{
    bool esc = false;
    int length = 0;

    for (int i = offset; i < RenderLine_p->length; ++i) {
        if (width == 0) {break;}
        if (RenderLine_p->p[i] == '\x1b') {esc = true;}
        if (!esc) {--width;}
        if (RenderLine_p->p[i] == 'm') {esc = false;}
        length++;
    }

    return length;
}

TTYR_CORE_RESULT ttyr_core_drawTextFileLine(
    ttyr_core_Program *Program_p, ttyr_core_TextFile *TextFile_p, ttyr_core_FileView *FileView_p, 
    ttyr_core_Glyph *Glyphs_p, int line)
{
    line += FileView_p->TextFile.rowOffset;

    if (line < TextFile_p->Lines.size) 
    {
        TextFile_p->lineNumberOffset = ttyr_core_renderTextFileLineNumber(Program_p, TextFile_p, &Glyphs_p, line);
        ttyr_core_TextFileLine *Line_p = TextFile_p->Lines.pp[line];

        int offset = 
            ttyr_core_computeRenderSegmentOffset(
                &Line_p->RenderCodepoints, FileView_p->TextFile.colOffset);
        int length = 
            ttyr_core_computeRenderSegment(
                &Line_p->RenderCodepoints, offset, FileView_p->width - TextFile_p->lineNumberOffset);

        for (int i = 0; i < length; ++i) {
            ttyr_core_setNextGlyph(&Glyphs_p, Line_p->RenderCodepoints.p[i+offset]);
        }
    }
    else {
        char maxX_p[16] = {'\0'};
        sprintf(maxX_p, "%d ~", TextFile_p->fileCursorXTarget);
        ttyr_core_Glyph *Glyph_p = NULL;
        for (int i = 0; i < strlen(maxX_p); ++i) {
            Glyph_p = ttyr_core_setNextGlyph(&Glyphs_p, ' ');
        }
        Glyph_p->codepoint = '~';
    }

    return TTYR_CORE_SUCCESS;
}

