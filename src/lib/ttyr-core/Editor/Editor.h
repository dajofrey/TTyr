#ifndef TTYR_CORE_EDITOR_H
#define TTYR_CORE_EDITOR_H

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

    typedef struct ttyr_core_EditorView {
        ttyr_core_FileEditorView FileEditor;
        ttyr_core_TreeListingView TreeListing;
        int height;
    } ttyr_core_EditorView;

    typedef struct ttyr_core_Editor {
        ttyr_core_EditorView View;
        int focus;
        bool insertMode;
        bool treeListing;
        ttyr_core_TreeListing TreeListing;
        ttyr_core_FileEditor FileEditor;
        nh_core_SystemTime LastUpdate;
        double updateIntervalInSeconds;
    } ttyr_core_Editor;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_core_Interface *ttyr_core_createEditorPrototype(
    );

/** @} */

#endif 
