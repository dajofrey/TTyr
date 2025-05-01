#ifndef TTYR_CORE_ABOUT_H
#define TTYR_CORE_ABOUT_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#endif

/** @defgroup lib_nhtty nhtty
 *  @ingroup lib
 *  @brief The nhtty library implements a terminal based interface. 
 *  Linux implementation based on https://viewsourcecode.org/snaptoken/kilo/index.html from Pailey with 
 *  original code by Salvatore Sanfilippo, also known as antirez.
 */

/** @defgroup lib_nhtty_version Version
 *  @ingroup lib_nhtty
 */
/** @defgroup lib_nhtty_macros Macros 
 *  @ingroup lib_nhtty
 */
/** @defgroup lib_nhtty_enums Enumerations
 *  @ingroup lib_nhtty
 */
/** @defgroup lib_nhtty_structs Data Structures
 *  @ingroup lib_nhtty
 */
/** @defgroup lib_nhtty_functions Functions
 *  @ingroup lib_nhtty
 */
/** @defgroup lib_nhtty_typedefs Typedefs
 *  @ingroup lib_nhtty
 */
/** @defgroup lib_nhtty_vars Variables
 *  @ingroup lib_nhtty
 */

/** @addtogroup lib_nhtty_version
 *  @{
 */

    /**
     * The API version is used for backwards-incompatible API changes. 
     */
    #define TTYR_CORE_API_VERSION 0

    /**
     * The major version is used for the completion of big functionalities. 
     */
    #define TTYR_CORE_MAJOR_VERSION 2

    /**
     * The minor version is used for the addition of small functionalities or consequential changes. 
     */
    #define TTYR_CORE_MINOR_VERSION 0

    /**
     * The patch version is used for bugfixes or non-consequential changes. 
     */
    #define TTYR_CORE_PATCH_VERSION 2

    /**
     * Version array.
     */
    extern int TTYR_CORE_VERSION_P[4];

/** @} */

#endif
