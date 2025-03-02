#ifndef TTYR_CORE_FILE_EDITOR_H
#define TTYR_CORE_FILE_EDITOR_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "File.h"

#include "../TTY/Program.h"
#include "../Common/Includes.h"

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_core_FileEditorView {
        nh_core_List FileViews;
        int currentOffset;
        int maxOnScreen;
        int fromLeft;
        int width;
        int helpScroll;
    } ttyr_core_FileEditorView;

    typedef struct ttyr_core_FileEditor {
        nh_LinkedList Files;
        nh_core_Array Copies;
        int current;
        int minCols;
        int tabSpaces;
        bool tabToSpaces;
    } ttyr_core_FileEditor;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_core_FileEditor ttyr_core_initFileEditor(
    );

    TTYR_CORE_RESULT ttyr_core_freeFileEditor(
        ttyr_core_FileEditor *FileEditor_p
    );

    ttyr_core_File *ttyr_core_openFile(
        ttyr_core_Program *Program_p, ttyr_core_TreeListingNode *Node_p, bool readOnly
    );

    TTYR_CORE_RESULT ttyr_core_closeFile(
        ttyr_core_FileEditor *Editor_p, ttyr_core_File *File_p
    );

    TTYR_CORE_RESULT ttyr_core_cycleThroughFiles(
        ttyr_core_Program *Program_p, NH_API_UTF32 c
    );

    TTYR_CORE_RESULT ttyr_core_handleFileEditorInput(
        ttyr_core_Program *Program_p, NH_API_UTF32 c
    );

    TTYR_CORE_RESULT ttyr_core_drawFileEditorRow(
        ttyr_core_Program *Program_p, ttyr_core_Glyph *Glyphs_p, int width, int height, int row
    );

/** @} */

#endif 
