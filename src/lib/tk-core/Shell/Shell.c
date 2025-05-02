// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Shell.h"
#include "Socket.h"

#include "../Common/Macros.h"
#include "../Common/Config.h"
#include "../TTY/Program.h"

#include "nh-core/Util/List.h"
#include "nh-core/Util/String.h"
#include "nh-core/System/Memory.h"
#include "nh-core/Loader/Loader.h"
#include "nh-encoding/Encodings/UTF8.h"
#include "nh-encoding/Encodings/UTF32.h"
#include "nh-wsi/Window/Listener.h"

#include "../../../../external/st-0.8.5/st.h"
#include "../../../../external/st-0.8.5/config.h"
#include "../../../../external/st-0.8.5/win.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

// DECLARE =========================================================================================

#define PASTE_KEY (Event.Keyboard.codepoint == 'V' && (Event.Keyboard.state & NH_API_MODIFIER_SHIFT) && (Event.Keyboard.state & NH_API_MODIFIER_CONTROL))
#define COPY_KEY (Event.Keyboard.codepoint == 'C' && (Event.Keyboard.state & NH_API_MODIFIER_SHIFT) && (Event.Keyboard.state & NH_API_MODIFIER_CONTROL))

typedef struct tk_core_ShellKey{
    NH_API_KEY_E key;
    char *s;
    /* three-valued logic variables: 0 indifferent, 1 on, -1 off */
    signed char appkey;    /* application keypad */
    signed char appcursor; /* application cursor */
    uint mask;
} tk_core_ShellKey;

/*
 * This is the huge key array which defines all compatibility to the Linux
 * world. Please decide about changes wisely.
 */
static tk_core_ShellKey KEYS_P[] = {
	/* keysym                   mask            string      appkey appcursor */
	{NH_API_KEY_KP_HOME,       "\033[2J",       0,   -1, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_KP_HOME,       "\033[1;2H",     0,   +1, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_KP_HOME,       "\033[H",        0,   -1, -1},
	{NH_API_KEY_KP_HOME,       "\033[1~",       0,   +1, -1},
	{NH_API_KEY_KP_UP,         "\033Ox",       +1,    0, -1},
	{NH_API_KEY_KP_UP,         "\033[A",        0,   -1, -1},
	{NH_API_KEY_KP_UP,         "\033OA",        0,   +1, -1},
	{NH_API_KEY_KP_DOWN,       "\033Or",       +1,    0, -1},
	{NH_API_KEY_KP_DOWN,       "\033[B",        0,   -1, -1},
	{NH_API_KEY_KP_DOWN,       "\033OB",        0,   +1, -1},
	{NH_API_KEY_KP_LEFT,       "\033Ot",       +1,    0, -1},
	{NH_API_KEY_KP_LEFT,       "\033[D",        0,   -1, -1},
	{NH_API_KEY_KP_LEFT,       "\033OD",        0,   +1, -1},
	{NH_API_KEY_KP_RIGHT,      "\033Ov",       +1,    0, -1},
	{NH_API_KEY_KP_RIGHT,      "\033[C",        0,   -1, -1},
	{NH_API_KEY_KP_RIGHT,      "\033OC",        0,   +1, -1},
	{NH_API_KEY_KP_PRIOR,      "\033[5;2~",     0,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_KP_PRIOR,      "\033[5~",       0,    0, -1},
	{NH_API_KEY_KP_BEGIN,      "\033[E",        0,    0, -1},
	{NH_API_KEY_KP_END,        "\033[J",       -1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_KP_END,        "\033[1;5F",    +1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_KP_END,        "\033[K",       -1,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_KP_END,        "\033[1;2F",    +1,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_KP_END,        "\033[4~",       0,    0, -1},
	{NH_API_KEY_KP_NEXT,       "\033[6;2~",     0,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_KP_NEXT,       "\033[6~",       0,    0, -1},
	{NH_API_KEY_KP_INSERT,     "\033[2;2~",    +1,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_KP_INSERT,     "\033[4l",      -1,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_KP_INSERT,     "\033[L",       -1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_KP_INSERT,     "\033[2;5~",    +1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_KP_INSERT,     "\033[4h",      -1,    0, -1},
	{NH_API_KEY_KP_INSERT,     "\033[2~",      +1,    0, -1},
	{NH_API_KEY_KP_DELETE,     "\033[M",       -1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_KP_DELETE,     "\033[3;5~",    +1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_KP_DELETE,     "\033[2K",      -1,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_KP_DELETE,     "\033[3;2~",    +1,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_KP_DELETE,     "\033[P",       -1,    0, -1},
	{NH_API_KEY_KP_DELETE,     "\033[3~",      +1,    0, -1},
	{NH_API_KEY_KP_MULTIPLY,   "\033Oj",       +2,    0, -1},
	{NH_API_KEY_KP_ADD,        "\033Ok",       +2,    0, -1},
	{NH_API_KEY_KP_ENTER,      "\033OM",       +2,    0, -1},
	{NH_API_KEY_KP_ENTER,      "\r",           -1,    0, -1},
	{NH_API_KEY_KP_SUBTRACT,   "\033Om",       +2,    0, -1},
	{NH_API_KEY_KP_DECIMAL,    "\033On",       +2,    0, -1},
	{NH_API_KEY_KP_DIVIDE,     "\033Oo",       +2,    0, -1},
	{NH_API_KEY_KP_0,          "\033Op",       +2,    0, -1},
	{NH_API_KEY_KP_1,          "\033Oq",       +2,    0, -1},
	{NH_API_KEY_KP_2,          "\033Or",       +2,    0, -1},
	{NH_API_KEY_KP_3,          "\033Os",       +2,    0, -1},
	{NH_API_KEY_KP_4,          "\033Ot",       +2,    0, -1},
	{NH_API_KEY_KP_5,          "\033Ou",       +2,    0, -1},
	{NH_API_KEY_KP_6,          "\033Ov",       +2,    0, -1},
	{NH_API_KEY_KP_7,          "\033Ow",       +2,    0, -1},
	{NH_API_KEY_KP_8,          "\033Ox",       +2,    0, -1},
	{NH_API_KEY_KP_9,          "\033Oy",       +2,    0, -1},
	{NH_API_KEY_UP,            "\033[1;2A",     0,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_UP,            "\033[1;3A",     0,    0, NH_API_MODIFIER_MOD1}, 
	{NH_API_KEY_UP,            "\033[1;4A",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_UP,            "\033[1;5A",     0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_UP,            "\033[1;6A",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_UP,            "\033[1;7A",     0,    0, NH_API_MODIFIER_CONTROL|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_UP,            "\033[1;8A",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_CONTROL|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_UP,            "\033[A",        0,   -1, -1},
	{NH_API_KEY_UP,            "\033OA",        0,   +1, -1},
	{NH_API_KEY_DOWN,          "\033[1;2B",     0,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_DOWN,          "\033[1;3B",     0,    0, NH_API_MODIFIER_MOD1},
	{NH_API_KEY_DOWN,          "\033[1;4B",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_DOWN,          "\033[1;5B",     0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_DOWN,          "\033[1;6B",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_DOWN,          "\033[1;7B",     0,    0, NH_API_MODIFIER_CONTROL|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_DOWN,          "\033[1;8B",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_CONTROL|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_DOWN,          "\033[B",        0,   -1, -1},
	{NH_API_KEY_DOWN,          "\033OB",        0,   +1, -1},
	{NH_API_KEY_LEFT,          "\033[1;2D",     0,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_LEFT,          "\033[1;3D",     0,    0, NH_API_MODIFIER_MOD1},
	{NH_API_KEY_LEFT,          "\033[1;4D",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_LEFT,          "\033[1;5D",     0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_LEFT,          "\033[1;6D",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_LEFT,          "\033[1;7D",     0,    0, NH_API_MODIFIER_CONTROL|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_LEFT,          "\033[1;8D",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_CONTROL|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_LEFT,          "\033[D",        0,   -1, -1},
	{NH_API_KEY_LEFT,          "\033OD",        0,   +1, -1},
	{NH_API_KEY_RIGHT,         "\033[1;2C",     0,    0, NH_API_MODIFIER_SHIFT},
	{NH_API_KEY_RIGHT,         "\033[1;3C",     0,    0, NH_API_MODIFIER_MOD1},
	{NH_API_KEY_RIGHT,         "\033[1;4C",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_RIGHT,         "\033[1;5C",     0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_RIGHT,         "\033[1;6C",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_RIGHT,         "\033[1;7C",     0,    0, NH_API_MODIFIER_CONTROL|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_RIGHT,         "\033[1;8C",     0,    0, NH_API_MODIFIER_SHIFT|NH_API_MODIFIER_CONTROL|NH_API_MODIFIER_MOD1},
	{NH_API_KEY_RIGHT,         "\033[C",        0,   -1, -1},                     
	{NH_API_KEY_RIGHT,         "\033OC",        0,   +1, -1},                     
//	{NH_API_KEY_ISO_LEFT_TAB,  "\033[Z",        0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_RETURN,        "\033\r",        0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_RETURN,        "\r",            0,    0, -1},                     
	{NH_API_KEY_INSERT,        "\033[4l",      -1,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_INSERT,        "\033[2;2~",    +1,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_INSERT,        "\033[L",       -1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_INSERT,        "\033[2;5~",    +1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_INSERT,        "\033[4h",      -1,    0, -1},                     
	{NH_API_KEY_INSERT,        "\033[2~",      +1,    0, -1},                     
	{NH_API_KEY_DELETE,        "\033[M",       -1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_DELETE,        "\033[3;5~",    +1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_DELETE,        "\033[2K",      -1,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_DELETE,        "\033[3;2~",    +1,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_DELETE,        "\033[P",       -1,    0, -1},                     
	{NH_API_KEY_DELETE,        "\033[3~",      +1,    0, -1},                     
	{NH_API_KEY_BACKSPACE,     "\177",          0,    0, 0},                      
	{NH_API_KEY_BACKSPACE,     "\033\177",      0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_HOME,          "\033[2J",       0,   -1, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_HOME,          "\033[1;2H",     0,   +1, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_HOME,          "\033[H",        0,   -1, -1},                     
	{NH_API_KEY_HOME,          "\033[1~",       0,   +1, -1},                     
	{NH_API_KEY_END,           "\033[J",       -1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_END,           "\033[1;5F",    +1,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_END,           "\033[K",       -1,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_END,           "\033[1;2F",    +1,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_END,           "\033[4~",       0,    0, -1},                     
	{NH_API_KEY_PRIOR,         "\033[5;5~",     0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_PRIOR,         "\033[5;2~",     0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_PRIOR,         "\033[5~",       0,    0, -1},                     
	{NH_API_KEY_NEXT,          "\033[6;5~",     0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_NEXT,          "\033[6;2~",     0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_NEXT,          "\033[6~",       0,    0, -1},                     
	{NH_API_KEY_F1,            "\033OP" ,       0,    0, 0},                      
	{NH_API_KEY_F1, /* F13 */  "\033[1;2P",     0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F1, /* F25 */  "\033[1;5P",     0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F1, /* F37 */  "\033[1;6P",     0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F1, /* F49 */  "\033[1;3P",     0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F1, /* F61 */  "\033[1;4P",     0,    0, NH_API_MODIFIER_MOD3},   
	{NH_API_KEY_F2,            "\033OQ" ,       0,    0, 0},                      
	{NH_API_KEY_F2, /* F14 */  "\033[1;2Q",     0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F2, /* F26 */  "\033[1;5Q",     0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F2, /* F38 */  "\033[1;6Q",     0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F2, /* F50 */  "\033[1;3Q",     0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F2, /* F62 */  "\033[1;4Q",     0,    0, NH_API_MODIFIER_MOD3},   
	{NH_API_KEY_F3,            "\033OR" ,       0,    0, 0},                      
	{NH_API_KEY_F3, /* F15 */  "\033[1;2R",     0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F3, /* F27 */  "\033[1;5R",     0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F3, /* F39 */  "\033[1;6R",     0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F3, /* F51 */  "\033[1;3R",     0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F3, /* F63 */  "\033[1;4R",     0,    0, NH_API_MODIFIER_MOD3},   
	{NH_API_KEY_F4,            "\033OS" ,       0,    0, 0},                      
	{NH_API_KEY_F4, /* F16 */  "\033[1;2S",     0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F4, /* F28 */  "\033[1;5S",     0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F4, /* F40 */  "\033[1;6S",     0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F4, /* F52 */  "\033[1;3S",     0,    0, NH_API_MODIFIER_MOD1},
	{NH_API_KEY_F5,            "\033[15~",      0,    0, 0},                      
	{NH_API_KEY_F5, /* F17 */  "\033[15;2~",    0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F5, /* F29 */  "\033[15;5~",    0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F5, /* F41 */  "\033[15;6~",    0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F5, /* F53 */  "\033[15;3~",    0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F6,            "\033[17~",      0,    0, 0},                      
	{NH_API_KEY_F6, /* F18 */  "\033[17;2~",    0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F6, /* F30 */  "\033[17;5~",    0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F6, /* F42 */  "\033[17;6~",    0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F6, /* F54 */  "\033[17;3~",    0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F7,            "\033[18~",      0,    0, 0},                      
	{NH_API_KEY_F7, /* F19 */  "\033[18;2~",    0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F7, /* F31 */  "\033[18;5~",    0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F7, /* F43 */  "\033[18;6~",    0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F7, /* F55 */  "\033[18;3~",    0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F8,            "\033[19~",      0,    0, 0},                      
	{NH_API_KEY_F8, /* F20 */  "\033[19;2~",    0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F8, /* F32 */  "\033[19;5~",    0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F8, /* F44 */  "\033[19;6~",    0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F8, /* F56 */  "\033[19;3~",    0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F9,            "\033[20~",      0,    0, 0},                      
	{NH_API_KEY_F9, /* F21 */  "\033[20;2~",    0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F9, /* F33 */  "\033[20;5~",    0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F9, /* F45 */  "\033[20;6~",    0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F9, /* F57 */  "\033[20;3~",    0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F10,           "\033[21~",      0,    0, 0},                      
	{NH_API_KEY_F10, /* F22 */ "\033[21;2~",    0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F10, /* F34 */ "\033[21;5~",    0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F10, /* F46 */ "\033[21;6~",    0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F10, /* F58 */ "\033[21;3~",    0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F11,           "\033[23~",      0,    0, 0},                      
	{NH_API_KEY_F11, /* F23 */ "\033[23;2~",    0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F11, /* F35 */ "\033[23;5~",    0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F11, /* F47 */ "\033[23;6~",    0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F11, /* F59 */ "\033[23;3~",    0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F12,           "\033[24~",      0,    0, 0},                      
	{NH_API_KEY_F12, /* F24 */ "\033[24;2~",    0,    0, NH_API_MODIFIER_SHIFT},  
	{NH_API_KEY_F12, /* F36 */ "\033[24;5~",    0,    0, NH_API_MODIFIER_CONTROL},
	{NH_API_KEY_F12, /* F48 */ "\033[24;6~",    0,    0, NH_API_MODIFIER_MOD4},   
	{NH_API_KEY_F12, /* F60 */ "\033[24;3~",    0,    0, NH_API_MODIFIER_MOD1},   
	{NH_API_KEY_F13,           "\033[1;2P",     0,    0, 0},                      
	{NH_API_KEY_F14,           "\033[1;2Q",     0,    0, 0},                      
	{NH_API_KEY_F15,           "\033[1;2R",     0,    0, 0},                      
	{NH_API_KEY_F16,           "\033[1;2S",     0,    0, 0},                      
	{NH_API_KEY_F17,           "\033[15;2~",    0,    0, 0},                      
	{NH_API_KEY_F18,           "\033[17;2~",    0,    0, 0},                      
	{NH_API_KEY_F19,           "\033[18;2~",    0,    0, 0},                      
	{NH_API_KEY_F20,           "\033[19;2~",    0,    0, 0},                      
	{NH_API_KEY_F21,           "\033[20;2~",    0,    0, 0},                      
	{NH_API_KEY_F22,           "\033[21;2~",    0,    0, 0},                      
	{NH_API_KEY_F23,           "\033[23;2~",    0,    0, 0},                      
	{NH_API_KEY_F24,           "\033[24;2~",    0,    0, 0},                      
	{NH_API_KEY_F25,           "\033[1;5P",     0,    0, 0},                      
	{NH_API_KEY_F26,           "\033[1;5Q",     0,    0, 0},                      
	{NH_API_KEY_F27,           "\033[1;5R",     0,    0, 0},                      
	{NH_API_KEY_F28,           "\033[1;5S",     0,    0, 0},                      
	{NH_API_KEY_F29,           "\033[15;5~",    0,    0, 0},                      
	{NH_API_KEY_F30,           "\033[17;5~",    0,    0, 0},                      
	{NH_API_KEY_F31,           "\033[18;5~",    0,    0, 0},                      
	{NH_API_KEY_F32,           "\033[19;5~",    0,    0, 0},                      
	{NH_API_KEY_F33,           "\033[20;5~",    0,    0, 0},                      
	{NH_API_KEY_F34,           "\033[21;5~",    0,    0, 0},                      
	{NH_API_KEY_F35,           "\033[23;5~",    0,    0, 0},                      
};

typedef struct tk_core_ShellSelection {
    nh_api_PixelPosition Start;
    int startScroll;
    nh_api_PixelPosition Stop;
    int stopScroll;
    int lines;
    NH_API_UTF32 **buffer_pp;
    bool active;
    bool draw;
    bool doubleClick;
    bool trippleClick;
    bool moved;
} tk_core_ShellSelection;

typedef struct tk_core_ShellScroll {
    nh_api_PixelPosition Position;
    nh_api_PixelPosition Current;
    bool active;
    nh_core_SystemTime LastScroll;
} tk_core_ShellScroll;

typedef struct tk_core_Shell {
    tk_core_ShellSocket Socket;
    tk_core_ShellSelection Selection;
    tk_core_ShellScroll Scroll;
    ST *ST_p;
    tk_core_Row *Rows_p;
    int ttyfd;
    fd_set rfd;
    struct timespec now, lastblink, trigger;
    double timeout;
    int drawing;
    int width;
    int height;
    int scroll;
    nh_api_KeyboardEvent LastEvent;
    nh_core_SystemTime LastClick;
} tk_core_Shell;

#define SHELL_PATH "/bin/sh"

// COLOR HANDLING ==================================================================================

#define TRUERED(x)   (((x) & 0xff0000) >> 8)
#define TRUEGREEN(x) (((x) & 0xff00))
#define TRUEBLUE(x)  (((x) & 0xff) << 8)

/* Terminal colors (16 first used in escape sequence) */
static const unsigned int FOURBIT_COLORS_PP[16][3] = {
        /* 8 normal colors */
        {1, 1, 1},       // black
        {222, 56, 43},   // red
        {57, 181, 74},   // green
        {255, 199, 6},   // yellow
        {0, 111, 184},   // blue
        {118, 38, 113},  // magenta
        {44, 181, 233},  // cyan
        {204, 204, 204}, // white
        /* 8 bright colors */
        {128, 128, 128}, // bright black
        {255, 0 , 0},    // bright red
        {0, 255, 0},     // bright green
        {255, 255, 0},   // bright yellow
        {0, 0, 255},     // bright blue
        {255, 0, 255},   // bright magenta
        {0, 255, 255},   // bright cyan
        {255, 255, 255}, // bright white
};

// https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences
static void tk_core_getShellColor(
    uint32_t color, tk_core_Color *Color_p)
{
    if (IS_TRUECOL(color)) {
        Color_p->r = (float)TRUERED(color)/255;
        Color_p->g = (float)TRUEGREEN(color)/255;
        Color_p->b = (float)TRUEBLUE(color)/255;
        Color_p->a = (float)1.0f;
    } else {
        Color_p->r = (float)FOURBIT_COLORS_PP[color][0]/255;
        Color_p->g = (float)FOURBIT_COLORS_PP[color][1]/255;
        Color_p->b = (float)FOURBIT_COLORS_PP[color][2]/255;
        Color_p->a = (float)1.0f;
    }
}

// COPY ============================================================================================

static void tk_core_copyShellRow(
    tk_core_Shell *Shell_p, Line line, int cols, tk_core_Row *Row_p)
{
    // Set range of glyphs in row y.
    for (int i = 0; i < cols && i < Shell_p->ST_p->col; ++i) 
    {
        // Copy attributes.
        Row_p->Glyphs_p[i].Attributes.bold      = line[i].mode & ATTR_BOLD;
        Row_p->Glyphs_p[i].Attributes.faint     = line[i].mode & ATTR_FAINT;
        Row_p->Glyphs_p[i].Attributes.italic    = line[i].mode & ATTR_ITALIC;
        Row_p->Glyphs_p[i].Attributes.underline = line[i].mode & ATTR_UNDERLINE;
        Row_p->Glyphs_p[i].Attributes.blink     = line[i].mode & ATTR_BLINK;
        Row_p->Glyphs_p[i].Attributes.reverse   = line[i].mode & ATTR_REVERSE;
        Row_p->Glyphs_p[i].Attributes.invisible = line[i].mode & ATTR_INVISIBLE;
        Row_p->Glyphs_p[i].Attributes.struck    = line[i].mode & ATTR_STRUCK;
        Row_p->Glyphs_p[i].Attributes.wrap      = line[i].mode & ATTR_WRAP;
        Row_p->Glyphs_p[i].Attributes.wide      = line[i].mode & ATTR_WIDE;

        // Copy codepoint.
        Row_p->Glyphs_p[i].codepoint = line[i].u;

        // Copy color.
        if (line[i].fg != defaultfg) {
            Row_p->Glyphs_p[i].Foreground.custom = true;
            tk_core_getShellColor(line[i].fg, &Row_p->Glyphs_p[i].Foreground.Color);
        } else {
            Row_p->Glyphs_p[i].Foreground.custom = false;
        }

        if (line[i].bg != defaultbg) {
            Row_p->Glyphs_p[i].Background.custom = true;
            tk_core_getShellColor(line[i].bg, &Row_p->Glyphs_p[i].Background.Color);
        } else {
            Row_p->Glyphs_p[i].Background.custom = false;
        }
    }
}

// UPDATE ==========================================================================================

static void tk_core_drawShell(
    tk_core_Shell *Shell_p)
{
    ST *ST_p = Shell_p->ST_p;

    int cx = ST_p->c.x, ocx = ST_p->ocx, ocy = ST_p->ocy;

    /* adjust cursor position */
    LIMIT(ST_p->ocx, 0, ST_p->col-1);
    LIMIT(ST_p->ocy, 0, ST_p->row-1);

    if (ST_p->line[ST_p->ocy][ST_p->ocx].mode & ATTR_WDUMMY)
        ST_p->ocx--;
    if (ST_p->line[ST_p->c.y][cx].mode & ATTR_WDUMMY)
        cx--;

    int historyLength = Shell_p->scroll >= ST_p->row ? ST_p->row : Shell_p->scroll;
    for (int row = 0, copyRow = ST_p->scrollup-Shell_p->scroll; historyLength && row < historyLength; row++, copyRow++) {
        tk_core_copyShellRow(Shell_p, ST_p->history[copyRow], ST_p->historyCols[copyRow], Shell_p->Rows_p+row);
    }
    for (int y = 0; y < (ST_p->row-historyLength); y++) {
        tk_core_copyShellRow(Shell_p, ST_p->line[y], ST_p->col, Shell_p->Rows_p+y+historyLength);
    }

    ST_p->ocx = cx;
    ST_p->ocy = ST_p->c.y;
}

static TK_CORE_RESULT tk_core_handleFastScroll(
    tk_core_Shell *Shell_p)
{
    nh_core_SystemTime Now = nh_core_getSystemTime();
    if (nh_core_getSystemTimeDiffInSeconds(Shell_p->Scroll.LastScroll, Now) > 0.05f) {
        return TK_CORE_SUCCESS;
    }

    int diff = Shell_p->Scroll.Current.y - Shell_p->Scroll.Position.y;
    if (diff < 0) {
        if ((Shell_p->scroll + abs(diff)) < Shell_p->ST_p->scrollup) {
            Shell_p->scroll += abs(diff);
        } else if (Shell_p->scroll < Shell_p->ST_p->scrollup) {
            Shell_p->scroll = Shell_p->ST_p->scrollup;
        }
   
    } else if (diff > 0) {
        if ((Shell_p->scroll - diff) > 0) {
            Shell_p->scroll -= diff;
        } else if (Shell_p->scroll > 0) {
            Shell_p->scroll = 0;
        }
    }
    Shell_p->Scroll.LastScroll = Now;
    Shell_p->drawing = 1;
 
    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_updateShell(
    tk_core_Program *Program_p)
{
    tk_core_Shell *Shell_p = Program_p->handle_p;
    if (!Shell_p->ST_p) {return TK_CORE_SUCCESS;}
 
    TK_CHECK(tk_core_handleShellSocket(&Shell_p->Socket))

    if (Shell_p->Scroll.active){ 
        TK_CHECK(tk_core_handleFastScroll(Shell_p))
    }

    FD_ZERO(&Shell_p->rfd);
    FD_SET(Shell_p->ttyfd, &Shell_p->rfd);

    // Set timeout to 10 milliseconds
    struct timeval timeout;
    timeout.tv_sec = 0.01f;
    timeout.tv_usec = 0;

    if (select(Shell_p->ttyfd+1, &Shell_p->rfd, NULL, NULL, &timeout) < 0) {
            if (errno == EINTR) {
                return TK_CORE_SUCCESS;
            }
            die("select failed: %s\n", strerror(errno));
    }
    clock_gettime(CLOCK_MONOTONIC, &Shell_p->now);

    if (FD_ISSET(Shell_p->ttyfd, &Shell_p->rfd)) {
        ttyread(Shell_p->ST_p);
    }
    if (Shell_p->ST_p->close) {
        Program_p->close = true;
        return TK_CORE_SUCCESS;
    }

    /*
     * To reduce flicker and tearing, when new content or event
     * triggers drawing, we first wait a bit to ensure we got
     * everything, and if nothing new arrives - we draw.
     * We start with trying to wait minlatency ms. If more content
     * arrives sooner, we retry with shorter and shorter periods,
     * and eventually draw even without idle after maxlatency ms.
     * Typically this results in low latency while interacting,
     * maximum latency intervals during `cat huge.txt`, and perfect
     * sync with periodic updates from animations/key-repeats/etc.
     */
    if (FD_ISSET(Shell_p->ttyfd, &Shell_p->rfd)) {
        if (!Shell_p->drawing) {
            Shell_p->trigger = Shell_p->now;
            Shell_p->drawing = 1;
        }
        Shell_p->timeout = (maxlatency - TIMEDIFF(Shell_p->now, Shell_p->trigger)) \
            / maxlatency * minlatency;
        if (Shell_p->timeout > 0) {
            return TK_CORE_SUCCESS;
        }
    }

    if (Shell_p->drawing) {
        tk_core_drawShell(Shell_p);
        Program_p->refresh = true;
    }

    Shell_p->drawing = 0;

    return TK_CORE_SUCCESS;
}

// COPY/PASTE ======================================================================================

static void tk_core_resetShellSelectionBuffer(
    tk_core_Shell *Shell_p)
{
    if (Shell_p->Selection.buffer_pp != NULL) {
        for (int i = 0; i < Shell_p->Selection.lines; ++i) {
            nh_core_free(Shell_p->Selection.buffer_pp[i]);
        }
        nh_core_free(Shell_p->Selection.buffer_pp);
        Shell_p->Selection.buffer_pp = NULL;
    }
    Shell_p->Selection.lines = 0;
}

static TK_CORE_RESULT tk_core_copyFromShell(
    tk_core_Shell *Shell_p)
{
    nh_encoding_UTF8String Clipboard = nh_encoding_initUTF8(32);

    if (Shell_p->Selection.lines == 0) {
        return TK_CORE_SUCCESS;
    }

    for (int i = 0; i < Shell_p->Selection.lines; ++i) {

        // Append linebreak character to end of each line.
        if (i > 0) {nh_encoding_appendUTF8Single(&Clipboard, '\n');}

        // Remove trailing whitespaces.
        int empty = 0;
        for (int c = Shell_p->ST_p->col-1; c >= 0; --c, ++empty) {
            NH_API_UTF32 codepoint = Shell_p->Selection.buffer_pp[i][c];
            if (codepoint != ' ' && codepoint != 0) {break;}
        }
        if (empty != 0) {empty--;}

        if (i == 0) {
            int length = 0;
            if (Shell_p->Selection.Start.y == Shell_p->Selection.Stop.y) {
                length = abs(Shell_p->Selection.Stop.x - Shell_p->Selection.Start.x)+1;
            } else if (Shell_p->Selection.Start.y > Shell_p->Selection.Stop.y) {
                length = (Shell_p->ST_p->col - Shell_p->Selection.Stop.x)-empty;
            } else if (Shell_p->Selection.Start.y < Shell_p->Selection.Stop.y) {
                length = (Shell_p->ST_p->col - Shell_p->Selection.Start.x)-empty;
            }
    
            // Start-pointer calculation of selection depends on direction from which the selection was initiated.
            NH_API_UTF32 *p = NULL;
            if (Shell_p->Selection.Start.x < Shell_p->Selection.Stop.x) {
                p = Shell_p->Selection.buffer_pp[i] + Shell_p->Selection.Start.x;
            } else {
                p = Shell_p->Selection.buffer_pp[i] + Shell_p->Selection.Stop.x;
            }
    
            if (length > 0) {nh_encoding_appendUTF8(&Clipboard, p, length);}
    
        } else if (Shell_p->ST_p->col - empty > 0) {
            nh_encoding_appendUTF8(&Clipboard, Shell_p->Selection.buffer_pp[i], Shell_p->ST_p->col - empty);
        }
    }

    // TODO sync?
    nh_wsi_setClipboard_f setClipboard_f = nh_core_loadExistingSymbol(NH_MODULE_WSI, 0, "nh_wsi_setClipboard");
    if (setClipboard_f) {
        setClipboard_f(Clipboard.p, Clipboard.length, true);
    }

    tk_core_resetShellSelectionBuffer(Shell_p);
    memset(&Shell_p->Selection, 0, sizeof(tk_core_ShellSelection));

    nh_encoding_freeUTF8(&Clipboard);

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_pasteIntoShell(
    tk_core_Shell *Shell_p)
{
    nh_wsi_getClipboard_f getClipboard_f = nh_core_loadExistingSymbol(NH_MODULE_WSI, 0, "nh_wsi_getClipboard");
    if (!getClipboard_f) {return TK_CORE_SUCCESS;}

    char *clipboard_p = getClipboard_f();
    if (!clipboard_p) {return TK_CORE_SUCCESS;}

    // In bracketed paste mode, text pasted into the terminal will be surrounded by ESC [200~ and ESC [201~; 
    // programs running in the terminal should not treat characters bracketed by those sequences as commands 
    // (Vim, for example, does not treat them as commands).
    // Source: https://en.wikipedia.org/wiki/ANSI_escape_code
    if (Shell_p->ST_p->windowMode & MODE_BRCKTPASTE) {
        ttywrite(Shell_p->ST_p, "\033[200~", 6, 0);
    }
    ttywrite(Shell_p->ST_p, clipboard_p, strlen(clipboard_p), 1);
    if (Shell_p->ST_p->windowMode & MODE_BRCKTPASTE) {
        ttywrite(Shell_p->ST_p, "\033[201~", 6, 0);
    }

    return TK_CORE_SUCCESS;
}

// INPUT ===========================================================================================

#define IS_STOP(u) (u == ' ' || u == ';' || u == '(' || u == ')' || u == '{' || u == '}' || u == '<' || u == '>')

static TK_CORE_RESULT tk_core_handleShellSelection(
    tk_core_Shell *Shell_p)
{
    tk_core_resetShellSelectionBuffer(Shell_p);

    int index1 = Shell_p->Selection.Start.y - Shell_p->Selection.startScroll;
    int index2 = Shell_p->Selection.Stop.y - Shell_p->Selection.stopScroll;

    int start = index1 < index2 ? index1 : index2;
    int stop = index1 > index2 ? index1 : index2;

    for (int i = start; i <= stop; ++i) {Shell_p->Selection.lines++;}

    if (Shell_p->Selection.doubleClick && Shell_p->Selection.lines == 1 && Shell_p->Selection.Start.x == Shell_p->Selection.Stop.x) {
        Shell_p->Selection.buffer_pp = (NH_API_UTF32**)nh_core_allocate(sizeof(NH_API_UTF32*)*Shell_p->Selection.lines);
        TK_CHECK_MEM(Shell_p->Selection.buffer_pp)
        NH_API_UTF32 *buffer_p = (NH_API_UTF32*)nh_core_allocate(sizeof(NH_API_UTF32)*Shell_p->ST_p->col);
        TK_CHECK_MEM(buffer_p)
        memset(buffer_p, 0, sizeof(NH_API_UTF32)*Shell_p->ST_p->col);
        Shell_p->Selection.buffer_pp[0] = buffer_p;
 
        if (start < 0) { 
           // In history.
           for (int i = 0; i < Shell_p->ST_p->col; ++i) {
               buffer_p[i] = Shell_p->ST_p->history[Shell_p->ST_p->scrollup-abs(start)][i].u;
           }
        } else {
           for (int i = 0; i < Shell_p->ST_p->col; ++i) {
               buffer_p[i] = Shell_p->ST_p->line[start][i].u;
           }
        }

        int i1 = 0, i2 = 0;
        if (Shell_p->Selection.Start.x == Shell_p->Selection.Stop.x) {
            if (start < 0) { 
                // In history.
                for (i1 = Shell_p->Selection.Start.x; i1 >= 0 && !IS_STOP(Shell_p->ST_p->history[Shell_p->ST_p->scrollup-abs(start)][i1].u); --i1);
                for (i2 = Shell_p->Selection.Start.x; i2 < Shell_p->ST_p->col && !IS_STOP(Shell_p->ST_p->history[Shell_p->ST_p->scrollup-abs(start)][i2].u); ++i2);
            } else {
                for (i1 = Shell_p->Selection.Start.x; i1 >= 0 && !IS_STOP(Shell_p->ST_p->line[start][i1].u); --i1);
                for (i2 = Shell_p->Selection.Start.x; i2 < Shell_p->ST_p->col && !IS_STOP(Shell_p->ST_p->line[start][i2].u); ++i2);
            }
        }
 
        if (i1 != i2) {
            Shell_p->Selection.Start.x = i1+1;
            Shell_p->Selection.Stop.x = i2-1;
        }

    } else if (Shell_p->Selection.trippleClick && Shell_p->Selection.lines == 1 && Shell_p->Selection.Start.x == Shell_p->Selection.Stop.x) {
        Shell_p->Selection.buffer_pp = (NH_API_UTF32**)nh_core_allocate(sizeof(NH_API_UTF32*)*Shell_p->Selection.lines);
        TK_CHECK_MEM(Shell_p->Selection.buffer_pp)
        NH_API_UTF32 *buffer_p = (NH_API_UTF32*)nh_core_allocate(sizeof(NH_API_UTF32)*Shell_p->ST_p->col);
        TK_CHECK_MEM(buffer_p)
        memset(buffer_p, 0, sizeof(NH_API_UTF32)*Shell_p->ST_p->col);
        Shell_p->Selection.buffer_pp[0] = buffer_p;
 
        if (start < 0) { 
           // In history.
           for (int i = 0; i < Shell_p->ST_p->col; ++i) {
               buffer_p[i] = Shell_p->ST_p->history[Shell_p->ST_p->scrollup-abs(start)][i].u;
           }
        } else {
           for (int i = 0; i < Shell_p->ST_p->col; ++i) {
               buffer_p[i] = Shell_p->ST_p->line[start][i].u;
           }
        }

        int i1 = 0, i2 = 0;
        if (Shell_p->Selection.Start.x == Shell_p->Selection.Stop.x) {
            if (start < 0) { 
                // In history.
                for (i1 = Shell_p->Selection.Start.x; i1 >= 0; --i1);
                for (i2 = Shell_p->Selection.Start.x; i2 < Shell_p->ST_p->col; ++i2);
            } else {
                for (i1 = Shell_p->Selection.Start.x; i1 >= 0; --i1);
                for (i2 = Shell_p->Selection.Start.x; i2 < Shell_p->ST_p->col; ++i2);
            }
        }
 
        if (i1 != i2) {
            Shell_p->Selection.Start.x = i1+1;
            Shell_p->Selection.Stop.x = i2-1;
        }

    } else {
        Shell_p->Selection.buffer_pp = (NH_API_UTF32**)nh_core_allocate(sizeof(NH_API_UTF32*)*Shell_p->Selection.lines);
        TK_CHECK_MEM(Shell_p->Selection.buffer_pp)
    
        for (int i = 0; i < Shell_p->Selection.lines; ++i) {
            NH_API_UTF32 *buffer_p = (NH_API_UTF32*)nh_core_allocate(sizeof(NH_API_UTF32)*Shell_p->ST_p->col);
            TK_CHECK_MEM(buffer_p)
            memset(buffer_p, 0, sizeof(NH_API_UTF32)*Shell_p->ST_p->col);
            Shell_p->Selection.buffer_pp[i] = buffer_p;
        }
     
        for (int i = start, buf = 0; i <= stop; ++i, ++buf) {
            if (i < 0) { 
                // In history.
               for (int j = 0; j < Shell_p->ST_p->col; ++j) {
                   Shell_p->Selection.buffer_pp[buf][j] = Shell_p->ST_p->history[Shell_p->ST_p->scrollup-abs(i)][j].u;
               }
            } else {
               for (int j = 0; j < Shell_p->ST_p->col; ++j) {
                   Shell_p->Selection.buffer_pp[buf][j] = Shell_p->ST_p->line[i][j].u;
               }
            }
        }
    }

    return TK_CORE_SUCCESS;
}

static char *tk_core_getShellKey(
    tk_core_Shell *Shell_p, NH_API_KEY_E k, uint state)
{
    for (tk_core_ShellKey *kp = KEYS_P; kp < KEYS_P + sizeof(KEYS_P)/sizeof(KEYS_P[0]); kp++) {
        if (kp->key != k)
                continue;

        if (kp->mask != -1 && kp->mask != state)
                continue;

        if ((Shell_p->ST_p->windowMode & MODE_APPKEYPAD) ? kp->appkey < 0 : kp->appkey > 0)
                continue;
        if ((Shell_p->ST_p->windowMode & MODE_NUMLOCK) && kp->appkey == 2)
                continue;

        if ((Shell_p->ST_p->windowMode & MODE_APPCURSOR) ? kp->appcursor < 0 : kp->appcursor > 0)
                continue;

        return kp->s;
    }

    return NULL;
}

static TK_CORE_RESULT tk_core_sendMouseEvent(
    tk_core_Shell *Shell_p, nh_api_MouseEvent Event)
{
    if (Shell_p->ST_p->windowMode & MODE_MOUSESGR) {

        unsigned int button = 0;
        switch (Event.type) {
            case NH_API_MOUSE_LEFT   : button = 0; break;
            case NH_API_MOUSE_MIDDLE : button = 1; break;
            case NH_API_MOUSE_RIGHT  : button = 2; break;
            case NH_API_MOUSE_SCROLL : 
                button = Event.trigger == NH_API_TRIGGER_UP ? 64 : 65;
                break;
        }
         
        char buf_p[64] = {};
        int len = snprintf(buf_p, sizeof(buf_p), "\033[<%d;%d;%d%c",
            button, Event.Position.x, Event.Position.y, Event.trigger == NH_API_TRIGGER_RELEASE ? 'm' : 'M');

        ttywrite(Shell_p->ST_p, buf_p, len, 0);

    } else if (Event.Position.x < 223 && Event.Position.y < 223) {

        unsigned int button = 0;
        switch (Event.type) {
            case NH_API_MOUSE_LEFT   : button = 0; break;
            case NH_API_MOUSE_MIDDLE : button = 1; break;
            case NH_API_MOUSE_RIGHT  : button = 2; break;
            case NH_API_MOUSE_SCROLL : 
                button = Event.trigger == NH_API_TRIGGER_UP ? 64 : 65;
                break;
        }
 
        char buf_p[64] = {}; 
        int len = snprintf(buf_p, sizeof(buf_p), "\033[M%c%c%c", 
            32+button, 32+Event.Position.x, 32+Event.Position.y);

        ttywrite(Shell_p->ST_p, buf_p, len, 0);
    }

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_handleShellInput(
    tk_core_Program *Program_p, nh_api_WSIEvent Event)
{
    tk_core_Shell *Shell_p = Program_p->handle_p;
    if (!Shell_p->ST_p) {return TK_CORE_SUCCESS;}

    if (Event.type == NH_API_WSI_EVENT_KEYBOARD) {
        Shell_p->LastEvent = Event.Keyboard;
    }

    // Handle key-press events.
    if (Event.type == NH_API_WSI_EVENT_KEYBOARD && Event.Keyboard.trigger == NH_API_TRIGGER_PRESS) {
        // Handle special key.
        char *specialkey = NULL;
        if ((specialkey = tk_core_getShellKey(Shell_p, Event.Keyboard.special, Event.Keyboard.state)) && Event.Keyboard.codepoint == 0) {
            if (specialkey) {
                ttywrite(Shell_p->ST_p, specialkey, strlen(specialkey), 1);
            }
        } else if (PASTE_KEY) {
            tk_core_pasteIntoShell(Shell_p);            

        } else if (COPY_KEY) {
            tk_core_copyFromShell(Shell_p);            

        // Handle normal key.
        } else if (Event.Keyboard.codepoint != 0) {
            // Reset scrolling.
            Shell_p->scroll = 0;
 
            char p[4] = {0};
            NH_API_UTF32 codepoint = Event.Keyboard.codepoint;
            int length = nh_encoding_encodeUTF8Single(codepoint, p);
            if (length == 1 && Event.Keyboard.state & NH_API_MODIFIER_MOD1) {
                if (Shell_p->ST_p->windowMode & MODE_8BIT) {
                    if (*p < 0177) {
                        NH_API_UTF32 c = *p | 0x80;
                        length = nh_encoding_encodeUTF8Single(c, p);
                    }
                } else {
                    p[1] = p[0];
                    p[0] = '\033';
                    length = 2;
                }
            }
            ttywrite(Shell_p->ST_p, p, length, 0);
        }
    }

    // Handle mouse events.
    if (Event.type == NH_API_WSI_EVENT_MOUSE) {
        if (Event.Mouse.type == NH_API_MOUSE_MIDDLE && Event.Mouse.trigger == NH_API_TRIGGER_PRESS) {
            Shell_p->Scroll.active = true;
            Shell_p->Scroll.Position = Event.Window.Position;
            Shell_p->Scroll.Current = Event.Window.Position;
            Shell_p->Scroll.LastScroll = nh_core_getSystemTime();
        }
        if (Event.Mouse.type == NH_API_MOUSE_MIDDLE && Event.Mouse.trigger == NH_API_TRIGGER_RELEASE) {
            Shell_p->Scroll.active = false;
        }
        if (Event.Mouse.type == NH_API_MOUSE_LEFT && Event.Mouse.trigger == NH_API_TRIGGER_PRESS) {
            Shell_p->Selection.Start = Event.Mouse.Position;
            Shell_p->Selection.startScroll = Shell_p->scroll;
            Shell_p->Selection.Stop = Shell_p->Selection.Start;
            Shell_p->Selection.stopScroll = Shell_p->Selection.startScroll;
            Shell_p->Selection.active = true;
            nh_core_SystemTime Now = nh_core_getSystemTime();
            bool fastClick = nh_core_getSystemTimeDiffInSeconds(Shell_p->LastClick, Now) < 0.3f;
            Shell_p->Selection.trippleClick = Shell_p->Selection.doubleClick && fastClick;
            Shell_p->Selection.doubleClick = fastClick && !Shell_p->Selection.trippleClick;
            Shell_p->Selection.draw = Shell_p->Selection.doubleClick || Shell_p->Selection.trippleClick;
            Shell_p->Selection.moved = false;
            Shell_p->LastClick = Now;
            if (Shell_p->Selection.doubleClick || Shell_p->Selection.trippleClick) {
                tk_core_handleShellSelection(Shell_p);
            }
        }
        if (Event.Mouse.type == NH_API_MOUSE_LEFT && Event.Mouse.trigger == NH_API_TRIGGER_RELEASE) {
            if (!Shell_p->Selection.doubleClick && !Shell_p->Selection.trippleClick) {
                Shell_p->Selection.Stop = Event.Mouse.Position;
                Shell_p->Selection.stopScroll = Shell_p->scroll;
            }
            Shell_p->Selection.active = false;
            Shell_p->Selection.draw = Shell_p->Selection.moved || Shell_p->Selection.doubleClick || Shell_p->Selection.trippleClick;
            tk_core_handleShellSelection(Shell_p);
        }
        if (Event.Mouse.type == NH_API_MOUSE_MOVE) {
            if (Shell_p->Selection.active) {
                Shell_p->Selection.doubleClick = false;
                Shell_p->Selection.trippleClick = false;
                Shell_p->Selection.draw = true;
                Shell_p->Selection.Stop = Event.Mouse.Position;
                Shell_p->Selection.stopScroll = Shell_p->scroll;
                Shell_p->Selection.moved = true;
            }
            if (Shell_p->Scroll.active) {
                Shell_p->Scroll.Current = Event.Window.Position;
            }
        }

        if (Shell_p->ST_p->mode & TERM_MODE_ALTSCREEN) {
            tk_core_sendMouseEvent(Shell_p, Event.Mouse);
        } else {
#define CTRL_ACTIVE ((Shell_p->LastEvent.state & NH_API_MODIFIER_CONTROL) && (Shell_p->LastEvent.trigger != NH_API_TRIGGER_RELEASE && (Shell_p->LastEvent.special != NH_API_KEY_CONTROL_L && Shell_p->LastEvent.special != NH_API_KEY_CONTROL_R)))
            if (Event.Mouse.type == NH_API_MOUSE_SCROLL && !CTRL_ACTIVE) {
                if (Event.Mouse.trigger == NH_API_TRIGGER_DOWN) {
                    if (Shell_p->scroll > 0) {Shell_p->scroll--;};
                }
                if (Event.Mouse.trigger == NH_API_TRIGGER_UP) {
                    if (Shell_p->scroll < Shell_p->ST_p->scrollup) {Shell_p->scroll++;}
                }
                tk_core_drawShell(Shell_p);
            }
        }
    }

    Program_p->refresh = true;

    return TK_CORE_SUCCESS;
}

// CURSOR ==========================================================================================

static TK_CORE_RESULT tk_core_getShellCursor(
    tk_core_Program *Program_p, int *x_p, int *y_p)
{
    if (((tk_core_Shell*)Program_p->handle_p)->scroll != 0) {return TK_CORE_SUCCESS;}

    // MODE_HIDE is activated by for example: echo -e "\e[?25l"
    if (((tk_core_Shell*)Program_p->handle_p)->ST_p->windowMode & MODE_HIDE) {
        *x_p = -1;
        *y_p = -1;
    }

    // ST_p is created late during first draw, which might not have happened before this is called.
    else if (((tk_core_Shell*)Program_p->handle_p)->ST_p) {
        *x_p = ((tk_core_Shell*)Program_p->handle_p)->ST_p->ocx;
        *y_p = ((tk_core_Shell*)Program_p->handle_p)->ST_p->ocy;
    }

    return TK_CORE_SUCCESS;
}

// DRAW ============================================================================================

static TK_CORE_RESULT tk_core_resizeShellIfRequired(
    tk_core_Shell *Shell_p, int width, int height)
{
    if (Shell_p->width == width && Shell_p->height == height) {
        return TK_CORE_SUCCESS;
    }

    if (Shell_p->Rows_p) {
        for (int row = 0; row < Shell_p->height; ++row) {
            nh_core_free(Shell_p->Rows_p[row].Glyphs_p);
        }
        nh_core_free(Shell_p->Rows_p);
    }

    Shell_p->Rows_p = (tk_core_Row*)nh_core_allocate(sizeof(tk_core_Row)*height);
    memset(Shell_p->Rows_p, 0, sizeof(tk_core_Row)*height);

    for (int row = 0; row < height; ++row) {
        Shell_p->Rows_p[row].Glyphs_p = (tk_core_Glyph*)nh_core_allocate(sizeof(tk_core_Glyph)*width);
        memset(Shell_p->Rows_p[row].Glyphs_p, 0, sizeof(tk_core_Glyph)*width);
    }

    if (Shell_p->ST_p) {
        cresize(Shell_p->ST_p, width, height);
    }

    Shell_p->width = width;
    Shell_p->height = height;

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_startShellIfRequired(
    tk_core_Shell *Shell_p)
{
    if (Shell_p->ST_p) {
        return TK_CORE_SUCCESS;
    }

    Shell_p->ST_p = tnew(Shell_p->width, Shell_p->height);
    Shell_p->ST_p->maxScrollup = 1000; // TODO re-add
    Shell_p->ttyfd = ttynew(Shell_p->ST_p, 0, SHELL_PATH, 0, 0);
    Shell_p->timeout = 0.01f;

    cresize(Shell_p->ST_p, Shell_p->width, Shell_p->height);

    TK_CHECK(tk_core_createShellSocket(&Shell_p->Socket, Shell_p->ST_p->pid))

    return TK_CORE_SUCCESS;
}

static void tk_core_drawSelectionIfRequired(
    tk_core_Shell *Shell_p, tk_core_Glyph *Glyphs_p, int width, int row)
{
    int index1 = Shell_p->Selection.Start.y - Shell_p->Selection.startScroll;
    int index2 = Shell_p->Selection.Stop.y - Shell_p->Selection.stopScroll;

    int start = index1 < index2 ? index1 : index2;
    int stop = index1 > index2 ? index1 : index2;

    if (Shell_p->Selection.draw && row - Shell_p->scroll >= start && row - Shell_p->scroll <= stop) {
        for (int i = 0; i < width; ++i) {
            if (index1 < index2 || index1 == index2) {
                if (Shell_p->Selection.Start.x < Shell_p->Selection.Stop.x) {
                    if (row - Shell_p->scroll == start && i < Shell_p->Selection.Start.x) {continue;}
                    if (row - Shell_p->scroll == stop && i > Shell_p->Selection.Stop.x) {continue;}
                } else {
                    if (row - Shell_p->scroll == start && i > Shell_p->Selection.Start.x) {continue;}
                    if (row - Shell_p->scroll == stop && i < Shell_p->Selection.Stop.x) {continue;}
                }
            }
            if (index1 > index2) {
                if (row - Shell_p->scroll == start && i < Shell_p->Selection.Stop.x) {continue;}
                if (row - Shell_p->scroll == stop && i > Shell_p->Selection.Start.x) {continue;}
            }
            if (Shell_p->Selection.Start.x != Shell_p->Selection.Stop.x || start != stop || Glyphs_p[i].codepoint != ' ') {
                Glyphs_p[i].Attributes.reverse = true;
                Glyphs_p[i].mark |= TK_CORE_MARK_ACCENT;
            }
        }
    }
}

static TK_CORE_RESULT tk_core_drawShellRow(
    tk_core_Program *Program_p, tk_core_Glyph *Glyphs_p, int width, int height, int row)
{
    tk_core_Shell *Shell_p = Program_p->handle_p;

    TK_CHECK(tk_core_resizeShellIfRequired(Shell_p, width, height))
    TK_CHECK(tk_core_startShellIfRequired(Shell_p))
    
    memcpy(Glyphs_p, Shell_p->Rows_p[row].Glyphs_p, sizeof(tk_core_Glyph)*width);

    tk_core_drawSelectionIfRequired(Shell_p, Glyphs_p, width, row);

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_drawShellTopbar(
    tk_core_Program *Program_p, tk_core_Glyph *Glyphs_p, int width)
{
    tk_core_Shell *Shell_p = Program_p->handle_p;

    char topbar_p[1024];
    memset(topbar_p, 0, 1024);

    if (Shell_p->ST_p) {
        char path_p[255] = {0};
        sprintf(path_p, "/proc/%ld/cwd", Shell_p->ST_p->pid);

        char buf_p[1024] = {0};
        readlink(path_p, buf_p, 1024);

        int offset = (width / 2) - (strlen(buf_p) / 2);
        if (offset >= 0) {
            sprintf(topbar_p+offset, "%s", buf_p);
        }
    }

    for (int i = 0; i < width; ++i) {
        Glyphs_p[i].codepoint = topbar_p[i];
    }

    if (Shell_p->scroll > 0) {
        char scroll_p[64];
        sprintf(scroll_p, "%d/%d", Shell_p->scroll, Shell_p->ST_p->scrollup);
        for (int i = 0, j = strlen(scroll_p)-1; i < strlen(scroll_p); ++i, --j) {
            Glyphs_p[(width-4)-i].codepoint = scroll_p[j];
        }
    }

    return TK_CORE_SUCCESS;
}

static TK_CORE_RESULT tk_core_getShellTitle(
    tk_core_Program *Program_p, NH_API_UTF32 *title_p, int length)
{
    tk_core_Shell *Shell_p = Program_p->handle_p;

    char topbar_p[1024];
    memset(topbar_p, 0, 1024);

    if (Shell_p->ST_p) {
        char path_p[255] = {0};
        sprintf(path_p, "/proc/%ld/cwd", Shell_p->ST_p->pid);

        char buf_p[1024] = {0};
        readlink(path_p, buf_p, 1024);

        for (int i = 0; i < strlen(buf_p); ++i) {
            title_p[i] = buf_p[i];
        }
    }

    return TK_CORE_SUCCESS;
}

// COMMANDS ========================================================================================

typedef enum TK_CORE_SHELL_COMMAND_E {
    TK_CORE_SHELL_COMMAND_COPY = 0,
    TK_CORE_SHELL_COMMAND_PASTE,
    TK_CORE_SHELL_COMMAND_E_COUNT,
} TK_CORE_SHELL_COMMAND_E;

static TK_CORE_RESULT tk_core_handleShellCommand(
    tk_core_Program *Program_p)
{
    tk_core_Shell *Shell_p = Program_p->handle_p;

    switch (Program_p->command)
    {
        case TK_CORE_SHELL_COMMAND_COPY:
            tk_core_copyFromShell(Program_p->handle_p);
            break;
        case TK_CORE_SHELL_COMMAND_PASTE:
            tk_core_pasteIntoShell(Program_p->handle_p);
            break;
    }

    return TK_CORE_SUCCESS;
}

// INIT/DESTROY ====================================================================================

static void *tk_core_initShell(
    void *arg_p)
{
    tk_core_Shell *Shell_p = (tk_core_Shell*)nh_core_allocate(sizeof(tk_core_Shell));
    memset(Shell_p, 0, sizeof(tk_core_Shell));
    return Shell_p;
}

static void tk_core_destroyShell(
    void *handle_p)
{
    tk_core_Shell *Shell_p = handle_p;

    if (Shell_p->ST_p) {
        // Kill shell program.
        ttyhangup(Shell_p->ST_p);
        tfree(Shell_p->ST_p);
    }

    for (int row = 0; row < Shell_p->height; ++row) {
        nh_core_free(Shell_p->Rows_p[row].Glyphs_p);
    }

    nh_core_free(Shell_p->Rows_p);
    nh_core_free(Shell_p);
}

// PROTOTYPE =======================================================================================

static void tk_core_destroyShellPrototype(
    tk_core_Interface *Prototype_p)
{
    nh_core_free(Prototype_p->commands_pp[0]);
    nh_core_free(Prototype_p->commands_pp[1]);
    nh_core_free(Prototype_p->commands_pp);
    nh_core_free(Prototype_p);
}

tk_core_Interface *tk_core_createShellInterface()
{
    tk_core_Interface *Prototype_p = (tk_core_Interface*)nh_core_allocate(sizeof(tk_core_Interface));
    TK_CHECK_MEM_2(NULL, Prototype_p)

    memset(Prototype_p, 0, sizeof(tk_core_Interface));

    Prototype_p->Callbacks.init_f = tk_core_initShell;
    Prototype_p->Callbacks.draw_f = tk_core_drawShellRow;
    Prototype_p->Callbacks.drawTopbar_f = tk_core_drawShellTopbar;
    Prototype_p->Callbacks.getTitle_f = tk_core_getShellTitle;
    Prototype_p->Callbacks.handleInput_f = tk_core_handleShellInput;
    Prototype_p->Callbacks.handleCommand_f = tk_core_handleShellCommand;
    Prototype_p->Callbacks.getCursorPosition_f = tk_core_getShellCursor;
    Prototype_p->Callbacks.update_f = tk_core_updateShell;
    Prototype_p->Callbacks.destroyPrototype_f = tk_core_destroyShellPrototype;
    Prototype_p->Callbacks.destroy_f = tk_core_destroyShell;

    NH_API_UTF32 name_p[6] = {'s', 'h', 'e', 'l', 'l', 0};
    memcpy(Prototype_p->name_p, name_p, sizeof(name_p));

    NH_API_UTF32 command1_p[5] = {'c', 'o', 'p', 'y', 0};
    NH_API_UTF32 command2_p[6] = {'p', 'a', 's', 't', 'e', 0};
 
    NH_API_UTF32 **commands_pp = (NH_API_UTF32**)nh_core_allocate(sizeof(NH_API_UTF32*)*2);
    TK_CHECK_MEM_2(NULL, commands_pp)

    commands_pp[0] = (NH_API_UTF32*)nh_core_allocate(sizeof(command1_p));
    TK_CHECK_MEM_2(NULL, commands_pp[0])
    commands_pp[1] = (NH_API_UTF32*)nh_core_allocate(sizeof(command2_p));
    TK_CHECK_MEM_2(NULL, commands_pp[1])

    memcpy(commands_pp[0], command1_p, sizeof(command1_p));
    memcpy(commands_pp[1], command2_p, sizeof(command2_p));

    Prototype_p->commands_pp = commands_pp;
    Prototype_p->commands = 2;

    return Prototype_p;
}
