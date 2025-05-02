#ifndef TK_CORE_TOPBAR_MESSAGE_H
#define TK_CORE_TOPBAR_MESSAGE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "nh-encoding/Encodings/UTF32.h"
#include "../Common/Includes.h"

typedef struct tk_core_Topbar tk_core_Topbar;

#endif

/** @addtogroup lib_nhtty_enums
 *  @{
 */

    typedef enum TK_CORE_MESSAGE {
        TK_CORE_MESSAGE_TILING_FOCUS,
        TK_CORE_MESSAGE_TILING_INSERT,
        TK_CORE_MESSAGE_MICRO_TILING_WASD,
        TK_CORE_MESSAGE_MICRO_TILING_WASDF,
        TK_CORE_MESSAGE_MACRO_TILING_WASD,
        TK_CORE_MESSAGE_MACRO_TILING_WASDF,
        TK_CORE_MESSAGE_TILING_FOCUS_SWITCHED,
        TK_CORE_MESSAGE_TILING_ALREADY_FOCUSED,
        TK_CORE_MESSAGE_EDITOR_SHOW_TREE,
        TK_CORE_MESSAGE_EDITOR_HIDE_TREE,
        TK_CORE_MESSAGE_EDITOR_INSERT_ACTIVATED,
        TK_CORE_MESSAGE_EDITOR_INSERT_DEACTIVATED,
        TK_CORE_MESSAGE_EDITOR_PREVIEW_ENABLED,
        TK_CORE_MESSAGE_EDITOR_PREVIEW_DISABLED,
        TK_CORE_MESSAGE_EDITOR_READ_ONLY,
        TK_CORE_MESSAGE_EDITOR_NEW_FILE,
        TK_CORE_MESSAGE_EDITOR_IGNORE_INPUT,
        TK_CORE_MESSAGE_EDITOR_FILE_EDIT,
        TK_CORE_MESSAGE_EDITOR_FILE_REMOVED,
        TK_CORE_MESSAGE_EDITOR_FILE_SAVED,
        TK_CORE_MESSAGE_EDITOR_FILE_CLOSED,
        TK_CORE_MESSAGE_EDITOR_FILE_OPENED,
        TK_CORE_MESSAGE_EDITOR_FILE_ALREADY_EXISTS,
        TK_CORE_MESSAGE_EDITOR_NEW_ROOT,
        TK_CORE_MESSAGE_EDITOR_EMPTY_DIRECTORY,
        TK_CORE_MESSAGE_EDITOR_TAB_TO_SPACES_ENABLED,
        TK_CORE_MESSAGE_EDITOR_TAB_TO_SPACES_DISABLED,
        TK_CORE_MESSAGE_EDITOR_NUMBER_OF_TAB_SPACES,
        TK_CORE_MESSAGE_BINARY_QUERY_YES_NO,
        TK_CORE_MESSAGE_BINARY_QUERY_DELETE,
        TK_CORE_MESSAGE_BINARY_QUERY_DELETE_INTERRUPTED,
        TK_CORE_MESSAGE_MISC_NO_CURRENT_PROGRAM,
        TK_CORE_MESSAGE_MISC_BASIC_CONTROLS,
        TK_CORE_MESSAGE_MISC_UNKNOWN_COMMAND,
        TK_CORE_MESSAGE_MISC_INVALID_ARGUMENT,
    } TK_CORE_MESSAGE;

/** @} */

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct tk_core_TopbarMessage {
        bool block;
        nh_encoding_UTF32String Text;
        void *args_p;
        TK_CORE_RESULT (*callback_f)(nh_api_KeyboardEvent Event, bool *continue_p);
    } tk_core_TopbarMessage;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    NH_API_UTF32 *tk_core_getMessage(
        TK_CORE_MESSAGE message, int *length_p
    );

    TK_CORE_RESULT tk_core_clearMessage(
        tk_core_Topbar *Topbar_p
    );

    TK_CORE_RESULT tk_core_setDefaultMessage(
        tk_core_Topbar *Topbar_p, TK_CORE_MESSAGE message
    );
    
    TK_CORE_RESULT tk_core_setCustomMessage(
        tk_core_Topbar *Topbar_p, NH_API_UTF32 *suffix_p, int suffixLength
    );

    TK_CORE_RESULT tk_core_setCustomSuffixMessage(
        tk_core_Topbar *Topbar_p, TK_CORE_MESSAGE prefix, NH_API_UTF32 *suffix_p, 
        int suffixLength
    );

    TK_CORE_RESULT tk_core_setBinaryQueryMessage(
        tk_core_Topbar *Topbar_p, NH_API_UTF32 *query_p, int length, void *args_p, 
        TK_CORE_RESULT (*callback_f)(nh_api_KeyboardEvent Event, bool *continue_p)
    );

/** @} */

#endif 
