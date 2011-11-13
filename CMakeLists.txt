cmake_minimum_required(VERSION 2.8.5)
project(ArxFatalis)

# For custom cmake modules.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckCXXSourceCompiles)

include(BuildSystem)
include(CompileCheck)
include(VersionString)

# Change CMAKE_LIBRARY_ARCHITECTURE to ensure the right libs are used
if(MSVC)
	if(CMAKE_CL_64)
		set(CMAKE_LIBRARY_ARCHITECTURE "x64")
	else()
		set(CMAKE_LIBRARY_ARCHITECTURE "x86")
	endif(CMAKE_CL_64)
endif()

# Accepts a variable holding the source files
# and creates source groups (for VS, Xcode etc)
# that replicate the folder hierarchy on disk
macro( create_source_groups source_files_variable )
	foreach( source_file ${${source_files_variable}} )
		string( REGEX REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" relative_directory "${source_file}")
		string( REGEX REPLACE "[\\\\/][^\\\\/]*$" "" relative_directory "${relative_directory}")
		string( REGEX REPLACE "^[\\\\/]" "" relative_directory "${relative_directory}")

		if( WIN32 )
			string( REGEX REPLACE "/" "\\\\" relative_directory "${relative_directory}" )
		endif( WIN32 )

		source_group( "${relative_directory}" FILES ${source_file} )
	endforeach()
endmacro()

##############################################################################

option(ARX_BUILD_TESTS "Build tests" OFF)
option(ARX_BUILD_TOOLS "Build tools" ON)
option(ARX_BUILD_EDITOR "Build editor" OFF)
option(ARX_BUILD_EDIT_LOADSAVE "Build save/load functions only used by the editor" ON)
option(ARX_USE_UNITYBUILD "Unity build" OFF)
option(ARX_DEBUG_EXTRA "Expensive debug options" OFF)
option(ARX_USE_OPENAL "Build the OpenAL audio backend." ON)
option(ARX_USE_DSOUND "Build the DirectSound audio backend." ON)
option(ARX_USE_OPENGL "Build the OpenGL renderer backend." ON)
option(ARX_USE_D3D9 "Build the Direct3D 9 renderer backend." ON)
option(ARX_USE_SDL "Build the SDL windowing backend." ON)
option(ARX_USE_DINPUT8 "Build the DirectInput 8 input backend." ON)

if((NOT LAST_CMAKE_CXX_FLAGS STREQUAL CMAKE_CXX_FLAGS) OR (NOT LAST_CMAKE_CXX_COMPILER STREQUAL CMAKE_CXX_COMPILER))
	force_recheck_library(DevIL IL)
	force_recheck_library(ZLIB)
	force_recheck_library(Freetype)
	force_recheck_library(Threads)
	force_recheck_library(OpenAL)
	force_recheck_library(OpenGL)
	force_recheck_library(GLEW)
	force_recheck_library(Boost)
	unset(Boost_INCLUDE_DIR CACHE)
	set(LAST_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE INTERNAL "The last C++ compiler flags.")
	set(LAST_CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE INTERNAL "The last C++ compiler.")
endif()

if(WINE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_WIN32 -U__CYGWIN__ -U__WIN32__ -UWIN32")
	set(HAVE_WINAPI 1)
elseif(WIN32)
	if(NOT MSVC)
		# We need to define WINVER for MinGW32 when requiring anything newer than Win95 / WinNT
		add_definitions(-DWINVER=0x0500) # Require at least Windows 2000
	endif()
	set(HAVE_WINAPI 1)
else()
	set(HAVE_WINAPI 0)
endif()

if(NOT HAVE_WINAPI)
	# The is no DirectX under linux
	set(ARX_USE_D3D9 OFF)
	set(ARX_USE_DSOUND OFF)
	set(ARX_USE_DINPUT8 OFF)
else()
	if(ARX_USE_D3D9)
		find_package(DirectX REQUIRED)
	endif()
endif()

find_package(Freetype REQUIRED)
find_package(DevIL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)
if(ARX_USE_OPENGL)
	find_package(OpenGL REQUIRED)
	find_package(GLEW REQUIRED)
endif()
if(ARX_USE_OPENAL)
	find_package(OpenAL 1.1 REQUIRED)
	find_package(OpenALEFX)
endif()
find_package(Doxygen)
find_package(PythonInterp)
if(ARX_USE_SDL)
	set(SDL_BUILDING_LIBRARY 1)		# Required to avoid linking with SDLmain...
	find_package(SDL 1.2 EXACT REQUIRED)
endif()

if(MSVC)
	# Link statically with Boost under Windows
	set(Boost_USE_STATIC_LIBS ON)
endif(MSVC)

find_package(Boost REQUIRED)

mark_as_advanced(IL_INCLUDE_DIR)
mark_as_advanced(ILUT_LIBRARIES)
mark_as_advanced(ILU_LIBRARIES)
mark_as_advanced(IL_LIBRARIES)

set(SRC_DIR src)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

if(MSVC)
	# Remove targets we don't really use (RelWithDebInfo, MinSizeRel)
	# TODO - Does not seem to work...
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Using only Debug and Release" FORCE)
	
	# Disable deprecation warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	add_definitions(/wd4995)		# 'func': name was marked as #pragma deprecated
	
	# TEMP - disable warning caused by the F2L removal
	add_definitions(/wd4244)    	# Conversion from 'float' to 'long', possible loss of data
	
	# TEMP - disable warning caused by conversion from a 64-bit type to a 32-bit one...
	if(CMAKE_CL_64)
		add_definitions(/wd4267)	# Conversion from 'size_t' to 'xxx', possible loss of data
	endif()

	if(NOT ARX_DEBUG_EXTRA)
		add_definitions(-D_HAS_ITERATOR_DEBUGGING=0)
		add_definitions(-D_SECURE_SCL=0)
		add_definitions(-D_SECURE_SCL_THROWS=0)
		add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
	endif()
	
	# Disable exceptions & rtti
	add_definitions(/wd4530)		# C++ exception handling was used but /EHsc was not selected.
	add_definitions(/GR-)			# No RTTI
		
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		# Disable exceptions
		string(REGEX REPLACE "/EHsc" "" ${flag_var} "${${flag_var}}")
		
		# Change runtime library from "Multi-threaded Debug DLL" to "Multi-threaded DLL"
			string(REGEX REPLACE "/MDd" "/MD" ${flag_var} "${${flag_var}}")
		
		# Remove definition of _DEBUG as it might conflict with libs we're linking with   		
		string(REGEX REPLACE "/D_DEBUG" "/DNDEBUG" ${flag_var} "${${flag_var}}")
	endforeach(flag_var)
	
	# Avoid warning during link
	set(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:LIBCMT)
	
	# This is needed to use ZLIB as a DLL
	add_definitions(-DZLIB_WINAPI)
	
else(MSVC)
	
	# Check the dependencies so there won't by cryptic link errors later on when found libraries have the wrong architecture (32- vs. 64-bit)
	if(CMAKE_THREAD_LIBS_INIT)
		check_link_library(Threads CMAKE_THREAD_LIBS_INIT)
	endif()
	check_link_library(DevIL IL_LIBRARIES IL)
	check_link_library(ZLIB ZLIB_LIBRARIES)
	check_link_library(Freetype FREETYPE_LIBRARIES)
	if(ARX_USE_OPENAL)
		check_link_library(OpenAL OPENAL_LIBRARY)
	endif()
	if(ARX_USE_OPENGL)
		check_link_library(OpenGL OPENGL_LIBRARIES)
		check_link_library(GLEW GLEW_LIBRARIES)
	endif()
	if(ARX_USE_SDL)
		check_link_library(SDL SDL_LIBRARY)
	endif()
	
	set(CMAKE_REQUIRED_DEFINITIONS "-D_POSIX_C_SOURCE=199309")
	set(CMAKE_REQUIRED_LIBRARIES "-lrt")
	check_symbol_exists(nanosleep "time.h" HAVE_NANOSLEEP_FUNC)
	check_symbol_exists(clock_gettime "time.h" HAVE_CLOCK_GETTIME)
	unset(CMAKE_REQUIRED_DEFINITIONS)
	unset(CMAKE_REQUIRED_LIBRARIES)
	if(HAVE_NANOSLEEP_FUNC OR HAVE_CLOCK_GETTIME)
		set(TIME_LIBRARIES "-lrt")
		add_definitions(-D_POSIX_C_SOURCE=199309)
	endif()
	
	check_include_files("sys/stat.h;sys/errno.h;dirent.h" HAVE_POSIX_FILESYSTEM)
	
	check_symbol_exists(strsignal "string.h" HAVE_STRSIGNAL)
	check_symbol_exists(fork "unistd.h" HAVE_FORK)
	check_symbol_exists(readlink "unistd.h" HAVE_READLINK)
	check_symbol_exists(dup2 "unistd.h" HAVE_DUP2)
	check_symbol_exists(execlp "unistd.h" HAVE_EXECLP)
	check_symbol_exists(fpathconf "unistd.h" HAVE_FPATHCONF)
	check_symbol_exists(dirfd "dirent.h" HAVE_DIRFD)
	if(NOT WIN32)
		check_symbol_exists(isatty "unistd.h" HAVE_ISATTY)
	endif()
	check_symbol_exists(waitpid "sys/wait.h" HAVE_WAITPID)
	check_symbol_exists(kill "signal.h" HAVE_KILL)
	check_symbol_exists(backtrace "execinfo.h" HAVE_BACKTRACE)
	check_symbol_exists(backtrace_symbols_fd "execinfo.h" HAVE_BACKTRACE_SYMBOLS_FD)
	
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
	
	if(CMAKE_BUILD_TYPE STREQUAL Debug)
		
		#Debug
		message("Debug Build")
		add_definitions(-D_DEBUG)
		
		check_compiler_flag(RESULT "-g3")
		if(NOT RESULT STREQUAL "")
			string(REGEX REPLACE "-g(|[0-9]|gdb)" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
		endif()
		
		check_compiler_flag(RESULT "-O0")
		string(REGEX REPLACE "-O[0-9]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
		
	else()
		
		#Release
		message("Release Build")
		add_definitions(-D_NDEBUG)
		
	endif()
	
	if(ARX_DEBUG_EXTRA)
		add_definitions(-D_GLIBCXX_DEBUG) # Runtime checks for STL containers.
		# add_cxxflag("-Wsuggest-attribute=const")
		# add_cxxflag("-Wsuggest-attribute=pure")
		add_cxxflag("-ftrapv") # to add checks for (undefined) signed integer overflow
		add_cxxflag("-fcatch-undefined-behavior") # (clang)
		add_cxxflag("-fbounds-checking")
	else()
		# -Wuninitialized causes too many false positives
		add_cxxflag("-Wno-uninitialized")
	endif()
	
	if(ARX_USE_UNITYBUILD)
		add_cxxflag("-fwhole-program")
	endif()
	
	# Disable exceptions & rtti
	add_cxxflag(-fno-rtti)
	
	if(CMAKE_CXX_COMPILER_ID STREQUAL "PathScale")
		# The standard library shipped with ekopath failes to compile with -fno-rtti
		# See https://github.com/pathscale/stdcxx/issues/3
		add_definitions(-D_RWSTD_NO_DYNAMIC_CAST)
	endif()
	
endif(MSVC)

# Check that all required functionality is available.
if(CMAKE_USE_PTHREADS_INIT)
	set(HAVE_PTHREADS 1)
elseif(NOT CMAKE_USE_WIN32_THREADS_INIT)
	message(SEND_ERROR "No supported thread libraries found.")
endif()
if(NOT ARX_USE_OPENAL AND NOT ARX_USE_DSOUND)
	message(WARNING "No audio backend enabled.")
endif()
if((NOT HAVE_NANOSLEEP_FUNC OR NOT HAVE_CLOCK_GETTIME) AND NOT HAVE_WINAPI)
	message(SEND_ERROR "Need either nanosleep and clock_gettime or WIN32.")
endif()

# Preprocessor definitions
if(HAVE_WINAPI)
	add_definitions(-DNOMINMAX)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
endif(HAVE_WINAPI)

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
	src/gui/Speech.cpp
	src/gui/Text.cpp
	src/gui/TextManager.cpp
)

set(INPUT_SOURCES src/input/Input.cpp)
set(INPUT_DINPUT8_SOURCES src/input/DInput8Backend.cpp)
set(INPUT_SDL_SOURCES src/input/SDLInputBackend.cpp)

set(IO_SOURCES
	src/io/Blast.cpp
	src/io/CinematicLoad.cpp
	src/io/Implode.cpp
	src/io/IniReader.cpp
	src/io/IniSection.cpp
	src/io/IniWriter.cpp
	src/io/IO.cpp
	src/io/PakEntry.cpp
	src/io/PakReader.cpp
	src/io/SaveBlock.cpp
	src/io/Screenshot.cpp
)
set(IO_LOGGER_SOURCES
	src/io/log/ConsoleLogger.cpp
	src/io/log/FileLogger.cpp
	src/io/log/LogBackend.cpp
	src/io/log/Logger.cpp
)
set(IO_LOGGER_POSIX_SOURCES src/io/log/ColorLogger.cpp)
set(IO_LOGGER_WINDOWS_SOURCES src/io/log/MsvcLogger.cpp)
set(IO_FILESYSTEM_SOURCES
	src/io/FilePath.cpp
	src/io/FileStream.cpp
	src/io/Filesystem.cpp
)
set(IO_FILESYSTEM_BOOST_SOURCES src/io/FilesystemBoost.cpp)
set(IO_FILESYSTEM_POSIX_SOURCES src/io/FilesystemPOSIX.cpp)
set(IO_FILESYSTEM_WINDOWS_SOURCES src/io/FilesystemWindows.cpp)

set(MATH_SOURCES src/math/Angle.cpp)

set(PHYSICS_SOURCES
	src/physics/Anchors.cpp
	src/physics/Attractors.cpp
	src/physics/Box.cpp
	src/physics/Clothes.cpp
	src/physics/Collisions.cpp
	src/physics/CollisionShapes.cpp
	src/physics/Physics.cpp
)

set(PLATFORM_SOURCES
	src/platform/CrashHandler.cpp
	src/platform/Lock.cpp
	src/platform/Platform.cpp
	src/platform/Random.cpp
	src/platform/String.cpp
	src/platform/Thread.cpp
	src/platform/Time.cpp
)

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

set(ARX_LIBRARIES ${FREETYPE_LIBRARIES} ${ZLIB_LIBRARIES} ${IL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
if(TIME_LIBRARIES)
	list(APPEND ARX_LIBRARIES ${TIME_LIBRARIES})
endif()
if(HAVE_WINAPI)
	list(APPEND ARX_LIBRARIES gdi32 shell32 comdlg32 ole32 comctl32)
endif()

if(ARX_USE_OPENAL)
	list(APPEND AUDIO_SOURCES ${AUDIO_OPENAL_SOURCES})
	list(APPEND ARX_LIBRARIES ${OPENAL_LIBRARY})
	include_directories(SYSTEM ${OPENAL_INCLUDE_DIR})
	set(HAVE_OPENAL 1)
	if(OPENALEFX_FOUND)
		include_directories(SYSTEM ${OPENAL_EFX_INCLUDE_DIR})
		set(HAVE_OPENAL_EFX 1)
	endif()
endif()
if(ARX_USE_DSOUND)
	list(APPEND AUDIO_SOURCES ${AUDIO_DSOUND_SOURCES})
	set(HAVE_DSOUND 1)
endif()

if(ARX_USE_D3D9)
	list(APPEND GRAPHICS_SOURCES ${GRAPHICS_D3D9_SOURCES})
	list(APPEND WINDOW_SOURCES ${WINDOW_WIN32_SOURCES} ${WINDOW_D3D9_SOURCES})
		
	list(APPEND ARX_LIBRARIES ${DIRECTX_D3D9_LIBRARY} ${DIRECTX_D3DX9_LIBRARY} ${DIRECTX_D3DCOMPILER_LIBRARY} ${DIRECTX_DXERR9_LIBRARY})
	include_directories(SYSTEM ${DIRECTX_INCLUDE_DIR})
	
	set(HAVE_D3D9 1)
endif()

if(ARX_USE_OPENGL)
	list(APPEND GRAPHICS_SOURCES ${GRAPHICS_OPENGL_SOURCES})
	list(APPEND ARX_LIBRARIES ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})
	include_directories(SYSTEM ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})
	set(HAVE_OPENGL 1)
	add_definitions(-DGL_GLEXT_PROTOTYPES)
endif()

if(ARX_USE_SDL)
	list(APPEND WINDOW_SOURCES ${WINDOW_SDL_SOURCES})
	list(APPEND INPUT_SOURCES ${INPUT_SDL_SOURCES})
	list(APPEND ARX_LIBRARIES ${SDL_LIBRARY})
	include_directories(SYSTEM ${SDL_INCLUDE_DIR})
	set(HAVE_SDL 1)
endif()

if(ARX_USE_DINPUT8)
	list(APPEND INPUT_SOURCES ${INPUT_DINPUT8_SOURCES})
	list(APPEND ARX_LIBRARIES ${DIRECTX_DINPUT8_LIBRARY} ${DIRECTX_DXGUID_LIBRARY} ${DIRECTX_DXERR9_LIBRARY})
	set(HAVE_DINPUT8 1)
	if(WINE)
		# TODO(broken-wine) work around missing symbols in wine's dinput8
		list(APPEND ARX_LIBRARIES dinput)
	endif()
endif()

if(HAVE_ISATTY)
	list(APPEND IO_LOGGER_SOURCES ${IO_LOGGER_POSIX_SOURCES})
endif()
if(HAVE_WINAPI)
	list(APPEND IO_LOGGER_SOURCES ${IO_LOGGER_WINDOWS_SOURCES})
endif()
list(APPEND IO_SOURCES ${IO_LOGGER_SOURCES})

if(HAVE_POSIX_FILESYSTEM)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_POSIX_SOURCES})
elseif(HAVE_WINAPI)
	add_definitions(-DHAVE_WINDOWS_FILESYSTEM)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_WINDOWS_SOURCES})
elseif((Boost_MAJOR_VERSION GREATER 1) OR (NOT Boost_MINOR_VERSION LESS 44))
	find_package(Boost 1.44 REQUIRED COMPONENTS filesystem system)
	set(HAVE_BOOST_FILESYSTEM_V3 1)
	add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_BOOST_SOURCES})
else()
	message(FATAL_ERROR "You need either Boost >= 1.44 or windows api or sys/stat.h, sys/errno.h and dirent.h; Found boost version ${Boost_VERSION}")
endif()
list(APPEND IO_SOURCES ${IO_FILESYSTEM_SOURCES})

if(NOT MSVC)
	check_link_library(Boost Boost_LIBRARIES)
endif()

set(ARX_SOURCES ${AI_SOURCES} ${ANIMATION_SOURCES} ${AUDIO_SOURCES} ${CORE_SOURCES} ${GAME_SOURCES} ${GRAPHICS_SOURCES} ${GUI_SOURCES} ${INPUT_SOURCES} ${IO_SOURCES} ${MATH_SOURCES} ${PHYSICS_SOURCES} ${PLATFORM_SOURCES} ${SCENE_SOURCES} ${SCRIPT_SOURCES} ${WINDOW_SOURCES})

if(ARX_BUILD_EDITOR)
	list(APPEND ARX_SOURCES ${EDITOR_SOURCES})
	set(BUILD_EDITOR 1)
endif()
if(ARX_BUILD_EDIT_LOADSAVE)
	set(BUILD_EDIT_LOADSAVE 1)
endif()

add_definitions(-DBOOST_NO_EXCEPTIONS)
add_definitions(-DBOOST_NO_TYPEID)
add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)

#create source groups
list(APPEND ALL_FILES_FOR_GROUPS ${ALL_INCLUDES} ${ARX_SOURCES})
create_source_groups(ALL_FILES_FOR_GROUPS)

if(ARX_USE_UNITYBUILD)
	set(UNITY_BUILD 1)
endif()

configure_file("${SRC_DIR}/Configure.h.cmake" "Configure.h")

version_file("${SRC_DIR}/core/Version.cpp.cmake" "${CMAKE_BINARY_DIR}/Version.cpp" "VERSION" ".git")
list(APPEND ARX_SOURCES "${CMAKE_BINARY_DIR}/Version.cpp")

# Add executables

add_executable_shared(arx WIN32 "${ARX_SOURCES}" "${ARX_LIBRARIES}" "${ALL_INCLUDES}")

if(ARX_BUILD_TOOLS)
	
	set(SAVETOOL_SOURCES
		${IO_FILESYSTEM_SOURCES}
		${IO_LOGGER_SOURCES}
		src/core/Localisation.cpp
		src/io/PakEntry.cpp
		src/io/PakReader.cpp
		src/io/Blast.cpp
		src/io/SaveBlock.cpp
		src/io/IniReader.cpp
		src/io/IniSection.cpp
		src/platform/String.cpp
		src/platform/Platform.cpp
		src/platform/Lock.cpp
		tools/savetool/savetool.cpp
		tools/savetool/SaveFix.cpp
		tools/savetool/SaveView.cpp
	)
	
	set(SAVETOOL_LIBRARIES ${ZLIB_LIBRARIES} ${Boost_LIBRARIES})
	
	add_executable_shared(arxsavetool "" "${SAVETOOL_SOURCES}" "${SAVETOOL_LIBRARIES}" "")
	
	set(UNPAK_SOURCES
		${IO_FILESYSTEM_SOURCES}
		${IO_LOGGER_SOURCES}
		src/io/PakEntry.cpp
		src/io/PakReader.cpp
		src/io/Blast.cpp
		src/io/log/ColorLogger.cpp
		src/io/log/ConsoleLogger.cpp
		src/io/log/Logger.cpp
		src/platform/String.cpp
		src/platform/Platform.cpp
		src/platform/Lock.cpp
		tools/unpak/unpak.cpp
	)
	
	set(UNPAK_LIBRARIES ${Boost_LIBRARIES})
	
	add_executable_shared(arxunpak "" "${UNPAK_SOURCES}" "${UNPAK_LIBRARIES}" "")
	
endif()

# Build and link executables

if(ARX_USE_UNITYBUILD)
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
	add_custom_target(doc
		#build the documentation
		COMMAND cd ${CMAKE_SOURCE_DIR}/src && ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doc/Doxyfile
	)
endif()

if(PYTHONINTERP_FOUND)
	
	file(GLOB_RECURSE TOOLS_INCLUDES ${CMAKE_SOURCE_DIR}/tools/*.h)
	set(ALL_SOURCES
		${ARX_SOURCES}
		${ALL_INCLUDES}
		${SAVETOOL_SOURCES}
		${UNPAK_SOURCES}
		${TOOLS_INCLUDES}
	)
	
	unset(STYLE_FILTER)
	
	# Complains about any c-style cast -> too annoying.
	set(STYLE_FILTER ${STYLE_FILTER},-readability/casting)
	
	# Insists on including evrything in the .cpp file even if it is included in the header.
	# This behaviour conflicts with orther tools.
	set(STYLE_FILTER ${STYLE_FILTER},-build/include_what_you_use)
	
	# Too many false positives and not very helpful error messages.
	set(STYLE_FILTER ${STYLE_FILTER},-build/include_order)
	
	# No thanks.
	set(STYLE_FILTER ${STYLE_FILTER},-readability/streams)
	
	# Ugh!
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/tab)
	
	# Yes it is!
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/blank_line)
	
	# Suggessts excessive indentation.
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/labels)
	
	# Why?
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/todo)
	set(STYLE_FILTER ${STYLE_FILTER},-readability/todo)
	
	# "For a static/global string constant, use a C style string"
	set(STYLE_FILTER ${STYLE_FILTER},-runtime/string)
	
	# TODO consider enabling these and fixing the warnings
	
	# Very noisy and perhaps a matter of taste.
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/braces)
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/parens)
	set(STYLE_FILTER ${STYLE_FILTER},-whitespace/newline)
	
	# Complains about using short, long, etc.
	set(STYLE_FILTER ${STYLE_FILTER},-runtime/int)
	
	# Complains about non-const references as parameters
	set(STYLE_FILTER ${STYLE_FILTER},-runtime/references)
	
	# TODO enable these!
	
	set(RELAXED_STYLE_FILTER ${STYLE_FILTER})
	
	# Very noisy but should be fixed.
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-whitespace/operators)
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-whitespace/comma)
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-whitespace/comments)
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-whitespace/end_of_line)
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-whitespace/align_tab)
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-whitespace/line_length)
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-whitespace/semicolon)
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-whitespace/ident_space)
	
	# Unsafe functions.
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-runtime/printf)
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-runtime/threadsafe_fn)
	
	# Very much known...
	set(RELAXED_STYLE_FILTER ${RELAXED_STYLE_FILTER},-readability/fn_size)
	
	add_custom_target(style
		COMMAND cmake -E chdir "${CMAKE_SOURCE_DIR}" "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/cpplint.py" "--filter=${RELAXED_STYLE_FILTER}" ${ALL_SOURCES}
		DEPENDS ${ALL_SOURCES} VERBATIM
	)
	
	add_custom_target(strict-style
		COMMAND cmake -E chdir "${CMAKE_SOURCE_DIR}" "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/cpplint.py" "--filter=${STYLE_FILTER}" ${ALL_SOURCES}
		DEPENDS ${ALL_SOURCES} VERBATIM
	)
endif()

if(ARX_BUILD_TESTS)
	add_subdirectory(tests ${CMAKE_SOURCE_DIR}/bin/tests)
endif()
