#ifndef TTYR_TTY_FILE_H
#define TTYR_TTY_FILE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "nh-core/Util/LinkedList.h"
#include "nh-core/Util/Array.h"
#include "nh-encoding/Encodings/UTF32.h"

#include "../TTY/Program.h"
#include "../Common/Includes.h"

typedef struct ttyr_tty_TreeListingNode ttyr_tty_TreeListingNode;
typedef struct ttyr_tty_EditorView ttyr_tty_EditorView;
typedef struct ttyr_tty_FileEditorView ttyr_tty_FileEditorView;

#endif

/** @addtogroup lib_nhtty_enums
 *  @{
 */

    typedef enum TTYR_TTY_FILE {
        TTYR_TTY_FILE_UNDEFINED,
        TTYR_TTY_FILE_TEXT,
        TTYR_TTY_FILE_CHANGES,
    } TTYR_TTY_FILE;

/** @} */


/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_File {
        ttyr_tty_TreeListingNode *Node_p;
        TTYR_TTY_FILE type;
        void *handle_p;
        bool readOnly;
    } ttyr_tty_File;

    typedef struct ttyr_tty_TextFileView {
        int screenCursorX;
        int screenCursorY;
        int rowOffset;
        int colOffset;
    } ttyr_tty_TextFileView;

    typedef struct ttyr_tty_FileView {
        ttyr_tty_File *File_p;
        ttyr_tty_TextFileView TextFile;
        int height;
        int width;
    } ttyr_tty_FileView;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_FileView ttyr_tty_initFileView(
        ttyr_tty_File *File_p
    );
    
    TTYR_TTY_RESULT ttyr_tty_createFileViews(
        ttyr_tty_FileEditorView *View_p, ttyr_tty_File *File_p
    );
    
    TTYR_TTY_RESULT ttyr_tty_destroyFileViews(
        ttyr_tty_FileEditorView *View_p, ttyr_tty_File *File_p
    );
    
    ttyr_tty_FileView *ttyr_tty_getFileView(
        ttyr_tty_EditorView *View_p, ttyr_tty_File *File_p
    );
    
    nh_core_List ttyr_tty_getFileViews(
        ttyr_tty_Program *Program_p, ttyr_tty_File *File_p
    );
    
    TTYR_TTY_RESULT ttyr_tty_updateFileViews(
        ttyr_tty_EditorView *View_p
    );
    
    TTYR_TTY_FILE ttyr_tty_getFileType(
        nh_encoding_UTF32String *Path_p
    );

    TTYR_TTY_RESULT ttyr_tty_renderFile(
        ttyr_tty_File *File_p
    );
    
    TTYR_TTY_RESULT ttyr_tty_writeFile(
        ttyr_tty_File *File_p
    );
    
    TTYR_TTY_RESULT ttyr_tty_clearFileSearch(
        ttyr_tty_File *File_p
    );
    
    TTYR_TTY_RESULT ttyr_tty_searchFile(
        ttyr_tty_File *File_p, NH_API_UTF32 *str_p, int length
    );
    
    TTYR_TTY_RESULT ttyr_tty_handleFileInput(
        ttyr_tty_Program *Program_p, ttyr_tty_File *File_p, NH_API_UTF32 c, bool insertMode, 
        bool *refresh_p
    );

    TTYR_TTY_RESULT ttyr_tty_drawFileRow(
        ttyr_tty_Program *Program_p, ttyr_tty_File *File_p, ttyr_tty_FileView *View_p, ttyr_tty_Glyph *Glyphs_p,
        int row
    );

/** @} */

#endif 
