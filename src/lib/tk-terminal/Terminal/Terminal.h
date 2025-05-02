#ifndef TK_TERMINAL_TERMINAL_H
#define TK_TERMINAL_TERMINAL_H

#include "Grid.h"
#include "Graphics.h"

#include "../Common/Config.h"
#include "../Common/Includes.h"
#include "../../tk-core/TTY/View.h"

#include "nh-core/Util/Time.h"

typedef struct tk_terminal_Terminal {
    nh_gfx_Text Text;
    tk_core_TTY *TTY_p;
    char namespace_p[255];
    tk_terminal_Config Config;
    tk_terminal_Grid Grid;
    tk_terminal_Grid ElevatedGrid;
    tk_terminal_Grid BackdropGrid;
    tk_terminal_Graphics Graphics;
    tk_core_View *View_p;
    bool ctrl;
    bool leftMouse;
    nh_core_SystemTime LastClick;
} tk_terminal_Terminal;

#endif 
