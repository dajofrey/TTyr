#ifndef TK_CORE_TEXT_FILE_INPUT_H
#define TK_CORE_TEXT_FILE_INPUT_H

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

    TK_CORE_RESULT tk_core_handleTextFileInput(
        nh_core_List *Views_p, tk_core_File *File_p, NH_API_UTF32 c, bool insertMode,  
        bool *refresh_p
    );

/** @} */

#endif 
