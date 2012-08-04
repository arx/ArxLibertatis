
This file describes additional build options that are recognized by the CMakeLists.txt but will ptobably not need to be modifed:

### Default data directories

| Option                | Windows default         |  Linux / other default | Mac default     |
|---------------------- | ----------------------- | ---------------------- | --------------- |
| `USER_DIR_PREFIXES`   | `%FOLDERID_SavedGames%` | `$XDG_DATA_HOME`       | `~/Library/Application Support` |
| `USER_DIR`            | `Arx Libertatis`        | `arx`                  | `ArxLibertatis` |
| `CONFIG_DIR_PREFIXES` |                         | `$XDG_CONFIG_HOME`     |                 |
| `CONFIG_DIR`          |                         | `arx`                  |                 |
| `DATA_DIR_PREFIXES`   |                         | `$XDG_DATA_DIRS:/opt`  | `/Applications` |
| `DATA_DIR`            |                         | `games/arx:arx`        | `ArxLibertatis` |

These pairs define prefixes and suffixes that are combined to form searched paths for the user, config and data directories respectively.

To avoid possible performance issues, there is `IGNORE_EXE_DIR` to list directories to *not* search for data files even if they contain the game executable. By default, this is only set for Linux: `/usr/bin:/usr/games:/usr/games/bin:/usr/local/bin:/usr/local/games:/usr/local/games/bin`

All the configuration options above can reference environment variables in operating-system specific shell syntax which will be expanded at run-time. For Windows `%FOLDERID_SavedGames%` is defined to the Windows saved games directory for the current user. For other systems arx will make sure that [XDG Base Directory Specification](http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html) variables are defined.

After environment variable expansion the variables are interpreted as colon-separated (Windows: semicolon-separated) lists of paths.

The config directory defaults to the user directory if not specified.
