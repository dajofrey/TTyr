#ifndef TTYR_TTY_SYNTAX_HIGHLIGHT_H
#define TTYR_TTY_SYNTAX_HIGHLIGHT_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nhtty_enums
 *  @{
 */

    typedef enum TTYR_TTY_TEXT {
        TTYR_TTY_TEXT_UNDEFINED,
        TTYR_TTY_TEXT_C,
        TTYR_TTY_TEXT_CPP,
    } TTYR_TTY_TEXT;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    nh_encoding_UTF32String ttyr_tty_cSyntaxHighlights(
        nh_encoding_UTF32String *String_p
    );

    nh_encoding_UTF32String ttyr_tty_cppSyntaxHighlights(
        nh_encoding_UTF32String *String_p
    );
    
    TTYR_TTY_TEXT ttyr_tty_getTextType(
        nh_encoding_UTF32String *Path_p 
    );

/** @} */

#endif 
