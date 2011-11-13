            _______________________
           /\                      \
           \_|         Arx         |
             |      Libertatis     |
             |   __________________|__
              \_/____________________/


Port of Arx Fatalis to x64_86 / GCC / OpenGL / Unix

Arx Libertatis is based on the publicly released [Arx Fatalis source code](http://www.arkane-studios.com/uk/arx_downloads.php).
GPLv3 - read ARX_PUBLIC_LICENSE.txt

## Dependencies

* SDL 1.2 and OpenGL 1.5 and GLEW **and/or** DirectInput 8 and Direct3D 9
* OpenAL 1.1 **and/or** DirectSound 9
* DevIL
* zlib
* Boost

## Compile

`$ mkdir build && cd build && cmake ..`

`$ make`

Build options:

* `ARX_BUILD_TOOLS` (default=ON): Build tools
* `ARX_USE_UNITYBUILD` (default=OFF): Unity build (faster build, better optimizations but no incremental build)
* `CMAKE_BUILD_TYPE` (default=Release): Set to `Debug` for debug Binaries
* `ARX_DEBUG_EXTRA` (default=OFF): Expensive debug options
* `ARX_USE_OPENAL` (default=ON): Build the OpenAL audio backend
* `ARX_USE_OPENGL` (default=ON): Build the OpenGL renderer backend
* `ARX_USE_SDL` (default=ON): Build the SDL windowing and input backends

Windows-only options (always OFF for non-windows platforms):

* `ARX_USE_DSOUND` (default=ON): Build the DirectSound audio backend
* `ARX_USE_D3D9` (default=ON): Build the Direct3D 9 renderer backend
* `ARX_USE_DINPUT8` (default=ON): Build the DirectInput 8 input backend

Enable by passing `-D<option>=1` to cmake, disable using `-D<option>=0`

To build 32-bit binaries on a 64-bit (multilib) system, pass `-DCMAKE_CXX_FLAGS=-m32` to cmake.

## Run

You will need to get either the full game or demo data of Arx Fatalis. See http://arx.parpg.net/Getting_the_game_data

Run from the directory containing the .pak files:

`$ ./arx`

The game will try to automatically rename all used files to lowercase on the first run. If you want to run with read-only permissions, you will need to do this manually.

You can close it with `Alt + F4` or `killall arx.exe.so`

## Tools

* arxunpak - Extracts the .pak files containing the game assets.
* arxsavetool - Extract, modify, view and fix savegames.

## Build documentation

Run this from the root directory:

`$ make doc`

## Chat

\#arxfatalis on irc.freenode.net

## Wiki

[http://arx.parpg.net/](http://arx.parpg.net/)

## Reddit

[http://www.reddit.com/r/ArxFatalis/](http://www.reddit.com/r/ArxFatalis/)
