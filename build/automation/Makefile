# Define the compiler and compile flags
CC = gcc
CFLAGS = -fPIC -std=gnu99 -Wl,-rpath,$(CURDIR)/external/Netzhaut/lib

# Define the linker and linker flags
LD = gcc
LDFLAGS_TTYR_TTY = -lutil -Lexternal/st-0.8.5/ -l:libst.so
LDFLAGS_TTYR_TERMINAL =
LDFLAGS_TTYR = -Lexternal/Netzhaut/lib -lttyr-api -lnhapi

# Define the source file directory for each library
SRC_DIR_TTYR_TTY = src/lib/ttyr-tty
SRC_DIR_TTYR_TERMINAL = src/lib/ttyr-terminal
SRC_DIR_TTYR_API = src/lib/ttyr-api
SRC_DIR_TTYR = src/bin/ttyr
SRC_DIR_ST = external/st-0.8.5

# List of source files for each library
SRC_FILES_TTYR_TTY = \
    TTY/TTY.c \
    TTY/Draw.c \
    TTY/Program.c \
    TTY/Macro.c \
    TTY/SideBar.c \
    TTY/Titlebar.c \
    TTY/TopBar.c \
    TTY/TopBarMessage.c \
    TTY/Tiling.c \
    TTY/StandardIO.c \
    TTY/View.c \
    TTY/Micro.c \
    TTY/ContextMenu.c \
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
    Vulkan/Pipeline.c \
    Vulkan/Text.c \
    Vulkan/Render.c \
    OpenGL/Foreground.c \
    OpenGL/Background.c \
    OpenGL/Boxes.c \
    OpenGL/Render.c \
    Common/Result.c \
    Common/About.c \
    Common/Log.c \
    Common/Initialize.c \
    Common/Terminate.c \
    Common/Config.c \
	
SRC_FILES_TTYR_API = \
    ttyr-tty.c \
    ttyr-terminal.c \
 
SRC_FILES_TTYR = Main.c

SRC_FILES_ST = st.c

# Object files derived from source files for each library
OBJ_FILES_TTYR_TTY = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TTYR_TTY)/, $(SRC_FILES_TTYR_TTY)))
OBJ_FILES_TTYR_TERMINAL = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TTYR_TERMINAL)/, $(SRC_FILES_TTYR_TERMINAL)))
OBJ_FILES_TTYR_API = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TTYR_API)/, $(SRC_FILES_TTYR_API)))
OBJ_FILES_TTYR = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_TTYR)/, $(SRC_FILES_TTYR)))
OBJ_FILES_ST = $(patsubst %.c, %.o, $(addprefix $(SRC_DIR_ST)/, $(SRC_FILES_ST)))

# Name of the shared libraries and binaries
LIB_TTYR_TTY = external/Netzhaut/lib/libttyr-tty.so
LIB_TTYR_TERMINAL = external/Netzhaut/lib/libttyr-terminal.so
LIB_TTYR_API = external/Netzhaut/lib/libttyr-api.so
LIB_ST = external/st-0.8.5/libst.so
BIN_TTYR = bin/ttyr

build_netzhaut:
	(cd external/Netzhaut && make -f build/automation/Makefile nhapi.so nhcore.so nhencoding.so nhwsi.so nhgfx.so)
create_bin_dir:
	mkdir -p bin

# Build targets for each library
all: build_netzhaut $(LIB_ST) $(LIB_TTYR_TTY) $(LIB_TTYR_TERMINAL) $(LIB_TTYR_API) $(BIN_TTYR)
lib: build_netzhaut $(LIB_ST) $(LIB_TTYR_TTY) $(LIB_TTYR_TERMINAL) $(LIB_TTYR_API)
bin: build_netzhaut $(BIN_TTYR)

# Custom compiler flags
$(OBJ_FILES_TTYR_TERMINAL): CFLAGS += -Iexternal/Netzhaut/external -Iexternal/Netzhaut/src/lib -Iexternal/Netzhaut/external/Vulkan-Headers/include -DINCLUDE_VOLK -DVK_VERSION_1_2 -DVK_USE_PLATFORM_XLIB_KHR -DVK_KHR_xlib_surface
$(OBJ_FILES_TTYR_TTY): CFLAGS += -Iexternal/Netzhaut/external -Iexternal/Netzhaut/src/lib
$(OBJ_FILES_TTYR_API): CFLAGS += -Iexternal/Netzhaut/external -Iexternal/Netzhaut/src/lib 
$(OBJ_FILES_TTYR): CFLAGS += -Iexternal -Iexternal/Netzhaut/src/lib -Isrc/lib

# Rule to compile source files into object files
%.o: $(SRC_DIR_TTYR_TTY)/%.c
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
$(LIB_TTYR_TTY): $(OBJ_FILES_TTYR_TTY)
	$(LD) $(CFLAGS) -Wl,-rpath,':$(CURDIR)/external/st-0.8.5' -shared -o $@ $(OBJ_FILES_TTYR_TTY) $(LDFLAGS_TTYR_TTY)
$(LIB_TTYR_TERMINAL): $(OBJ_FILES_TTYR_TERMINAL)
	$(LD) $(CFLAGS) -shared -o $@ $(OBJ_FILES_TTYR_TERMINAL) $(LDFLAGS_TTYR_TERMINAL)
$(LIB_TTYR_API): $(OBJ_FILES_TTYR_API)
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

# Clean rule
clean:
	rm -f $(OBJ_FILES_TTYR_TTY) $(LIB_TTYR_TTY)
	rm -f $(OBJ_FILES_TTYR_TERMINAL) $(LIB_TTYR_TERMINAL)
	rm -f $(OBJ_FILES_TTYR_API) $(LIB_TTYR_API)
	rm -f $(OBJ_FILES_ST) $(LIB_ST)
	rm -f $(OBJ_FILES_TTYR) $(BIN_TTYR)
	rm -rf bin

.PHONY: all lib bin clean
