// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// DEFINE ==========================================================================================

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

// INCLUDES ========================================================================================

#include "StandardIO.h"
#include "Draw.h"
#include "Macro.h"

#include "../Common/Macros.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Thread.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/System/Process.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/RingBuffer.h"

#include "../../../../external/Netzhaut/src/lib/nhencoding/Encodings/UTF8.h"
#include "../../../../external/Netzhaut/src/lib/nhencoding/Encodings/UTF32.h"

#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

#ifdef __unix__
//    #include <curses.h>
//    #include <term.h>
    #include <termios.h>
    #include <sys/types.h>
    #include <sys/ioctl.h>
#endif

// WINDOW SIZE =====================================================================================

static TTYR_TTY_RESULT ttyr_tty_getMaxCursorPosition(
    short unsigned int *rows, short unsigned int *cols) 
{
#ifdef __unix__

    char buf[32];
    unsigned int i = 0;
  
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
        return TTYR_TTY_ERROR_BAD_STATE;
    }
  
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';
  
    if (buf[0] != '\x1b' || buf[1] != '[') {
        return TTYR_TTY_ERROR_BAD_STATE;
    }
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
        return TTYR_TTY_ERROR_BAD_STATE;
    }
  
#endif

    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_getStandardOutputWindowSize(
    int *cols_p, int *rows_p)
{
#ifdef __unix__

    struct winsize ws;
  
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
            return TTYR_TTY_ERROR_BAD_STATE;
        }
        TTYR_CHECK(ttyr_tty_getMaxCursorPosition(&ws.ws_row, &ws.ws_col))
    } 

    *cols_p = ws.ws_col;
    *rows_p = ws.ws_row;

#endif

    return TTYR_TTY_SUCCESS;
}

// KEYS ============================================================================================

static TTYR_TTY_RESULT ttyr_tty_readLinuxStandardInput(
    NH_ENCODING_UTF32 codepoints_p[4], int *count_p)
{
#ifdef __unix__

    fd_set set;
    FD_ZERO(&set);
    FD_SET(fileno(stdin), &set);
  
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
  
    int rv = select(fileno(stdin) + 1, &set, NULL, NULL, &timeout);
    if (rv == -1 || rv == 0) {return TTYR_TTY_SUCCESS;}

    NH_BYTE p[4];
    int nread = read(STDIN_FILENO, p, 4);

    for (int i = 0, offset = 0; nread - offset > 0; ++i) {
        unsigned long count = 0;
        codepoints_p[i] = nh_encoding_decodeUTF8Single(p+offset, nread-offset, &count);
        offset += count;
        (*count_p)++;
    }

#endif

    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_readStandardInput(
    ttyr_tty_TTY *TTY_p)
{
    int count = 0;

    do {

    count = 0;
    NH_ENCODING_UTF32 codepoints_p[4];

#ifdef __unix__
    TTYR_CHECK(ttyr_tty_readLinuxStandardInput(codepoints_p, &count))
#endif

    for (int i = 0; i < count; ++i) {
        nh_wsi_Event *Event_p = nh_core_advanceRingBuffer(&TTY_p->Events);
        Event_p->type = NH_WSI_EVENT_KEYBOARD;
        Event_p->Keyboard.codepoint = codepoints_p[i];
        Event_p->Keyboard.trigger = NH_WSI_TRIGGER_PRESS;
    }

    }  while (count);

    return TTYR_TTY_SUCCESS;
}

// WRITE ===========================================================================================

TTYR_TTY_RESULT ttyr_tty_writeCursorToStandardOutput(
    int x, int y)
{
    nh_String String = nh_core_initString(255);

    NH_BYTE buf[32] = {'\0'};
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", y, x);
    nh_core_appendFormatToString(&String, buf);

#ifdef __unix__
    write(STDOUT_FILENO, String.p, String.length);
#endif

    nh_core_freeString(&String);

    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_writeToStandardOutput(
    ttyr_tty_Row *Rows_p, int cols, int rows)
{
    nh_String String = nh_core_initString(255);

    // Move cursor to home.
    nh_core_appendFormatToString(&String, "\x1b[H");

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // Reset state.
            nh_core_appendToString(&String, "\e[0m", 4);

            // Set foreground color if necessary, use TrueColor notation for that.
            if (Rows_p[row].Glyphs_p[col].Foreground.custom == NH_TRUE) {
                NH_BYTE foreground_p[64] = {0};
                sprintf(foreground_p, "\e[38;2;%d;%d;%dm", 
                    (int)(Rows_p[row].Glyphs_p[col].Foreground.Color.r*255.0f), 
                    (int)(Rows_p[row].Glyphs_p[col].Foreground.Color.g*255.0f), 
                    (int)(Rows_p[row].Glyphs_p[col].Foreground.Color.b*255.0f));
                nh_core_appendToString(&String, foreground_p, strlen(foreground_p));
            }
 
            // Set background color if necessary, use TrueColor notation for that.
            if (Rows_p[row].Glyphs_p[col].Background.custom == NH_TRUE) {
                NH_BYTE background_p[64] = {0};
                sprintf(background_p, "\e[48;2;%d;%d;%dm", 
                    (int)(Rows_p[row].Glyphs_p[col].Background.Color.r*255.0f), 
                    (int)(Rows_p[row].Glyphs_p[col].Background.Color.g*255.0f), 
                    (int)(Rows_p[row].Glyphs_p[col].Background.Color.b*255.0f));
                nh_core_appendToString(&String, background_p, strlen(background_p));
            }

            if (Rows_p[row].Glyphs_p[col].Attributes.bold) {
                nh_core_appendToString(&String, "\e[1m", 4);
            }
            if (Rows_p[row].Glyphs_p[col].Attributes.faint) {
                nh_core_appendToString(&String, "\e[2m", 4);
            }
            if (Rows_p[row].Glyphs_p[col].Attributes.italic) {
                nh_core_appendToString(&String, "\e[3m", 4);
            }
            if (Rows_p[row].Glyphs_p[col].Attributes.underline) {
                nh_core_appendToString(&String, "\e[4m", 4);
            }
            if (Rows_p[row].Glyphs_p[col].Attributes.blink) {
                nh_core_appendToString(&String, "\e[5m", 4);
            }
            if (Rows_p[row].Glyphs_p[col].Attributes.reverse) {
                nh_core_appendToString(&String, "\e[7m", 4);
            }
            if (Rows_p[row].Glyphs_p[col].Attributes.invisible) {
                nh_core_appendToString(&String, "\e[8m", 4);
            }
            if (Rows_p[row].Glyphs_p[col].Attributes.struck) {
                nh_core_appendToString(&String, "\e[9m", 4);
            }
            if (Rows_p[row].Glyphs_p[col].Attributes.wrap) {
            }
            if (Rows_p[row].Glyphs_p[col].Attributes.wide) {
            }

            // Get codepoint in UTF8 format.
            NH_BYTE codepoint_p[4] = {0};
            int length = nh_encoding_encodeUTF8Single(Rows_p[row].Glyphs_p[col].codepoint, codepoint_p);

            // If the glyph is used for line graphics, we need to wrap the codepoint up.
            if (Rows_p[row].Glyphs_p[col].mark & TTYR_TTY_MARK_LINE_GRAPHICS) {
                nh_core_appendToString(&String, "\e(0", 3);
                nh_core_appendToString(&String, codepoint_p, length);
                nh_core_appendToString(&String, "\e(B", 3);
            } else {
                nh_core_appendToString(&String, codepoint_p, length);
            }
        }
        if ((row + 1) < rows) {
            nh_core_appendFormatToString(&String, "\r\n");
        }
    }

#ifdef __unix__
    write(STDOUT_FILENO, String.p, String.length);
#endif

    nh_core_freeString(&String);

    return TTYR_TTY_SUCCESS;
}

// RAW MODE ========================================================================================
 
static TTYR_TTY_RESULT ttyr_tty_enterRawMode(
    ttyr_tty_View *View_p) 
{
#ifdef __unix__

    // https://stackoverflow.com/questions/43202800/when-exiting-terminal-rawmode-my-contents-stays-on-the-screen
    write(STDOUT_FILENO, "\033[?1049h\033[2J\033[H", 15);

    if (tcgetattr(STDIN_FILENO, &View_p->Termios) == -1) {
        return TTYR_TTY_ERROR_BAD_STATE;
    }
  
    struct termios raw = View_p->Termios;

    raw.c_iflag    &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag    &= ~(OPOST);
    raw.c_cflag    |= (CS8);
    raw.c_lflag    &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 1;
  
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return TTYR_TTY_ERROR_BAD_STATE;
    }

#endif

    return TTYR_TTY_SUCCESS;
}

static TTYR_TTY_RESULT ttyr_tty_exitRawMode(
    ttyr_tty_View *View_p) 
{
#ifdef __unix__

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &View_p->Termios) == -1) {
         return TTYR_TTY_ERROR_BAD_STATE;
    }

    // https://stackoverflow.com/questions/43202800/when-exiting-terminal-rawmode-my-contents-stays-on-the-screen
    write(STDOUT_FILENO, "\033[2J\033[H\033[?1049l", 15);

#endif

    return TTYR_TTY_SUCCESS;
}

// CLAIM STDOUT ====================================================================================

static NH_BOOL claimed = NH_FALSE;

TTYR_TTY_RESULT ttyr_tty_claimStandardIO(
    ttyr_tty_TTY *TTY_p)
{
    if (claimed) {return TTYR_TTY_ERROR_BAD_STATE;}

    ttyr_tty_View *View_p = ttyr_tty_createView(TTY_p, NULL, NH_TRUE);
    TTYR_CHECK_NULL(View_p)

    TTYR_TTY_RESULT error = ttyr_tty_enterRawMode(View_p);
    if (error) {
        ttyr_tty_destroyView(TTY_p, View_p);
        return error;
    }

    claimed = NH_TRUE;

    return TTYR_TTY_SUCCESS;
}

TTYR_TTY_RESULT ttyr_tty_unclaimStandardIO(
    ttyr_tty_TTY *TTY_p)
{
    if (!claimed) {return TTYR_TTY_ERROR_BAD_STATE;}

    ttyr_tty_View *View_p = NULL;
    for (int i = 0; i < TTY_p->Views.size; ++i) {
        View_p = TTY_p->Views.pp[i];
        if (View_p->standardIO) {break;}
        View_p = NULL;
    }

    TTYR_CHECK_NULL(View_p)

    TTYR_CHECK(ttyr_tty_exitRawMode(View_p))
    ttyr_tty_destroyView(TTY_p, View_p);

    claimed = NH_FALSE;

    return TTYR_TTY_SUCCESS;
}

NH_BOOL ttyr_tty_claimsStandardIO(
    ttyr_tty_TTY *TTY_p)
{
    for (int i = 0; i < TTY_p->Views.size; ++i) {
        if (((ttyr_tty_View*)TTY_p->Views.pp[i])->standardIO) {return NH_TRUE;}
    }
    return NH_FALSE;
}

