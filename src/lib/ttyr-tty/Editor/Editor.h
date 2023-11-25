#ifndef TTYR_TTY_EDITOR_H
#define TTYR_TTY_EDITOR_H

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

#include "../../../../external/Netzhaut/src/lib/nhcore/Util/String.h"

#endif

/** @addtogroup lib_nhtty_structs
 *  @{
 */

    typedef struct ttyr_tty_EditorView {
        ttyr_tty_FileEditorView FileEditor;
        ttyr_tty_TreeListingView TreeListing;
        int height;
    } ttyr_tty_EditorView;

    typedef struct ttyr_tty_Editor {
        ttyr_tty_EditorView View;
        int focus;
        NH_BOOL insertMode;
        NH_BOOL treeListing;
        ttyr_tty_TreeListing TreeListing;
        ttyr_tty_FileEditor FileEditor;
        nh_SystemTime LastUpdate;
        double updateIntervalInSeconds;
    } ttyr_tty_Editor;

/** @} */

/** @addtogroup lib_nhtty_functions
 *  @{
 */

    ttyr_tty_ProgramPrototype *ttyr_tty_createEditorPrototype(
    );

/** @} */

#endif 
