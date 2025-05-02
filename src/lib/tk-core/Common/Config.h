#ifndef TK_CORE_CONFIG_H
#define TK_CORE_CONFIG_H

#include "Includes.h"
#include <stddef.h>

typedef struct tk_core_MenuConfig {
    bool program;
    bool split;
    bool append;
    bool window;
    bool tab;
    bool close;
    bool debug;
} tk_core_MenuConfig;

typedef struct tk_core_TitlebarConfig {
    bool on;
    tk_core_Color Foreground;
    tk_core_Color Background;
} tk_core_TitlebarConfig;

typedef struct tk_core_TopbarConfig {
    bool on;
} tk_core_TopbarConfig;

typedef struct tk_core_ShellConfig {
    unsigned int maxScroll;
} tk_core_ShellConfig;

typedef struct tk_core_Config {
    char *name_p;
    tk_core_TitlebarConfig Titlebar;
    tk_core_TopbarConfig Topbar;
    tk_core_ShellConfig Shell;
    tk_core_MenuConfig Menu;
    int windows;
    int tabs;
} tk_core_Config;

tk_core_Config tk_core_updateConfig(
    void *TTY_p
);

#endif
