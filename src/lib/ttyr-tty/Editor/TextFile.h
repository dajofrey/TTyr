#ifndef TTYR_TTY_TEXT_FILE_H
#define TTYR_TTY_TEXT_FILE_H

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

    typedef struct ttyr_tty_TextFileLine {
	NH_BOOL copy;
        nh_Array Copy;
        nh_Array Unsaved;
        nh_Array Search;
        nh_encoding_UTF32String Codepoints;
        nh_encoding_UTF32String RenderCodepoints;
    } ttyr_tty_TextFileLine;

    typedef struct ttyr_tty_TextFile {
        TTYR_TTY_TEXT textType;
        int lineNumberOffset;
        int fileCursorXTarget;
        int fileCursorX;
        int fileCursorY;
        int select;
        nh_List Lines;
    } ttyr_tty_TextFile;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_TextFileLine *ttyr_tty_newTextFileLine(
        ttyr_tty_TextFile *TextFile_p, int index
    );
    
    TTYR_TTY_RESULT ttyr_tty_insertIntoTextFileLine(
        ttyr_tty_TextFileLine *Line_p, int index, NH_ENCODING_UTF32 c
    );
    
    TTYR_TTY_RESULT ttyr_tty_removeFromTextFileLine(
        ttyr_tty_TextFileLine *Line_p, int index, int length
    );

    ttyr_tty_TextFile *ttyr_tty_openTextFile(
        nh_encoding_UTF32String *Path_p 
    );

    TTYR_TTY_RESULT ttyr_tty_closeTextFile(
        ttyr_tty_TextFile *File_p
    );

    TTYR_TTY_RESULT ttyr_tty_clearTextFileSearch(
        ttyr_tty_TextFile *TextFile_p
    );

    TTYR_TTY_RESULT ttyr_tty_searchTextFile(
        ttyr_tty_TextFile *TextFile_p, NH_ENCODING_UTF32 *str_p, int length
    );

    TTYR_TTY_RESULT ttyr_tty_writeTextFile(
        ttyr_tty_TextFile *File_p, nh_encoding_UTF32String *Path_p
    );

    TTYR_TTY_RESULT ttyr_tty_renderTextFileLine(
        ttyr_tty_TextFile *TextFile_p, int line
    );

    TTYR_TTY_RESULT ttyr_tty_drawTextFileLine(
        ttyr_tty_Program *Program_p, ttyr_tty_TextFile *TextFile_p, ttyr_tty_FileView *FileView_p, 
        ttyr_tty_Glyph *Glyphs_p, int line
    );

/** @} */

#endif 
