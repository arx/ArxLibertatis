project(ArxLibertatis)

if(MSVC)
	# CMake 2.8.5 or newer is needed for CMAKE_LIBRARY_ARCHITECTURE support.
	cmake_minimum_required(VERSION 2.8.5)
	# Change CMAKE_LIBRARY_ARCHITECTURE to ensure the right libs are used
	if(CMAKE_CL_64)
		set(CMAKE_LIBRARY_ARCHITECTURE "x64")
	else()
		set(CMAKE_LIBRARY_ARCHITECTURE "x86")
	endif(CMAKE_CL_64)
else()
	cmake_minimum_required(VERSION 2.8)
endif()

# For custom cmake modules.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckCXXSourceCompiles)

include(BuildSystem)
include(CompileCheck)
include(VersionString)
include(StyleCheck)
include(PrintConfiguration)
include(CreateSourceGroups)

##############################################################################

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_TOOLS "Build tools" ON)
if(APPLE)
	option(BUILD_CRASHREPORTER "Build the crash reporter" OFF)
else(APPLE)
	option(BUILD_CRASHREPORTER "Build the crash reporter" ON)
endif(APPLE)
option(BUILD_EDITOR "Build editor" OFF)
option(BUILD_EDIT_LOADSAVE "Build save/load functions only used by the editor" ON)
option(UNITY_BUILD "Unity build" OFF)
option(DEBUG_EXTRA "Expensive debug options" OFF)
option(USE_OPENAL "Build the OpenAL audio backend." ON)
option(USE_DSOUND "Build the DirectSound audio backend." ON)
option(USE_OPENGL "Build the OpenGL renderer backend." ON)
option(USE_D3D9 "Build the Direct3D 9 renderer backend." ON)
option(USE_SDL "Build the SDL windowing backend." ON)
option(USE_DINPUT8 "Build the DirectInput 8 input backend." ON)
option(USE_NATIVE_FS "Use the native (POSIX / Win32) filesystem backend instead of boost." ON)


if(NOT WIN32)
	if(CMAKE_VERSION VERSION_LESS 2.8.5)
		set(CMAKE_INSTALL_DATAROOTDIR "share" CACHE
		    STRING "read-only architecture-independent data root (share) (relative to prefix).")
		mark_as_advanced(CMAKE_INSTALL_DATAROOTDIR)
		set(CMAKE_INSTALL_BINDIR "bin" CACHE
		    STRING "user executables (bin) (relative to prefix).")
		mark_as_advanced(CMAKE_INSTALL_BINDIR)
		set(CMAKE_INSTALL_LIBEXECDIR "libexec" CACHE
		    STRING "program executables (libexec) (relative to prefix).")
		mark_as_advanced(CMAKE_INSTALL_LIBEXECDIR)
		set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_DATAROOTDIR}/man" CACHE
		    STRING "man documentation (DATAROOTDIR/man) (relative to prefix).")
		mark_as_advanced(CMAKE_INSTALL_MANDIR)
		foreach(dir BINDIR LIBEXECDIR DATAROOTDIR MANDIR)
			if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_${dir}})
				set(CMAKE_INSTALL_FULL_${dir} "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_${dir}}")
			else()
				set(CMAKE_INSTALL_FULL_${dir} "${CMAKE_INSTALL_${dir}}")
			endif()
		endforeach()
	else()
		include(GNUInstallDirs)
	endif()
	set(ICONDIR "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps" CACHE
	    STRING "Install location for icons (relative to prefix).")
	mark_as_advanced(ICONDIR)
	set(APPDIR "${CMAKE_INSTALL_DATAROOTDIR}/applications" CACHE
	    STRING "Install location for .desktop files (relative to prefix).")
	mark_as_advanced(APPDIR)
	set(GAMESBINDIR "${CMAKE_INSTALL_BINDIR}" CACHE
	    STRING "Install location for game executables (relative to prefix).")
	mark_as_advanced(GAMESBINDIR)
endif()


if((NOT LAST_CMAKE_CXX_FLAGS STREQUAL CMAKE_CXX_FLAGS) OR (NOT LAST_CMAKE_CXX_COMPILER STREQUAL CMAKE_CXX_COMPILER))
	force_recheck_library(DevIL IL)
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
	set(LAST_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE INTERNAL "The last C++ compiler flags.")
	set(LAST_CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE INTERNAL "The last C++ compiler.")
endif()

if(WIN32)
	if(NOT MSVC)
		# We need to define WINVER for MinGW32 when requiring anything newer than Win95 / WinNT
		add_definitions(-DWINVER=0x0500) # Require at least Windows 2000
	endif()
	set(ARX_HAVE_WINAPI 1)
else()
	set(ARX_HAVE_WINAPI 0)
endif()

if(NOT ARX_HAVE_WINAPI)
	# The is no DirectX under linux
	set(USE_D3D9 OFF)
	set(USE_DSOUND OFF)
	set(USE_DINPUT8 OFF)
else()
	if(USE_D3D9 OR USE_DINPUT8)
		find_package(DirectX REQUIRED)
	endif()
endif()

find_package(Freetype REQUIRED)
find_package(DevIL REQUIRED)
find_package(ZLIB REQUIRED)
set(CMAKE_THREAD_PREFER_PTHREAD 1)
find_package(Threads REQUIRED)
if(USE_OPENGL)
	find_package(OpenGL)
	find_package(GLEW)
endif()
if(USE_OPENAL)
	find_package(OpenAL 1.1)
	find_package(OpenALEFX)
endif()
find_package(Doxygen)
find_package(PythonInterp)
if(USE_SDL)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	else()
		# Required to avoid linking with SDLmain except for OS X where it is necessary
		# due to the need to have NSApplication correctly setup by SDLmain.
		set(SDL_BUILDING_LIBRARY 1)
	endif()
	find_package(SDL 1.2 EXACT)
endif()

if(MSVC)
	# Link statically with Boost under Windows
	set(Boost_USE_STATIC_LIBS ON)
endif(MSVC)

set(BOOST_COMPONENTS program_options)
find_package(Boost 1.39 REQUIRED COMPONENTS ${BOOST_COMPONENTS})

set(ARX_HAVE_CRASHREPORTER 0)
if(BUILD_CRASHREPORTER)
	# Needed by the crash reporter
	find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork)
	if(QT_FOUND)
		if(MSVC)
			find_package(DbgHelp)
			if(DBGHELP_FOUND)
				set(ARX_HAVE_CRASHREPORTER 1)
			endif()
		else()
			set(ARX_HAVE_CRASHREPORTER 1)
		endif()
	endif()
endif()

mark_as_advanced(IL_INCLUDE_DIR)
mark_as_advanced(ILUT_LIBRARIES)
mark_as_advanced(ILU_LIBRARIES)
mark_as_advanced(IL_LIBRARIES)

set(SRC_DIR src)

if(MSVC)
	# Disable deprecation warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	add_definitions(/wd4995)        # 'func': name was marked as #pragma deprecated

	# TEMP - disable warning caused by the F2L removal
	add_definitions(/wd4244)        # Conversion from 'float' to 'long', possible loss of data

	# TEMP - disable warning caused by conversion from a 64-bit type to a 32-bit one...
	if(CMAKE_CL_64)
		add_definitions(/wd4267)      # Conversion from 'size_t' to 'xxx', possible loss of data
	endif()

	add_definitions(/wd4127)        # warning C4127: conditional expression is constant
	add_definitions(/wd4201)        # warning C4201: nonstandard extension used : nameless struct/union
	add_definitions(/wd4503)        # warning C4503: 'xxx' : decorated name length exceeded, name was truncated
	
	if(NOT DEBUG_EXTRA)
		add_definitions(-D_HAS_ITERATOR_DEBUGGING=0)
		add_definitions(-D_SECURE_SCL=0)
		add_definitions(-D_SECURE_SCL_THROWS=0)
		add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
	endif()

	# Disable exceptions & rtti
	add_definitions(/GR-)           # No RTTI

	# Enable multiprocess build
	add_definitions(/MP)
	
	# Ensure we aren't using functionalities not found under Window XP SP1
	add_definitions(-D_WIN32_WINNT=0x0502)

	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
		# Disable Run time checks
		string(REGEX REPLACE "/RTC1" "" ${flag_var} "${${flag_var}}")

		# Change runtime library from "Multi-threaded Debug DLL" to "Multi-threaded DLL"
		string(REGEX REPLACE "/MDd" "/MD" ${flag_var} "${${flag_var}}")

		# Remove definition of _DEBUG as it might conflict with libs we're linking with
		string(REGEX REPLACE "/D_DEBUG" "/DNDEBUG" ${flag_var} "${${flag_var}}")
		
		# Force to always compile with warning level 3
		string(REGEX REPLACE "/W[0-4]" "/W3" ${flag_var} "${${flag_var}}")
	endforeach(flag_var)

	# Avoid warning during link
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
	
	# Disable randomized base address (for better callstack matching)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DYNAMICBASE:NO")
	
	# Always generate a PDB file
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")

	# Enable compiler optimization in release
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Oi /Ot /GL /arch:SSE2 /GS- /fp:fast")
	
	# Always build with debug information
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
	
	# Enable linker optimization in release
	#  /OPT:REF   Eliminate unreferenced code
	#  /OPT:ICF   COMDAT folding (merge functions generating the same code)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /LTCG")

	# This is needed to use ZLIB
	add_definitions(-DZLIB_WINAPI)

else(MSVC)
	
	# Check the dependencies so there won't by cryptic link errors later on when found libraries have the wrong architecture (32- vs. 64-bit)
	if(CMAKE_THREAD_LIBS_INIT)
		check_link_library(Threads CMAKE_THREAD_LIBS_INIT)
	endif()
	check_link_library(DevIL IL_LIBRARIES IL)
	check_link_library(ZLIB ZLIB_LIBRARIES)
	check_link_library(Freetype FREETYPE_LIBRARIES)
	if(USE_OPENAL AND OPENAL_FOUND)
		check_link_library(OpenAL OPENAL_LIBRARY)
	endif()
	if(USE_OPENGL AND OPENGL_FOUND AND GLEW_FOUND)
		check_link_library(OpenGL OPENGL_gl_LIBRARY)
		check_link_library(GLEW GLEW_LIBRARIES)
	endif()
	if(USE_SDL AND SDL_FOUND)
		check_link_library(SDL SDL_LIBRARY)
	endif()
	
	if(ARX_HAVE_CRASHREPORTER)
		check_link_library(QtCore QT_QTCORE_LIBRARY_RELEASE QT_QTCORE)
		check_link_library(QtGui QT_QTGUI_LIBRARY_RELEASE QT_QTGUI)
		check_link_library(QtNetwork QT_QTNETWORK_LIBRARY_RELEASE QT_QTNETWORK)
	endif()
	
	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "GNU"
	   OR ${CMAKE_SYSTEM_NAME} MATCHES "kFreeBSD")
		set(CMAKE_REQUIRED_DEFINITIONS "-D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=600")
		add_definitions(-D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=600)
		if(${CMAKE_SYSTEM_NAME} MATCHES "kFreeBSD")
			set(CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS} -D_BSD_SOURCE")
			add_definitions(-D_BSD_SOURCE)
		endif()
	endif()
	
	check_symbol_exists(nanosleep "time.h" ARX_HAVE_NANOSLEEP)
	
	check_symbol_exists(pthread_setname_np "pthread.h" ARX_HAVE_PTHREAD_SETNAME_NP)
	check_symbol_exists(pthread_set_name_np "pthread.h" ARX_HAVE_PTHREAD_SET_NAME_NP)
	check_symbol_exists(prctl "sys/prctl.h" ARX_HAVE_PRCTL)
	check_symbol_exists(PR_SET_NAME "linux/prctl.h" ARX_HAVE_PR_SET_NAME)
	
	check_symbol_exists(uname "sys/utsname.h" ARX_HAVE_UNAME)
	check_symbol_exists(getrusage "sys/resource.h" ARX_HAVE_GETRUSAGE)
	
	check_symbol_exists(popen "stdio.h" ARX_HAVE_POPEN)
	check_symbol_exists(pclose "stdio.h" ARX_HAVE_PCLOSE)
	
	find_library(LIBRT_LIBRARY rt)
	
	if(LIBRT_LIBRARY)
		set(CMAKE_REQUIRED_LIBRARIES "${LIBRT_LIBRARY}")
	endif()
	check_symbol_exists(clock_gettime "time.h" ARX_HAVE_CLOCK_GETTIME)
	if(LIBRT_LIBRARY)
		unset(CMAKE_REQUIRED_LIBRARIES)
	endif()
	if(NOT ARX_HAVE_CLOCK_GETTIME AND NOT ARX_HAVE_WINAPI)
		check_include_files("mach/clock.h;mach/clock_types.h;mach/mach_host.h" ARX_HAVE_MACH_CLOCK)
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
	
	check_symbol_exists(sysconf "unistd.h" ARX_HAVE_SYSCONF)
	
	check_symbol_exists(sigaction "signal.h" ARX_HAVE_SIGACTION)
	
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
		endif()
		
	endif()
	
	# Warning level
	add_cxxflag("-Wall")
	add_cxxflag("-Wextra")
	add_cxxflag("-Wformat=2")
	add_cxxflag("-Wundef")
	add_cxxflag("-Wpointer-arith")
	add_cxxflag("-Wcast-qual")
	add_cxxflag("-Woverloaded-virtual")
	add_cxxflag("-Wlogical-op")
	
	add_cxxflag("-Wliteral-conversion")
	add_cxxflag("-Wshift-overflow")
	add_cxxflag("-Woverflow")
	add_cxxflag("-Wbool-conversions")
	
	# TODO enable:
	# add_cxxflag("-Wconversion") # very noisy
	# add_cxxflag("-Wsign-conversion") # very noisy
	# add_cxxflag("-Wmissing-declarations") # to catch functions that should be marked as static
	# add_cxxflag("-Wredundant-decls") # to catch extern definitions in .cpp files (with UNITYBUILD)
	
	if(CMAKE_BUILD_TYPE STREQUAL "")
		set(CMAKE_BUILD_TYPE "Release")
	endif()
	
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		
		#Debug
		add_definitions(-D_DEBUG)
		
		check_compiler_flag(RESULT "-g3")
		if(NOT RESULT STREQUAL "")
			string(REGEX REPLACE "-g(|[0-9]|gdb)" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
		endif()
		
		check_compiler_flag(RESULT "-O0")
		string(REGEX REPLACE "-O[0-9]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
		
	elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
		
		if((NOT CMAKE_CXX_FLAGS MATCHES "-g(|[0-9]|gdb)")
			 AND (NOT CMAKE_CXX_FLAGS_RELEASE MATCHES "-g(|[0-9]|gdb)"))
			add_cxxflag("-g2")
		endif()
		
	endif()
	
	if(DEBUG_EXTRA)
		add_cxxflag("-ftrapv") # to add checks for (undefined) signed integer overflow
		add_cxxflag("-fbounds-checking")
		add_cxxflag("-fcatch-undefined-behavior")
		add_cxxflag("-Wstrict-aliasing=1")
	else()
		# -Wuninitialized causes too many false positives
		add_cxxflag("-Wno-uninitialized")
		# (clang only) Conflicts with using const variables for configuration.
		add_cxxflag("-Wno-constant-logical-operand")
	endif()
	
	# Because we are lazy
	add_ldflag("-Wl,--as-needed")

	# Xcode does not support -isystem yet
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		add_cxxflag("-Wno-undef")
	endif()
	
	# check support for __attribute__((format(printf, i, j)))
	check_cxx_source_compiles(
		"void my_printf(const char *, ...) __attribute__((format(printf, 1, 2))); int main() { return 0; };\n"
		ARX_HAVE_ATTRIBUTE_FORMAT_PRINTF
	)
	
endif(MSVC)

if(WIN32)
	# Define default user and data directories.
	if(NOT DEFINED USER_DIR)
		set(USER_DIR "Arx Libertatis")
	endif()
	if(NOT DEFINED USER_DIR_PREFIXES)
		set(USER_DIR_PREFIXES "%FOLDERID_SavedGames%")
	endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# Define default user and data directories.
	if(NOT DEFINED DATA_DIR)
		set(DATA_DIR "ArxLibertatis")
	endif()
	if(NOT DEFINED USER_DIR)
		set(USER_DIR "ArxLibertatis")
	endif()
	if(NOT DEFINED DATA_DIR_PREFIXES)
		set(DATA_DIR_PREFIXES "/Applications")
	endif()
	if(NOT DEFINED USER_DIR_PREFIXES)
		set(USER_DIR_PREFIXES "~/Library/Application Support")
	endif()
else()
	# Define default user and data directories.
	if(NOT DEFINED DATA_DIR)
		set(DATA_DIR "games/arx")
	endif()
	if(NOT DEFINED USER_DIR)
		set(USER_DIR "arx")
	endif()
	if(NOT DEFINED CONFIG_DIR)
		set(CONFIG_DIR "arx")
	endif()
	if(NOT DEFINED DATA_DIR_PREFIXES)
		set(DATA_DIR_PREFIXES "$XDG_DATA_DIRS")
	endif()
	if(NOT DEFINED USER_DIR_PREFIXES)
		set(USER_DIR_PREFIXES "$XDG_DATA_HOME")
	endif()
	if(NOT DEFINED CONFIG_DIR_PREFIXES)
		set(CONFIG_DIR_PREFIXES "$XDG_CONFIG_HOME")
	endif()
endif()

# Check that all required functionality is available.
if(CMAKE_USE_PTHREADS_INIT AND ARX_HAVE_SYS_TYPES_H AND ARX_HAVE_GETPID)
	set(ARX_HAVE_PTHREADS 1)
endif()

# Preprocessor definitions
if(ARX_HAVE_WINAPI)
	add_definitions(-DNOMINMAX)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
endif(ARX_HAVE_WINAPI)

# Sources
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
)

set(EDITOR_SOURCES
	src/core/Dialog.cpp
	src/script/ScriptDebugger.cpp
	src/script/ScriptDebuggerDialog.cpp
)

set(GAME_SOURCES
	src/game/Damage.cpp
	src/game/Equipment.cpp
	src/game/Inventory.cpp
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
	src/graphics/spells/Spells08.cpp
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
	src/platform/Platform.cpp
	src/platform/String.cpp
	src/platform/Time.cpp
)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
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

set(WINDOW_SOURCES
	src/window/RenderWindow.cpp
	src/window/Window.cpp
)
set(WINDOW_SDL_SOURCES src/window/SDLWindow.cpp)
set(WINDOW_WIN32_SOURCES src/window/Win32Window.cpp)
set(WINDOW_D3D9_SOURCES src/window/D3D9Window.cpp)

file(GLOB_RECURSE ALL_INCLUDES ${SRC_DIR}/*.h)

include_directories(${SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/tools)
include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS} ${IL_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})

# Setting the BOOST_ROOT will result in linking failures on Visual Studio as found
# release and debug libs get mixed up. On windows we will empty out the
# Boost_LIBRARIES list and count on the auto-linking feature boost provides.
# http://www.boost.org/doc/libs/1_47_0/more/getting_started/windows.html#auto-linking
if (MSVC)
	# Reset libraries list so VC will perform auto-linking.
	set (Boost_LIBRARIES "")
	
	# Not needed anymore as BOOST_ROOT finds these properly.
	# Didnt remove yet eiher so nothing breaks, added empty checks instead to not add duplicated.
	if ("${Boost_INCLUDE_DIRS}" STREQUAL "")
		set (Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} $ENV{BOOST_ROOT}/include)
	endif ()
	if ("${Boost_LIBRARY_DIRS}" STREQUAL "")
		set (Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} $ENV{BOOST_ROOT}/lib)
	endif ()
	
	# Add boost lib directory to the list of libraries paths
	link_directories(${Boost_LIBRARY_DIRS})
endif ()

set(ARX_LIBRARIES ${FREETYPE_LIBRARIES} ${ZLIB_LIBRARIES} ${IL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

if(ARX_HAVE_WINAPI)
	list(APPEND ARX_LIBRARIES gdi32 shell32 comdlg32 ole32 comctl32)
endif()

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
	# TODO does this need DIRECTX_FOUND? if so, the find_package(DirectX) should also be run if USE_DSOUND is set but USE_D3D9 isn't
	list(APPEND AUDIO_SOURCES ${AUDIO_DSOUND_SOURCES})
	set(ARX_HAVE_DSOUND 1)
endif()

if(USE_D3D9 AND DIRECTX_FOUND)
	list(APPEND GRAPHICS_SOURCES ${GRAPHICS_D3D9_SOURCES})
	list(APPEND WINDOW_SOURCES ${WINDOW_WIN32_SOURCES} ${WINDOW_D3D9_SOURCES})
		
	list(APPEND ARX_LIBRARIES ${DIRECTX_D3D9_LIBRARY} ${DIRECTX_D3DX9_LIBRARY} ${DIRECTX_D3DCOMPILER_LIBRARY} ${DIRECTX_DXERR9_LIBRARY})
	include_directories(SYSTEM ${DIRECTX_INCLUDE_DIR})
	
	set(ARX_HAVE_D3D9 1)
endif()

if(USE_OPENGL AND OPENGL_FOUND AND GLEW_FOUND)
	list(APPEND GRAPHICS_SOURCES ${GRAPHICS_OPENGL_SOURCES})
	list(APPEND ARX_LIBRARIES ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES})
	include_directories(SYSTEM ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})
	set(ARX_HAVE_OPENGL 1)
	add_definitions(-DGL_GLEXT_PROTOTYPES)
endif()

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

if(ARX_HAVE_ISATTY)
	list(APPEND IO_LOGGER_SOURCES ${IO_LOGGER_POSIX_SOURCES})
endif()
if(ARX_HAVE_WINAPI)
	list(APPEND IO_LOGGER_SOURCES ${IO_LOGGER_WINDOWS_SOURCES})
endif()
list(APPEND IO_SOURCES ${IO_LOGGER_SOURCES} ${IO_LOGGER_EXTRA_SOURCES})

list(APPEND IO_SOURCES ${IO_RESOURCE_SOURCES})

set(BASE_LIBRARIES)

if(ARX_HAVE_POSIX_FILESYSTEM)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_POSIX_SOURCES})
elseif(USE_NATIVE_FS AND ARX_HAVE_WINAPI)
	add_definitions(-DARX_HAVE_WINDOWS_FILESYSTEM)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_WINDOWS_SOURCES})
	set(ARX_HAVE_WIN32_FILESYSTEM 1)
elseif((Boost_MAJOR_VERSION GREATER 1) OR (NOT Boost_MINOR_VERSION LESS 44))
	find_package(Boost 1.44 REQUIRED COMPONENTS ${BOOST_COMPONENTS} filesystem system)
	set(ARX_HAVE_BOOST_FILESYSTEM_V3 1)
	add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_BOOST_SOURCES})
	list(APPEND BASE_LIBRARIES ${Boost_LIBRARIES})
else()
	message(FATAL_ERROR "You need either Boost >= 1.44 or Windows API or enough POSIX functionality; Found boost version ${Boost_VERSION}")
endif()
list(APPEND IO_SOURCES ${IO_FILESYSTEM_SOURCES})

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
	${WINDOW_SOURCES}
)

# Set the icon for the Windows executable by adding this resource file to the sources
if(MSVC)
	SET(ARX_SOURCES ${ARX_SOURCES} data/icons/arx-libertatis.rc)
endif()

if(BUILD_EDITOR)
	list(APPEND ARX_SOURCES ${EDITOR_SOURCES})
endif()

add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)

#create source groups
list(APPEND ALL_FILES_FOR_GROUPS ${ALL_INCLUDES} ${ARX_SOURCES})
create_source_groups(ALL_FILES_FOR_GROUPS)

configure_file("${SRC_DIR}/Configure.h.in" "Configure.h" ESCAPE_QUOTES)

configure_file("${SRC_DIR}/io/fs/PathConstants.h.in" "io/fs/PathConstants.h"
               ESCAPE_QUOTES)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/platform")
set(PLATFORM_CONFIG_H "platform/PlatformConfig.h")
configure_file("${SRC_DIR}/${PLATFORM_CONFIG_H}.in" "${PLATFORM_CONFIG_H}" ESCAPE_QUOTES)

set(VERSION_FILE "${CMAKE_BINARY_DIR}/core/Version.cpp")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/core")
version_file("${SRC_DIR}/core/Version.cpp.in" "${VERSION_FILE}" "VERSION" ".git")
list(APPEND ARX_SOURCES "${VERSION_FILE}")


# Add executables

if(WIN32 OR APPLE)
	set(arx_INSTALL DESTINATION "bin")
else()
	set(arx_INSTALL DESTINATION "${GAMESBINDIR}")
endif()

add_executable_shared(arx WIN32 "${ARX_SOURCES}" "${ARX_LIBRARIES}"
                      "${ALL_INCLUDES}" "${arx_INSTALL}")

if(ARX_HAVE_CRASHREPORTER)
	
	add_definitions(${QT_DEFINITIONS} -DQT_NO_DEBUG)
	
	include_directories(SYSTEM
		"${QT_INCLUDE_DIR}"
		"${QT_QTCORE_INCLUDE_DIR}"
		"${QT_QTGUI_INCLUDE_DIR}"
		"${QT_QTNETWORK_INCLUDE_DIR}"
	)
	
	set(arxcrashreporter_SOURCES
		${PLATFORM_SOURCES}
		${IO_FILESYSTEM_SOURCES}
		${IO_LOGGER_SOURCES}
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
	
	qt4_wrap_ui(arxcrashreporter_SOURCES ${arxcrashreporter_UIs})
	qt4_wrap_cpp(arxcrashreporter_SOURCES ${arxcrashreporter_MOC_HEADERS})
	qt4_add_resources(arxcrashreporter_SOURCES ${arxcrashreporter_RESOURCES})
	
	set(SRC_DIR tools/crashreporter/)
	
	create_source_groups(arxcrashreporter_SOURCES)
	
	set(arxcrashreporter_LIBRARIES
		${BASE_LIBRARIES}
		${QT_QTCORE_LIBRARY_RELEASE}
		${QT_QTGUI_LIBRARY_RELEASE}
		${QT_QTNETWORK_LIBRARY_RELEASE}
		${CMAKE_THREAD_LIBS_INIT}
	)
	
	if(MSVC)
		list(APPEND arxcrashreporter_LIBRARIES winmm psapi imm32)
		list(APPEND arxcrashreporter_LIBRARIES ${DBGHELP_LIBRARIES})
		include_directories(SYSTEM ${DBGHELP_INCLUDE_DIR})
	endif()
	
	if(WIN32 OR APPLE)
		set(arxcrashreporter_INSTALL DESTINATION "bin")
	else()
		set(arxcrashreporter_INSTALL DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}")
	endif()
	
	add_executable_shared(arxcrashreporter WIN32 "${arxcrashreporter_SOURCES}"
	                      "${arxcrashreporter_LIBRARIES}" "" "${arxcrashreporter_INSTALL}")
	
endif()

if(BUILD_TOOLS)
	
	set(arxsavetool_SOURCES
		${PLATFORM_SOURCES}
		${IO_FILESYSTEM_SOURCES}
		${IO_LOGGER_SOURCES}
		${IO_RESOURCE_SOURCES}
		src/core/Localisation.cpp
		src/io/SaveBlock.cpp
		src/io/IniReader.cpp
		src/io/IniSection.cpp
		tools/savetool/SaveFix.cpp
		tools/savetool/SaveTool.cpp
		tools/savetool/SaveView.cpp
	)
	
	set(arxsavetool_LIBRARIES ${BASE_LIBRARIES} ${ZLIB_LIBRARIES})
	
	add_executable_shared(arxsavetool "" "${arxsavetool_SOURCES}" "${arxsavetool_LIBRARIES}" "")
	
	set(arxunpak_SOURCES
		${PLATFORM_SOURCES}
		${IO_FILESYSTEM_SOURCES}
		${IO_LOGGER_SOURCES}
		${IO_RESOURCE_SOURCES}
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

add_custom_target(remake
	#clean and compile with 1 thread per core
	COMMAND make clean && rm CMakeCache.txt && cmake ${CMAKE_SOURCE_DIR} -G\"Unix Makefiles\" && make -j`getconf _NPROCESSORS_ONLN`
)

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

file(GLOB_RECURSE TOOLS_INCLUDES ${CMAKE_SOURCE_DIR}/tools/*.h)
set(STYLE_CHECKED_SOURCES
	${ARX_SOURCES}
	${arxsavetool_SOURCES}
	${arxunpak_SOURCES}
	${arxcrashreporter_MANUAL_SOURCES}
)
set(STYLE_CHECKED_INCLUDES
	${ALL_INCLUDES}
	${TOOLS_INCLUDES}
)

add_style_check_target(style "${STYLE_CHECKED_SOURCES}" "${STYLE_CHECKED_INCLUDES}")

if(BUILD_TESTS)
	add_subdirectory(tests ${CMAKE_SOURCE_DIR}/bin/tests)
endif()


# Install icon and desktop entry
if(NOT WIN32 AND NOT APPLE)
	
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
if(NOT WIN32)
	install(FILES data/man/arx.6 DESTINATION "${CMAKE_INSTALL_MANDIR}/man6" OPTIONAL)
	if(BUILD_TOOLS)
		install(FILES data/man/arxsavetool.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1" OPTIONAL)
		install(FILES data/man/arxunpak.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1" OPTIONAL)
	endif()
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
	if(CMAKE_BUILD_TYPE STREQUAL "")
		message(" - Build type: Release${BUILD_TYPE_SUFFIX}")
	else()
		message(" - Build type: ${CMAKE_BUILD_TYPE}${BUILD_TYPE_SUFFIX}")
	endif()
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
	ARX_HAVE_CRASHREPORTER "enabled"
	1                      "disabled"
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
