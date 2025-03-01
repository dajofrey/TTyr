// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "Titlebar.h"
#include "Program.h"
#include "TTY.h"

#include "../Common/Macros.h"

#include "nh-core/System/Memory.h"
#include "nh-core/System/Thread.h"
#include "nh-core/System/System.h"

#include "nh-encoding/Encodings/UTF32.h"
#include "nh-encoding/Encodings/UTF8.h"

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>

//// INPUT ===========================================================================================
//
//TTYR_TTY_RESULT ttyr_tty_handleSideBarHit(
//    NH_API_MOUSE_E mouse, int row)
//{
//TTYR_TTY_BEGIN()
//
//    ttyr_tty_TTY *TTY_p = nh_core_getWorkloadArg();
//
//    switch (mouse) {
//        case NH_API_MOUSE_LEFT :
//            ttyr_tty_insertAndFocusWindow(TTY_p, row);
//            break;
//        case NH_API_MOUSE_RIGHT :
//            TTYR_TTY_MACRO_TILE(TTY_p->Window_p->Tile_p)->current = row;
//            break;
//    }
//    
//TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
//}

// INPUT =========================================================================================== 

void ttyr_tty_handleTitlebarHit( 
    nh_api_MouseEvent Event, int cCol) 
{
    ttyr_tty_Config Config = ttyr_tty_getConfig(); 
    if (Event.trigger == NH_API_TRIGGER_PRESS && cCol-1 < Config.windows) { 
        ttyr_tty_MacroWindow *Window_p = ttyr_tty_insertAndFocusWindow(nh_core_getWorkloadArg(), cCol-1);
        Window_p->refreshGrid2 = true; 
        Window_p->refreshTitlebar = true; 
        Window_p->Tile_p->refresh = true; 
    } 
} 

// CHECK ===========================================================================================

void ttyr_tty_checkTitlebar(
    ttyr_tty_Titlebar *Titlebar_p, bool *refresh_p)
{
    ttyr_tty_Config Config = ttyr_tty_getConfig();
    if (Config.Titlebar.on == false) {return;}

    if (NH_SYSTEM.LocalTime.days != Titlebar_p->Time.days || NH_SYSTEM.LocalTime.hours != Titlebar_p->Time.hours
    ||  NH_SYSTEM.LocalTime.minutes != Titlebar_p->Time.minutes || NH_SYSTEM.RAM.free != Titlebar_p->RAM.free) {
        *refresh_p = true;
    }
}

// DRAW ============================================================================================

TTYR_TTY_RESULT ttyr_tty_drawTitlebar(
    ttyr_tty_TTY *TTY_p)
{
    ttyr_tty_View *View_p = TTY_p->Views.pp[0];
    ttyr_tty_Row *Row_p = &(View_p->Grid1_p[View_p->rows]);
    int cols = View_p->cols;
    ttyr_tty_Config Config = ttyr_tty_getConfig();
    if (Config.Titlebar.on == false) {return TTYR_TTY_SUCCESS;}

    // Set titlebar color.
    for (int i = 0; i < cols ; ++i) {
        Row_p->Glyphs_p[i].mark |= TTYR_TTY_MARK_ACCENT;
        Row_p->Glyphs_p[i].mark |= TTYR_TTY_MARK_ACCENT;
    }

    for (int i = 1; ttyr_tty_getConfig().Topbar.on == 1 && i < (Config.windows+1) && i < cols ; ++i) {
        Row_p->Glyphs_p[i].codepoint = 0x25a1;
        Row_p->update_p[i] = true;
        if (nh_core_getListIndex(&TTY_p->Windows, TTY_p->Window_p) == i-1) {
            Row_p->Glyphs_p[i].codepoint = 0x25a0;
        }
    }

//    for (int i = 1, j = 1; ttyr_tty_getConfig().Topbar.on == 0 && i < (Config.windows+1) && i < cols ; ++i, j+=3) {
//        for (int k = j; k < j+2; ++k) {
//            Row_p->Glyphs_p[k].codepoint = 'f';
//            Row_p->Glyphs_p[k].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
//            Row_p->update_p[k] = true;
//            if (nh_core_getListIndex(&TTY_p->Windows, TTY_p->Window_p) == i-1) {
//                Row_p->Glyphs_p[k].codepoint = 'o';
//            }
//        }
//    }
//
//    for (int i = 1; i < cols ; ++i) {
//        Row_p->update_p[i] = 1;
//    }
//
//    char rightSide_p[255] = {};
//    sprintf(rightSide_p+strlen(rightSide_p), "RAM:%.1f%%", fabs((((float)NH_SYSTEM.RAM.free)/((float)NH_SYSTEM.RAM.total))*100.0f-100.0f));
//
//    for (int i = 0; i < strlen(rightSide_p); ++i) {
//        if (rightSide_p[i] == 'x') {
//            Row_p->Glyphs_p[(cols-strlen(rightSide_p))+i-3].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
//        }
//        Row_p->Glyphs_p[(cols-strlen(rightSide_p))+i-3].codepoint = rightSide_p[i];
//        Row_p->update_p[(cols-strlen(rightSide_p))+i-3] = 1;
//    }
//
//    char ram_p[255] = {};
//    sprintf(ram_p, "xHelpxSettingsx", (((float)NH_SYSTEM.RAM.free)/((float)NH_SYSTEM.RAM.total))*100.0f);
//
//    for (int i = 0; i < strlen(ram_p); ++i) {
//        if (ram_p[i] == 'x') {
//            Row_p->Glyphs_p[Config.windows+2+i].mark |= TTYR_TTY_MARK_LINE_GRAPHICS;
//        }
//        Row_p->Glyphs_p[Config.windows+2+i].codepoint = ram_p[i];
//        Row_p->update_p[Config.windows+2+i] = 1;
//    }

    char month_p[6] = {};
    switch (NH_SYSTEM.LocalTime.months) {
        case 1 : sprintf(month_p, "Jan"); break;
        case 2 : sprintf(month_p, "Feb"); break;
        case 3 : sprintf(month_p, "Mar"); break;
        case 4 : sprintf(month_p, "Apr"); break;
        case 5 : sprintf(month_p, "May"); break;
        case 6 : sprintf(month_p, "Jun"); break;
        case 7 : sprintf(month_p, "Jul"); break;
        case 8 : sprintf(month_p, "Aug"); break;
        case 9 : sprintf(month_p, "Sep"); break;
        case 10 : sprintf(month_p, "Oct"); break;
        case 11 : sprintf(month_p, "Nov"); break;
        case 12 : sprintf(month_p, "Dec"); break;
    }

    char middle_p[255] = {};
    sprintf(middle_p, "%s.%.2d %.2d:%.2d", month_p, NH_SYSTEM.LocalTime.days, NH_SYSTEM.LocalTime.hours, NH_SYSTEM.LocalTime.minutes);
    int offset = (cols/2)-(strlen(middle_p)/2);
    for (int i = 0; i <  strlen(middle_p); ++i) {
        Row_p->Glyphs_p[offset+i].codepoint = middle_p[i];
        Row_p->update_p[offset+i] = 1;
    }

    TTY_p->Titlebar.RAM = NH_SYSTEM.RAM;
    TTY_p->Titlebar.Time = NH_SYSTEM.LocalTime;

    return TTYR_TTY_SUCCESS;
}
