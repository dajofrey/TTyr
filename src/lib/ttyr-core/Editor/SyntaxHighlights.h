#ifndef TTYR_CORE_SYNTAX_HIGHLIGHT_H
#define TTYR_CORE_SYNTAX_HIGHLIGHT_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "nh-encoding/Encodings/UTF32.h"
#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nhtty_enums
 *  @{
 */

    typedef enum TTYR_CORE_TEXT {
        TTYR_CORE_TEXT_UNDEFINED,
        TTYR_CORE_TEXT_C,
        TTYR_CORE_TEXT_CPP,
    } TTYR_CORE_TEXT;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    nh_encoding_UTF32String ttyr_core_cSyntaxHighlights(
        nh_encoding_UTF32String *String_p
    );

    nh_encoding_UTF32String ttyr_core_cppSyntaxHighlights(
        nh_encoding_UTF32String *String_p
    );
    
    TTYR_CORE_TEXT ttyr_core_getTextType(
        nh_encoding_UTF32String *Path_p 
    );

/** @} */

#endif 
