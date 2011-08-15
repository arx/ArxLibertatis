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

* DevIL
* zlib
* Boost
* OpenAL 1.1 or DirectSound
* OpenGL 1.5 + GLEW or Direct3D
* SDL 1.2 or DirectInput

## Compile

`$ mkdir build && cd build && cmake ..`

`$ make`

Build options:

* `ARX_BUILD_TOOLS` (default=ON): Build tools
* `ARX_USE_UNITYBUILD` (default=OFF): Unity build (faster build, better optimizations but no inclemental build)
* `ARX_DEBUG` (default=ON): Normal debug options
* `ARX_DEBUG_EXTRA` (default=OFF): Expensive debug options
* `ARX_FORCE_32BIT` (default=OFF): Force a 32-bit build on 64-bit systems
* `ARX_USE_OPENAL` (default=ON): Build the OpenAL audio backend
* `ARX_USE_OPENGL` (default=ON): Build the OpenGL renderer backend
* `ARX_USE_SDL` (default=ON): Build the SDL windowing and input backends
* `ARX_USE_DSOUND` (default=OFF): Build the DirectSound audio backend

Enable by passing `-D<option>=1` to cmake, disable using `-D<option>=0`

## Run

Rename all .pak files in the Arx Fatalis installation to lowercase and the run from the directory containing the .pak files.

`$ WINEDEBUG=-all ./arx.exe`

You can close it with `Alt + F4` or `killall arx.exe.so`

## Debugging

Because ARX is compiled as a winelib application, normal gdb won't work. Instead use winedbg:

`$ WINEDEBUG=-all winedbg ./arx.exe.so`

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
