#ifndef TTYR_CORE_API_H
#define TTYR_CORE_API_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "nh-api/nh-api.h"
#include <stdbool.h>

#endif

/** @defgroup api_nhtty nhtty
 *  @brief Environment for running text based programs.
 *
 *  In this API, TTY stands for TeleType and provides an interface for
 *  writing text based programs. Using these programs, it processes input streams from either
 *  standard input or \ref ttyr_core_sendInput and sends the result to either standard
 *  output or a \ref ttyr_terminal_Terminal. Text based programs can be either added
 *  by using \ref ttyr_core_addDefaultProgram or \ref ttyr_core_addCustomProgram.
 */

/** @addtogroup api_nhtty
 *  @{
 */

// ENUMS ===========================================================================================

    /**
     * Return values for functions.
     */
    typedef enum TTYR_CORE_RESULT {
        TTYR_CORE_SUCCESS, /**<Indicates that something worked as planned.*/ 
        TTYR_CORE_ERROR_NULL_POINTER,         
        TTYR_CORE_ERROR_BAD_STATE, 
        TTYR_CORE_ERROR_MEMORY_ALLOCATION,
        TTYR_CORE_ERROR_TERMINFO_DATA_CANNOT_BE_FOUND,
        TTYR_CORE_ERROR_UNKNOWN_TERMINAL_TYPE,
        TTYR_CORE_ERROR_TERMINAL_IS_HARDCOPY,
        TTYR_CORE_ERROR_UNKNOWN_COMMAND,
        TTYR_CORE_ERROR_INVALID_ARGUMENT,
    } TTYR_CORE_RESULT;

    typedef enum TTYR_CORE_PROGRAM_E {
        TTYR_CORE_PROGRAM_SHELL,
        TTYR_CORE_PROGRAM_LOGGER,
        TTYR_CORE_PROGRAM_EDITOR,
        TTYR_CORE_PROGRAM_TAGGER,
    } TTYR_CORE_PROGRAM_E;

    typedef enum TTYR_CORE_MARK_E {
        TTYR_CORE_MARK_LINE_VERTICAL   = 1 << 0, // Internal use.
        TTYR_CORE_MARK_LINE_HORIZONTAL = 1 << 1, // Internal use.
        TTYR_CORE_MARK_LINE_GRAPHICS   = 1 << 2,
        TTYR_CORE_MARK_ELEVATED        = 1 << 3,
        TTYR_CORE_MARK_ACCENT          = 1 << 4,
    } TTYR_CORE_MARK_E;

// TYPEDEFS ========================================================================================

    typedef struct ttyr_core_TTY ttyr_core_TTY;
    typedef struct ttyr_core_Program ttyr_core_Program;
    typedef struct ttyr_core_Interface ttyr_core_Interface;
    typedef struct ttyr_core_Glyph ttyr_core_Glyph;
    
    typedef void *(*ttyr_core_init_f)(void *arg_p);
    typedef TTYR_CORE_RESULT (*ttyr_core_draw_f)(ttyr_core_Program *Program_p, ttyr_core_Glyph *Glyphs_p, int width, int height, int row);
    typedef TTYR_CORE_RESULT (*ttyr_core_drawTopbar_f)(ttyr_core_Program *Program_p, ttyr_core_Glyph *Glyphs_p, int width);
    typedef TTYR_CORE_RESULT (*ttyr_core_getTitle_f)(ttyr_core_Program *Program_p, NH_API_UTF32 *title_p, int length);
    typedef TTYR_CORE_RESULT (*ttyr_core_getCursorPosition_f)(ttyr_core_Program *Program_p, int *x_p, int *y_p);
    typedef TTYR_CORE_RESULT (*ttyr_core_handleInput_f)(ttyr_core_Program *Program_p, nh_api_WSIEvent Event);
    typedef TTYR_CORE_RESULT (*ttyr_core_update_f)(ttyr_core_Program *Program_p);
    typedef TTYR_CORE_RESULT (*ttyr_core_handleCommand_f)(ttyr_core_Program *Program_p);
    typedef void (*ttyr_core_destroy_f)(void *p);
    typedef void (*ttyr_core_destroyPrototype_f)(ttyr_core_Interface *Prototype_p);

// STRUCTS =========================================================================================

    typedef struct ttyr_core_GlyphAttributes {
        bool bold;
        bool faint;
        bool italic;
        bool underline;
        bool blink;
        bool reverse;
        bool invisible;
        bool struck;
        bool wrap;
        bool wide;
    } ttyr_core_GlyphAttributes;
    
    typedef struct ttyr_core_Color { 
        float r, g, b, a; 
    } ttyr_core_Color; 

    typedef struct ttyr_core_GlyphColor {
        ttyr_core_Color Color;
        bool custom;
    } ttyr_core_GlyphColor;

    typedef struct ttyr_core_Glyph {
        ttyr_core_GlyphAttributes Attributes;
        ttyr_core_GlyphColor Foreground;
        ttyr_core_GlyphColor Background;
        NH_API_UTF32 codepoint;       /* character code */
        TTYR_CORE_MARK_E mark;
    } ttyr_core_Glyph;
    
    typedef struct ttyr_core_Row {
        ttyr_core_Glyph *Glyphs_p;
        bool *update_p;
    } ttyr_core_Row;

    typedef struct ttyr_core_InterfaceCallbacks {
        ttyr_core_init_f init_f;
        ttyr_core_draw_f draw_f;
        ttyr_core_drawTopbar_f drawTopbar_f;
        ttyr_core_getTitle_f getTitle_f;
        ttyr_core_handleInput_f handleInput_f;
        ttyr_core_getCursorPosition_f getCursorPosition_f;
        ttyr_core_update_f update_f;
        ttyr_core_handleCommand_f handleCommand_f;
        ttyr_core_destroy_f destroy_f;
        ttyr_core_destroyPrototype_f destroyPrototype_f;
    } ttyr_core_InterfaceCallbacks;
    
    typedef struct ttyr_core_Interface {
        NH_API_UTF32 name_p[64];
        NH_API_UTF32 **commands_pp;
        unsigned int commands;
        ttyr_core_InterfaceCallbacks Callbacks;
        void *initArg_p;
    } ttyr_core_Interface;

    typedef struct ttyr_core_Program {
        ttyr_core_Interface *Prototype_p;
        void *handle_p;
        bool refresh;
        bool close;
        bool once;
        int command;
    } ttyr_core_Program;

// FUNCTIONS =======================================================================================

    /**
     * @brief Get a new TTY handle. 
     * 
     * You can check for user-closure with @ref ttyr_core_getWorkload. If @ref ttyr_core_getWorkload returns NULL, 
     * the user probably closed the TTY or there was an error. You can force-close the TTY with 
     * @ref ttyr_core_closeTTY.
     *
     * @param Interface_p Pointer to an interface or NULL. If NULL, shell interface will be used.
     *
     * @return Pointer to a new TTY handle. NULL on failure.
     */
    ttyr_core_TTY *ttyr_api_openTTY(
        char *config_p, ttyr_core_Interface *Interface_p
    );

    /**
     * @brief Close the specified TTY.
     *
     * Only call this if you want to force-close the TTY. There is no need to call this if you expect
     * the user to close the TTY since this case is handled internally.
     *
     * @param Pointer to TTY. Must not be NULL.
     *
     * @return TTYR_CORE_SUCCESS on success.
     */
    TTYR_CORE_RESULT ttyr_api_closeTTY(
        ttyr_core_TTY *TTY_p
    );

    /**
     * Todo.
     */
    TTYR_CORE_RESULT ttyr_api_claimStandardIO(
        ttyr_core_TTY *TTY_p
    );
    
    /**
     * Todo.
     */
    TTYR_CORE_RESULT ttyr_api_unclaimStandardIO(
        ttyr_core_TTY *TTY_p
    );

    /**
     * Todo.
     */
    TTYR_CORE_RESULT ttyr_api_sendEvent(
        ttyr_core_TTY *TTY_p, nh_api_WSIEvent Event
    );

/** @} */

#endif // TTYR_CORE_API_H
