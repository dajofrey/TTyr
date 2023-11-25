#ifndef TTYR_TTY_TOPBAR_MESSAGE_H
#define TTYR_TTY_TOPBAR_MESSAGE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "../Common/Includes.h"

typedef struct ttyr_tty_TopBar ttyr_tty_TopBar;

#endif

/** @addtogroup lib_nhtty_enums
 *  @{
 */

    typedef enum TTYR_TTY_MESSAGE {
        TTYR_TTY_MESSAGE_TILING_FOCUS,
        TTYR_TTY_MESSAGE_TILING_INSERT,
        TTYR_TTY_MESSAGE_TILING_WASD,
        TTYR_TTY_MESSAGE_TILING_WASDF,
        TTYR_TTY_MESSAGE_TILING_FOCUS_SWITCHED,
        TTYR_TTY_MESSAGE_TILING_ALREADY_FOCUSED,
        TTYR_TTY_MESSAGE_EDITOR_SHOW_TREE,
        TTYR_TTY_MESSAGE_EDITOR_HIDE_TREE,
        TTYR_TTY_MESSAGE_EDITOR_INSERT_ACTIVATED,
        TTYR_TTY_MESSAGE_EDITOR_INSERT_DEACTIVATED,
        TTYR_TTY_MESSAGE_EDITOR_PREVIEW_ENABLED,
        TTYR_TTY_MESSAGE_EDITOR_PREVIEW_DISABLED,
        TTYR_TTY_MESSAGE_EDITOR_READ_ONLY,
        TTYR_TTY_MESSAGE_EDITOR_NEW_FILE,
        TTYR_TTY_MESSAGE_EDITOR_IGNORE_INPUT,
        TTYR_TTY_MESSAGE_EDITOR_FILE_EDIT,
        TTYR_TTY_MESSAGE_EDITOR_FILE_REMOVED,
        TTYR_TTY_MESSAGE_EDITOR_FILE_SAVED,
        TTYR_TTY_MESSAGE_EDITOR_FILE_CLOSED,
        TTYR_TTY_MESSAGE_EDITOR_FILE_OPENED,
        TTYR_TTY_MESSAGE_EDITOR_FILE_ALREADY_EXISTS,
        TTYR_TTY_MESSAGE_EDITOR_NEW_ROOT,
        TTYR_TTY_MESSAGE_EDITOR_EMPTY_DIRECTORY,
        TTYR_TTY_MESSAGE_EDITOR_TAB_TO_SPACES_ENABLED,
        TTYR_TTY_MESSAGE_EDITOR_TAB_TO_SPACES_DISABLED,
        TTYR_TTY_MESSAGE_EDITOR_NUMBER_OF_TAB_SPACES,
        TTYR_TTY_MESSAGE_BINARY_QUERY_YES_NO,
        TTYR_TTY_MESSAGE_BINARY_QUERY_DELETE,
        TTYR_TTY_MESSAGE_BINARY_QUERY_DELETE_INTERRUPTED,
        TTYR_TTY_MESSAGE_MISC_NO_CURRENT_PROGRAM,
        TTYR_TTY_MESSAGE_MISC_BASIC_CONTROLS,
        TTYR_TTY_MESSAGE_MISC_UNKNOWN_COMMAND,
        TTYR_TTY_MESSAGE_MISC_INVALID_ARGUMENT,
    } TTYR_TTY_MESSAGE;

/** @} */

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_TopBarMessage {
        NH_BOOL block;
        nh_encoding_UTF32String Text;
        void *args_p;
        TTYR_TTY_RESULT (*callback_f)(nh_wsi_KeyboardEvent Event, NH_BOOL *continue_p);
    } ttyr_tty_TopBarMessage;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    NH_ENCODING_UTF32 *ttyr_tty_getMessage(
        TTYR_TTY_MESSAGE message, int *length_p
    );

    TTYR_TTY_RESULT ttyr_tty_clearMessage(
        ttyr_tty_TopBar *TopBar_p
    );

    TTYR_TTY_RESULT ttyr_tty_setDefaultMessage(
        ttyr_tty_TopBar *TopBar_p, TTYR_TTY_MESSAGE message
    );
    
    TTYR_TTY_RESULT ttyr_tty_setCustomMessage(
        ttyr_tty_TopBar *TopBar_p, NH_ENCODING_UTF32 *suffix_p, int suffixLength
    );

    TTYR_TTY_RESULT ttyr_tty_setCustomSuffixMessage(
        ttyr_tty_TopBar *TopBar_p, TTYR_TTY_MESSAGE prefix, NH_ENCODING_UTF32 *suffix_p, 
        int suffixLength
    );

    TTYR_TTY_RESULT ttyr_tty_setBinaryQueryMessage(
        ttyr_tty_TopBar *TopBar_p, NH_ENCODING_UTF32 *query_p, int length, void *args_p, 
        TTYR_TTY_RESULT (*callback_f)(nh_wsi_KeyboardEvent Event, NH_BOOL *continue_p)
    );

/** @} */

#endif 
