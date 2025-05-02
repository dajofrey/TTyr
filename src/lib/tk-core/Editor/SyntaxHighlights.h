#ifndef TK_CORE_SYNTAX_HIGHLIGHT_H
#define TK_CORE_SYNTAX_HIGHLIGHT_H

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

    typedef enum TK_CORE_TEXT {
        TK_CORE_TEXT_UNDEFINED,
        TK_CORE_TEXT_C,
        TK_CORE_TEXT_CPP,
    } TK_CORE_TEXT;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    nh_encoding_UTF32String tk_core_cSyntaxHighlights(
        nh_encoding_UTF32String *String_p
    );

    nh_encoding_UTF32String tk_core_cppSyntaxHighlights(
        nh_encoding_UTF32String *String_p
    );
    
    TK_CORE_TEXT tk_core_getTextType(
        nh_encoding_UTF32String *Path_p 
    );

/** @} */

#endif 
