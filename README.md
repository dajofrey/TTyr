<p align="center">
 <img src="./build/data/icons/128x128.png"/>
</p>

# TTýr

Terminal Emulator for conquering the command line.

![screenshot](./build/data/image/screenshot.png)

Written in C99 | Noiseless UI | Hybrid Tabing/Tiling | GPU Acceleration | Cross-Platform with API

## README Contents
  
 - [Compatibilities](#Compatibilities)
 - [Dependencies](#Dependencies)
 - [Build](#Build)
 - [Design](#Design)

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
git clone https://github.com/dajofrey/TTyr   
cd TTyr && git submodule update --init --recursive    
make -f build/automation/Makefile all
```

## Design

### Noiseless UI
TTyr's graphics focus on mono-space font drawing, which is primarily used in terminals. Even TTyr's UI is made using the same technique. This makes everything look proportional and clean, even after scaling. Also, it simplifies development a lot. 
   
A big failure point when designing a UI is clutter. Sometimes even terminals have too much UI elements which make the experience distracting and annoying. TTyr tries to avoid this.

### Tabing and Tiling
TTyr uses a hybrid approach for tabing and tiling:   
  
The user can tab between windows, and split these windows into tiles. This is quite common. However, each tile can be tabbed and tiled individually.

#### Rules

* TTyr consists of multiple Macro-Windows. 
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
