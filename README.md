<p align="center">
 <img src="./build/data/icons/128x128.png"/>
</p>

# TTýr

Terminal Emulator for conquering the command line.

![screenshot](./build/data/image/screenshot.png)

## Contents
  
 - [Keypoints](#Keypoints)
 - [Compatibilities](#Compatibilities)
 - [Dependencies](#Dependencies)
 - [Build](#Build)

## Keypoints 

 - Written in C99
 - Minimal, Flat UI
 - Hybrid Tabing/Tiling
 - GPU Acceleration
 - Cross-Platform with API

## Compatibilities

| Operating System | Compatibility |
| --- | --- |
| Linux | Yes |
| MacOS |  No |
| Windows |  No |

| Window System | Compatibility |
| --- | --- |
| X11 |  Yes |
| Wayland | No |

| Graphics Driver | Compatibility |
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
