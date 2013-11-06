
This file describes additional build options that are recognized by the CMakeLists.txt but will probably not need to be modifed:

### Default data directories

| Option                | Windows default                                        |
|---------------------- | ------------------------------------------------------ |
| `USER_DIR_PREFIXES`   | `%FOLDERID_SavedGames%`                                |
| `USER_DIR`            | `Arx Libertatis`                                       |
| `CONFIG_DIR_PREFIXES` |                                                        |
| `CONFIG_DIR`          |                                                        |
| `DATA_DIR_PREFIXES`   |                                                        |
| `DATA_DIR`            |                                                        |

| Option                | Mac default                                            |
|---------------------- | ------------------------------------------------------ |
| `USER_DIR_PREFIXES`   | `~/Library/Application Support`                        |
| `USER_DIR`            | `ArxLibertatis`                                        |
| `CONFIG_DIR_PREFIXES` |                                                        |
| `CONFIG_DIR`          |                                                        |
| `DATA_DIR_PREFIXES`   | `/Applications`                                        |
| `DATA_DIR`            | `ArxLibertatis`                                        |

| Option                |  Linux / BSD / other default                           |
|---------------------- | ------------------------------------------------------ |
| `USER_DIR_PREFIXES`   | `${XDG_DATA_HOME:-$HOME/.local/share}`                 |
| `USER_DIR`            | `arx`                                                  |
| `CONFIG_DIR_PREFIXES` | `${XDG_CONFIG_HOME:-$HOME/.config}`                    |
| `CONFIG_DIR`          | `arx`                                                  |
| `DATA_DIR_PREFIXES`   | `${XDG_DATA_DIRS:-/usr/local/share/:/usr/share/}:/opt` |
| `DATA_DIR`            | `games/arx:arx`                                        |

These pairs define prefixes and suffixes that are combined to form searched paths for the user, config and data directories respectively.

To avoid possible performance issues, there is `IGNORE_EXE_DIR` to list directories to *not* search for data files even if they contain the game executable. By default, this is only set for Linux: `/usr/bin:/usr/games:/usr/games/bin:/usr/local/bin:/usr/local/games:/usr/local/games/bin`

All the configuration options above can reference environment variables in operating-system specific shell syntax which will be expanded at run-time. For Windows `%FOLDERID_SavedGames%` is defined to the Windows saved games directory for the current user. For other systems arx will make sure that [XDG Base Directory Specification](http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html) variables are defined.

After environment variable expansion the variables are interpreted as colon-separated (Windows: semicolon-separated) lists of paths.

The config directory defaults to the user directory if not specified.

### Compiler flags

Arx Libertatis adjust the compiler flags to provide an optimal configuration for developers and most users. The following CMake options can be used to disable this and use only the default flags provided by the build environment.

* `SET_WARNING_FLAGS` (default: `ON`): Adjust compiler warning flags. This should not affect the produced binaries but is useful to catch potential problems.
* `SET_OPTIMIZATION_FLAGS` (default: `ON`): Adjust compiler optimization flags. For non-debug builds the only thing this does is instruct the linker to only link against libraries that are actually needed.
* `USE_CXX11` (default: `ON`): Try to compile in C++11 mode if available.

### Static linking

* `USE_STATIC_LIBS` (default: `ON` on Windows, `OFF` elsewhere): Turns on static linking for all libraries, including `-static-libgcc` and `-static-libstdc++`. You can also use the individual options below:
* `GLEW_USE_STATIC_LIBS` (default: `ON` iff `USE_STATIC_LIBS` is enabled): Statically link GLEW.
* `Boost_USE_STATIC_LIBS` (default: `ON` iff `USE_STATIC_LIBS` is enabled): Statically link Boost. See also `FindBoost.cmake` in your CMake installation.

### Install options

The following options can be used to customize where `make install` puts the various components. All of the following paths can either be absolute or relative to the `CMAKE_INSTALL_PREFIX`.

* `CMAKE_INSTALL_DATAROOTDIR` (default: `share`): Where to install data files
* `ICONDIR` (default: `${DATAROOTDIR}/pixmaps`): Where to install icons
* `APPDIR` (default: `${DATAROOTDIR}/applications`): Where to install .desktop files
* `CMAKE_INSTALL_MANDIR` (default: `${DATAROOTDIR}/man`): Where to install man pages
* `CMAKE_INSTALL_BINDIR` (default: `bin`): Where to install user executables
* `GAMESBINDIR` (default: `${BINDIR}`): Where to install game executables
* `CMAKE_INSTALL_LIBEXECDIR` (default: `libexec`): Where to install non-user executables
* `SCRIPTDIR` (default: `${BINDIR}`): Where to install the data install script
* `INSTALL_DATADIR` (default: `${DATAROOTDIR}/games/arx`): Where to install Arx Libertatis data files. This should be one of the directories found from DATA_DIR_PREFIXES + DATA_DIR combinations at runtime.

* `INSTALL_SCRIPTS` (default: `ON`): Install the data install script. There is no data install script on Windows, so there this option does nothing.

### Optional dependencies

By default, optional components will be automatically disabled if their dependencies could not be found. This might be undesirable in some situations, so the following option can be used to change this behavior:

* `STRICT_USE` (default: OFF): Abort the configure step if one of the dependencies enabled with a `USE_*` configuration variable could not be found or if one of the components enabled with a `BUILD_*`configuration variable has missing dependencies. As most dependencies are enabled by default, you may need to explicitly disable some of them. Windows-specifc dependencies are still automatically disabled on non-Windows systems.
* `USE_OPENGL` (default=ON): Build the OpenGL renderer backend^1
* `USE_OPENAL` (default=ON): Build the OpenAL audio backend^2
* `WITH_SDL` (default=*not set*): Select the SDL version to use: 1 or 2. If not set, we will try to use either version, preferring SDL 2. ^3
* `WITH_QT` (default=*not set*): Select the Qt version to use: 4 or 5. If not set, we will try to use either version, preferring Qt 5. Ignored if `BUILD_CRASHREPORTER` is disabled. ^3

1. There is currently no other rendering backend, disabling this will make the build fail.
2. There is currently no other audo backend, there will be no audio when disabling this. Additionally, builds without audio are not well tested and there may be other problems.
3. Existing options can be unset by passing `-U<option>` to cmake.
