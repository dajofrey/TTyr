// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "ttyr-api.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <elf.h>
#include <link.h>
#include <string.h>

// FUNCTIONS =======================================================================================

char TTYR_API_PATH_P[255] = {NULL};

void ttyr_api_initialize() 
{
    const ElfW(Dyn) *dyn = _DYNAMIC;
    const ElfW(Dyn) *rpath = NULL;
    const ElfW(Dyn) *runpath = NULL;
    const char *strtab = NULL;

    for (; dyn->d_tag != DT_NULL; ++dyn) {
        if (dyn->d_tag == DT_RPATH) {
            rpath = dyn;
        } else if (dyn->d_tag == DT_RUNPATH) {
            runpath = dyn;
        } else if (dyn->d_tag == DT_STRTAB) {
            strtab = (const char *)dyn->d_un.d_val;
        }
    }

    assert(strtab != NULL);

    if (rpath != NULL) {
        sprintf(TTYR_API_PATH_P, strtab + rpath->d_un.d_val);
    } else if (runpath != NULL) {
        sprintf(TTYR_API_PATH_P, strtab + runpath->d_un.d_val);
    }

    for (int i = 0; i < strlen(TTYR_API_PATH_P); ++i) {
        if (TTYR_API_PATH_P[i] == ':') {
            TTYR_API_PATH_P[i] = 0;
            break;
        }
    }
}
