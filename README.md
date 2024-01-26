# TTýr
 
Terminal Emulator Written In C99.

![screenshot](./build/data/image/screenshot.png)

What makes TTýr different?

 - Minimalistic Design
 - Hybrid Tabing/Tiling
 - GPU Acceleration
 - API

## Contents
  
 - [Compatibilities](#Compatibilities)
 - [Dependencies](#Dependencies)
 - [Build](#Build)

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
TTýr is companion project to [Netzhaut](https://github.com/dajofrey/netzhaut), from which it requires nhapi, nhcore, nhencoding, nhwsi and nhgfx modules.
 * [st](https://st.suckless.org/)  
For shell functionality, TTýr uses parts of the great [st](https://st.suckless.org/).   

## Build

### Linux

```bash
git clone https://github.com/dajofrey/TTyr   
cd TTyr && git submodule update --init --recursive    
make -f build/automation/Makefile all
```
