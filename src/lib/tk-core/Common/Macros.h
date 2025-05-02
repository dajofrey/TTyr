#ifndef TK_CORE_COMMON_MACROS_H
#define TK_CORE_COMMON_MACROS_H

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

#define TK_CORE_TILING_KEY CTRL_KEY('t')
#define TK_CORE_TOPBAR_KEY CTRL_KEY('e')
#define TK_CORE_INSERT_TILE_LEFT_KEY   'a'
#define TK_CORE_INSERT_TILE_RIGHT_KEY  'd'
#define TK_CORE_INSERT_TILE_TOP_KEY    'w'
#define TK_CORE_INSERT_TILE_BOTTOM_KEY 's'
#define TK_CORE_SPLIT_KEY 'f'

#define TK_CORE_IS_MACRO_TILE(tile_pointer) (((tk_core_Tile*)tile_pointer)->type == TK_CORE_TILE_TYPE_MACRO)
#define TK_CORE_IS_MICRO_TILE(tile_pointer) (((tk_core_Tile*)tile_pointer)->type == TK_CORE_TILE_TYPE_MICRO)

#define TK_CORE_MACRO_TAB(tile_pointer) ((tk_core_MacroTab*)((tk_core_MacroTile*)((tk_core_Tile*)tile_pointer)->p)->MacroTabs.pp[((tk_core_MacroTile*)((tk_core_Tile*)tile_pointer)->p)->current])
#define TK_CORE_MACRO_TAB_2(tile_pointer, custom_index) ((tk_core_MacroTab*)((tk_core_MacroTile*)((tk_core_Tile*)tile_pointer)->p)->MacroTabs.pp[custom_index])

#define TK_CORE_MACRO_TILE(tile_pointer) ((tk_core_MacroTile*)((tk_core_Tile*)tile_pointer)->p)
#define TK_CORE_MICRO_TILE(tile_pointer) ((tk_core_MicroTile*)((tk_core_Tile*)tile_pointer)->p)

#define TK_CORE_MICRO_TAB(macro_tab_pointer) ((tk_core_MicroTab*)((tk_core_MacroTab*)macro_tab_pointer)->MicroWindow.Tabs_p->pp[((tk_core_MacroTab*)macro_tab_pointer)->MicroWindow.current])

// ERROR LOGGING ===================================================================================

#define TK_CHECK(checkable)                                  \
{                                                              \
    TK_CORE_RESULT checkResult = checkable;                   \
    if (checkResult != TK_CORE_SUCCESS) {return checkResult;} \
}

#define TK_CHECK_2(checkReturn, checkable)                   \
{                                                              \
    TK_CORE_RESULT checkResult = checkable;                   \
    if (checkResult != TK_CORE_SUCCESS) {return checkReturn;} \
}

#define TK_CHECK_NULL(checkable)                                   \
{                                                                    \
    void *checkResult_p = checkable;                                 \
    if (checkResult_p == NULL) {return TK_CORE_ERROR_NULL_POINTER;} \
}

#define TK_CHECK_NULL_2(checkReturn, checkable)     \
{                                                     \
    void *checkResult_p = checkable;                  \
    if (checkResult_p == NULL) {return checkReturn;}  \
}

#define TK_CHECK_MEM(checkable)                                         \
{                                                                         \
    void *checkResult_p = checkable;                                      \
    if (checkResult_p == NULL) {return TK_CORE_ERROR_MEMORY_ALLOCATION;} \
}

#define TK_CHECK_MEM_2(checkReturn, checkable)     \
{                                                    \
    void *checkResult_p = checkable;                 \
    if (checkResult_p == NULL) {return checkReturn;} \
}

#endif // TK_CORE_COMMON_MACROS_H
