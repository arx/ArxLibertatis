            _______________________
           /\                      \
           \_|         Arx         |
             |      Libertatis     |
             |   __________________|__
              \_/____________________/


Cross-platform port of Arx Fatalis

Arx Libertatis is based on the publicly released [Arx Fatalis source code](http://www.arkane-studios.com/uk/arx_downloads.php).
GPLv3 - read ARX_PUBLIC_LICENSE.txt

## Contact

IRC: \#arxfatalis on irc.freenode.net

Wiki: [http://arx.parpg.net/](http://arx.parpg.net/)

Reddit: [http://www.reddit.com/r/ArxFatalis/](http://www.reddit.com/r/ArxFatalis/)

## Dependencies

* SDL 1.2 and OpenGL 1.5 and GLU and GLEW **and/or** DirectInput 8 and Direct3D 9
* OpenAL 1.1 **and/or** DirectSound 9
* DevIL
* zlib
* Boost (headers, program_options library)
* Freetype

Systems without Win32 or POSIX filesystem support will also need the boost filesystem library.

## Compile

`$ mkdir build && cd build && cmake ..`

`$ make`

Build options:

* `BUILD_TOOLS` (default=ON): Build tools
* `UNITY_BUILD` (default=OFF): Unity build (faster build, better optimizations but no incremental build)
* `CMAKE_BUILD_TYPE` (default=Release): Set to `Debug` for debug binaries
* `DEBUG_EXTRA` (default=OFF): Expensive debug options
* `USE_OPENAL` (default=ON): Build the OpenAL audio backend
* `USE_OPENGL` (default=ON): Build the OpenGL renderer backend
* `USE_SDL` (default=ON): Build the SDL windowing and input backends

Windows-only options (always OFF for non-windows platforms):

* `USE_DSOUND` (default=ON): Build the DirectSound audio backend
* `USE_D3D9` (default=ON): Build the Direct3D 9 renderer backend
* `USE_DINPUT8` (default=ON): Build the DirectInput 8 input backend

Enable by passing `-D<option>=1` to cmake, disable using `-D<option>=0`

The wiki has detailed instructions on compiling [under linux](http://arx.parpg.net/Downloading_and_Compiling_under_Linux) and [under windows](http://arx.parpg.net/Downloading_and_Compiling_under_Windows)

## Data files / config / savegame location

You will need to get either the full game or demo data of Arx Fatalis. See http://arx.parpg.net/Getting_the_game_data

Arx will consider the following folders (in order) when looking for data files. `DATA_DIR` and `DATA_DIR_PREFIXES` are build-time options that can be passed to cmake.
1. The **`--data-dir`** / **`-d`** comand-line option
1. (windows only) the **`HKEY_CURRENT_USER\Software\ArxLibertatis\DataDir`** registry key.
2. (windows only) the `HKEY_LOCAL_MACHINE\Software\ArxLibertatis\DataDir` registry key.
3. If `DATA_DIR` is a relative path





* `DATA_DIR` (default="arx"): Where to look for data files.
* `USER_DIR` (default="arx"): Where to store config files and save games.
* `DATA_DIR_PREFIXES`
* `USER_DIR_PREFIXES`

## Run

Run from the directory containing the .pak files:

`$ ./arx`

The game will try to automatically rename all used files to lowercase on the first run. If you want to run with read-only permissions, you will need to do this manually.

You can close it with `Alt + F4` or `killall arx.exe.so`

## Tools

* `arxunpak <pakfile> [<pakfile>...]` - Extracts the .pak files containing the game assets.

* `arxsavetool <command> <savefile> [<options>...]` - commands are:
  * `extract <savefile>` - Extract the contents of the given savefile to the current directly.
  * `add <savefile> [<files>...]` - Add files to a savefile, create it if needed.
  * `fix <savefile>` - Fix savegame issues created by previous builds of Arx Libertatis
  * `view <savefile> <ident>` - Print savegame information.

## Build documentation

To build developer documentation (doxygen), run this from the root directory:

`$ make doc`
