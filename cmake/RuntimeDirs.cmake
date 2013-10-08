
# Config variables for Windows / OS X / XDG-basedir-spec runtime directories
#


# Default runtime user and data directories
if(WIN32)
	set(USER_DIR            "Arx Libertatis"                CACHE STRING "User dir names")
	set(USER_DIR_PREFIXES   "%FOLDERID_SavedGames%"         CACHE STRING "User dir paths")
elseif(MACOSX)
	set(DATA_DIR            "ArxLibertatis"                 CACHE STRING "Data dir names")
	set(DATA_DIR_PREFIXES   "/Applications"                 CACHE STRING "Data dir paths")
	set(USER_DIR            "ArxLibertatis"                 CACHE STRING "User dir names")
	set(USER_DIR_PREFIXES   "~/Library/Application Support" CACHE STRING "User dir paths")
else()
	set(DATA_DIR
		"games/arx:arx"
		CACHE STRING "Data dir names"
	)
	set(DATA_DIR_PREFIXES
		"\${XDG_DATA_DIRS:-/usr/local/share/:/usr/share/}:/opt"
		CACHE STRING "Data dir paths"
	)
	set(USER_DIR
		"arx"
		CACHE STRING "User dir names"
	)
	set(USER_DIR_PREFIXES
		"\${XDG_DATA_HOME:-$HOME/.local/share}"
		CACHE STRING "User dir paths"
	)
	set(CONFIG_DIR
		"arx"
		CACHE STRING "Config dir names"
	)
	set(CONFIG_DIR_PREFIXES
		"\${XDG_CONFIG_HOME:-$HOME/.config}"
		CACHE STRING "Config dir paths"
	)
	set(IGNORE_EXE_DIR
		"/usr/bin:/usr/games:/usr/games/bin:/usr/local/bin:/usr/local/games:/usr/local/games/bin"
		CACHE STRING "Executable locations that should not be used as a data dir"
	)
endif()
mark_as_advanced(
	DATA_DIR
	DATA_DIR_PREFIXES
	USER_DIR
	USER_DIR_PREFIXES
	CONFIG_DIR
	CONFIG_DIR_PREFIXES
	IGNORE_EXE_DIR
)
