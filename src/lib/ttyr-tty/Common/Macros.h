#ifndef TTYR_TTY_COMMON_MACROS_H
#define TTYR_TTY_COMMON_MACROS_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#ifdef NH_LOG_FLOW
    #include "Log.h"
#endif

#endif // DOXYGEN_SHOULD_SKIP_THIS

// HELPER ==========================================================================================

#define CTRL_KEY(k) ((k) & 0x1f)

#define TTYR_TTY_TILING_KEY CTRL_KEY('t')
#define TTYR_TTY_TOPBAR_KEY CTRL_KEY('e')
#define TTYR_TTY_INSERT_TILE_LEFT_KEY   'a'
#define TTYR_TTY_INSERT_TILE_RIGHT_KEY  'd'
#define TTYR_TTY_INSERT_TILE_TOP_KEY    'w'
#define TTYR_TTY_INSERT_TILE_BOTTOM_KEY 's'
#define TTYR_TTY_SPLIT_KEY 'f'

#define TTYR_TTY_IS_MACRO_TILE(tile_pointer) (((ttyr_tty_Tile*)tile_pointer)->type == TTYR_TTY_TILE_TYPE_MACRO)
#define TTYR_TTY_IS_MICRO_TILE(tile_pointer) (((ttyr_tty_Tile*)tile_pointer)->type == TTYR_TTY_TILE_TYPE_MICRO)

#define TTYR_TTY_MACRO_TAB(tile_pointer) ((ttyr_tty_MacroTab*)((ttyr_tty_MacroTile*)((ttyr_tty_Tile*)tile_pointer)->p)->MacroTabs.pp[((ttyr_tty_MacroTile*)((ttyr_tty_Tile*)tile_pointer)->p)->current])
#define TTYR_TTY_MACRO_TAB_2(tile_pointer, custom_index) ((ttyr_tty_MacroTab*)((ttyr_tty_MacroTile*)((ttyr_tty_Tile*)tile_pointer)->p)->MacroTabs.pp[custom_index])

#define TTYR_TTY_MACRO_TILE(tile_pointer) ((ttyr_tty_MacroTile*)((ttyr_tty_Tile*)tile_pointer)->p)
#define TTYR_TTY_MICRO_TILE(tile_pointer) ((ttyr_tty_MicroTile*)((ttyr_tty_Tile*)tile_pointer)->p)

#define TTYR_TTY_MICRO_TAB(macro_tab_pointer) ((ttyr_tty_MicroTab*)((ttyr_tty_MacroTab*)macro_tab_pointer)->MicroWindow.Tabs_p->pp[((ttyr_tty_MacroTab*)macro_tab_pointer)->MicroWindow.current])

// ERROR LOGGING ===================================================================================

#define TTYR_CHECK(checkable)                                  \
{                                                              \
    TTYR_TTY_RESULT checkResult = checkable;                   \
    if (checkResult != TTYR_TTY_SUCCESS) {return checkResult;} \
}

#define TTYR_CHECK_2(checkReturn, checkable)                   \
{                                                              \
    TTYR_TTY_RESULT checkResult = checkable;                   \
    if (checkResult != TTYR_TTY_SUCCESS) {return checkReturn;} \
}

#define TTYR_CHECK_NULL(checkable)                                   \
{                                                                    \
    void *checkResult_p = checkable;                                 \
    if (checkResult_p == NULL) {return TTYR_TTY_ERROR_NULL_POINTER;} \
}

#define TTYR_CHECK_NULL_2(checkReturn, checkable)     \
{                                                     \
    void *checkResult_p = checkable;                  \
    if (checkResult_p == NULL) {return checkReturn;}  \
}

#define TTYR_CHECK_MEM(checkable)                                         \
{                                                                         \
    void *checkResult_p = checkable;                                      \
    if (checkResult_p == NULL) {return TTYR_TTY_ERROR_MEMORY_ALLOCATION;} \
}

#define TTYR_CHECK_MEM_2(checkReturn, checkable)     \
{                                                    \
    void *checkResult_p = checkable;                 \
    if (checkResult_p == NULL) {return checkReturn;} \
}

#endif // TTYR_TTY_COMMON_MACROS_H
