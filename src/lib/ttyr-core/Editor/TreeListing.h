#ifndef TTYR_CORE_TREE_LISTING_H
#define TTYR_CORE_TREE_LISTING_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "FileEditor.h"

#include "../Common/Includes.h"
#include "../TTY/Program.h"

#include "nh-core/Util/Time.h"
#include "nh-core/Util/List.h"
#include "nh-core/Util/String.h"

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_core_TreeListingNodeOverflow {
        int offset;
        nh_core_SystemTime LastShift;
    } ttyr_core_TreeListingNodeOverflow;

    typedef struct ttyr_core_TreeListingNode {
        bool open;
        bool unsaved;
        ttyr_core_File *File_p;
        nh_encoding_UTF32String Path;
        nh_core_List Children; 
        struct ttyr_core_TreeListingNode *Parent_p;
        ttyr_core_TreeListingNodeOverflow Overflow;
    } ttyr_core_TreeListingNode;
 
    typedef struct ttyr_core_TreeListing {
        ttyr_core_TreeListingNode *Root_p;
        bool preview;
        bool dirty;
        ttyr_core_TreeListingNode *Preview_p;
        int current;
        nh_core_Array RenderLines;
        char wrkDir_p[2048];
    } ttyr_core_TreeListing;

    typedef struct ttyr_core_TreeListingView {
        int width;
        int current;
        int offset;
        int maxOffset;
    } ttyr_core_TreeListingView;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_core_TreeListing ttyr_core_initTreeListing(
    );

    TTYR_CORE_RESULT ttyr_core_freeTreeListing(
        ttyr_core_TreeListing *TreeListing_p
    );

    TTYR_CORE_RESULT ttyr_core_handleTreeListingInput(
        ttyr_core_Program *Program_p, NH_API_UTF32 c
    );

    TTYR_CORE_RESULT ttyr_core_drawTreeListingRow(
        ttyr_core_Program *Program_p, ttyr_core_Glyph *Glyph_p, int width, int height, int row
    );

    TTYR_CORE_RESULT ttyr_core_setTreeListingCursor(
        ttyr_core_Program *Program_p, ttyr_core_File *File_p 
    );

    ttyr_core_TreeListingNode *ttyr_core_insertTreeListingNode(
        ttyr_core_TreeListing *Listing_p, NH_API_UTF32 *name_p, int length
    );

/** @} */

#endif 
