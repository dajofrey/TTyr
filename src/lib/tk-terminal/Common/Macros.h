#ifndef TK_TERMINAL_COMMON_MACROS_H
#define TK_TERMINAL_COMMON_MACROS_H

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

#define TK_TERMINAL_CHECK(checkable)                                              \
{                                                                             \
    TK_TERMINAL_RESULT checkResult = checkable;                                   \
    if (checkResult != TK_TERMINAL_SUCCESS) {return checkResult;} \
}

#define TK_TERMINAL_CHECK_2(checkReturn, checkable)                    \
{                                                                  \
    TK_TERMINAL_RESULT checkResult = checkable;                        \
    if (checkResult != TK_TERMINAL_SUCCESS) {return checkReturn;} \
}

#define TK_TERMINAL_CHECK_NULL(checkable)                                               \
{                                                                                   \
    void *checkResult_p = checkable;                                                \
    if (checkResult_p == NULL) {return TK_TERMINAL_ERROR_NULL_POINTER;} \
}

#define TK_TERMINAL_CHECK_NULL_2(checkReturn, checkable)      \
{                                                         \
    void *checkResult_p = checkable;                      \
    if (checkResult_p == NULL) {return checkReturn;} \
}

#define TK_TERMINAL_CHECK_MEM(checkable)                                                     \
{                                                                                        \
    void *checkResult_p = checkable;                                                     \
    if (checkResult_p == NULL) {return TK_TERMINAL_ERROR_MEMORY_ALLOCATION;} \
}

#define TK_TERMINAL_CHECK_MEM_2(checkReturn, checkable)       \
{                                                         \
    void *checkResult_p = checkable;                      \
    if (checkResult_p == NULL) {return checkReturn;} \
}

#endif // TK_TERMINAL_COMMON_MACROS_H
