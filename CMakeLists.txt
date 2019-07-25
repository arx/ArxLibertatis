project(ArxLibertatis)

if(MSVC)
	# CMake 2.8.5 or newer is needed for CMAKE_LIBRARY_ARCHITECTURE support.
	cmake_minimum_required(VERSION 2.8.5)
	# Change CMAKE_LIBRARY_ARCHITECTURE to ensure the right libs are used
	if(CMAKE_CL_64)
		set(CMAKE_LIBRARY_ARCHITECTURE "x64")
	else()
		set(CMAKE_LIBRARY_ARCHITECTURE "x86")
	endif()
else()
	cmake_minimum_required(VERSION 2.8)
endif()

if(POLICY CMP0054)
	# CMake 3.1+: Only interpret if() arguments as variables or keywords when unquoted.
	cmake_policy(SET CMP0054 NEW)
endif()


# Define configuration options

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(MACOSX 1)
else()
	set(MACOSX 0)
endif()

# Components
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_TOOLS "Build tools" ON)
set(def_BUILD_CRASHREPORTER ON)
if(MACOSX)
	set(def_BUILD_CRASHREPORTER OFF)
elseif(WIN32 AND NOT MSVC)
	set(def_BUILD_CRASHREPORTER OFF)
endif()
option(BUILD_CRASHREPORTER "Build the crash reporter" ${def_BUILD_CRASHREPORTER})
option(BUILD_EDITOR "Build editor" OFF)
option(BUILD_EDIT_LOADSAVE "Build save/load functions only used by the editor" ON)
option(INSTALL_SCRIPTS "Install the data install script" ON)

# Optional dependencies
option(USE_OPENAL "Build the OpenAL audio backend" ON)
option(USE_DSOUND "Build the DirectSound audio backend" ON)
option(USE_OPENGL "Build the OpenGL renderer backend" ON)
option(USE_D3D9 "Build the Direct3D 9 renderer backend" ON)
option(USE_SDL "Build the SDL windowing backend" ON)
option(USE_DINPUT8 "Build the DirectInput 8 input backend" ON)
option(USE_NATIVE_FS "Use the native filesystem backend directly instead of boost" ON)
option(USE_QT5 "Use Qt 5 if available" ON)
option(USE_QT4 "Use Qt 4 if available" ON)

# Build types
option(UNITY_BUILD "Unity build" OFF)
option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)
option(USE_CXX11 "Try to use C++11 if available" ON)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(IS_DEBUG_BUILD ON)
else()
	set(IS_DEBUG_BUILD OFF)
endif()
option(DEBUG "Enable debug output and runtime checks" ${IS_DEBUG_BUILD})
if(DEBUG)
	add_definitions(-DARX_DEBUG=1)
endif()

# Static libs
set(default_USE_STATIC_LIBS OFF)
if(WIN32)
	set(default_USE_STATIC_LIBS ON)
endif()
option(USE_STATIC_LIBS       "Statically link libraries" ${default_USE_STATIC_LIBS})
option(GLEW_USE_STATIC_LIBS  "Statically link GLEW"      ${USE_STATIC_LIBS})
option(Boost_USE_STATIC_LIBS "Statically link Boost"     ${USE_STATIC_LIBS})

# Make optional dependencies required
option(STRICT_USE "Abort if there are missing optional dependencies"  OFF)
if(STRICT_USE)
	set(OPTIONAL_DEPENDENCY REQUIRED)
else()
	set(OPTIONAL_DEPENDENCY)
endif()

# Install destinations
if(CMAKE_VERSION VERSION_LESS 2.8.5)
	set(CMAKE_INSTALL_DATAROOTDIR "share" CACHE
	    STRING "read-only architecture-independent data root (share) (relative to prefix).")
	set(CMAKE_INSTALL_BINDIR "bin" CACHE
	    STRING "user executables (bin) (relative to prefix).")
	set(CMAKE_INSTALL_LIBEXECDIR "libexec" CACHE
	    STRING "program executables (libexec) (relative to prefix).")
	set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_DATAROOTDIR}/man" CACHE
	    STRING "man documentation (DATAROOTDIR/man) (relative to prefix).")
	foreach(dir BINDIR LIBEXECDIR DATAROOTDIR MANDIR)
		if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_${dir}})
			set(CMAKE_INSTALL_FULL_${dir} "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_${dir}}")
		else()
			set(CMAKE_INSTALL_FULL_${dir} "${CMAKE_INSTALL_${dir}}")
		endif()
	endforeach()
	mark_as_advanced(
		CMAKE_INSTALL_DATAROOTDIR
		CMAKE_INSTALL_BINDIR
		CMAKE_INSTALL_LIBEXECDIR
		CMAKE_INSTALL_MANDIR
	)
else()
	include(GNUInstallDirs)
endif()
set(ICONDIR "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps" CACHE
    STRING "Install location for icons (relative to prefix).")
set(APPDIR "${CMAKE_INSTALL_DATAROOTDIR}/applications" CACHE
    STRING "Install location for .desktop files (relative to prefix).")
set(GAMESBINDIR "${CMAKE_INSTALL_BINDIR}" CACHE
    STRING "Install location for game executables (relative to prefix).")
set(SCRIPTDIR "${CMAKE_INSTALL_BINDIR}" CACHE
    STRING "Where to install the data install script (relative to prefix).")
mark_as_advanced(
	ICONDIR
	APPDIR
	GAMESBINDIR
	SCRIPTDIR
)

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


# Helper scrips

include(CheckCXXSourceCompiles)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckTypeSize)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake") # For custom cmake modules
include(BuildSystem)
include(BuildType)
include(CompileCheck)
include(CreateSourceGroups)
include(PrintConfiguration)
include(StyleCheck)
include(VersionString)


# Find required libraries

set(ARX_LIBRARIES)
set(BASE_LIBRARIES)

# Force re-checking libraries if the compiler or compiler flags change
if((NOT LAST_CMAKE_CXX_FLAGS STREQUAL CMAKE_CXX_FLAGS)
   OR (NOT LAST_CMAKE_CXX_COMPILER STREQUAL CMAKE_CXX_COMPILER))
	force_recheck_library(ZLIB)
	force_recheck_library(Freetype)
	force_recheck_library(Threads)
	force_recheck_library(OpenAL)
	force_recheck_library(OpenGL)
	force_recheck_library(GLEW)
	force_recheck_library(Boost)
	force_recheck_library(QtCore QT_QTCORE)
	force_recheck_library(QtGui QT_QTGUI)
	force_recheck_library(QtNetwork QT_QTNETWORK)
	unset(Boost_INCLUDE_DIR CACHE)
	set(LAST_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE INTERNAL
	    "The last C++ compiler flags")
	set(LAST_CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE INTERNAL
	    "The last C++ compiler")
endif()

# Win32 API
if(WIN32)
	# Ensure we aren't using functionalities not found under Window XP SP1
	add_definitions(-D_WIN32_WINNT=0x0502)
	if(NOT MSVC)
		# We need to define WINVER for MinGW when requiring anything newer than Win95/WinNT
		add_definitions(-DWINVER=0x0500) # Require at least Windows 2000
		add_definitions(-D_WIN32_IE=0x0500) # Required for "SHGFP_TYPE_CURRENT"
	endif()
	
	add_definitions(-DNOMINMAX)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	list(APPEND ARX_LIBRARIES gdi32 shell32 comdlg32 ole32 comctl32)
	set(ARX_HAVE_WINAPI 1)
else()
	set(ARX_HAVE_WINAPI 0)
endif()

# Direct X
if(NOT ARX_HAVE_WINAPI)
	# DirectX builds on the Win32 api
	set(USE_D3D9 OFF)
	set(USE_DSOUND OFF)
	set(USE_DINPUT8 OFF)
elseif(USE_D3D9 OR USE_DINPUT8)
	find_package(DirectX REQUIRED)
endif()

# pthread / Win32 threads
set(CMAKE_THREAD_PREFER_PTHREAD 1)
find_package(Threads REQUIRED)
if(NOT MSVC AND CMAKE_THREAD_LIBS_INIT)
	check_link_library(Threads CMAKE_THREAD_LIBS_INIT)
endif()
list(APPEND ARX_LIBARIES ${CMAKE_THREAD_LIBS_INIT})

# FreeType
find_package(Freetype REQUIRED)
if(NOT MSVC)
	check_link_library(Freetype FREETYPE_LIBRARIES)
endif()
include_directories(SYSTEM ${FREETYPE_INCLUDE_DIRS})
list(APPEND ARX_LIBRARIES ${FREETYPE_LIBRARIES})

# zlib
find_package(ZLIB REQUIRED)
if(MSVC)
	add_definitions(-DZLIB_WINAPI)
else()
	check_link_library(ZLIB ZLIB_LIBRARIES)
endif()
include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
list(APPEND ARX_LIBRARIES ${ZLIB_LIBRARIES})

# OpenGL
if(USE_OPENGL)
	find_package(OpenGL ${OPTIONAL_DEPENDENCY})
	find_package(GLEW ${OPTIONAL_DEPENDENCY})
	if(NOT MSVC AND OPENGL_FOUND AND GLEW_FOUND)
		check_link_library(OpenGL OPENGL_gl_LIBRARY)
		check_link_library(GLEW GLEW_LIBRARIES)
	endif()
endif()

# OpenAL
if(USE_OPENAL)
	find_package(OpenAL 1.1 ${OPTIONAL_DEPENDENCY})
	find_package(OpenALEFX)
	if(NOT MSVC AND OPENAL_FOUND)
		check_link_library(OpenAL OPENAL_LIBRARY)
	endif()
endif()

# SDL
if(USE_SDL)
	if(NOT MACOSX)
		# Required to avoid linking with SDLmain except for OS X where it is necessary
		# due to the need to have NSApplication correctly setup by SDLmain.
		set(SDL_BUILDING_LIBRARY 1)
	endif()
	find_package(SDL 1.2 ${OPTIONAL_DEPENDENCY})
	if(SDL_FOUND)
		if(NOT MSVC)
			check_link_library(SDL SDL_LIBRARY)
		endif()
	endif(SDL_FOUND)
endif()

# Boost
find_package(Boost 1.39 REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# Qt and DbgHelp
set(ARX_HAVE_CRASHREPORTER 0)
if(BUILD_CRASHREPORTER)
	
	# Try to find Qt 5 modules
	if(NOT HAVE_QT AND USE_QT5)
		if(USE_QT4)
			# Missing Qt5*Config.cmake files are not errors, reuduce log spam
			set(Qt_OPTIONAL_DEPENDENCY QUIET)
		else()
			set(Qt_OPTIONAL_DEPENDENCY ${OPTIONAL_DEPENDENCY})
		endif()
		find_package(Qt5Core ${Qt_OPTIONAL_DEPENDENCY})
		find_package(Qt5Gui ${Qt_OPTIONAL_DEPENDENCY})
		find_package(Qt5Widgets ${Qt_OPTIONAL_DEPENDENCY})
		find_package(Qt5Network ${Qt_OPTIONAL_DEPENDENCY})
		if(Qt5Core_FOUND AND Qt5Gui_FOUND AND Qt5Widgets_FOUND AND Qt5Network_FOUND)
			message(STATUS "Using Qt 5")
			set(HAVE_QT5 1)
			set(HAVE_QT 1)
		endif()
	endif()
	
	# Otherwise, look for Qt 4
	if(NOT HAVE_QT AND USE_QT4)
		find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork ${OPTIONAL_DEPENDENCY})
		if(QT_FOUND)
			if(NOT MSVC)
				check_link_library(QtCore QT_QTCORE_LIBRARY_RELEASE QT_QTCORE)
				check_link_library(QtGui QT_QTGUI_LIBRARY_RELEASE QT_QTGUI)
				check_link_library(QtNetwork QT_QTNETWORK_LIBRARY_RELEASE QT_QTNETWORK)
			endif()
			message(STATUS "Using Qt 4")
			set(HAVE_QT4 1)
			set(HAVE_QT 1)
		endif()
	endif()
	
	# Needed by the crash reporter
	
	if(HAVE_QT)
		if(MSVC)
			find_package(DbgHelp ${OPTIONAL_DEPENDENCY})
			if(DBGHELP_FOUND)
				set(ARX_HAVE_CRASHREPORTER 1)
			endif()
		else()
			set(ARX_HAVE_CRASHREPORTER 1)
		endif()
	endif()
	
	if(ARX_HAVE_CRASHREPORTER)
		if(HAVE_QT5)
			set(ARX_HAVE_CRASHREPORTER_QT5 1)
		elseif(HAVE_QT4)
			set(ARX_HAVE_CRASHREPORTER_QT4 1)
		endif()
	endif()
endif()

find_package(Doxygen)

find_package(PythonInterp)


# Check for optional functionality and system configuration

if(NOT MSVC)
	
	if(USE_STATIC_LIBS)
		add_ldflag("-static-libstdc++")
		if(WIN32)
			add_ldflag("-static-libgcc")
		endif()
	endif()
	
	if(${Boost_VERSION} LESS 104800)
		# Older Boost versions don't work with C++11
	elseif(USE_CXX11)
		add_cxxflag("-std=c++11")
		
		if(WIN32)
			add_definitions(-U__STRICT_ANSI__) # __argc and __argv
		endif()
	endif()
	
	# Don't expose internal symbols to the outside world by default
	add_cxxflag("-fvisibility=hidden")
	add_cxxflag("-fvisibility-inlines-hidden")
	
	# Define _POSIX_C_SOURCE and _XOPEN_SOURCE for GNU systems
	check_symbol_exists(__GLIBC__ "features.h" HAVE_GLIBC)
	set(libc_features)
	if(HAVE_GLIBC)
		list(APPEND libc_features
			-D_ISOC11_SOURCE
			-D_POSIX_C_SOURCE=200809L
			-D_XOPEN_SOURCE=600
			-D_BSD_SOURCE # Required for dirfd with glibc <= 2.19
			-D_DEFAULT_SOURCE # Required not to warn on _BSD_SOURCE with glibc >= 2.20
			-D_ATFILE_SOURCE # Required for fstatat with glibc <= 2.9
			-D_GNU_SOURCE # Not set by default because we use -std=c++?? instead of -std=gnu++??
		)
	endif()
	if(USE_NATIVE_FS AND NOT WIN32)
		list(APPEND libc_features -D_FILE_OFFSET_BITS=64)
	endif()
	string(REPLACE ";" " " CMAKE_REQUIRED_DEFINITIONS "${libc_features}")
	add_definitions(${libc_features})
	
	# Not a symbol, so we can't use check_symbol_exists
	check_compile(ARX_HAVE_BUILTIN_TRAP
		"${CMAKE_MODULE_PATH}/check_compiler_builtin_trap.cpp"
		"__builtin_trap" "compiler feature"
	)
	
	check_compile(ARX_HAVE_ATTRIBUTE_FORMAT_PRINTF
		"${CMAKE_MODULE_PATH}/check_compiler_attribute_format_printf.cpp"
		"__attribute__((format(printf, i, j)))" "compiler feature"
	)
	
	check_symbol_exists(nanosleep "time.h" ARX_HAVE_NANOSLEEP)
	
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")
	check_symbol_exists(pthread_setname_np "pthread.h" ARX_HAVE_PTHREAD_SETNAME_NP)
	if(NOT ARX_HAVE_PTHREAD_SETNAME_NP)
		check_symbol_exists(pthread_set_name_np "pthread_np.h" ARX_HAVE_PTHREAD_SET_NAME_NP)
	endif()
	check_symbol_exists(prctl "sys/prctl.h" ARX_HAVE_PRCTL)
	unset(CMAKE_REQUIRED_LIBRARIES)
	
	check_symbol_exists(uname "sys/utsname.h" ARX_HAVE_UNAME)
	check_symbol_exists(getrusage "sys/resource.h" ARX_HAVE_GETRUSAGE)
	
	check_symbol_exists(popen "stdio.h" ARX_HAVE_POPEN)
	check_symbol_exists(pclose "stdio.h" ARX_HAVE_PCLOSE)
	
	check_symbol_exists(getexecname "stdlib.h" ARX_HAVE_GETEXECNAME)
	check_symbol_exists(setenv "stdlib.h" ARX_HAVE_SETENV)
	
	find_library(LIBRT_LIBRARY rt)
	
	if(LIBRT_LIBRARY)
		set(CMAKE_REQUIRED_LIBRARIES "${LIBRT_LIBRARY}")
	endif()
	check_symbol_exists(clock_gettime "time.h" ARX_HAVE_CLOCK_GETTIME)
	if(LIBRT_LIBRARY)
		unset(CMAKE_REQUIRED_LIBRARIES)
	endif()
	if(NOT ARX_HAVE_CLOCK_GETTIME AND NOT ARX_HAVE_WINAPI)
		check_include_files("mach/clock.h;mach/clock_types.h;mach/mach_host.h"
		                    ARX_HAVE_MACH_CLOCK)
	endif()
	
	check_include_file("wordexp.h" ARX_HAVE_WORDEXP_H)
	
	check_symbol_exists(fork "unistd.h" ARX_HAVE_FORK)
	check_symbol_exists(readlink "unistd.h" ARX_HAVE_READLINK)
	check_symbol_exists(dup2 "unistd.h" ARX_HAVE_DUP2)
	check_symbol_exists(execl "unistd.h" ARX_HAVE_EXECL)
	check_symbol_exists(execlp "unistd.h" ARX_HAVE_EXECLP)
	if(NOT WIN32)
		check_symbol_exists(isatty "unistd.h" ARX_HAVE_ISATTY)
	endif()
	
	check_symbol_exists(sched_getscheduler "sched.h" ARX_HAVE_SCHED_GETSCHEDULER)
	
	check_symbol_exists(kill "signal.h" ARX_HAVE_KILL)
	
	check_symbol_exists(backtrace "execinfo.h" ARX_HAVE_BACKTRACE)
	
	check_include_file("sys/types.h" ARX_HAVE_SYS_TYPES_H)
	check_symbol_exists(getpid "unistd.h" ARX_HAVE_GETPID)
	if(CMAKE_USE_PTHREADS_INIT AND ARX_HAVE_SYS_TYPES_H AND ARX_HAVE_GETPID)
		set(ARX_HAVE_PTHREADS 1)
	endif()
	
	check_symbol_exists(sysconf "unistd.h" ARX_HAVE_SYSCONF)
	
	check_symbol_exists(sigaction "signal.h" ARX_HAVE_SIGACTION)
	
	check_symbol_exists(sysctl "sys/sysctl.h" ARX_HAVE_SYSCTL)
	
	if(USE_NATIVE_FS)
		
		check_include_file("sys/stat.h" ARX_HAVE_SYS_STAT_H)
		check_include_file("sys/errno.h" ARX_HAVE_SYS_ERRNO_H)
		check_include_file("dirent.h" ARX_HAVE_DIRENT_H)
		
		check_symbol_exists(readdir_r "dirent.h" ARX_HAVE_READDIR_R)
		
		check_symbol_exists(getcwd "unistd.h" ARX_HAVE_GETCWD)
		check_symbol_exists(fpathconf "unistd.h" ARX_HAVE_FPATHCONF)
		check_symbol_exists(pathconf "unistd.h" ARX_HAVE_PATHCONF)
		check_symbol_exists(_PC_NAME_MAX "unistd.h" ARX_HAVE_PC_NAME_MAX)
		check_symbol_exists(_PC_CASE_SENSITIVE "unistd.h" ARX_HAVE_PC_CASE_SENSITIVE)
		
		check_symbol_exists(dirfd "dirent.h" ARX_HAVE_DIRFD)
		
		check_symbol_exists(fstatat "sys/stat.h" ARX_HAVE_FSTATAT)
		
		check_symbol_exists(NAME_MAX "dirent.h" ARX_HAVE_NAME_MAX)
		
		if(ARX_HAVE_SYS_STAT_H AND ARX_HAVE_SYS_ERRNO_H AND ARX_HAVE_DIRENT_H
		   AND ARX_HAVE_READDIR_R AND ARX_HAVE_GETCWD
			 AND ((((ARX_HAVE_DIRFD AND ARX_HAVE_FPATHCONF) OR ARX_HAVE_PATHCONF)
			       AND ARX_HAVE_PC_NAME_MAX) OR ARX_HAVE_NAME_MAX))
			set(ARX_HAVE_POSIX_FILESYSTEM 1)
		elseif(STRICT_USE)
			message(FATAL_ERROR "Missing dependencies for native filesystem backend.")
		endif()
		
	endif(USE_NATIVE_FS)
	
endif(NOT MSVC)


# Sources

set(SRC_DIR src)

set(AI_SOURCES
	src/ai/PathFinder.cpp
	src/ai/PathFinderManager.cpp
	src/ai/Paths.cpp
)

set(ANIMATION_SOURCES
	src/animation/Animation.cpp
	src/animation/AnimationRender.cpp
	src/animation/Cinematic.cpp
	src/animation/CinematicKeyframer.cpp
	src/animation/Intro.cpp
)

set(AUDIO_SOURCES
	src/audio/Ambiance.cpp
	src/audio/Audio.cpp
	src/audio/AudioEnvironment.cpp
	src/audio/AudioGlobal.cpp
	src/audio/AudioResource.cpp
	src/audio/AudioSource.cpp
	src/audio/Mixer.cpp
	src/audio/Sample.cpp
	src/audio/Stream.cpp
	src/audio/codec/ADPCM.cpp
	src/audio/codec/RAW.cpp
	src/audio/codec/WAV.cpp
)

set(AUDIO_OPENAL_SOURCES
	src/audio/openal/OpenALBackend.cpp
	src/audio/openal/OpenALSource.cpp
	src/audio/openal/OpenALUtils.cpp
)

set(AUDIO_DSOUND_SOURCES
	src/audio/dsound/DSoundBackend.cpp
	src/audio/dsound/DSoundSource.cpp
)

set(CORE_SOURCES
	src/core/Application.cpp
	src/core/ArxGame.cpp
	src/core/Config.cpp
	src/core/Core.cpp
	src/core/GameTime.cpp
	src/core/Localisation.cpp
	src/core/SaveGame.cpp
	src/core/Startup.cpp
	src/util/cmdline/Parser.cpp # TODO: move to UTIL_SOURCES once it's used in the tools
)

set(GAME_SOURCES
	src/game/Camera.cpp
	src/game/Damage.cpp
	src/game/Entity.cpp
	src/game/EntityId.cpp
	src/game/EntityManager.cpp
	src/game/Equipment.cpp
	src/game/Inventory.cpp
	src/game/Item.cpp
	src/game/Levels.cpp
	src/game/Map.cpp
	src/game/Missile.cpp
	src/game/NPC.cpp
	src/game/Player.cpp
	src/game/Spells.cpp
)

set(GRAPHICS_SOURCES
	src/graphics/Draw.cpp
	src/graphics/GraphicsModes.cpp
	src/graphics/GraphicsUtility.cpp
	src/graphics/Math.cpp
	src/graphics/Renderer.cpp
	src/graphics/data/CinematicTexture.cpp
	src/graphics/data/FTL.cpp
	src/graphics/data/Mesh.cpp
	src/graphics/data/MeshManipulation.cpp
	src/graphics/data/Progressive.cpp
	src/graphics/data/TextureContainer.cpp
	src/graphics/effects/CinematicEffects.cpp
	src/graphics/effects/DrawEffects.cpp
	src/graphics/effects/Fog.cpp
	src/graphics/effects/SpellEffects.cpp
	src/graphics/font/Font.cpp
	src/graphics/font/FontCache.cpp
	src/graphics/image/Image.cpp
	src/graphics/image/stb_image.cpp
	src/graphics/image/stb_image_write.cpp
	src/graphics/particle/Particle.cpp
	src/graphics/particle/ParticleEffects.cpp
	src/graphics/particle/ParticleManager.cpp
	src/graphics/particle/ParticleSystem.cpp
	src/graphics/spells/Spells01.cpp
	src/graphics/spells/Spells02.cpp
	src/graphics/spells/Spells03.cpp
	src/graphics/spells/Spells04.cpp
	src/graphics/spells/Spells05.cpp
	src/graphics/spells/Spells06.cpp
	src/graphics/spells/Spells07.cpp
	src/graphics/spells/Spells09.cpp
	src/graphics/spells/Spells10.cpp
	src/graphics/texture/PackedTexture.cpp
	src/graphics/texture/Texture.cpp
	src/graphics/texture/TextureStage.cpp
)

set(GRAPHICS_D3D9_SOURCES
	src/graphics/d3d9/D3D9Renderer.cpp
	src/graphics/d3d9/D3D9Texture2D.cpp
	src/graphics/d3d9/D3D9TextureStage.cpp
)
set(GRAPHICS_OPENGL_SOURCES
	src/graphics/opengl/GLTexture2D.cpp
	src/graphics/opengl/GLTextureStage.cpp
	src/graphics/opengl/OpenGLRenderer.cpp
)

set(GUI_SOURCES
	src/gui/Credits.cpp
	src/gui/Interface.cpp
	src/gui/Menu.cpp
	src/gui/MenuPublic.cpp
	src/gui/MenuWidgets.cpp
	src/gui/MiniMap.cpp
	src/gui/Note.cpp
	src/gui/Speech.cpp
	src/gui/Text.cpp
	src/gui/TextManager.cpp
)

set(INPUT_SOURCES src/input/Input.cpp)
set(INPUT_DINPUT8_SOURCES src/input/DInput8Backend.cpp)
set(INPUT_SDL_SOURCES src/input/SDLInputBackend.cpp)

set(IO_SOURCES
	src/io/CinematicLoad.cpp
	src/io/Implode.cpp
	src/io/IniReader.cpp
	src/io/IniSection.cpp
	src/io/IniWriter.cpp
	src/io/IO.cpp
	src/io/SaveBlock.cpp
	src/io/Screenshot.cpp
)
set(IO_LOGGER_SOURCES
	src/io/log/ConsoleLogger.cpp
	src/io/log/LogBackend.cpp
	src/io/log/Logger.cpp
)
set(IO_LOGGER_EXTRA_SOURCES
	src/io/log/FileLogger.cpp
	src/io/log/CriticalLogger.cpp
)
set(IO_RESOURCE_SOURCES
	src/io/Blast.cpp
	src/io/resource/PakEntry.cpp
	src/io/resource/PakReader.cpp
	src/io/resource/ResourcePath.cpp
)
set(IO_LOGGER_POSIX_SOURCES src/io/log/ColorLogger.cpp)
set(IO_LOGGER_WINDOWS_SOURCES src/io/log/MsvcLogger.cpp)
set(IO_FILESYSTEM_SOURCES
	src/io/fs/FilePath.cpp
	src/io/fs/FileStream.cpp
	src/io/fs/Filesystem.cpp
	src/io/fs/SystemPaths.cpp
)
set(IO_FILESYSTEM_BOOST_SOURCES src/io/fs/FilesystemBoost.cpp)
set(IO_FILESYSTEM_POSIX_SOURCES src/io/fs/FilesystemPOSIX.cpp)
set(IO_FILESYSTEM_WINDOWS_SOURCES src/io/fs/FilesystemWindows.cpp)

set(MATH_SOURCES
	src/math/Angle.cpp
	src/math/Random.cpp
)

set(PHYSICS_SOURCES
	src/physics/Anchors.cpp
	src/physics/Attractors.cpp
	src/physics/Box.cpp
	src/physics/Clothes.cpp
	src/physics/Collisions.cpp
	src/physics/CollisionShapes.cpp
	src/physics/Physics.cpp
)

# Basic platform abstraction sources
set(PLATFORM_SOURCES
	src/platform/Dialog.cpp
	src/platform/Environment.cpp
	src/platform/Lock.cpp
	src/platform/OS.cpp
	src/platform/Platform.cpp
	src/platform/ProgramOptions.cpp
	src/platform/Time.cpp
)
if(MACOSX)
	list(APPEND PLATFORM_SOURCES src/platform/Dialog.mm)
endif()

# Extra platform abstraction - depends on the crash handler
set(PLATFORM_EXTRA_SOURCES
	src/platform/Thread.cpp
)

# Crash handler sources
set(PLATFORM_CRASHHANDLER_SOURCES src/platform/CrashHandler.cpp)
set(PLATFORM_CRASHHANDLER_IMPL_SOURCES src/platform/crashhandler/CrashHandlerImpl.cpp)
set(PLATFORM_CRASHHANDLER_POSIX_SOURCES src/platform/crashhandler/CrashHandlerPOSIX.cpp)
set(PLATFORM_CRASHHANDLER_WINDOWS_SOURCES src/platform/crashhandler/CrashHandlerWindows.cpp)

set(SCENE_SOURCES
	src/scene/ChangeLevel.cpp
	src/scene/CinematicSound.cpp
	src/scene/GameSound.cpp
	src/scene/Interactive.cpp
	src/scene/Light.cpp
	src/scene/LinkedObject.cpp
	src/scene/LoadLevel.cpp
	src/scene/Object.cpp
	src/scene/Scene.cpp
)

set(SCRIPT_SOURCES
	src/script/Script.cpp
	src/script/ScriptedAnimation.cpp
	src/script/ScriptedCamera.cpp
	src/script/ScriptedControl.cpp
	src/script/ScriptedConversation.cpp
	src/script/ScriptedInterface.cpp
	src/script/ScriptedInventory.cpp
	src/script/ScriptedIOControl.cpp
	src/script/ScriptedIOProperties.cpp
	src/script/ScriptedItem.cpp
	src/script/ScriptedLang.cpp
	src/script/ScriptedNPC.cpp
	src/script/ScriptedPlayer.cpp
	src/script/ScriptedVariable.cpp
	src/script/ScriptEvent.cpp
	src/script/ScriptUtils.cpp
)

set(UTIL_SOURCES
	src/util/String.cpp
)

set(WINDOW_SOURCES
	src/window/RenderWindow.cpp
	src/window/Window.cpp
)
set(WINDOW_SDL_SOURCES src/window/SDLWindow.cpp)
set(WINDOW_WIN32_SOURCES src/window/Win32Window.cpp)
set(WINDOW_D3D9_SOURCES src/window/D3D9Window.cpp)

# TODO manually specify theese like we do for the tools
file(GLOB_RECURSE ALL_INCLUDES RELATIVE ${CMAKE_SOURCE_DIR} ${SRC_DIR}/*.h)

include_directories(${SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/tools)


# Check for optional / alternative subsystem implementations

# Audio
if(USE_OPENAL AND OPENAL_FOUND)
	list(APPEND AUDIO_SOURCES ${AUDIO_OPENAL_SOURCES})
	list(APPEND ARX_LIBRARIES ${OPENAL_LIBRARY})
	include_directories(SYSTEM ${OPENAL_INCLUDE_DIR})
	set(ARX_HAVE_OPENAL 1)
	if(OPENALEFX_FOUND)
		include_directories(SYSTEM ${OPENAL_EFX_INCLUDE_DIR})
		set(ARX_HAVE_OPENAL_EFX 1)
	endif()
endif()
if(USE_DSOUND)
	# TODO does this need DIRECTX_FOUND?if so, the find_package(DirectX) should
	# also be run if USE_DSOUND is set but USE_D3D9 isn't
	list(APPEND AUDIO_SOURCES ${AUDIO_DSOUND_SOURCES})
	set(ARX_HAVE_DSOUND 1)
endif()

# Graphics
if(USE_D3D9 AND DIRECTX_FOUND)
	list(APPEND GRAPHICS_SOURCES ${GRAPHICS_D3D9_SOURCES})
	list(APPEND WINDOW_SOURCES ${WINDOW_WIN32_SOURCES} ${WINDOW_D3D9_SOURCES})
	list(APPEND ARX_LIBRARIES ${DIRECTX_D3D9_LIBRARY} ${DIRECTX_D3DX9_LIBRARY} ${DIRECTX_D3DCOMPILER_LIBRARY} ${DIRECTX_DXERR9_LIBRARY})
	include_directories(SYSTEM ${DIRECTX_INCLUDE_DIR})
	set(ARX_HAVE_D3D9 1)
endif()
if(USE_OPENGL AND OPENGL_FOUND AND GLEW_FOUND)
	list(APPEND GRAPHICS_SOURCES ${GRAPHICS_OPENGL_SOURCES})
	list(APPEND ARX_LIBRARIES ${GLEW_LIBRARIES} ${OPENGL_gl_LIBRARY})
	include_directories(SYSTEM ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})
	add_definitions(${GLEW_DEFINITIONS})
	set(ARX_HAVE_OPENGL 1)
endif()

# Windowing / input
if(USE_SDL AND ARX_HAVE_OPENGL AND SDL_FOUND)
	list(APPEND WINDOW_SOURCES ${WINDOW_SDL_SOURCES})
	list(APPEND INPUT_SOURCES ${INPUT_SDL_SOURCES})
	list(APPEND ARX_LIBRARIES ${SDL_LIBRARY})
	include_directories(SYSTEM ${SDL_INCLUDE_DIR})
	set(ARX_HAVE_SDL 1)
endif()
if(USE_DINPUT8 AND DIRECTX_FOUND)
	list(APPEND INPUT_SOURCES ${INPUT_DINPUT8_SOURCES})
	list(APPEND ARX_LIBRARIES ${DIRECTX_DINPUT8_LIBRARY} ${DIRECTX_DXGUID_LIBRARY} ${DIRECTX_DXERR9_LIBRARY})
	set(ARX_HAVE_DINPUT8 1)
endif()

# Logging
if(ARX_HAVE_ISATTY)
	list(APPEND IO_LOGGER_SOURCES ${IO_LOGGER_POSIX_SOURCES})
endif()
if(ARX_HAVE_WINAPI)
	list(APPEND IO_LOGGER_SOURCES ${IO_LOGGER_WINDOWS_SOURCES})
endif()
list(APPEND IO_SOURCES ${IO_LOGGER_SOURCES} ${IO_LOGGER_EXTRA_SOURCES})

list(APPEND IO_SOURCES ${IO_RESOURCE_SOURCES})

# Filesystem
if(ARX_HAVE_POSIX_FILESYSTEM)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_POSIX_SOURCES})
elseif(USE_NATIVE_FS AND ARX_HAVE_WINAPI)
	add_definitions(-DARX_HAVE_WINDOWS_FILESYSTEM)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_WINDOWS_SOURCES})
	set(ARX_HAVE_WIN32_FILESYSTEM 1)
elseif(NOT ${Boost_VERSION} LESS 104400)
	
	find_package(Boost 1.44 REQUIRED COMPONENTS filesystem system)
	set(ARX_HAVE_BOOST_FILESYSTEM_V3 1)
	add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
	add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_BOOST_SOURCES})
	list(APPEND BASE_LIBRARIES ${Boost_LIBRARIES})
	
	# BOOST_SCOPED_ENUM is implemented differently if compiled with C++11 support
	# This means the Boost.Filesystem ABI depends on what C++ version was used
	# to compile the library - check for that!
	check_compile(boost_filesystem_abi_matches
		"${CMAKE_MODULE_PATH}/check_boostfs_cxx11_scoped_enum.cpp"
		"scoped enum" "Boost ABI"
		"${Boost_LIBRARIES}"
		"mismatch, fixing"
	)
	if(NOT boost_filesystem_abi_matches)
		add_definitions(
			# Different names for different Boost versions :/
			-DBOOST_NO_SCOPED_ENUMS=1
			-DBOOST_NO_CXX11_SCOPED_ENUMS=1
		)
	endif()
	
else()
	message(FATAL_ERROR "You need either Boost >= 1.44 or Windows API"
	        "or enough POSIX functionality; Found boost version ${Boost_VERSION}")
endif()
list(APPEND IO_SOURCES ${IO_FILESYSTEM_SOURCES})

# Crash reporter
if(ARX_HAVE_CRASHREPORTER)
	if((ARX_HAVE_EXECLP OR ARX_HAVE_EXECL) AND ARX_HAVE_FORK AND ARX_HAVE_KILL)
		list(APPEND PLATFORM_CRASHHANDLER_SOURCES ${PLATFORM_CRASHHANDLER_IMPL_SOURCES})
		list(APPEND PLATFORM_CRASHHANDLER_SOURCES ${PLATFORM_CRASHHANDLER_POSIX_SOURCES})
		set(ARX_HAVE_CRASHHANDLER_POSIX 1)
	elseif(MSVC)
		list(APPEND PLATFORM_CRASHHANDLER_SOURCES ${PLATFORM_CRASHHANDLER_IMPL_SOURCES})
		list(APPEND PLATFORM_CRASHHANDLER_SOURCES ${PLATFORM_CRASHHANDLER_WINDOWS_SOURCES})
		set(ARX_HAVE_CRASHHANDLER_WINDOWS 1)
		list(APPEND ARX_LIBRARIES ${DBGHELP_LIBRARIES})
		include_directories(SYSTEM ${DBGHELP_INCLUDE_DIR})
	else()
		# Don't bother building arxcrashreporter if it will never be used.
		set(ARX_HAVE_CRASHREPORTER 0)
	endif()
endif()

if(LIBRT_LIBRARY AND (ARX_HAVE_CLOCK_GETTIME OR ARX_HAVE_CRASHHANDLER_POSIX))
	# Needed for clock_gettime and boost::interprocess on some system.
	list(APPEND BASE_LIBRARIES ${LIBRT_LIBRARY})
endif()

list(APPEND ARX_LIBRARIES ${BASE_LIBRARIES})

if(NOT MSVC)
	check_link_library(Boost Boost_LIBRARIES)
endif()

set(ARX_SOURCES
	${AI_SOURCES}
	${ANIMATION_SOURCES}
	${AUDIO_SOURCES}
	${CORE_SOURCES}
	${GAME_SOURCES}
	${GRAPHICS_SOURCES}
	${GUI_SOURCES}
	${INPUT_SOURCES}
	${IO_SOURCES}
	${MATH_SOURCES}
	${PHYSICS_SOURCES}
	${PLATFORM_SOURCES}
	${PLATFORM_EXTRA_SOURCES}
	${PLATFORM_CRASHHANDLER_SOURCES}
	${SCENE_SOURCES}
	${SCRIPT_SOURCES}
	${UTIL_SOURCES}
	${WINDOW_SOURCES}
)

# Set the icon for the Windows executable by adding this resource file to the sources
if(WIN32)
	list(APPEND ARX_SOURCES data/icons/arx-libertatis.rc)
endif()


# Prepare sources

# Create source groups
list(APPEND ALL_FILES_FOR_GROUPS ${ALL_INCLUDES} ${ARX_SOURCES})
create_source_groups(ALL_FILES_FOR_GROUPS)

configure_file("${SRC_DIR}/Configure.h.in" "Configure.h" ESCAPE_QUOTES)

configure_file("${SRC_DIR}/io/fs/PathConstants.h.in" "io/fs/PathConstants.h"
               ESCAPE_QUOTES)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/platform")
set(PLATFORM_CONFIG_H "platform/PlatformConfig.h")
configure_file("${SRC_DIR}/${PLATFORM_CONFIG_H}.in" "${PLATFORM_CONFIG_H}" ESCAPE_QUOTES)

set(VERSION_TEMPLATE "${SRC_DIR}/core/Version.cpp.in")
set(VERSION_FILE     "${CMAKE_BINARY_DIR}/core/Version.cpp")
set(VERSION_SOURCES  VERSION "VERSION" AUTHORS "AUTHORS")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/core")
version_file("${VERSION_TEMPLATE}" "${VERSION_FILE}" "${VERSION_SOURCES}" ".git")
list(APPEND ARX_SOURCES "${VERSION_FILE}")


# Add executables

add_executable_shared(arx WIN32 "${ARX_SOURCES}" "${ARX_LIBRARIES}"
                      "${ALL_INCLUDES}" "${GAMESBINDIR}")

if(ARX_HAVE_CRASHREPORTER)
	
	if(HAVE_QT5)
		
		add_definitions(
			${Qt5Core_DEFINITIONS}
			${Qt5Gui_DEFINITIONS}
			${Qt5Widgets_DEFINITIONS}
			${Qt5Network_DEFINITIONS}
			-DQT_NO_DEBUG
		)
		
		include_directories(SYSTEM
			${Qt5Core_INCLUDE_DIRS}
			${Qt5Gui_INCLUDE_DIRS}
			${Qt5Widgets_INCLUDE_DIRS}
			${Qt5Network_INCLUDE_DIRS}
		)
		
	elseif(HAVE_QT4)
		
		add_definitions(${QT_DEFINITIONS} -DQT_NO_DEBUG)
		
		include_directories(SYSTEM
			${QT_INCLUDE_DIR}
			${QT_QTCORE_INCLUDE_DIR}
			${QT_QTGUI_INCLUDE_DIR}
			${QT_QTNETWORK_INCLUDE_DIR}
		)
		
	endif()
	
	set(arxcrashreporter_SOURCES
		${PLATFORM_SOURCES}
		${IO_FILESYSTEM_SOURCES}
		${IO_LOGGER_SOURCES}
		${UTIL_SOURCES}
		tools/crashreporter/CrashReporter.cpp
		tools/crashreporter/ErrorReport.h
		tools/crashreporter/ErrorReport.cpp
		tools/crashreporter/qhexedit/Commands.h
		tools/crashreporter/qhexedit/Commands.cpp
		tools/crashreporter/qhexedit/QHexEdit.h
		tools/crashreporter/qhexedit/QHexEdit.cpp
		tools/crashreporter/qhexedit/QHexEditPrivate.h
		tools/crashreporter/qhexedit/QHexEditPrivate.cpp
		tools/crashreporter/qhexedit/XByteArray.h
		tools/crashreporter/qhexedit/XByteArray.cpp
		tools/crashreporter/tbg/TBG.h
		tools/crashreporter/tbg/TBG.cpp
		tools/crashreporter/ui/ErrorReportDialog.h
		tools/crashreporter/ui/ErrorReportDialog.cpp
		"${VERSION_FILE}"
	)
	
	if(ARX_HAVE_WINAPI)
		list(APPEND arxcrashreporter_SOURCES
			tools/crashreporter/Win32Utilities.h
			tools/crashreporter/Win32Utilities.cpp
		)
	endif()
	
	set(arxcrashreporter_UIs
		tools/crashreporter/ui/ErrorReportDialog.ui
	)
	
	set(arxcrashreporter_MOC_HEADERS
		tools/crashreporter/ui/ErrorReportDialog.h
		tools/crashreporter/qhexedit/QHexEdit.h
		tools/crashreporter/qhexedit/QHexEditPrivate.h
	)
	
	set(arxcrashreporter_RESOURCES
		tools/crashreporter/resources/CrashReporter.qrc
	)
	
	set(arxcrashreporter_MANUAL_SOURCES "${arxcrashreporter_SOURCES}")
	
	set(SRC_DIR tools/crashreporter/)
	
	create_source_groups(arxcrashreporter_SOURCES)
	
	set(arxcrashreporter_LIBRARIES
		${BASE_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
	)
	
	if(HAVE_QT5)
		
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")
		
		qt5_wrap_ui(arxcrashreporter_SOURCES ${arxcrashreporter_UIs})
		qt5_wrap_cpp(arxcrashreporter_SOURCES ${arxcrashreporter_MOC_HEADERS})
		qt5_add_resources(arxcrashreporter_SOURCES ${arxcrashreporter_RESOURCES})
		
		list(APPEND arxcrashreporter_LIBRARIES
			${Qt5Widgets_LIBRARIES}
			${Qt5Network_LIBRARIES}
		)
		
	elseif(HAVE_QT4)
		
		qt4_wrap_ui(arxcrashreporter_SOURCES ${arxcrashreporter_UIs})
		qt4_wrap_cpp(arxcrashreporter_SOURCES ${arxcrashreporter_MOC_HEADERS})
		qt4_add_resources(arxcrashreporter_SOURCES ${arxcrashreporter_RESOURCES})
		
		list(APPEND arxcrashreporter_LIBRARIES
			${QT_QTCORE_LIBRARY_RELEASE}
			${QT_QTGUI_LIBRARY_RELEASE}
			${QT_QTNETWORK_LIBRARY_RELEASE}
		)
		
	endif()
	
	if(MSVC)
		list(APPEND arxcrashreporter_LIBRARIES winmm psapi imm32)
		list(APPEND arxcrashreporter_LIBRARIES ${DBGHELP_LIBRARIES})
		include_directories(SYSTEM ${DBGHELP_INCLUDE_DIR})
	endif()
	
	add_executable_shared(arxcrashreporter WIN32 "${arxcrashreporter_SOURCES}"
	                      "${arxcrashreporter_LIBRARIES}" "" "${CMAKE_INSTALL_LIBEXECDIR}")
	
endif()

if(BUILD_TOOLS)
	
	set(arxsavetool_SOURCES
		${PLATFORM_SOURCES}
		${IO_FILESYSTEM_SOURCES}
		${IO_LOGGER_SOURCES}
		${IO_RESOURCE_SOURCES}
		${UTIL_SOURCES}
		src/core/Localisation.cpp
		src/io/SaveBlock.cpp
		src/io/IniReader.cpp
		src/io/IniSection.cpp
		tools/savetool/SaveFix.h
		tools/savetool/SaveFix.cpp
		tools/savetool/SaveTool.cpp
		tools/savetool/SaveView.h
		tools/savetool/SaveView.cpp
	)
	
	set(arxsavetool_LIBRARIES ${BASE_LIBRARIES} ${ZLIB_LIBRARIES})
	
	add_executable_shared(arxsavetool "" "${arxsavetool_SOURCES}" "${arxsavetool_LIBRARIES}" "")
	
	set(arxunpak_SOURCES
		${PLATFORM_SOURCES}
		${IO_FILESYSTEM_SOURCES}
		${IO_LOGGER_SOURCES}
		${IO_RESOURCE_SOURCES}
		${UTIL_SOURCES}
		tools/unpak/UnPak.cpp
	)
	
	set(arxunpak_LIBRARIES ${BASE_LIBRARIES})
	
	add_executable_shared(arxunpak "" "${arxunpak_SOURCES}" "${arxunpak_LIBRARIES}" "")
	
endif()


# Build and link executables

if(UNITY_BUILD)
	unity_build()
else()
	shared_build()
endif()


# Custom make targets

if(DOXYGEN_EXECUTABLE)
	set(DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/doc")
	configure_file("scripts/Doxyfile" "Doxyfile" ESCAPE_QUOTES)
	add_custom_target(doc
		#build the documentation
		COMMAND "${CMAKE_COMMAND}" -E make_directory "${DOXYGEN_OUTPUT_DIR}"
		COMMAND "${CMAKE_COMMAND}" -E chdir ${CMAKE_SOURCE_DIR}
			${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
		COMMENT "Building doxygen documentation."
		VERBATIM
	)
endif()

set(STYLE_CHECKED_SOURCES
	${ARX_SOURCES}
	${ALL_INCLUDES}
	${arxsavetool_SOURCES}
	${arxunpak_SOURCES}
	${arxcrashreporter_MANUAL_SOURCES}
)

add_style_check_target(style "${STYLE_CHECKED_SOURCES}" ARX)

if(BUILD_TESTS)
	add_subdirectory(tests ${CMAKE_SOURCE_DIR}/bin/tests)
endif()


# Install the data install script
if(INSTALL_SCRIPTS AND NOT WIN32)
	install(PROGRAMS scripts/arx-install-data DESTINATION "${SCRIPTDIR}")
endif()

# Install icon and desktop entry
if(NOT WIN32 AND NOT MACOSX)
	
	find_program(DESKTOP_FILE_VALIDATE desktop-file-validate)
	
	if(DESKTOP_FILE_VALIDATE)
		get_filename_component(ABS_DESKTOP_FILE data/icons/arx-libertatis.desktop ABSOLUTE)
		add_custom_target(
			validate_desktop_file ALL
			"${DESKTOP_FILE_VALIDATE}" "${ABS_DESKTOP_FILE}" VERBATIM
			DEPENDS "${ABS_DESKTOP_FILE}"
		)
	endif()
	
	install(FILES data/icons/arx-libertatis.png DESTINATION "${ICONDIR}" OPTIONAL)
	install(FILES data/icons/arx-libertatis.desktop DESTINATION "${APPDIR}" OPTIONAL)
	
endif()

# Install man pages
install(FILES data/man/arx.6 DESTINATION "${CMAKE_INSTALL_MANDIR}/man6" OPTIONAL)
if(BUILD_TOOLS)
	install(FILES data/man/arxsavetool.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
	        OPTIONAL)
	install(FILES data/man/arxunpak.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
	        OPTIONAL)
endif()


# Print a configuration summary

message("")
message("Configuration:")
if(NOT MSVC)
	if(UNITY_BUILD)
		set(BUILD_TYPE_SUFFIX " (unity build)")
	else()
		set(BUILD_TYPE_SUFFIX "")
	endif()
	message(" - Build type: ${CMAKE_BUILD_TYPE}${BUILD_TYPE_SUFFIX}")
endif(NOT MSVC)
print_configuration("Filesystem backend" FIRST
	ARX_HAVE_POSIX_FILESYSTEM "POSIX"
	ARX_HAVE_WIN32_FILESYSTEM "Win32"
	1                         "Boost"
)
print_configuration("Renderer"
	ARX_HAVE_OPENGL "OpenGL"
	ARX_HAVE_D3D9   "Direct3D 9"
)
print_configuration("Audio backend"
	ARX_HAVE_OPENAL "OpenAL"
	ARX_HAVE_DSOUND "Direct Sound"
)
print_configuration("Input backend"
	ARX_HAVE_SDL     "SDL"
	ARX_HAVE_DINPUT8 "DirectInput 8"
)
print_configuration("Windowing"
	ARX_HAVE_SDL     "SDL"
	ARX_HAVE_D3D9    "Win32"
)
print_configuration("Crash handler"
	ARX_HAVE_CRASHHANDLER_POSIX   "POSIX"
	ARX_HAVE_CRASHHANDLER_WINDOWS "Win32"
)
print_configuration("Crash reporter" FIRST
	ARX_HAVE_CRASHREPORTER_QT5 "Qt 5"
	ARX_HAVE_CRASHREPORTER_QT4 "Qt 4"
	1                          "disabled"
)
print_configuration("Tools" FIRST
	BUILD_TOOLS "enabled"
	1           "disabled"
)
message("")


# Detect configuration errors

if(NOT (ARX_HAVE_PTHREADS OR ARX_HAVE_WINAPI))
	message(SEND_ERROR "No supported thread libraries found.")
endif()
if(NOT (ARX_HAVE_OPENGL OR ARX_HAVE_D3D9))
	message(SEND_ERROR "No renderer available - need either OpenGL and GLEW or Direct3D 9")
endif()
if(NOT (ARX_HAVE_OPENAL OR ARX_HAVE_DSOUND))
	message(WARNING "No audio backend enabled - need either OpenAL or Direct Sound")
endif()
if(NOT (ARX_HAVE_SDL OR ARX_HAVE_DINPUT8))
	message(SEND_ERROR "No input backend available - need either SDL or DirectInput 8")
endif()
if(NOT (ARX_HAVE_SDL OR ARX_HAVE_WINAPI))
	message(SEND_ERROR "No windowing backend available - need either SDL or Windows")
endif()
if(NOT (ARX_HAVE_NANOSLEEP OR ARX_HAVE_WINAPI))
	message(SEND_ERROR "Need either nanosleep or WIN32.")
endif()
if(NOT (ARX_HAVE_CLOCK_GETTIME OR ARX_HAVE_WINAPI OR ARX_HAVE_MACH_CLOCK))
	message(SEND_ERROR "Need either clock_gettime or WIN32 or mach clock_get_time.")
endif()
