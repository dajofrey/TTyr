#ifndef TTYR_TTY_TEXT_FILE_INPUT_H
#define TTYR_TTY_TEXT_FILE_INPUT_H

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

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    TTYR_TTY_RESULT ttyr_tty_handleTextFileInput(
        nh_List *Views_p, ttyr_tty_File *File_p, NH_ENCODING_UTF32 c, NH_BOOL insertMode,  
        NH_BOOL *refresh_p
    );

/** @} */

#endif 
