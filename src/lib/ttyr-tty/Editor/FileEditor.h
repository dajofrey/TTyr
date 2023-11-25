#ifndef TTYR_TTY_FILE_EDITOR_H
#define TTYR_TTY_FILE_EDITOR_H

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

    typedef struct ttyr_tty_FileEditorView {
        nh_List FileViews;
        int currentOffset;
        int maxOnScreen;
        int fromLeft;
        int width;
        int helpScroll;
    } ttyr_tty_FileEditorView;

    typedef struct ttyr_tty_FileEditor {
        nh_LinkedList Files;
        nh_Array Copies;
        int current;
        int minCols;
        int tabSpaces;
        NH_BOOL tabToSpaces;
    } ttyr_tty_FileEditor;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_FileEditor ttyr_tty_initFileEditor(
    );

    TTYR_TTY_RESULT ttyr_tty_freeFileEditor(
        ttyr_tty_FileEditor *FileEditor_p
    );

    ttyr_tty_File *ttyr_tty_openFile(
        ttyr_tty_Program *Program_p, ttyr_tty_TreeListingNode *Node_p, NH_BOOL readOnly
    );

    TTYR_TTY_RESULT ttyr_tty_closeFile(
        ttyr_tty_FileEditor *Editor_p, ttyr_tty_File *File_p
    );

    TTYR_TTY_RESULT ttyr_tty_cycleThroughFiles(
        ttyr_tty_Program *Program_p, NH_ENCODING_UTF32 c
    );

    TTYR_TTY_RESULT ttyr_tty_handleFileEditorInput(
        ttyr_tty_Program *Program_p, NH_ENCODING_UTF32 c
    );

    TTYR_TTY_RESULT ttyr_tty_drawFileEditorRow(
        ttyr_tty_Program *Program_p, ttyr_tty_Glyph *Glyphs_p, int width, int height, int row
    );

/** @} */

#endif 
