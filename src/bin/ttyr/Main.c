// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator 
 * Copyright (C) 2022  Dajo Frey
 * Published under MIT.
 */

// INCLUDES ========================================================================================

#include "ttyr-api/ttyr-terminal.h"
#include "Netzhaut/src/lib/nhapi/nhapi.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// TYPES ===========================================================================================

typedef struct Arguments {
    NH_GFX_API_E renderer;
    bool no_unload;
} Arguments;

static Arguments Args;
static nh_PixelPosition Position = {0};
static nh_gfx_Viewport *Viewport_p = NULL;
static ttyr_tty_TTY *TTY_p = NULL;

// HELPER ==========================================================================================

static int handleArgs(
    int argc, char **argv_pp)
{
    Args.renderer = NH_GFX_API_OPENGL;
    Args.no_unload = false;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv_pp[i], "--vulkan")) {
            Args.renderer = NH_GFX_API_VULKAN;
        }
        if (!strcmp(argv_pp[i], "--no-unload")) {
            Args.no_unload = true;
        }
        else {
            puts("--vulkan    Use Vulkan as graphics renderer. Default is OpenGL.");
            puts("--no-unload Don't unload Netzhaut modules at shut-down.");
            puts("            Useful for stack tracing.");
            return 1;
        }
    }

    return 0;
}

static void handleInput(
    nh_wsi_Window *Window_p, nh_wsi_Event Event)
{
    switch (Event.type)
    {
        case NH_WSI_EVENT_MOUSE :
        case NH_WSI_EVENT_KEYBOARD :
            ttyr_api_sendEvent(TTY_p, Event);
            break;
        case NH_WSI_EVENT_WINDOW :
            switch (Event.Window.type) 
            {
                case NH_WSI_WINDOW_CONFIGURE :
                    if (Viewport_p) {
                        nh_api_configureViewport(Viewport_p, Position, Event.Window.Size); 
                    }
                    break;
                case NH_WSI_WINDOW_FOCUS_OUT :
                case NH_WSI_WINDOW_FOCUS_IN :
                    ttyr_api_sendEvent(TTY_p, Event);
                    break;
            }
            break;
    }
}

// MAIN ============================================================================================
 
int main(int argc, char **argv_pp) 
{
    if (handleArgs(argc, argv_pp)) {return 1;}
    if (nh_api_initialize(NH_LOADER_SCOPE_SYSTEM, NULL, NULL, 0) != NH_CORE_SUCCESS) {return 1;}

    nh_api_registerConfig("/etc/ttyr.conf", 14);

    TTY_p = ttyr_api_openTTY(NULL, NULL);
    if (!TTY_p) {return 1;}

    ttyr_terminal_Terminal *Terminal_p = ttyr_api_openTerminal(NULL, TTY_p);
    if (!Terminal_p) {return 1;}

    nh_wsi_Window *Window_p = 
        nh_api_createWindow(NULL, nh_api_getSurfaceRequirements());
    if (!Window_p) {return 1;}

    nh_gfx_Surface *Surface_p = nh_api_createSurface(Window_p, Args.renderer);
    if (!Surface_p) {return 1;}

    nh_PixelSize Size;
    Size.width  = 3000;
    Size.height = 2000;

    Viewport_p = nh_api_createViewport(Surface_p, Position, Size);
    if (!Viewport_p) {return 1;}

    if (ttyr_api_setViewport(Terminal_p, Viewport_p) != TTYR_TERMINAL_SUCCESS) {
        return 1;
    }
    
    nh_api_setWindowEventListener(Window_p, handleInput);

    while (nh_api_getWorkload(TTY_p)) {
        if (!nh_api_run()) {usleep(10000);} // 10 milliseconds
    }

    nh_api_terminate();
}
