#ifndef TTYR_CORE_TEXT_FILE_INPUT_H
#define TTYR_CORE_TEXT_FILE_INPUT_H

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

    TTYR_CORE_RESULT ttyr_core_handleTextFileInput(
        nh_core_List *Views_p, ttyr_core_File *File_p, NH_API_UTF32 c, bool insertMode,  
        bool *refresh_p
    );

/** @} */

#endif 
