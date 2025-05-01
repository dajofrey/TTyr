#ifndef TTYR_CORE_TEXT_FILE_H
#define TTYR_CORE_TEXT_FILE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "FileEditor.h"
#include "SyntaxHighlights.h"

#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct tk_core_TextFileLine {
	bool copy;
        nh_core_Array Copy;
        nh_core_Array Unsaved;
        nh_core_Array Search;
        nh_encoding_UTF32String Codepoints;
        nh_encoding_UTF32String RenderCodepoints;
    } tk_core_TextFileLine;

    typedef struct tk_core_TextFile {
        TTYR_CORE_TEXT textType;
        int lineNumberOffset;
        int fileCursorXTarget;
        int fileCursorX;
        int fileCursorY;
        int select;
        nh_core_List Lines;
    } tk_core_TextFile;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    tk_core_TextFileLine *tk_core_newTextFileLine(
        tk_core_TextFile *TextFile_p, int index
    );
    
    TTYR_CORE_RESULT tk_core_insertIntoTextFileLine(
        tk_core_TextFileLine *Line_p, int index, NH_API_UTF32 c
    );
    
    TTYR_CORE_RESULT tk_core_removeFromTextFileLine(
        tk_core_TextFileLine *Line_p, int index, int length
    );

    tk_core_TextFile *tk_core_openTextFile(
        nh_encoding_UTF32String *Path_p 
    );

    TTYR_CORE_RESULT tk_core_closeTextFile(
        tk_core_TextFile *File_p
    );

    TTYR_CORE_RESULT tk_core_clearTextFileSearch(
        tk_core_TextFile *TextFile_p
    );

    TTYR_CORE_RESULT tk_core_searchTextFile(
        tk_core_TextFile *TextFile_p, NH_API_UTF32 *str_p, int length
    );

    TTYR_CORE_RESULT tk_core_writeTextFile(
        tk_core_TextFile *File_p, nh_encoding_UTF32String *Path_p
    );

    TTYR_CORE_RESULT tk_core_renderTextFileLine(
        tk_core_TextFile *TextFile_p, int line
    );

    TTYR_CORE_RESULT tk_core_drawTextFileLine(
        tk_core_Program *Program_p, tk_core_TextFile *TextFile_p, tk_core_FileView *FileView_p, 
        tk_core_Glyph *Glyphs_p, int line
    );

/** @} */

#endif 
