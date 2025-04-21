# Defaults
NETZHAUT_PATH ?= $(CURDIR)/external/Netzhaut

# Define the compiler and compile flags
CC = gcc
CFLAGS = -fPIC -std=gnu99 -Wl,-rpath,$(CURDIR)/lib,-rpath,$(NETZHAUT_PATH)/lib

# Define the linker and linker flags
LD = gcc
LDFLAGS_TTYR_CORE = -lutil -Lexternal/st-0.8.5/ -l:libst.so
LDFLAGS_TTYR_TERMINAL =
LDFLAGS_TTYR = -Llib -lttyr-api -L$(NETZHAUT_PATH)/lib -lnh-api

# Define the source file directory for each library
SRC_DIR_TTYR_CORE = src/lib/ttyr-core
SRC_DIR_TTYR_TERMINAL = src/lib/ttyr-terminal
SRC_DIR_TTYR_API = src/lib/ttyr-api
SRC_DIR_TTYR = src/bin/ttyr
SRC_DIR_ST = external/st-0.8.5

# List of source files for each library
SRC_FILES_TTYR_CORE = \
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

SRC_FILES_TTYR_TERMINAL = \
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
	
SRC_FILES_TTYR_API = \
    ttyr-api.c \
    ttyr-core.c \
    ttyr-terminal.c \
 
SRC_FILES_TTYR = Main.c

SRC_FILES_ST = st.c

# Object files derived from source files for each library
OBJ_FILES_TTYR_CORE = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TTYR_CORE)/, $(SRC_FILES_TTYR_CORE)))
OBJ_FILES_TTYR_TERMINAL = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TTYR_TERMINAL)/, $(SRC_FILES_TTYR_TERMINAL)))
OBJ_FILES_TTYR_API = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TTYR_API)/, $(SRC_FILES_TTYR_API)))
OBJ_FILES_TTYR = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TTYR)/, $(SRC_FILES_TTYR)))
OBJ_FILES_ST = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_ST)/, $(SRC_FILES_ST)))

# Name of the shared libraries and binaries
LIB_TTYR_CORE = lib/libttyr-core.so
LIB_TTYR_TERMINAL = lib/libttyr-terminal.so
LIB_TTYR_API = lib/libttyr-api.so
LIB_ST = external/st-0.8.5/libst.so
BIN_TTYR = bin/ttyr

# Build targets for each library
all: build_netzhaut $(LIB_ST) $(LIB_TTYR_CORE) $(LIB_TTYR_TERMINAL) $(LIB_TTYR_API) $(BIN_TTYR)
lib: build_netzhaut $(LIB_ST) $(LIB_TTYR_CORE) $(LIB_TTYR_TERMINAL) $(LIB_TTYR_API)
bin: build_netzhaut $(BIN_TTYR)

build_netzhaut:
ifeq ($(NETZHAUT_PATH),$(CURDIR)/external/Netzhaut)
	(cd external/Netzhaut && git submodule update --init --recursive && make -f build/automation/lib.mk nh-api.so nh-core.so nh-encoding.so nh-wsi.so nh-gfx.so)
endif
create_lib_dir:
	mkdir -p lib
create_bin_dir:
	mkdir -p bin

# Custom compiler flags
$(OBJ_FILES_TTYR_TERMINAL): CFLAGS += -I$(NETZHAUT_PATH)/external -I$(NETZHAUT_PATH)/src/lib -I$(NETZHAUT_PATH)/external/Vulkan-Headers/include -DINCLUDE_VOLK -DVK_VERSION_1_2 -DVK_USE_PLATFORM_XLIB_KHR -DVK_KHR_xlib_surface
$(OBJ_FILES_TTYR_CORE): CFLAGS += -I$(NETZHAUT_PATH)/external -I$(NETZHAUT_PATH)/src/lib
$(OBJ_FILES_TTYR_API): CFLAGS += -I$(NETZHAUT_PATH)/external -I$(NETZHAUT_PATH)/src/lib
$(OBJ_FILES_TTYR): CFLAGS += -Iexternal -I$(NETZHAUT_PATH)/src/lib -Isrc/lib

# Rule to compile source files into object files
%.o: $(SRC_DIR_TTYR_CORE)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: $(SRC_DIR_TTYR_TERMINAL)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: $(SRC_DIR_TTYR_API)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: $(SRC_DIR_TTYR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: $(SRC_DIR_ST)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Rule to link object files into the shared libraries
$(LIB_TTYR_CORE): create_lib_dir $(OBJ_FILES_TTYR_CORE)
	$(LD) $(CFLAGS) -Wl,-rpath,':$(CURDIR)/external/st-0.8.5' -shared -o $@ $(OBJ_FILES_TTYR_CORE) $(LDFLAGS_TTYR_CORE)
$(LIB_TTYR_TERMINAL): create_lib_dir $(OBJ_FILES_TTYR_TERMINAL)
	$(LD) $(CFLAGS) -shared -o $@ $(OBJ_FILES_TTYR_TERMINAL) $(LDFLAGS_TTYR_TERMINAL)
$(LIB_TTYR_API): create_lib_dir $(OBJ_FILES_TTYR_API)
	$(LD) $(CFLAGS) -shared -o $@ $(OBJ_FILES_TTYR_API) $(LDFLAGS_TTYR_API)
$(LIB_ST): $(OBJ_FILES_ST)
	$(LD) $(CFLAGS) -shared -o $@ $(OBJ_FILES_ST) $(LDFLAGS_ST)
$(BIN_TTYR): create_bin_dir $(OBJ_FILES_TTYR)
	$(CC) $(CFLAGS) -o $@ $(OBJ_FILES_TTYR) $(LDFLAGS_TTYR)

# Install rule
INSTALL_DIR = /usr/local/bin
install: $(BIN_TTYR)
	install -m 755 $(BIN_TTYR) $(INSTALL_DIR)
	cp build/data/desktop/ttyr.desktop /usr/share/applications/ttyr.desktop
	mkdir -p /usr/share/ttyr/icons
	cp build/data/icons/ttyr.png /usr/share/ttyr/icons

# Clean rules
clean-netzhaut:
	@if [ -d external/Netzhaut ] && [ -n "$$(ls -A external/Netzhaut 2>/dev/null)" ]; then \
		echo "Cleaning Netzhaut..."; \
		(cd external/Netzhaut && make -f build/automation/lib.mk clean); \
	else \
		echo "Skipping Netzhaut cleanup (directory missing or empty)."; \
	fi
clean: clean-netzhaut
	rm -f $(OBJ_FILES_TTYR_CORE) $(LIB_TTYR_CORE)
	rm -f $(OBJ_FILES_TTYR_TERMINAL) $(LIB_TTYR_TERMINAL)
	rm -f $(OBJ_FILES_TTYR_API) $(LIB_TTYR_API)
	rm -f $(OBJ_FILES_ST) $(LIB_ST)
	rm -f $(OBJ_FILES_TTYR) $(BIN_TTYR)
	rm -rf bin lib

.PHONY: all lib bin clean
