<p align="center">
 <img src="./build/icons/128x128.png"/>
</p>

# Termoskanne

Terminal Emulator  

## README Content

 - [Compatibilities](#Compatibilities)
 - [Dependencies](#Dependencies)
 - [Build](#Build)
 - [Design](#Design)
 - [Development](#Development)

## Compatibilities

| Operating System | Compatible? |
| --- | --- |
| Linux | Yes |
| MacOS |  No |
| Windows |  No |

| Window System | Compatible? |
| --- | --- |
| X11 |  Yes |
| Wayland | No |

| Graphics Driver | Compatible? |
| --- | --- |
| OpenGL | Yes |
| Vulkan | No |
| DirectX | No |

## Dependencies

The following dependencies **must be present** on the system. 

 * [Git](https://git-scm.com)  
For submodules.
 * [Make](https://www.gnu.org/software/make)  
For building. 

The following dependencies **are automatically compiled** during the build process.

 * [Netzhaut](https://github.com/dajofrey/netzhaut)  
TTýr is companion project to [Netzhaut](https://github.com/dajofrey/netzhaut), from which it requires nh-api, nh-core, nh-encoding, nh-wsi and nh-gfx modules.
 * [st](https://st.suckless.org/)  
For shell functionality, TTýr uses parts of the great [st](https://st.suckless.org/).   

## Build

### Linux

```bash
git clone https://github.com/dajofrey/termoskanne
cd termoskanne && git submodule update --init --recursive    
make -f build/automation/Makefile all
```

## Design

### Noiseless UI
Termoskanne's graphics focus on mono-space font drawing, which is primarily used in terminals. Even Termoskanne's UI is made using the same technique. This makes everything look proportional and clean, even after scaling. Also, it simplifies development a lot. 
   
A big failure point when designing a UI is clutter. Sometimes even terminals have too much UI elements which make the experience distracting and annoying. Termoskanne tries to avoid this.

### Tabing and Tiling
Termoskanne uses a hybrid approach for tabing and tiling:   
  
The user can tab between windows, and split these windows into tiles. This is quite common. However, each tile can be tabbed and tiled individually.

#### Rules

* Termoskanne consists of multiple Macro-Windows. 
* Macro-Windows can be macro-split into multiple Macro-Tiles.
* Macro-Tiles contain multiple Micro-Windows.  
* Micro-Windows can be micro-split into Micro-Tiles.   
* Micro-Tiles cannot be split or tabbed any further.  
 
#### Example

Each newly opened Macro-Window  
&nbsp; contains one Macro-Tile   
&nbsp; &nbsp; which contains multiple Micro-Windows  
&nbsp; &nbsp; &nbsp; with each containing one Micro-Tile  

#### Note 
The hybrid approach makes it necessary to have seperate controls for macro-tabing/micro-tabing and macro-tiling/micro-tiling.

### Rendering

1. Backdrop  
Render grid of ACCENT color tiles.
2. Clear (if style == 0)  
Clear inside borders with BACKGROUND color.
3. Dim (if style > 0)  
Render quads for dimming.
4. TTy Background   
Render grid of BACKGROUND color tiles.
5. TTy Foreground  
Render grid of char tiles with FOREGROUND or ACCENT color.
TODO

## Development
If you want to contribute, please run `./scripts/prepare-git.sh` which installs a prepare-commit-msg hook for versioning.
