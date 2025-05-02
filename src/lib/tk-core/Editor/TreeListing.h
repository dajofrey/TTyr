#ifndef TK_CORE_TREE_LISTING_H
#define TK_CORE_TREE_LISTING_H

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

    typedef struct tk_core_TreeListingNodeOverflow {
        int offset;
        nh_core_SystemTime LastShift;
    } tk_core_TreeListingNodeOverflow;

    typedef struct tk_core_TreeListingNode {
        bool open;
        bool unsaved;
        tk_core_File *File_p;
        nh_encoding_UTF32String Path;
        nh_core_List Children; 
        struct tk_core_TreeListingNode *Parent_p;
        tk_core_TreeListingNodeOverflow Overflow;
    } tk_core_TreeListingNode;
 
    typedef struct tk_core_TreeListing {
        tk_core_TreeListingNode *Root_p;
        bool preview;
        bool dirty;
        tk_core_TreeListingNode *Preview_p;
        int current;
        nh_core_Array RenderLines;
        char wrkDir_p[2048];
    } tk_core_TreeListing;

    typedef struct tk_core_TreeListingView {
        int width;
        int current;
        int offset;
        int maxOffset;
    } tk_core_TreeListingView;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    tk_core_TreeListing tk_core_initTreeListing(
    );

    TK_CORE_RESULT tk_core_freeTreeListing(
        tk_core_TreeListing *TreeListing_p
    );

    TK_CORE_RESULT tk_core_handleTreeListingInput(
        tk_core_Program *Program_p, NH_API_UTF32 c
    );

    TK_CORE_RESULT tk_core_drawTreeListingRow(
        tk_core_Program *Program_p, tk_core_Glyph *Glyph_p, int width, int height, int row
    );

    TK_CORE_RESULT tk_core_setTreeListingCursor(
        tk_core_Program *Program_p, tk_core_File *File_p 
    );

    tk_core_TreeListingNode *tk_core_insertTreeListingNode(
        tk_core_TreeListing *Listing_p, NH_API_UTF32 *name_p, int length
    );

/** @} */

#endif 
