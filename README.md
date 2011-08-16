            _______________________
           /\                      \
           \_|         Arx         |
             |      Libertatis     |
             |   __________________|__
              \_/____________________/


Port to x64_86 / GCC / OpenGL / Unix

Arx Libertatis is based on the publicly released Arx Fatalis source code.
GPLv3 - read ARX_PUBLIC_LICENSE.txt

## Dependencies

* SDL 1.2 and OpenGL 1.5 and GLEW **and/or** DirectInput 7 and Direct3D 7
* OpenAL 1.1 **and/or** DirectSound 7
* DevIL
* zlib
* Boost

## Compile

`$ mkdir build && cd build && cmake ..`

`$ make`

Build options:

* `ARX_BUILD_TOOLS` (default=ON): Build tools
* `ARX_USE_UNITYBUILD` (default=OFF): Unity build (faster build, better optimizations but no incremental build)
* `ARX_DEBUG` (default=ON): Normal debug options
* `ARX_DEBUG_EXTRA` (default=OFF): Expensive debug options
* `ARX_FORCE_32BIT` (default=OFF): Force a 32-bit build on 64-bit systems
* `ARX_USE_OPENAL` (default=ON): Build the OpenAL audio backend
* `ARX_USE_OPENGL` (default=ON): Build the OpenGL renderer backend
* `ARX_USE_SDL` (default=ON): Build the SDL windowing and input backends
* `ARX_USE_DSOUND` (default=OFF): Build the DirectSound audio backend
* `ARX_USE_D3D` (default=OFF): Build the Direct3D 7 renderer backend

Enable by passing `-D<option>=1` to cmake, disable using `-D<option>=0`

## Run

Run from the directory containing the .pak files:

`$ ./arx'

The game will try to automatically rename all used files to lowercase on the first run. If you want to run with read-only permissions, you will need to do this manually.

You can close it with `Alt + F4` or `killall arx.exe.so`

## Tools

* unpak - Extracts the .pak files containing the game assets.
* savetool - Extract, modify, view and fix savegames.

## Build documentation

Run this from the root directory:

`$ make doc`

## Chat

\#arxfatalis on irc.freenode.net

## Wiki

[http://arx.parpg.net/](http://arx.parpg.net/)

## Reddit

[http://www.reddit.com/r/ArxFatalis/](http://www.reddit.com/r/ArxFatalis/)
