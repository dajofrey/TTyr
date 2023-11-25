#ifndef TTYR_TERMINAL_COMMON_MACROS_H
#define TTYR_TERMINAL_COMMON_MACROS_H

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

// FLOW LOGGING ====================================================================================

#ifdef NH_LOG_FLOW
    #define TTYR_TERMINAL_BEGIN() {_ttyr_terminal_logBegin(__FILE__, __func__);}
#else
    #define TTYR_TERMINAL_BEGIN() 
#endif

#ifdef NH_LOG_FLOW
    #define TTYR_TERMINAL_END(result) {_ttyr_terminal_logEnd(__FILE__, __func__); return result;}
#else
    #define TTYR_TERMINAL_END(result) {return result;} 
#endif

#ifdef NH_LOG_FLOW
    #define TTYR_TERMINAL_SILENT_END() {_ttyr_terminal_logEnd(__FILE__, __func__); return;}
#else
    #define TTYR_TERMINAL_SILENT_END() {return;} 
#endif

#ifdef NH_LOG_FLOW
    #define TTYR_TERMINAL_DIAGNOSTIC_END(result) {return _ttyr_terminal_logDiagnosticEnd(__FILE__, __func__, result, __LINE__);}
#else
    #define TTYR_TERMINAL_DIAGNOSTIC_END(result) {return result;} 
#endif

// ERROR LOGGING ===================================================================================

#define TTYR_TERMINAL_CHECK(checkable)                                              \
{                                                                             \
    TTYR_TERMINAL_RESULT checkResult = checkable;                                   \
    if (checkResult != TTYR_TERMINAL_SUCCESS) {TTYR_TERMINAL_DIAGNOSTIC_END(checkResult)} \
}

#define TTYR_TERMINAL_CHECK_2(checkReturn, checkable)                    \
{                                                                  \
    TTYR_TERMINAL_RESULT checkResult = checkable;                        \
    if (checkResult != TTYR_TERMINAL_SUCCESS) {TTYR_TERMINAL_END(checkReturn)} \
}

#define TTYR_TERMINAL_CHECK_NULL(checkable)                                               \
{                                                                                   \
    void *checkResult_p = checkable;                                                \
    if (checkResult_p == NULL) {TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_ERROR_NULL_POINTER)} \
}

#define TTYR_TERMINAL_CHECK_NULL_2(checkReturn, checkable)      \
{                                                         \
    void *checkResult_p = checkable;                      \
    if (checkResult_p == NULL) {TTYR_TERMINAL_END(checkReturn)} \
}

#define TTYR_TERMINAL_CHECK_MEM(checkable)                                                     \
{                                                                                        \
    void *checkResult_p = checkable;                                                     \
    if (checkResult_p == NULL) {TTYR_TERMINAL_DIAGNOSTIC_END(TTYR_TERMINAL_ERROR_MEMORY_ALLOCATION)} \
}

#define TTYR_TERMINAL_CHECK_MEM_2(checkReturn, checkable)       \
{                                                         \
    void *checkResult_p = checkable;                      \
    if (checkResult_p == NULL) {TTYR_TERMINAL_END(checkReturn)} \
}

#endif // TTYR_TERMINAL_COMMON_MACROS_H
