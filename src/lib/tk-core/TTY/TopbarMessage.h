#ifndef TTYR_CORE_TOPBAR_MESSAGE_H
#define TTYR_CORE_TOPBAR_MESSAGE_H

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

    typedef enum TTYR_CORE_MESSAGE {
        TTYR_CORE_MESSAGE_TILING_FOCUS,
        TTYR_CORE_MESSAGE_TILING_INSERT,
        TTYR_CORE_MESSAGE_MICRO_TILING_WASD,
        TTYR_CORE_MESSAGE_MICRO_TILING_WASDF,
        TTYR_CORE_MESSAGE_MACRO_TILING_WASD,
        TTYR_CORE_MESSAGE_MACRO_TILING_WASDF,
        TTYR_CORE_MESSAGE_TILING_FOCUS_SWITCHED,
        TTYR_CORE_MESSAGE_TILING_ALREADY_FOCUSED,
        TTYR_CORE_MESSAGE_EDITOR_SHOW_TREE,
        TTYR_CORE_MESSAGE_EDITOR_HIDE_TREE,
        TTYR_CORE_MESSAGE_EDITOR_INSERT_ACTIVATED,
        TTYR_CORE_MESSAGE_EDITOR_INSERT_DEACTIVATED,
        TTYR_CORE_MESSAGE_EDITOR_PREVIEW_ENABLED,
        TTYR_CORE_MESSAGE_EDITOR_PREVIEW_DISABLED,
        TTYR_CORE_MESSAGE_EDITOR_READ_ONLY,
        TTYR_CORE_MESSAGE_EDITOR_NEW_FILE,
        TTYR_CORE_MESSAGE_EDITOR_IGNORE_INPUT,
        TTYR_CORE_MESSAGE_EDITOR_FILE_EDIT,
        TTYR_CORE_MESSAGE_EDITOR_FILE_REMOVED,
        TTYR_CORE_MESSAGE_EDITOR_FILE_SAVED,
        TTYR_CORE_MESSAGE_EDITOR_FILE_CLOSED,
        TTYR_CORE_MESSAGE_EDITOR_FILE_OPENED,
        TTYR_CORE_MESSAGE_EDITOR_FILE_ALREADY_EXISTS,
        TTYR_CORE_MESSAGE_EDITOR_NEW_ROOT,
        TTYR_CORE_MESSAGE_EDITOR_EMPTY_DIRECTORY,
        TTYR_CORE_MESSAGE_EDITOR_TAB_TO_SPACES_ENABLED,
        TTYR_CORE_MESSAGE_EDITOR_TAB_TO_SPACES_DISABLED,
        TTYR_CORE_MESSAGE_EDITOR_NUMBER_OF_TAB_SPACES,
        TTYR_CORE_MESSAGE_BINARY_QUERY_YES_NO,
        TTYR_CORE_MESSAGE_BINARY_QUERY_DELETE,
        TTYR_CORE_MESSAGE_BINARY_QUERY_DELETE_INTERRUPTED,
        TTYR_CORE_MESSAGE_MISC_NO_CURRENT_PROGRAM,
        TTYR_CORE_MESSAGE_MISC_BASIC_CONTROLS,
        TTYR_CORE_MESSAGE_MISC_UNKNOWN_COMMAND,
        TTYR_CORE_MESSAGE_MISC_INVALID_ARGUMENT,
    } TTYR_CORE_MESSAGE;

/** @} */

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct tk_core_TopbarMessage {
        bool block;
        nh_encoding_UTF32String Text;
        void *args_p;
        TTYR_CORE_RESULT (*callback_f)(nh_api_KeyboardEvent Event, bool *continue_p);
    } tk_core_TopbarMessage;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    NH_API_UTF32 *tk_core_getMessage(
        TTYR_CORE_MESSAGE message, int *length_p
    );

    TTYR_CORE_RESULT tk_core_clearMessage(
        tk_core_Topbar *Topbar_p
    );

    TTYR_CORE_RESULT tk_core_setDefaultMessage(
        tk_core_Topbar *Topbar_p, TTYR_CORE_MESSAGE message
    );
    
    TTYR_CORE_RESULT tk_core_setCustomMessage(
        tk_core_Topbar *Topbar_p, NH_API_UTF32 *suffix_p, int suffixLength
    );

    TTYR_CORE_RESULT tk_core_setCustomSuffixMessage(
        tk_core_Topbar *Topbar_p, TTYR_CORE_MESSAGE prefix, NH_API_UTF32 *suffix_p, 
        int suffixLength
    );

    TTYR_CORE_RESULT tk_core_setBinaryQueryMessage(
        tk_core_Topbar *Topbar_p, NH_API_UTF32 *query_p, int length, void *args_p, 
        TTYR_CORE_RESULT (*callback_f)(nh_api_KeyboardEvent Event, bool *continue_p)
    );

/** @} */

#endif 
