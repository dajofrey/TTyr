#ifndef TTYR_TTY_TREE_LISTING_H
#define TTYR_TTY_TREE_LISTING_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "FileEditor.h"

#include "../Common/Includes.h"
#include "../TTY/Program.h"

#include "../../../../external/Netzhaut/src/lib/nhcore/Util/Time.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/List.h"
#include "../../../../external/Netzhaut/src/lib/nhcore/Util/String.h"

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_TreeListingNodeOverflow {
        int offset;
        nh_SystemTime LastShift;
    } ttyr_tty_TreeListingNodeOverflow;

    typedef struct ttyr_tty_TreeListingNode {
        NH_BOOL open;
        NH_BOOL unsaved;
        ttyr_tty_File *File_p;
        nh_encoding_UTF32String Path;
        nh_List Children; 
        struct ttyr_tty_TreeListingNode *Parent_p;
        ttyr_tty_TreeListingNodeOverflow Overflow;
    } ttyr_tty_TreeListingNode;
 
    typedef struct ttyr_tty_TreeListing {
        ttyr_tty_TreeListingNode *Root_p;
        NH_BOOL preview;
        NH_BOOL dirty;
        ttyr_tty_TreeListingNode *Preview_p;
        int current;
        nh_Array RenderLines;
        NH_BYTE wrkDir_p[2048];
    } ttyr_tty_TreeListing;

    typedef struct ttyr_tty_TreeListingView {
        int width;
        int current;
        int offset;
        int maxOffset;
    } ttyr_tty_TreeListingView;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_TreeListing ttyr_tty_initTreeListing(
    );

    TTYR_TTY_RESULT ttyr_tty_freeTreeListing(
        ttyr_tty_TreeListing *TreeListing_p
    );

    TTYR_TTY_RESULT ttyr_tty_handleTreeListingInput(
        ttyr_tty_Program *Program_p, NH_ENCODING_UTF32 c
    );

    TTYR_TTY_RESULT ttyr_tty_drawTreeListingRow(
        ttyr_tty_Program *Program_p, ttyr_tty_Glyph *Glyph_p, int width, int height, int row
    );

    TTYR_TTY_RESULT ttyr_tty_setTreeListingCursor(
        ttyr_tty_Program *Program_p, ttyr_tty_File *File_p 
    );

    ttyr_tty_TreeListingNode *ttyr_tty_insertTreeListingNode(
        ttyr_tty_TreeListing *Listing_p, NH_ENCODING_UTF32 *name_p, int length
    );

/** @} */

#endif 
