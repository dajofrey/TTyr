#ifndef TK_CORE_EDITOR_H
#define TK_CORE_EDITOR_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * TTýr - Terminal Emulator
 * Copyright (C) 2022  Dajo Frey
 * Published under GNU LGPL. See TTyr/LICENSE.LGPL file.
 */

#include "TreeListing.h"
#include "FileEditor.h"

#include "../TTY/Program.h"
#include "../Common/Includes.h"

#include "nh-core/Util/String.h"

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct tk_core_EditorView {
        tk_core_FileEditorView FileEditor;
        tk_core_TreeListingView TreeListing;
        int height;
    } tk_core_EditorView;

    typedef struct tk_core_Editor {
        tk_core_EditorView View;
        int focus;
        bool insertMode;
        bool treeListing;
        tk_core_TreeListing TreeListing;
        tk_core_FileEditor FileEditor;
        nh_core_SystemTime LastUpdate;
        double updateIntervalInSeconds;
    } tk_core_Editor;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    tk_core_Interface *tk_core_createEditorPrototype(
    );

/** @} */

#endif 
