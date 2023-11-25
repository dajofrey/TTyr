#ifndef TTYR_TTY_INDEXMAP_H
#define TTYR_TTY_INDEXMAP_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "Includes.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/HashMap.h"

#endif

/** @addtogroup lib_nhcore_structs
 *  @{
 */

    typedef struct ttyr_tty_IndexMap {
        nh_HashMap SettingNames;
    } ttyr_tty_IndexMap;

/** @} */

/** @addtogroup lib_nhcore_vars
 *  @{
 */

    extern ttyr_tty_IndexMap TTYR_TTY_INDEXMAP;

/** @} */

/** @addtogroup lib_nhcore_functions
 *  @{
 */

    TTYR_TTY_RESULT ttyr_tty_createIndexMap(
    );

    void ttyr_tty_freeIndexMap(
    );

/** @} */

#endif // TTYR_TTY_INDEXMAP_H 
