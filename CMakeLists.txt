cmake_minimum_required(VERSION 2.8)
project(ArxFatalis)

# For custom cmake modules.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckCXXSourceCompiles)

include(BuildSystem)
include(CompileCheck)

function(create_source_groups)
	# Source groups
	set(SOURCE_GROUPS ai animation audio core game graphics gui input io physics platform scene script)
	foreach(SOURCE_GROUP ${SOURCE_GROUPS})
		file(GLOB_RECURSE SOURCES ${SRC_DIR}/${SOURCE_GROUP}/*.*)
		source_group("${SOURCE_GROUP}" FILES ${SOURCES})
	endforeach(SOURCE_GROUP)
endfunction()

##############################################################################

option(ARX_BUILD_TESTS "Build tests" OFF)
option(ARX_BUILD_TOOLS "Build tools" ON)
option(ARX_BUILD_EDITOR "Build editor" OFF)
option(ARX_BUILD_EDIT_LOADSAVE "Build save/load functions only used by the editor" ON)
option(ARX_USE_UNITYBUILD "Unity build" OFF)
option(ARX_DEBUG "Normal debug options" ON)
option(ARX_DEBUG_EXTRA "Expensive debug options" OFF)
option(ARX_USE_OPENAL "Build the OpenAL audio backend." ON)
option(ARX_USE_DSOUND "Build the DirectSound audio backend." ON)
option(ARX_USE_OPENGL "Build the OpenGL renderer backend." ON)
option(ARX_USE_D3D7 "Build the Direct3D 7 renderer backend." ON)
option(ARX_USE_SDL "Build the SDL windowing backend." ON)
option(ARX_USE_DINPUT7 "Build the DirectInput 7 input backend." ON)

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

if(WIN32 OR WINE)
	set(HAVE_WINAPI 1)
else()
	set(HAVE_WINAPI 0)
endif()

if(NOT HAVE_WINAPI)
	# The is no DirectX under linux
	set(ARX_USE_D3D7 OFF)
	set(ARX_USE_DSOUND OFF)
	set(ARX_USE_DINPUT7 OFF)
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
	# Set optimal options for msvc:

	# Remove targets we don't really use (RelWithDebInfo, MinSizeRel)
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Using only Debug and Release" FORCE)
	
	# Disable deprecation warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	add_definitions(-DBOOST_NO_EXCEPTIONS)
	add_definitions(/wd4995)
	
	# TEMP - disable warning caused by the F2L removal
	add_definitions(/wd4244)    # conversion from 'float' to 'long', possible loss of data

	# This is needed to use ZLIB as a DLL
	add_definitions(-DZLIB_WINAPI)

	if(NOT ARX_DEBUG_EXTRA)
		add_definitions(-D_HAS_ITERATOR_DEBUGGING=0)
		add_definitions(-D_SECURE_SCL=0)
		add_definitions(-D_SECURE_SCL_THROWS=0)
		add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
	endif()
	
	# Add dx7 include & lib paths
	if("$ENV{DX7SDK_PATH}" STREQUAL "")
		message(WARNING "DX7SDK_PATH environment variable not found")
	else("$ENV{DX7SDK_PATH}" STREQUAL "")
		message(STATUS "Using DirectX 7 SDK found in " $ENV{DX7SDK_PATH})
		include_directories("$ENV{DX7SDK_PATH}/include")
		link_directories("$ENV{DX7SDK_PATH}/lib")
	endif("$ENV{DX7SDK_PATH}" STREQUAL "")

	# Add boost lib path
	link_directories(${Boost_LIBRARY_DIRS})
	
	# Disable exceptions & rtti
	string(REGEX REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	add_definitions(/wd4530)
	add_definitions(/GR-)
	
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
	
	if(ARX_DEBUG)
		#Debug
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
		add_definitions(-D_DEBUG)
	else()
		#Release
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g3") # TODO fix strict aliasing
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
	add_cxxflag(-fno-exceptions)
	add_cxxflag(-fno-rtti)
	
endif(MSVC)

check_type_size(char CHAR_SIZE)
if(NOT CHAR_SIZE EQUAL 1 AND NOT CHAR_SIZE STREQUAL "")
	message(SEND_ERROR "Unexpected char size: ${CHAR_SIZE}")
endif()

check_cxx_source_compiles("#include <stdlib.h>\n int main() { size_t s = rand(); char test[s]; }" HAVE_DYNAMIC_STACK_ALLOCATION)

# Check that all required functionality is available.
if(CMAKE_USE_PTHREADS_INIT)
	set(HAVE_PTHREADS 1)
elseif(NOT CMAKE_USE_WIN32_THREADS_INIT)
	message(SEND_ERROR "No supported thread libraries found.")
endif()
if(NOT ARX_USE_OPENAL AND NOT ARX_USE_DSOUND)
	message(WARNING "No audio backend enabled.")
endif()
if((NOT HAVE_NANOSLEEP_FUNC OR NOT HAVE_CLOCK_GETTIME) AND NOT WIN32)
	message(SEND_ERROR "Need either nanosleep and clock_gettime or WIN32.")
endif()

# Preprocessor definitions
add_definitions(-DNOMINMAX)
add_definitions(-DWIN32_LEAN_AND_MEAN)

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
set(GRAPHICS_D3D7_SOURCES
	src/graphics/d3d7/D3D7Renderer.cpp
	src/graphics/d3d7/D3D7Texture2D.cpp
	src/graphics/d3d7/D3D7TextureStage.cpp
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
set(INPUT_DINPUT7_SOURCES src/input/DInput7Backend.cpp)
set(INPUT_SDL_SOURCES src/input/SDLInputBackend.cpp)

set(IO_SOURCES
	src/io/Blast.cpp
	src/io/CinematicLoad.cpp
	src/io/Implode.cpp
	src/io/IniReader.cpp
	src/io/IniSection.cpp
	src/io/IniWriter.cpp
	src/io/IO.cpp
	src/io/Logger.cpp
	src/io/PakEntry.cpp
	src/io/PakReader.cpp
	src/io/SaveBlock.cpp
	src/io/Screenshot.cpp
)
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
set(WINDOW_D3D7_SOURCES src/window/D3D7Window.cpp)

file(GLOB_RECURSE ALL_INCLUDES ${SRC_DIR}/*.h)

include_directories(${SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR})
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

if(ARX_USE_D3D7)
	list(APPEND GRAPHICS_SOURCES ${GRAPHICS_D3D7_SOURCES})
	list(APPEND WINDOW_SOURCES ${WINDOW_WIN32_SOURCES} ${WINDOW_D3D7_SOURCES})
	list(APPEND ARX_LIBRARIES dxguid ddraw)
	set(HAVE_D3D7 1)
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

if(ARX_USE_DINPUT7)
	list(APPEND INPUT_SOURCES ${INPUT_DINPUT7_SOURCES})
	list(APPEND ARX_LIBRARIES dinput)
	set(HAVE_DINPUT7 1)
endif()

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
create_source_groups()

configure_file(${SRC_DIR}/Configure.h.cmake Configure.h)


# Add executables

add_executable_shared(arx "${ARX_SOURCES}" "${ARX_LIBRARIES}" "${ALL_INCLUDES}")

if(ARX_BUILD_TOOLS)
	
	set(SAVETOOL_SOURCES
		${IO_FILESYSTEM_SOURCES}
		src/core/Localisation.cpp
		src/io/PakEntry.cpp
		src/io/PakReader.cpp
		src/io/Blast.cpp
		src/io/Logger.cpp
		src/io/SaveBlock.cpp
		src/io/IniReader.cpp
		src/io/IniSection.cpp
		src/platform/String.cpp
		src/platform/Platform.cpp
		tools/savetool/savetool.cpp
		tools/savetool/SaveFix.cpp
		tools/savetool/SaveView.cpp
	)
	
	set(SAVETOOL_LIBRARIES ${ZLIB_LIBRARIES} ${Boost_LIBRARIES})
	
	add_executable_shared(savetool "${SAVETOOL_SOURCES}" "${SAVETOOL_LIBRARIES}" "")
	
	set(UNPAK_SOURCES
		${IO_FILESYSTEM_SOURCES}
		src/io/PakEntry.cpp
		src/io/PakReader.cpp
		src/io/Logger.cpp
		src/io/Blast.cpp
		src/platform/String.cpp
		src/platform/Platform.cpp
		tools/unpak/unpak.cpp
	)
	
	set(UNPAK_LIBRARIES ${Boost_LIBRARIES})
	
	add_executable_shared(unpak "${UNPAK_SOURCES}" "${UNPAK_LIBRARIES}" "")
	
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
		COMMAND cd ${CMAKE_SOURCE_DIR} && ${DOXYGEN_EXECUTABLE} doc/Doxyfile
	)
endif()

if(PYTHONINTERP_FOUND)
	set(STYLE_FILTER -readability/casting,-build/header_guard,-build/include,-readability/streams,-whitespace,-runtime/int)
	add_custom_target(style
		COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/cpplint.py --filter=${STYLE_FILTER} ${ARX_SOURCES} ${ALL_INCLUDES}
	)
endif()

if(ARX_BUILD_TESTS)
	add_subdirectory(tests ${CMAKE_SOURCE_DIR}/bin/tests)
endif()
