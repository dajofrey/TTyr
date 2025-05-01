# Defaults
NETZHAUT_PATH ?= $(CURDIR)/external/Netzhaut

# Define the compiler and compile flags
CC = gcc
CFLAGS = -fPIC -std=gnu99 -Wl,-rpath,$(CURDIR)/lib,-rpath,$(NETZHAUT_PATH)/lib

# Define the linker and linker flags
LD = gcc
LDFLAGS_TK_CORE = -lutil -Lexternal/st-0.8.5/ -l:libst.so
LDFLAGS_TK_TERMINAL =
LDFLAGS_TK = -Llib -ltk-api -L$(NETZHAUT_PATH)/lib -lnh-api

# Define the source file directory for each library
SRC_DIR_TK_CORE = src/lib/tk-core
SRC_DIR_TK_TERMINAL = src/lib/tk-terminal
SRC_DIR_TK_API = src/lib/tk-api
SRC_DIR_TK = src/bin/ttyr
SRC_DIR_ST = external/st-0.8.5

# List of source files for each library
SRC_FILES_TK_CORE = \
    TTY/TTY.c \
    TTY/Draw.c \
    TTY/Program.c \
    TTY/Macro.c \
    TTY/Titlebar.c \
    TTY/Topbar.c \
    TTY/TopbarMessage.c \
    TTY/Tiling.c \
    TTY/StandardIO.c \
    TTY/View.c \
    TTY/Micro.c \
    TTY/ContextMenu.c \
    TTY/Menu.c \
    Editor/TreeListing.c \
    Editor/Editor.c \
    Editor/FileEditor.c \
    Editor/File.c \
    Editor/TextFile.c \
    Editor/TextFileInput.c \
    Editor/SyntaxHighlights.c \
    Shell/Shell.c \
    Shell/Socket.c \
    Common/Result.c \
    Common/About.c \
    Common/Log.c \
    Common/Config.c \
    Common/Initialize.c \
    Common/Terminate.c \

SRC_FILES_TK_TERMINAL = \
    Terminal/Terminal.c \
    Terminal/Grid.c \
    Terminal/Graphics.c \
    Terminal/Vertices.c \
    Terminal/Color.c \
    Vulkan/Pipeline.c \
    Vulkan/Text.c \
    Vulkan/Render.c \
    OpenGL/Foreground.c \
    OpenGL/Background.c \
    OpenGL/Boxes.c \
    OpenGL/Render.c \
    OpenGL/Dim.c \
    Common/Result.c \
    Common/About.c \
    Common/Log.c \
    Common/Initialize.c \
    Common/Terminate.c \
    Common/Config.c \
	
SRC_FILES_TK_API = \
    tk-api.c \
    tk-core.c \
    tk-terminal.c \
 
SRC_FILES_TK = Main.c

SRC_FILES_ST = st.c

# Object files derived from source files for each library
OBJ_FILES_TK_CORE = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TK_CORE)/, $(SRC_FILES_TK_CORE)))
OBJ_FILES_TK_TERMINAL = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TK_TERMINAL)/, $(SRC_FILES_TK_TERMINAL)))
OBJ_FILES_TK_API = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TK_API)/, $(SRC_FILES_TK_API)))
OBJ_FILES_TK = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TK)/, $(SRC_FILES_TK)))
OBJ_FILES_ST = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_ST)/, $(SRC_FILES_ST)))

# Name of the shared libraries and binaries
LIB_TK_CORE = lib/libtk-core.so
LIB_TK_TERMINAL = lib/libtk-terminal.so
LIB_TK_API = lib/libtk-api.so
LIB_ST = external/st-0.8.5/libst.so
BIN_TK = bin/termoskanne

# Build targets for each library
all: build_netzhaut $(LIB_ST) $(LIB_TK_CORE) $(LIB_TK_TERMINAL) $(LIB_TK_API) $(BIN_TK)
lib: build_netzhaut $(LIB_ST) $(LIB_TK_CORE) $(LIB_TK_TERMINAL) $(LIB_TK_API)
bin: build_netzhaut $(BIN_TK)

build_netzhaut:
ifeq ($(NETZHAUT_PATH),$(CURDIR)/external/Netzhaut)
	(cd external/Netzhaut && git submodule update --init --recursive && make -f build/automation/lib.mk lib-nh-api lib-nh-core lib-nh-encoding lib-nh-wsi lib-nh-gfx)
endif
create_lib_dir:
	mkdir -p lib
create_bin_dir:
	mkdir -p bin

# Custom compiler flags
$(OBJ_FILES_TK_TERMINAL): CFLAGS += -I$(NETZHAUT_PATH)/external -I$(NETZHAUT_PATH)/src/lib -I$(NETZHAUT_PATH)/external/Vulkan-Headers/include -DINCLUDE_VOLK -DVK_VERSION_1_2 -DVK_USE_PLATFORM_XLIB_KHR -DVK_KHR_xlib_surface
$(OBJ_FILES_TK_CORE): CFLAGS += -I$(NETZHAUT_PATH)/external -I$(NETZHAUT_PATH)/src/lib
$(OBJ_FILES_TK_API): CFLAGS += -I$(NETZHAUT_PATH)/external -I$(NETZHAUT_PATH)/src/lib
$(OBJ_FILES_TK): CFLAGS += -Iexternal -I$(NETZHAUT_PATH)/src/lib -Isrc/lib

# Rule to compile source files into object files
%.o: $(SRC_DIR_TK_CORE)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: $(SRC_DIR_TK_TERMINAL)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: $(SRC_DIR_TK_API)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: $(SRC_DIR_TK)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: $(SRC_DIR_ST)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to link object files into the shared libraries
$(LIB_TK_CORE): create_lib_dir $(OBJ_FILES_TK_CORE)
	$(LD) $(CFLAGS) -Wl,-rpath,':$(CURDIR)/external/st-0.8.5' -shared -o $@ $(OBJ_FILES_TK_CORE) $(LDFLAGS_TK_CORE)
$(LIB_TK_TERMINAL): create_lib_dir $(OBJ_FILES_TK_TERMINAL)
	$(LD) $(CFLAGS) -shared -o $@ $(OBJ_FILES_TK_TERMINAL) $(LDFLAGS_TK_TERMINAL)
$(LIB_TK_API): create_lib_dir $(OBJ_FILES_TK_API)
	$(LD) $(CFLAGS) -shared -o $@ $(OBJ_FILES_TK_API) $(LDFLAGS_TK_API)
$(LIB_ST): $(OBJ_FILES_ST)
	$(LD) $(CFLAGS) -shared -o $@ $(OBJ_FILES_ST) $(LDFLAGS_ST)
$(BIN_TK): create_bin_dir $(OBJ_FILES_TK)
	$(CC) $(CFLAGS) -o $@ $(OBJ_FILES_TK) $(LDFLAGS_TK)

# Install rule
INSTALL_DIR = /usr/local/bin
install: $(BIN_TK)
	install -m 755 $(BIN_TK) $(INSTALL_DIR)
	cp build/data/desktop/termoskanne.desktop /usr/share/applications/termoskanne.desktop
	mkdir -p /usr/share/termoskanne/icons
	cp build/data/icons/gradient512x512.png /usr/share/termoskanne/icons
	cp build/data/icons/gradient512x512.png /usr/share/pixmaps
	cp build/data/icons/gradient256x256.png /usr/share/icons/hicolor/256x256/apps/termoskanne.png
	cp build/data/icons/gradient128x128.png /usr/share/icons/hicolor/128x128/apps/termoskanne.png
	cp build/data/icons/64x64.png /usr/share/icons/hicolor/64x64/apps/termoskanne.png
	cp build/data/icons/32x32.png /usr/share/icons/hicolor/32x32/apps/termoskanne.png

# Clean rules
clean-netzhaut:
	@if [ -d external/Netzhaut ] && [ -n "$$(ls -A external/Netzhaut 2>/dev/null)" ]; then \
		echo "Cleaning Netzhaut..."; \
		(cd external/Netzhaut && make -f build/automation/lib.mk clean); \
	else \
		echo "Skipping Netzhaut cleanup (directory missing or empty)."; \
	fi
clean: clean-netzhaut
	rm -f $(OBJ_FILES_TK_CORE) $(LIB_TK_CORE)
	rm -f $(OBJ_FILES_TK_TERMINAL) $(LIB_TK_TERMINAL)
	rm -f $(OBJ_FILES_TK_API) $(LIB_TK_API)
	rm -f $(OBJ_FILES_ST) $(LIB_ST)
	rm -f $(OBJ_FILES_TK) $(BIN_TK)
	rm -rf bin lib

.PHONY: all lib bin clean
