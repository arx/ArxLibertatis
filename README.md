            _______________________
           /\                      \
           \_|         Arx         |
             |      Libertatis     |
             |   __________________|__
              \_/____________________/


Cross-platform port of Arx Fatalis

Arx Libertatis is based on the publicly released [Arx Fatalis source code](http://web.archive.org/web/20180105233341/https://www.arkane-studios.com/uk/arx_downloads.php).
The source code is available under the GPLv3+ license with some additional terms - see the COPYING and LICENSE files for details.

## Contact

Website: [http://arx-libertatis.org/](http://arx-libertatis.org/)

Bug Tracker: [https://bugs.arx-libertatis.org/](https://bugs.arx-libertatis.org/)

IRC: \#arxfatalis on irc.freenode.net

Wiki: [http://wiki.arx-libertatis.org/](http://wiki.arx-libertatis.org/)

Reddit: [http://old.reddit.com/r/ArxFatalis/](http://www.reddit.com/r/ArxFatalis/)

## Dependencies

* **[CMake](http://www.cmake.org/) 2.8.3**+ (compile-time only)
* **[zlib](http://zlib.net/)**
* **[Boost](http://www.boost.org/) 1.48**+ (headers only¹)
* **[GLM](http://glm.g-truc.net/) 0.9.5.0**+
* **[FreeType](http://www.freetype.org/) 2.3.0**+
* **OpenAL 1.1**+ ([OpenAL Soft](http://openal-soft.org/) strongly recommended!)
* **iconutil** (from Xcode) or **[icnsutil](https://github.com/pornel/libicns)** (macOS only)

1. Systems without Win32 or POSIX filesystem support will also need the `filesystem` and `system` libraries from Boost.

### Renderer

There is currently a single rendering backend for OpenGL:

* **[SDL](http://www.libsdl.org/)** **1.2.10**+ *or* **2.0.0**+
* **OpenGL 1.5**+ (OpenGL 2.1 or newer is recommended) *or* **OpenGL ES-CM 1.x**¹
* **[libepoxy](https://github.com/anholt/libepoxy) 1.2**+ (recommended) *or* **[GLEW](http://glew.sourceforge.net/) 1.5.2**+

1. OpenGL ES support requires libepoxy

### Crash Reporter

Arx Libertatis comes with an optional gui crash reporter which has additional dependencies:

* **[Qt](http://www.qt.io/) 4.7**+ or **5** (`QtCore`, `QtConcurrent`¹, `QtGui` and `QtWidgets`¹ libraries)
* **[libcurl](http://curl.haxx.se/libcurl/) 7.20.0**+ (not required on Windows)
* **GDB** (Linux-only, optional, run-time only)
* **DbgHelp** (Windows-only)

1. Qt 5 only

While the crash reporter can be run without GDB, it's main usefulness comes from generating and submitting detailed back-traces in the event of a crash. On non-window systems we use GDB, the GNU Debugger, to accomplish that. If you want to help out the arx project, please install GDB before running arx. GDB is however purely a run-time dependency and is not needed when building the crash reporter.

### Tests

Building and running tests has additional dependencies:

* **[CppUnit](https://freedesktop.org/wiki/Software/cppunit/)**

### Git Build Dependencies

Building checkouts from git on their own requires additional dependencies:
* **[Inkscape](https://inkscape.org/)**
* **[ImageMagick](http://www.imagemagick.org/script/index.php)**
* **[OptiPNG](http://optipng.sourceforge.net/)**

These are needed to render and scale the SVG icons, which currently only render correctly in Inkscape. Release and development snapshot source tarballs include pre-built icon files and do not need these dependencies to build.

To avoid the Inkscape (as well as ImageMagick and OptiPNG) dependency for git builds, pre-built icons can be downloaded from http://arx-libertatis.org/files/data/ or the [ArxLibertatisData](https://github.com/arx/ArxLibertatisData/) repository. The required data version is listed in the VERSION file. Place `arx-libertatis-data-$version` directory into the build directory or tell the build system about it's location using the `DATA_FILES` CMake variable (`-DDATA_FILES=…` on the command-line).

Alternatively, icons can be disabled by setting the `ICON_TYPE` CMake variable to `none`. See **OPTIONS.md** for other supported icon type values.

## Compile and install

For Linux run:

    $ mkdir build && cd build
    $ cmake ..
    $ make

The default build settings are tuned for users - if you plan to make changes to Arx Libertatis you should append the `-DDEVELOPER=1` option to the `cmake` command to enable tests, debug checks and fast incremental builds.

To install the binaries system-wide, run as root:

    # make install

Alternatively you can run the game by specifying the full path to the `arx` binary in the `build` directory.

The wiki has more detailed instructions on [compiling under Linux](http://wiki.arx-libertatis.org/Downloading_and_Compiling_under_Linux).

Getting all the dependencies set up for Windows is more tricky. Pre-built dependencies are available in the [ArxWindows repository](https://github.com/arx/ArxWindows) and [instructions on how to use them](http://wiki.arx-libertatis.org/Downloading_and_Compiling_under_Windows) are available on the wiki.

### Build options:

* `BUILD_TOOLS` (default: `ON`): Build tools
* `BUILD_IO_LIBRARY` (default: `ON`): Build helper library for the Blender plugin
* `BUILD_CRASHHANDLER` (default: `ON`): Enable the built-in crash handler (default OFF for macOS)
* `BUILD_CRASHREPORTER` (default: `ON`): Build the Qt crash reporter gui - requires `BUILD_CRASHHANDLER` (default OFF for macOS)
* `BUILD_PROFILER` (default: `OFF`¹): Build the profiler GUI
* `BUILD_TESTS` (default: `OFF`²): Build tests that can be run using `make check`
* `BUILD_ALL` (default: `OFF`): Enable all the BUILD_* options above by default - they can still be disabled individually
* `UNITY_BUILD` (default: `ON`): Unity build (faster build, better optimizations but no incremental build)
* `CMAKE_BUILD_TYPE` (default: `Release`): Set to `Debug` for debug binaries
* `DEBUG` (default: `OFF`³): Enable debug output and runtime checks
* `DEBUG_GL` (default: `OFF`⁴): Enable OpenGL debug output by default
* `DEBUG_EXTRA` (default: `OFF`): Expensive debug options
* `DEVELOPER` (default: `OFF`): Enable build options suitable for developers⁵
* `BUILD_PROFILER_INSTRUMENT` (default: `OFF`): Add profiling instructions to the main arx binary

1. Enabled automatically if `BUILD_ALL` or `BUILD_PROFILER_INSTRUMENT` is enabled
2. Enabled automatically if `BUILD_ALL` or `DEVELOPER` is enabled
3. Enabled automatically if `CMAKE_BUILD_TYPE` is set to `Debug` or if `DEVELOPER` is enabled.
4. Enabled automatically if `DEBUG` is enabled. If disabled, OpenGL debug output can be enabled at run-time using the `--debug-gl` command-line option.
5. Currently this disables `UNITY_BUILD` and enables `DEBUG`, `BUILD_TESTS` and `FASTLINK` for faster incremental builds and improved debug checks, unless those options have been explicitly specified by the user.

Install options:

* `CMAKE_INSTALL_PREFIX` (default: `/usr/local` on UNIX and `C:/Program Files` on Windows): Where to install Arx Libertatis

Set options by passing `-D<option>=<value>` to `cmake`.

Backends that are not available are disabled by default. The `cmake` run should display a summary of the enabled backends at the end.

Advanced options not listed here are documented in **OPTIONS.md**.

## Data file, config and savegame locations

You will need to [get either the full game or demo data of Arx Fatalis](http://arx.vg/data).

Where arx will look for data files and write config and save files depends on the operating system and environment - the wiki has a page detailing the [full data directory detection algorithm](http://arx.vg/paths).

**For Unix-like systems**:
The game will try to rename all used files in the user directory (but not the data directory) to lowercase on the first run. System-wide installations with case-sensitive filesystems always need to manually rename the files to lowercase. The `arx-install-data` script can be used to install the data files, convert them to lowercase and verify that all required files are present.

To print all directories searched by arx, run

    $ arx --list-dirs

By default, user, config and data files will be loaded from and saved to standard system locations depending on the OS:

**Windows**:
* user and config dir:<br>
*XP*: `%USERPROFILE%\My Documents\My Games\Arx Libertatis`<br>
*Vista* and up: `%USERPROFILE%\Saved Games\Arx Libertatis`
* data dir: location stored in `HKCU\Software\ArxLibertatis\DataDir` or `HKLM\Software\ArxLibertatis\DataDir` registry keys

**macOS**:
* user and config dir: `~/Library/Application Support/ArxLibertatis/`
* data dir: `/Applications/ArxLibertatis/`

**Linux** and others:
* user dir: `~/.local/share/arx/`
* config dir: `~/.config/arx/`
* data dir: `/usr/share/games/arx/`, `/usr/local/share/games/arx/` and more

Arx will also try to load data files from the directory containing the game executable.

To use the current working directory for user, config and data files (e.g. for a portable install) run the game as

    $ arx --no-data-dir --user-dir=. --config-dir=.

## Run

Provided the data files are installed at the correct location, you can simply play the game using the installed shortcut or by running

    $ arx

See the `arx --help` and `man arx` output for more details.

## Tools

* `arxunpak [options] <pakfile> [<pakfile>...]` <br>
  Extracts Arx Fatalis .pak files containing the game assets. <br>
  See the `arxunpak --help` and `man arxunpak` output for more details.

* `arxunpak [options] [--all]` <br>
  Extracts all game assets. <br>
  See the `arxunpak --help` and `man arxunpak` output for more details.

* `arxsavetool <command> <savefile> [<options>...]` - commands are:
  * `extract <savefile>` <br>
    Extract the contents of the given savefile to the current directory
  * `add <savefile> [<files>...]` <br>
    Add files to a savefile, create it if needed
  * `fix <savefile>` <br>
    Fix savegame issues created by previous builds of Arx Libertatis
  * `view <savefile> [<ident>]` <br>
    Print savegame information - leave out `<ident>` to list root files

## Scripts

The `arx-install-data` script can extract and install the game data under Linux and FreeBSD from the CD, demo, [GOG.com](http://www.gog.com/) installer or any Arx Fatalis install (such as on Steam) - simply run it and follow the GUI dialogs. Also see the [wiki page on installing the game data under non-Windows systems](http://arx.vg/install-data).

Or, if you prefer a command-line interface, run it as

    $ arx-install-data --cli

More options and required tools (depending on the source file) are documented in the help output:

    $ arx-install-data --help

## Developer information

New contributors should first read the CONTRIBUTING.md file.

To build developer documentation (doxygen), run this from the build directory:

    $ make doc

To check for coding style problems, run the following: (requires python)

    $ make style
