            _______________________
           /\                      \
           \_|         Arx         |
             |      Libertatis     |
             |   __________________|__
              \_/____________________/


Cross-platform port of Arx Fatalis

Arx Libertatisis based on the publicly released [Arx Fatalis source code](http://www.arkane-studios.com/uk/arx_downloads.php).
GPLv3 - read ARX_PUBLIC_LICENSE.txt

## Contact

Website: [http://arx-libertatis.org](http://arx-libertatis.org)

Bug Tracker: [https://bugs.arx-libertatis.org/](https://bugs.arx-libertatis.org/)

IRC: \#arxfatalis on irc.freenode.net

Wiki: [http://wiki.arx-libertatis.org/](http://wiki.arx-libertatis.org/)

Reddit: [http://www.reddit.com/r/ArxFatalis/](http://www.reddit.com/r/ArxFatalis/)

## Dependencies

* **CMake 2.8**+ (compile-time only, 2.8.5+ under Windows)
* **zlib**
* **Boost 1.39**+ (headers, `program_options` library)
* **Freetype**
* **OpenAL 1.1**+ *and/or* **DirectSound 9**

Systems without Win32 or POSIX filesystem support will also need **Boost 1.44** or newer including the `filesystem` and `system` libraries.

### Renderer

There are rendering backends for both OpenGL and Direct3D. You need either:

* **SDL 1.2**
* **OpenGL 1.5**+ (OpenGL 2.1 or newer is recommended)
* **GLEW 1.5.2**+

*and/or*

* **DirectInput 8** (included in the DirectX 9 SDK)
* **Direct3D 9**

Both OpenGL and Direct3D backends can be built at the same time.

### Crash Reporter

Arx Libertatis comes with an optional gui crash reporter which has additional dependencies:

* **Qt 4** (`QtCore`, `QtGui` and `QtNetwork` libraries)
* **GDB** (Linux-only, optional, run-time only)
* **DbgHelp** (Windows-only)

While the crash reporter can be run without GDB, it's main usefulness comes from generating and submitting detailed back-traces in the event of a crash. On non-window systems we use GDB, the GNU Debugger, to accomplish that. If you want to help out the arx project, please install GDB before running arx. GDB is however purely a run-time dependency and is not needed when building the crash reporter.

## Compile and install

For Linux run:

    $ mkdir build && cd build && cmake ..
    $ make

To install the binaries system-wide, run as root:

    # make install

Alternatively you can run the game by specifying the full path to the `arx` binary in the `build` directory.

The wiki has more detailed instructions on [compiling under Linux](http://wiki.arx-libertatis.org/Downloading_and_Compiling_under_Linux).

Getting all the dependencies set up for Windows is more tricky. Pre-build dependencies are available in the [ArxWindows repository](https://github.com/arx/ArxWindows) and [instructions on how to use them](http://wiki.arx-libertatis.org/Downloading_and_Compiling_under_Windows) are available on the wiki.

### Build options:

* `BUILD_TOOLS` (default=ON): Build tools
* `BUILD_CRASHREPORTER` (default=ON): Build the Qt crash reporter gui (default OFF for Mac)
* `UNITY_BUILD` (default=OFF): Unity build (faster build, better optimizations but no incremental build)
* `CMAKE_BUILD_TYPE` (default=Release): Set to `Debug` for debug binaries
* `DEBUG_EXTRA` (default=OFF): Expensive debug options
* `USE_OPENAL` (default=ON): Build the OpenAL audio backend
* `USE_OPENGL` (default=ON): Build the OpenGL renderer backend
* `USE_SDL` (default=ON): Build the SDL windowing and input backends
* `USE_NATIVE_FS` (default=ON): Use the native filesystem backend (POSIX / Win32) if available and not boost::filesystem.

Windows-only options (always OFF for non-windows platforms):

* `USE_DSOUND` (default=ON): Build the DirectSound audio backend
* `USE_D3D9` (default=ON): Build the Direct3D 9 renderer backend
* `USE_DINPUT8` (default=ON): Build the DirectInput 8 input backend

Install options:

* `CMAKE_INSTALL_PREFIX` (default: `/usr/local` on UNIX and `C:/Program Files` on Windows): Where to install Arx Libertatis

Linux-only install options (absolute or relative to `CMAKE_INSTALL_PREFIX`):

* `CMAKE_INSTALL_DATAROOTDIR` (default: `share`): Where to install data files
* `ICONDIR` (default: `DATAROOTDIR/pixmaps`): Where to install icons
* `APPDIR` (default: `DATAROOTDIR/applications`): Where to install .desktop files
* `CMAKE_INSTALL_MANDIR` (default: `DATAROOTDIR/man`): Where to install man pages
* `CMAKE_INSTALL_BINDIR` (default: `bin`): Where to install user executables
* `GAMESBINDIR` (default: BINDIR): Where to install game executables
* `CMAKE_INSTALL_LIBEXECDIR` (default: `libexec`): Where to install non-user executables

Set options by passing `-D<option>=<value>` to cmake.

Backends that are not available are disabled by default. The `cmake` run should display a summary of the enabled backends at the end.

## Data file, config and savegame locations

You will need to [get either the full game or demo data of Arx Fatalis](http://wiki.arx-libertatis.org/Getting_the_game_data).

Where arx will look for data files and write config and save files depends on the operating system and environment - the wiki has a page detailing the [full data directory detection algorithm](http://wiki.arx-libertatis.org/Data_directories).

To print all directories searched by arx, run

    $ arx --list-dirs

By default, user, config and data files will be loaded from and saved to standard system locations depending on the OS:

**Windows**:
* user and config dir:<br>
*XP*: `%USERPROFILE%\My Documents\My Games\Arx Libertatis`<br>
*Vista* and up: `%USERPROFILE%\Saved Games\Arx Libertatis`
* data dir: location stored in `HKCU\Software\ArxLibertatis\DataDir` or `HKLM\Software\ArxLibertatis\DataDir` registry keys

**Mac OS X**:
* user and config dir: `~/Library/Application Support/ArxLibertatis/`
* data dir: `/Applications/ArxLibertatis/`

**Linux** and others:
* user dir: `~/.local/share/arx/`
* config dir: `~/.config/arx/`
* data dir: `/usr/share/games/arx/`, `/usr/local/share/games/arx/` and more

Arx will also try to load data files from the directory containing the game executable.

To use the current working directory for load user, config and data files (e.g. for a portable install) run the game as

    $ arx --no-data-dir --user-dir=. --config-dir=.

See the `arx --help` and `man arx` output for more details.

The default directories can be adjusted with additional build options described in OPTIONS.md.

## Run

Provided the data files are installed at the correct location, you can simply play the game using the installed shortcut or by running

    $ arx

The game will try to automatically rename all used files in the user directory (but not the data directory) to lowercase on the first run if possible. System-wide installations with case-sensitive filesystems always need to manually rename the files to lowercase - you can use the install-copy script.

## Tools

* `arxunpak <pakfile> [<pakfile>...]` - Extracts the .pak files containing the game assets.

* `arxsavetool <command> <savefile> [<options>...]` - commands are:
  * `extract <savefile>` - Extract the contents of the given savefile to the current directly.
  * `add <savefile> [<files>...]` - Add files to a savefile, create it if needed.
  * `fix <savefile>` - Fix savegame issues created by previous builds of Arx Libertatis
  * `view <savefile> <ident>` - Print savegame information.

## Scripts

The `scripts` directory contains shell scripts that allow to extract/install the game data under Linux without Wine from the CD, demo, GOG.com installer or any fully patched Arx Fatalis install respectively. Also see the [wiki page on installing the game data under Linux](http://wiki.arx-libertatis.org/Installing_the_game_data_under_Linux).

* `scripts/install-cd path/to/cd path/to/ArxFatalis_1.21_MULTILANG.exe [output_dir]`<br>
  requires [cabextract](http://www.cabextract.org.uk/) and [innoextract](http://constexpr.org/innoextract/)<br>
  `ArxFatalis_1.21_MULTILANG.exe` can be downloaded from [the official Arx Fatalis website](http://www.arkane-studios.com/uk/arx_downloads.php).

* `scripts/install-demo path/to/arx_demo_english.zip [output_dir]`<br>
  requires [unzip](http://www.info-zip.org/) and [cabextract](http://www.cabextract.org.uk/)

* `scripts/install-gog path/to/setup_arx_fatalis.exe [output_dir]`<br>
  requires [innoextract](http://constexpr.org/innoextract/)<br>
  `setup_arx_fatalis.exe` can be downloaded from your [GOG.com](http://www.gog.com/) account after buying Arx Fatalis

* `scripts/install-copy path/to/ArxFatalis/ [output_dir]`<br>
  `path/to/ArxFatalis/` should point to a fully pached (1.21) Arx Fatalis installation, such as the one from Steam

## Developer information

To build developer documentation (doxygen), run this from the build directory:

    $ make doc

To check for coding style problems, run the following: (requires python)

    $ make style
