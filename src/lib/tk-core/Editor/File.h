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

typedef struct tk_core_TreeListingNode tk_core_TreeListingNode;
typedef struct tk_core_EditorView tk_core_EditorView;
typedef struct tk_core_FileEditorView tk_core_FileEditorView;

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

    typedef struct tk_core_File {
        tk_core_TreeListingNode *Node_p;
        TTYR_CORE_FILE type;
        void *handle_p;
        bool readOnly;
    } tk_core_File;

    typedef struct tk_core_TextFileView {
        int screenCursorX;
        int screenCursorY;
        int rowOffset;
        int colOffset;
    } tk_core_TextFileView;

    typedef struct tk_core_FileView {
        tk_core_File *File_p;
        tk_core_TextFileView TextFile;
        int height;
        int width;
    } tk_core_FileView;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    tk_core_FileView tk_core_initFileView(
        tk_core_File *File_p
    );
    
    TTYR_CORE_RESULT tk_core_createFileViews(
        tk_core_FileEditorView *View_p, tk_core_File *File_p
    );
    
    TTYR_CORE_RESULT tk_core_destroyFileViews(
        tk_core_FileEditorView *View_p, tk_core_File *File_p
    );
    
    tk_core_FileView *tk_core_getFileView(
        tk_core_EditorView *View_p, tk_core_File *File_p
    );
    
    nh_core_List tk_core_getFileViews(
        tk_core_Program *Program_p, tk_core_File *File_p
    );
    
    TTYR_CORE_RESULT tk_core_updateFileViews(
        tk_core_EditorView *View_p
    );
    
    TTYR_CORE_FILE tk_core_getFileType(
        nh_encoding_UTF32String *Path_p
    );

    TTYR_CORE_RESULT tk_core_renderFile(
        tk_core_File *File_p
    );
    
    TTYR_CORE_RESULT tk_core_writeFile(
        tk_core_File *File_p
    );
    
    TTYR_CORE_RESULT tk_core_clearFileSearch(
        tk_core_File *File_p
    );
    
    TTYR_CORE_RESULT tk_core_searchFile(
        tk_core_File *File_p, NH_API_UTF32 *str_p, int length
    );
    
    TTYR_CORE_RESULT tk_core_handleFileInput(
        tk_core_Program *Program_p, tk_core_File *File_p, NH_API_UTF32 c, bool insertMode, 
        bool *refresh_p
    );

    TTYR_CORE_RESULT tk_core_drawFileRow(
        tk_core_Program *Program_p, tk_core_File *File_p, tk_core_FileView *View_p, tk_core_Glyph *Glyphs_p,
        int row
    );

/** @} */

#endif 
