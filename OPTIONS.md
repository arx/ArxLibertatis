
This file describes additional build options that are recognized by the CMakeLists.txt but will probably not need to be modified:

### Default data directories

| Option                | Windows default                                        |
|---------------------- | ------------------------------------------------------ |
| `USER_DIR_PREFIXES`   | ¹                                                      |
| `USER_DIR`            | `Arx Libertatis`                                       |
| `CONFIG_DIR_PREFIXES` |                                                        |
| `CONFIG_DIR`          |                                                        |
| `DATA_DIR_PREFIXES`   |                                                        |
| `DATA_DIR`            |                                                        |

| Option                | macOS default                                          |
|---------------------- | ------------------------------------------------------ |
| `USER_DIR_PREFIXES`   | `$HOME/Library/Application Support`                    |
| `USER_DIR`            | `ArxLibertatis`                                        |
| `CONFIG_DIR_PREFIXES` |                                                        |
| `CONFIG_DIR`          |                                                        |
| `DATA_DIR_PREFIXES`   | `/Applications`                                        |
| `DATA_DIR`            | `ArxLibertatis`                                        |

| Option                |  Linux / BSD / other default                           |
|---------------------- | ------------------------------------------------------ |
| `USER_DIR_PREFIXES`   | `${XDG_DATA_HOME:-$HOME/.local/share}`²                |
| `USER_DIR`            | `arx`                                                  |
| `CONFIG_DIR_PREFIXES` | `${XDG_CONFIG_HOME:-$HOME/.config}`                    |
| `CONFIG_DIR`          | `arx`                                                  |
| `DATA_DIR_PREFIXES`   | `${XDG_DATA_DIRS:-/usr/local/share/:/usr/share/}:/opt` |
| `DATA_DIR`            | `games/arx:arx`                                        |

These pairs define prefixes and suffixes that are combined to form searched paths for the user, config and data directories respectively.

1. Arx Libertatis uses the default `Saved Games` directory (FOLDERID_SavedGames) on Windows Vista and newer, and `My Documents\My Games` on Windows XP.
2. This matches the [XDG Base Directory Specification](http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html).

To avoid possible performance issues, there is `IGNORE_EXE_DIR` to list directories to *not* search for data files even if they contain the game executable. By default, this is only set for Linux: `/usr/bin:/usr/games:/usr/games/bin:/usr/local/bin:/usr/local/games:/usr/local/games/bin`
To completely disable searching for data locations relative to the executable, set `RUNTIME_DATADIR` to an empty string.

All the configuration options above can reference environment variables in operating-system specific shell syntax which will be expanded at run-time.

After environment variable expansion the variables are interpreted as colon-separated (Windows: semicolon-separated) lists of paths.

The config directory defaults to the user directory if not specified.

### Compiler flags

Arx Libertatis adjust the compiler flags to provide an optimal configuration for developers and most users. The following CMake options can be used to disable this and use only the default flags provided by the build environment.

* `SET_WARNING_FLAGS` (default: `ON`): Adjust compiler warning flags. This should not affect the produced binaries but is useful to catch potential problems.
* `SET_OPTIMIZATION_FLAGS` (default: `ON`): Adjust compiler optimization flags. For non-debug builds the only thing this does is instruct the linker to only link against libraries that are actually needed.
* `FASTLINK` (default: `OFF`¹: Optimize for link speed
* `USE_LTO` (default: `ON`²: Use link-time code generation
* `CXX_STD_VERSION` (default: `2017`): Maximum C++ standard version to enable.

1. Enabled automatically if `DEVELOPER` is enabled.
2. Disabled automatically if `SET_OPTIMIZATION_FLAGS` is disabled or `FASTLINK` is enabled.

### Static linking

* `USE_STATIC_LIBS` (default: `ON` on Windows, `OFF` elsewhere): Turns on static linking for all libraries, including `-static-libgcc` and `-static-libstdc++`. You can also use the individual options below:
* `epoxy_USE_STATIC_LIBS` (default: `ON` iff `USE_STATIC_LIBS` is enabled): Statically link libepoxy.
* `GLEW_USE_STATIC_LIBS` (default: `ON` iff `USE_STATIC_LIBS` is enabled): Statically link GLEW.
* `Boost_USE_STATIC_LIBS` (default: `ON` iff `USE_STATIC_LIBS` is enabled): Statically link Boost. See also `FindBoost.cmake` in your CMake installation.
* `Freetype_USE_STATIC_LIBS` (default: `ON` iff `USE_STATIC_LIBS` is enabled): Statically link FreeType.
* `ZLIB_USE_STATIC_LIBS` (default: `ON` iff `USE_STATIC_LIBS` is enabled): Statically link ZLIB.

### Install options

The following options can be used to customize where `make install` puts the various components. All of the following paths can either be absolute or relative to the `CMAKE_INSTALL_PREFIX`.

* `CMAKE_INSTALL_DATAROOTDIR` (default: `share`): Where to install data files
* `ICONDIR` (default: `${DATAROOTDIR}/pixmaps`): Where to install standalone icons
* `ICONTHEMEDIR` (default: `${DATAROOTDIR}/icons/hicolor`): Where to install themable icon sets
* `APPDIR` (default: `${DATAROOTDIR}/applications`): Where to install .desktop files
* `CMAKE_INSTALL_INCLUDEDIR` (default: `include`): Where to install C include files
* `CMAKE_INSTALL_MANDIR` (default: `${DATAROOTDIR}/man`): Where to install man pages
* `CMAKE_INSTALL_BINDIR` (default: `bin`): Where to install user executables
* `GAMESBINDIR` (default: `${BINDIR}`): Where to install game executables
* `CMAKE_INSTALL_LIBDIR` (default: `lib`): Where to install libraries
* `CMAKE_INSTALL_LIBEXECDIR` (default: `libexec`): Where to install non-user executables
* `RUNTIME_LIBEXECDIR` (default: `${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR}`): Runtime search locations for utilities, relative to the game executable
* `RUNTIME_DATADIR` (default: `.`): Paths relative to the game executable to search for data files
* `SCRIPTDIR` (default: `${BINDIR}`): Where to install the data install script
* `INSTALL_DATADIR` (default: `${DATAROOTDIR}/games/arx`): Where to install Arx Libertatis data files. This should be one of the directories found from DATA_DIR_PREFIXES + DATA_DIR combinations at runtime.
* `INSTALL_BLENDER_PLUGINDIR` (default: `${DATAROOTDIR}/blender/scripts/addons/arx"`): Where to install the Arx Libertatis Blender plugin.

* `INSTALL_SCRIPTS` (default: `ON`): Install the data install script. There is no data install script on Windows, so there this option does nothing.
* `INSTALL_BLENDER_PLUGIN` (default: `ON`): Install the Arx Libertatis Blender plugin. Implies `BUILD_IO_LIBRARY`.

### Optional dependencies

By default, optional components will be automatically disabled if their dependencies could not be found. This might be undesirable in some situations, so the following option can be used to change this behavior:

* `STRICT_USE` (default: OFF): Abort the configure step if one of the dependencies enabled with a `USE_*` configuration variable could not be found or if one of the components enabled with a `BUILD_*`configuration variable has missing dependencies. As most dependencies are enabled by default, you may need to explicitly disable some of them. Windows-specific dependencies are still automatically disabled on non-Windows systems.
* `USE_OPENGL` (default: ON): Build the OpenGL renderer backend¹
* `USE_OPENAL` (default: ON): Build the OpenAL audio backend²
* `WITH_OPENGL` (default: *not set*): Select the OpenGL wrangler to use: `epoxy` or `glew`. If not set, we will try to use either, preferring SDL `epoxy`. ³
* `WITH_SDL` (default: *not set*): Select the SDL version to use: 1 or 2. If not set, we will try to use either version, preferring SDL 2. ³
* `WITH_QT` (default: *not set*): Select the Qt version to use: 4 or 5. If not set, we will try to use either version, preferring Qt 5. Ignored if `BUILD_CRASHREPORTER` is disabled. ³
* `USE_WINHTTP` (default: ON): Use the native WinHTTP API instead of CURL on Windows.
* `USE_NATIVE_FS` (default: ON): Use the native filesystem backend (POSIX / Win32) if available and not boost::filesystem.

1. There is currently no other rendering backend, disabling this will make the build fail.
2. There is currently no other audio backend, there will be no audio when disabling this. Additionally, builds without audio are not well tested and there may be other problems.
3. Existing options can be unset by passing `-U<option>` to cmake.

### Icons

* `ICON_TYPE` (default: *platform specific*): Type(s) of icons to generate and install. Valid options are:
  * `ico` Windows .ico files (linked into the appropriate executables) [default on Windows]
  * `icns` macOS .icns files (installed under `ICONDIR`) [default on macOS]
  * `iconset` Themable .png icon sets (installed in a `${size}x${size}/apps/${name}.png` hierarchy under `ICONTHEMEDIR`) [default on Linux and other systems]
  * `png` Portable .png  icons (installed under `ICONDIR`)
  * `overview` Icon size comparison montage (not installed)
  * `all` Generate all possible icon types
  * `none` Don't generate any icons

* `DATA_FILES` (default: *not set*): Locations to search for pre-built data files. This is only useful when building git checkouts as release and development snapshot tarballs already include those files in the source tree. See the *Git Build Dependencies* section in README.md.
* `OPTIMIZE_ICONS` (default: ON): Optimize the compression of generated PNG files using OptiPNG. This is only useful when building git checkouts as release and development snapshot tarballs already include pre-built images that this option doesn't apply to.
