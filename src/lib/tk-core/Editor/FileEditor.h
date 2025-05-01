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

    typedef struct tk_core_FileEditorView {
        nh_core_List FileViews;
        int currentOffset;
        int maxOnScreen;
        int fromLeft;
        int width;
        int helpScroll;
    } tk_core_FileEditorView;

    typedef struct tk_core_FileEditor {
        nh_LinkedList Files;
        nh_core_Array Copies;
        int current;
        int minCols;
        int tabSpaces;
        bool tabToSpaces;
    } tk_core_FileEditor;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    tk_core_FileEditor tk_core_initFileEditor(
    );

    TTYR_CORE_RESULT tk_core_freeFileEditor(
        tk_core_FileEditor *FileEditor_p
    );

    tk_core_File *tk_core_openFile(
        tk_core_Program *Program_p, tk_core_TreeListingNode *Node_p, bool readOnly
    );

    TTYR_CORE_RESULT tk_core_closeFile(
        tk_core_FileEditor *Editor_p, tk_core_File *File_p
    );

    TTYR_CORE_RESULT tk_core_cycleThroughFiles(
        tk_core_Program *Program_p, NH_API_UTF32 c
    );

    TTYR_CORE_RESULT tk_core_handleFileEditorInput(
        tk_core_Program *Program_p, NH_API_UTF32 c
    );

    TTYR_CORE_RESULT tk_core_drawFileEditorRow(
        tk_core_Program *Program_p, tk_core_Glyph *Glyphs_p, int width, int height, int row
    );

/** @} */

#endif 
