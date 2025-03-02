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

    typedef struct ttyr_core_TextFileLine {
	bool copy;
        nh_core_Array Copy;
        nh_core_Array Unsaved;
        nh_core_Array Search;
        nh_encoding_UTF32String Codepoints;
        nh_encoding_UTF32String RenderCodepoints;
    } ttyr_core_TextFileLine;

    typedef struct ttyr_core_TextFile {
        TTYR_CORE_TEXT textType;
        int lineNumberOffset;
        int fileCursorXTarget;
        int fileCursorX;
        int fileCursorY;
        int select;
        nh_core_List Lines;
    } ttyr_core_TextFile;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_core_TextFileLine *ttyr_core_newTextFileLine(
        ttyr_core_TextFile *TextFile_p, int index
    );
    
    TTYR_CORE_RESULT ttyr_core_insertIntoTextFileLine(
        ttyr_core_TextFileLine *Line_p, int index, NH_API_UTF32 c
    );
    
    TTYR_CORE_RESULT ttyr_core_removeFromTextFileLine(
        ttyr_core_TextFileLine *Line_p, int index, int length
    );

    ttyr_core_TextFile *ttyr_core_openTextFile(
        nh_encoding_UTF32String *Path_p 
    );

    TTYR_CORE_RESULT ttyr_core_closeTextFile(
        ttyr_core_TextFile *File_p
    );

    TTYR_CORE_RESULT ttyr_core_clearTextFileSearch(
        ttyr_core_TextFile *TextFile_p
    );

    TTYR_CORE_RESULT ttyr_core_searchTextFile(
        ttyr_core_TextFile *TextFile_p, NH_API_UTF32 *str_p, int length
    );

    TTYR_CORE_RESULT ttyr_core_writeTextFile(
        ttyr_core_TextFile *File_p, nh_encoding_UTF32String *Path_p
    );

    TTYR_CORE_RESULT ttyr_core_renderTextFileLine(
        ttyr_core_TextFile *TextFile_p, int line
    );

    TTYR_CORE_RESULT ttyr_core_drawTextFileLine(
        ttyr_core_Program *Program_p, ttyr_core_TextFile *TextFile_p, ttyr_core_FileView *FileView_p, 
        ttyr_core_Glyph *Glyphs_p, int line
    );

/** @} */

#endif 
