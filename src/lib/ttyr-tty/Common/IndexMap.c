// LICENSE NOTICE ==================================================================================

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

// INCLUDES ========================================================================================

#include "IndexMap.h"
#include "Macros.h"
#include "Config.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/System/Memory.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/Array.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/MediaType.h"

#include <string.h>

// DECLARE =========================================================================================

typedef enum TTYR_TTY_INDEXMAP_E {
    TTYR_TTY_INDEXMAP_SETTING_NAMES = 0,
    TTYR_TTY_INDEXMAP_E_COUNT,
} TTYR_TTY_INDEXMAP_E;

// DATA ============================================================================================

ttyr_tty_IndexMap TTYR_TTY_INDEXMAP;
static unsigned int *indices_pp[TTYR_TTY_INDEXMAP_E_COUNT] = {NULL};

// CREATE/DESTROY ==================================================================================

static TTYR_TTY_RESULT ttyr_tty_getNames(
    TTYR_TTY_INDEXMAP_E type, NH_BYTE ***array_ppp, int *count_p)
{
TTYR_TTY_BEGIN()

    switch (type)
    {
        case TTYR_TTY_INDEXMAP_SETTING_NAMES :
        {
            *array_ppp = (NH_BYTE**) TTYR_TTY_SETTING_NAMES_PP; 
            *count_p = TTYR_TTY_SETTING_NAMES_PP_COUNT; 
            break;
        }
        default : TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_ERROR_BAD_STATE)
    }

    if (*array_ppp == NULL) {TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_ERROR_BAD_STATE)}

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

static TTYR_TTY_RESULT nh_core_createSingleIndexMap(
    TTYR_TTY_INDEXMAP_E type, nh_HashMap *map_p)
{
TTYR_TTY_BEGIN()

    int count = 0; NH_BYTE **names_pp = NULL;
    TTYR_TTY_CHECK(ttyr_tty_getNames(type, &names_pp, &count))

    *map_p = nh_core_createHashMap();

    for (int i = 0; i < count; ++i) {
        TTYR_TTY_CHECK(nh_core_addToHashMap(map_p, names_pp[i], &indices_pp[type][i]))
    }

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

TTYR_TTY_RESULT ttyr_tty_createIndexMap()
{
TTYR_TTY_BEGIN()

    for (int type = 0; type < TTYR_TTY_INDEXMAP_E_COUNT; ++type)  
    {
        int count = 0;
        const NH_BYTE **names_pp = NULL;
        if (ttyr_tty_getNames(type, (NH_BYTE***)&names_pp, &count) != TTYR_TTY_SUCCESS) {
            TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_ERROR_BAD_STATE)
        }

        indices_pp[type] = nh_core_allocate(sizeof(unsigned int) * count);
        TTYR_TTY_CHECK_MEM(indices_pp[type])
        
        for (int i = 0; i < count; ++i) {
            indices_pp[type][i] = i;
        }
    }    

    TTYR_TTY_CHECK(nh_core_createSingleIndexMap(TTYR_TTY_INDEXMAP_SETTING_NAMES, &TTYR_TTY_INDEXMAP.SettingNames))

TTYR_TTY_DIAGNOSTIC_END(TTYR_TTY_SUCCESS)
}

void ttyr_tty_freeIndexMap()
{
TTYR_TTY_BEGIN()

    for (int type = 0; type < TTYR_TTY_INDEXMAP_E_COUNT; ++type)  
    {
         nh_core_free(indices_pp[type]);
         indices_pp[type] = NULL;
    }

    nh_core_freeHashMap(TTYR_TTY_INDEXMAP.SettingNames);

TTYR_TTY_SILENT_END()
}

