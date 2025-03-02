#ifndef TTYR_CORE_FILE_H
#define TTYR_CORE_FILE_H

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

typedef struct ttyr_core_TreeListingNode ttyr_core_TreeListingNode;
typedef struct ttyr_core_EditorView ttyr_core_EditorView;
typedef struct ttyr_core_FileEditorView ttyr_core_FileEditorView;

#endif

/** @addtogroup lib_nhtty_enums
 *  @{
 */

    typedef enum TTYR_CORE_FILE {
        TTYR_CORE_FILE_UNDEFINED,
        TTYR_CORE_FILE_TEXT,
        TTYR_CORE_FILE_CHANGES,
    } TTYR_CORE_FILE;

/** @} */


/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_core_File {
        ttyr_core_TreeListingNode *Node_p;
        TTYR_CORE_FILE type;
        void *handle_p;
        bool readOnly;
    } ttyr_core_File;

    typedef struct ttyr_core_TextFileView {
        int screenCursorX;
        int screenCursorY;
        int rowOffset;
        int colOffset;
    } ttyr_core_TextFileView;

    typedef struct ttyr_core_FileView {
        ttyr_core_File *File_p;
        ttyr_core_TextFileView TextFile;
        int height;
        int width;
    } ttyr_core_FileView;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_core_FileView ttyr_core_initFileView(
        ttyr_core_File *File_p
    );
    
    TTYR_CORE_RESULT ttyr_core_createFileViews(
        ttyr_core_FileEditorView *View_p, ttyr_core_File *File_p
    );
    
    TTYR_CORE_RESULT ttyr_core_destroyFileViews(
        ttyr_core_FileEditorView *View_p, ttyr_core_File *File_p
    );
    
    ttyr_core_FileView *ttyr_core_getFileView(
        ttyr_core_EditorView *View_p, ttyr_core_File *File_p
    );
    
    nh_core_List ttyr_core_getFileViews(
        ttyr_core_Program *Program_p, ttyr_core_File *File_p
    );
    
    TTYR_CORE_RESULT ttyr_core_updateFileViews(
        ttyr_core_EditorView *View_p
    );
    
    TTYR_CORE_FILE ttyr_core_getFileType(
        nh_encoding_UTF32String *Path_p
    );

    TTYR_CORE_RESULT ttyr_core_renderFile(
        ttyr_core_File *File_p
    );
    
    TTYR_CORE_RESULT ttyr_core_writeFile(
        ttyr_core_File *File_p
    );
    
    TTYR_CORE_RESULT ttyr_core_clearFileSearch(
        ttyr_core_File *File_p
    );
    
    TTYR_CORE_RESULT ttyr_core_searchFile(
        ttyr_core_File *File_p, NH_API_UTF32 *str_p, int length
    );
    
    TTYR_CORE_RESULT ttyr_core_handleFileInput(
        ttyr_core_Program *Program_p, ttyr_core_File *File_p, NH_API_UTF32 c, bool insertMode, 
        bool *refresh_p
    );

    TTYR_CORE_RESULT ttyr_core_drawFileRow(
        ttyr_core_Program *Program_p, ttyr_core_File *File_p, ttyr_core_FileView *View_p, ttyr_core_Glyph *Glyphs_p,
        int row
    );

/** @} */

#endif 
