project(ArxLibertatis)

cmake_minimum_required(VERSION 2.8.3)

if(CMAKE_VERSION VERSION_GREATER 3.12)
	cmake_policy(VERSION 3.12)
else()
	cmake_policy(VERSION ${CMAKE_VERSION})
endif()


# Define configuration options

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(MACOS 1)
else()
	set(MACOS 0)
endif()

macro(suboption _var _comment _type _default)
	if(NOT DEFINED ${_var})
		set(${_var} "${_default}")
	else()
		set(${_var} "${${_var}}" CACHE ${_type} "${_comment}")
	endif()
endmacro()

option(DEVELOPER "Use build settings suitable for developers" OFF)

# Components
option(BUILD_ALL OFF)
set(default_BUILD_TESTS OFF)
if(BUILD_ALL OR DEVELOPER)
	set(default_BUILD_TESTS ON)
endif()
suboption(BUILD_TESTS "Build tests" BOOL ${default_BUILD_TESTS})
option(BUILD_TOOLS "Build tools" ON)
set(def_BUILD_CRASHHANDLER ON)
if(MACOS)
	set(def_BUILD_CRASHHANDLER OFF)
elseif(WIN32 AND NOT MSVC)
	set(def_BUILD_CRASHHANDLER OFF)
endif()
option(BUILD_CRASHHANDLER "Build the crash handler" ${def_BUILD_CRASHHANDLER})
suboption(BUILD_CRASHREPORTER "Build the crash reporter" BOOL ${BUILD_CRASHHANDLER})
if(NOT BUILD_CRASHHANDLER)
	set(BUILD_CRASHREPORTER OFF)
endif()
option(BUILD_PROFILER_INSTRUMENT "Enable instrumentation for the profiler" OFF)
set(def_BUILD_PROFILER OFF)
if(BUILD_PROFILER_INSTRUMENT OR BUILD_ALL)
	set(def_BUILD_PROFILER ON)
endif()
suboption(BUILD_PROFILER "Build the profiler GUI" BOOL ${def_BUILD_PROFILER})
option(INSTALL_BLENDER_PLUGIN "Install the Arx Libertatis blender plugin" ON)
suboption(BUILD_IO_LIBRARY "Build shared library for reading and writing arx data"
          BOOL ${INSTALL_BLENDER_PLUGIN})
if(INSTALL_BLENDER_PLUGIN)
	set(BUILD_IO_LIBRARY ON)
endif()
option(INSTALL_SCRIPTS "Install the data install script" ON)

# Optional dependencies
option(USE_OPENAL "Build the OpenAL audio backend" ON)
option(USE_OPENGL "Build the OpenGL renderer backend" ON)
option(USE_WINHTTP "Use the native WinHTTP API instead of CURL on Windows" ON)
option(USE_NATIVE_FS "Use the native filesystem backend directly instead of boost" ON)
option(USE_X11 "Check for X11 support in used libraries" OFF)
option(USE_WAYLAND "Check for Wayland support in used libraries" OFF)

# Alternative dependencies
set(WITH_SDL CACHE STRING "The SDL version to use: 1 or 2 or empty (use any)")
set(WITH_QT CACHE STRING "The Qt version to use: 4 or 5 or empty (use any)")
set(WITH_OPENGL CACHE STRING "The OpenGL wrangler to use: epoxy or glew or empty (use any)")

# Build types
set(default_UNITY_BUILD ON)
if(DEVELOPER)
	set(default_UNITY_BUILD OFF)
endif()
suboption(UNITY_BUILD "Unity build" BOOL ${default_UNITY_BUILD})
option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_NOISY_WARNING_FLAGS "Enable noisy compiler warnings" OFF)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)
suboption(USE_LDGOLD "Use the Gold linker" BOOL ${SET_OPTIMIZATION_FLAGS})
suboption(FASTLINK "Optimize (incremental) linking speed" BOOL ${DEVELOPER})
set(default_USE_LTO OFF)
if(SET_OPTIMIZATION_FLAGS AND NOT FASTLINK)
	set(default_USE_LTO ON)
endif()
suboption(USE_LTO "Use link-time code generation" BOOL ${default_USE_LTO})
	

suboption(CXX_STD_VERSION "Maximum C++ standard version to enable" STRING 2017)
if(DEVELOPER OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(default_DEBUG ON)
else()
	set(default_DEBUG OFF)
endif()
suboption(DEBUG "Build with debug output" BOOL ${default_DEBUG})
if(DEBUG)
	add_definitions(-DARX_DEBUG=1)
endif()
suboption(DEBUG_GL "Enable OpenGL debug output by default" BOOL ${DEBUG})
set(ARX_DEBUG_GL ${DEBUG_GL})

# Static libs
set(default_USE_STATIC_LIBS OFF)
if(WIN32)
	set(default_USE_STATIC_LIBS ON)
endif()
option(USE_STATIC_LIBS          "Statically link libraries" ${default_USE_STATIC_LIBS})
suboption(Epoxy_USE_STATIC_LIBS    "Statically link libepoxy" BOOL ${USE_STATIC_LIBS})
suboption(GLEW_USE_STATIC_LIBS     "Statically link GLEW"     BOOL ${USE_STATIC_LIBS})
suboption(Boost_USE_STATIC_LIBS    "Statically link Boost"    BOOL ${USE_STATIC_LIBS})
# Should be called FreeType_*, but keep it consistent with the FindFreetype.cmake module.
suboption(Freetype_USE_STATIC_LIBS "Statically link FreeType" BOOL ${USE_STATIC_LIBS})
suboption(ZLIB_USE_STATIC_LIBS     "Statically link ZLIB"     BOOL ${USE_STATIC_LIBS})

# Icons
if(WIN32)
	set(default_ICON_TYPE "ico")
elseif(MACOS)
	set(default_ICON_TYPE "icns")
else()
	set(default_ICON_TYPE "iconset")
endif()
suboption(ICON_TYPE "Icon type (ico/icns/iconset/png)" STRING ${default_ICON_TYPE})
suboption(DATA_FILES "Location of prebuilt data files" STRING "")
option(OPTIMIZE_ICONS "Optimize PNG compression for generated icons" ON)

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
	set(CMAKE_INSTALL_LIBDIR "lib" CACHE
	    STRING "libraries (lib) (relative to prefix).")
	set(CMAKE_INSTALL_LIBEXECDIR "libexec" CACHE
	    STRING "program executables (libexec) (relative to prefix).")
	set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_DATAROOTDIR}/man" CACHE
	    STRING "man documentation (DATAROOTDIR/man) (relative to prefix).")
	set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE
	    STRING "C header files (include) (relative to prefix).")
	foreach(dir BINDIR LIBDIR LIBEXECDIR DATAROOTDIR MANDIR INCLUDEDIR)
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
		CMAKE_INSTALL_INCLUDEDIR
	)
else()
	include(GNUInstallDirs)
endif()
set(ICONDIR "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps" CACHE
    STRING "Install location for icons (relative to prefix).")
set(ICONTHEMEDIR "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor" CACHE
    STRING "Install location for themable icons (relative to prefix).")
set(APPDIR "${CMAKE_INSTALL_DATAROOTDIR}/applications" CACHE
    STRING "Install location for .desktop files (relative to prefix).")
set(GAMESBINDIR "${CMAKE_INSTALL_BINDIR}" CACHE
    STRING "Install location for game executables (relative to prefix).")
set(SCRIPTDIR "${CMAKE_INSTALL_BINDIR}" CACHE
    STRING "Where to install the data install script (relative to prefix).")
set(INSTALL_DATADIR "${CMAKE_INSTALL_DATAROOTDIR}/games/arx" CACHE
    STRING "Install location for Arx Libertatis data files (relative to prefix).")
set(INSTALL_BLENDER_PLUGINDIR "${CMAKE_INSTALL_DATAROOTDIR}/blender/scripts/addons/arx" CACHE
    STRING "Install location for the Arx Libertatis blender plugin (relative to prefix).")
set(RUNTIME_LIBEXECDIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}" CACHE
    STRING "Runtime search locations for helpers, relative to the game executable.")
mark_as_advanced(
	ICONDIR
	APPDIR
	GAMESBINDIR
	SCRIPTDIR
	INSTALL_DATADIR
	RUNTIME_LIBEXECDIR
)

# Default runtime user and data directories
if(WIN32)
	set(USER_DIR          "Arx Libertatis"                CACHE STRING "User dir names")
elseif(MACOS)
	set(DATA_DIR          "ArxLibertatis"                 CACHE STRING "Data dir names")
	set(DATA_DIR_PREFIXES "/Applications"                 CACHE STRING "Data dir paths")
	set(USER_DIR          "ArxLibertatis"                 CACHE STRING "User dir names")
	set(USER_DIR_PREFIXES "$HOME/Library/Application Support" CACHE STRING "User dir paths")
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
set(RUNTIME_DATADIR "." CACHE STRING "Data dir paths relative to the executable")
mark_as_advanced(
	DATA_DIR
	DATA_DIR_PREFIXES
	USER_DIR
	USER_DIR_PREFIXES
	CONFIG_DIR
	CONFIG_DIR_PREFIXES
	IGNORE_EXE_DIR
	RUNTIME_DATADIR
)


# Helper scrips

include(CheckCXXSourceCompiles)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckTypeSize)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake") # For custom cmake modules

include(VersionString)
set(version_names VERSION ICON_NAME DATA_VERSION IO_LIBRARY_ABI_VERSION)
parse_version_file("${version_names}" "VERSION")
if(VERSION_ERROR OR DATA_VERSION_ERROR OR IO_LIBRARY_ABI_VERSION_ERROR)
	message(WARNING "Error parsing version file!")
endif()

include(BuildSystem)
include(BuildType)
include(CompileCheck)
include(CXXVersionCheck)
include(CreateSourceGroups)
include(PrintConfiguration)
include(StyleCheck)
include(UseStaticLibs)


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
	force_recheck_library(Epoxy)
	force_recheck_library(GLEW)
	force_recheck_library(Boost)
	force_recheck_library(QtCore)
	force_recheck_library(QtConcurrent)
	force_recheck_library(QtGui)
	force_recheck_library(QtWidgets)
	force_recheck_library(CURL)
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
	# Define this so that we don't accitenally use ANSI functions
	add_definitions(-DUNICODE)
	add_definitions(-D_UNICODE)
	if(MSVC)
		if(CMAKE_CL_64)
			set(subsystem_xp "5.02")
		else()
			set(subsystem_xp "5.01")
		endif()
		set(CMAKE_CREATE_WIN32_EXE "/subsystem:windows,${subsystem_xp}")
		set(CMAKE_CREATE_CONSOLE_EXE "/subsystem:console,${subsystem_xp}")
	else()
		# We need to define WINVER for MinGW when requiring anything newer than Win95/WinNT
		add_definitions(-DWINVER=0x0500) # Require at least Windows 2000
		add_definitions(-D_WIN32_IE=0x0500) # Required for "SHGFP_TYPE_CURRENT"
	endif()
	
	add_definitions(-DNOMINMAX)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	list(APPEND ARX_LIBRARIES gdi32 shell32 comdlg32 ole32 comctl32)
endif()

# pthread / Win32 threads
set(CMAKE_THREAD_PREFER_PTHREAD 1)
find_package(Threads REQUIRED)
if(NOT MSVC AND CMAKE_THREAD_LIBS_INIT)
	check_link_library(Threads CMAKE_THREAD_LIBS_INIT)
endif()
list(APPEND ARX_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

# zlib
use_static_libs(ZLIB)
find_package(ZLIB REQUIRED)
use_static_libs_restore()
if(MSVC)
	add_definitions(-DZLIB_WINAPI)
else()
	check_link_library(ZLIB ZLIB_LIBRARIES)
endif()
include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
list(APPEND ARX_LIBRARIES ${ZLIB_LIBRARIES})

# FreeType
use_static_libs(Freetype)
find_package(Freetype REQUIRED)
has_static_libs(Freetype FREETYPE_LIBRARIES)
if(Freetype_HAS_STATIC_LIBS)
	# Freetype may require other libraries depending on how it was configured
	find_package(PkgConfig QUIET)
	pkg_check_modules(PC_Freetype freetype2)
	foreach(lib IN LISTS PC_Freetype_STATIC_LIBRARIES)
		if(lib MATCHES "(^|^\-l|/|\\\\)(lib)?z(lib)?(\\.so|\\.a||.dll|\\.lib)?$")
			if(NOT Freetype_LINKING_ZLIB)
				message(STATUS "FreeType: linking ZLIB")
				set(Freetype_LINKING_ZLIB 1 CACHE INTERNAL "foobar")
			endif()
			# We always link ZLIB
			list(APPEND FREETYPE_LIBRARIES ${ZLIB_LIBRARIES})
		elseif(lib MATCHES "(^|^\-l|/|\\\\)(lib)?freetype")
			# The main lib we linked above
		elseif(lib MATCHES "(^|^\-l|/|\\\\)(lib)?bz(ip)?2")
			if(NOT Freetype_LINKING_BZip2)
				message(STATUS "FreeType: linking BZip2")
				set(Freetype_LINKING_BZip2 1 CACHE INTERNAL "foobar")
			endif()
			find_package(BZip2 REQUIRED)
			list(APPEND FREETYPE_LIBRARIES ${BZIP2_LIBRARIES})
		elseif(lib MATCHES "(^|^\-l|/|\\\\)(lib)?png")
			if(NOT Freetype_LINKING_PNG)
				message(STATUS "FreeType: linking libpng")
				set(Freetype_LINKING_PNG 1 CACHE INTERNAL "foobar")
			endif()
			find_package(PNG REQUIRED)
			list(APPEND FREETYPE_LIBRARIES ${PNG_LIBRARIES} ${ZLIB_LIBRARIES})
		else()
			# We don't know this library so we can't easily search for a static version.
			# Just looking for a static library with the same name is generally not enough
			# as it too may require other libraries (and naming can differ).
			message(WARNING "FreeType: linking unknown library \"${lib}\"")
			if(EXISTS "${lib}" OR lib MATCHES "(/|\\\\)")
				list(APPEND FREETYPE_LIBRARIES "${lib}")
			else()
				list(APPEND FREETYPE_LIBRARIES "-l${lib}")
			endif()
		endif()
	endforeach()
endif()
use_static_libs_restore()
if(NOT MSVC)
	check_link_library(Freetype FREETYPE_LIBRARIES)
endif()
include_directories(SYSTEM ${FREETYPE_INCLUDE_DIRS})
list(APPEND ARX_LIBRARIES ${FREETYPE_LIBRARIES})

# OpenGL
if(USE_OPENGL)
	if(NOT ARX_HAVE_OPENGL AND (WITH_OPENGL STREQUAL "" OR WITH_OPENGL STREQUAL "epoxy"))
	if(NOT WITH_OPENGL)
		set(EPOXY_OPTIONAL_DEPENDENCY)
	else()
		set(EPOXY_OPTIONAL_DEPENDENCY ${OPTIONAL_DEPENDENCY})
	endif()
		find_package(Epoxy ${EPOXY_OPTIONAL_DEPENDENCY})
		if(EPOXY_FOUND)
			if(NOT MSVC)
				check_link_library(Epoxy Epoxy_LIBRARIES)
			endif()
			set(ARX_HAVE_EPOXY 1)
			set(ARX_HAVE_OPENGL 1)
			set(ARX_HAVE_GL_STATIC ${Epoxy_USE_STATIC_LIBS})
		endif()
	endif()
	if(NOT ARX_HAVE_OPENGL AND (WITH_OPENGL STREQUAL "" OR WITH_OPENGL STREQUAL "glew"))
		find_package(OpenGL ${OPTIONAL_DEPENDENCY})
		find_package(GLEW 1.5.2 ${OPTIONAL_DEPENDENCY})
		list(APPEND GLEW_DEFINITIONS -DGLEW_NO_GLU)
		if(OPENGL_FOUND AND GLEW_FOUND)
			if(NOT MSVC)
				check_link_library(OpenGL OPENGL_gl_LIBRARY)
				check_link_library(GLEW GLEW_LIBRARIES)
			endif()
			set(ARX_HAVE_GLEW 1)
			set(ARX_HAVE_OPENGL 1)
			set(ARX_HAVE_GL_STATIC ${GLEW_USE_STATIC_LIBS})
		endif()
	endif()
endif()

# OpenAL
if(USE_OPENAL)
	find_package(OpenAL ${OPTIONAL_DEPENDENCY})
	find_package(OpenALEFX)
	if(NOT MSVC AND OPENAL_FOUND)
		check_link_library(OpenAL OPENAL_LIBRARY)
	endif()
endif()

# SDL
if(NOT ARX_HAVE_SDL AND (NOT WITH_SDL OR WITH_SDL EQUAL 2))
	if(NOT WITH_SDL)
		set(SDL_OPTIONAL_DEPENDENCY)
	else()
		set(SDL_OPTIONAL_DEPENDENCY ${OPTIONAL_DEPENDENCY})
	endif()
	set(SDL2_BUILDING_LIBRARY 1) # we don't need SDL2main!
	find_package(SDL2 ${SDL_OPTIONAL_DEPENDENCY})
	if(SDL2_FOUND)
		if(NOT MSVC)
			check_link_library(SDL2 SDL2_LIBRARIES)
		endif()
		set(ARX_HAVE_SDL2 1)
		set(ARX_HAVE_SDL 1)
	endif()
endif()
if(NOT ARX_HAVE_SDL AND (NOT WITH_SDL OR WITH_SDL EQUAL 1))
	if(NOT MACOS)
		# Required to avoid linking with SDLmain except for macOS where it is necessary
		# due to the need to have NSApplication correctly setup by SDLmain.
		set(SDL_BUILDING_LIBRARY 1)
	endif()
	find_package(SDL 1.2 ${OPTIONAL_DEPENDENCY})
	if(SDL_FOUND)
		if(NOT WIN32 AND NOT MACOS)
			# Some builds of SDL1 have references to X11 symbols but do not link to libX11.
			# If we do not use GLEW then we do not indirectly link to libX11 anywhere and those
			# symbols remain unresolved, resulting in a linker error.
			# If libX11 was not actually needed, then the linker will discard it.
			find_package(X11)
			if(X11_FOUND)
				list(APPEND SDL_LIBRARY ${X11_LIBRARIES})
				list(APPEND SDL_INCLUDE_DIR ${X11_INCLUDE_DIR})
			endif()
		endif()
		if(NOT MSVC)
			check_link_library(SDL SDL_LIBRARY)
		endif()
		set(ARX_HAVE_SDL1 1)
		set(ARX_HAVE_SDL 1)
	endif(SDL_FOUND)
endif()

# Boost
find_package(Boost 1.48 REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
add_definitions(-DBOOST_ALL_NO_LIB)
# We compile with -fno-rtti and boost is not able to detect that for all compilers.
add_definitions(-DBOOST_NO_RTTI -DBOOST_NO_TYPEID)
# Boost 1.62 has a bug in the type_index library:
# BOOST_TYPE_INDEX_REGISTER_CTTI_PARSING_PARAMS are wrong for 32-bit GCC builds in C++14 mode
# This leads to a static assert when (indirectly) including <boost/type_index/ctti_type_index.hpp>
#   /usr/include/boost/type_index/detail/compile_time_type_info.hpp:78:9: error: static assertion failed: TypeIndex library is misconfigured for your compiler. Please define BOOST_TYPE_INDEX_CTTI_USER_DEFINED_PARSING to correct values. See section 'RTTI emulation limitations' of the documentation for more information.
# Fixed in Boost type_index commit b669f0244e4b0bf146f18e9293e6e87842f4bbe0
if(NOT Boost_VERSION VERSION_LESS 106200 AND Boost_VERSION VERSION_LESS 106300)
	if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_SIZEOF_VOID_P EQUAL 4)
		message("Applying workaround for Boost.TypeIndex bug")
		add_definitions("-DBOOST_NO_CXX14_CONSTEXPR")
	endif()
endif()
# Boost 1.69 has warnings about deprecated headers when using Boost.Random
# https://github.com/boostorg/random/issues/49
if(NOT Boost_VERSION VERSION_LESS 106900 AND Boost_VERSION VERSION_LESS 107000)
	message("Suppressing warnings for deprecated headers used internally in Boost")
	add_definitions("-DBOOST_ALLOW_DEPRECATED_HEADERS")
endif()

# glm
find_package(GLM 0.9.5.0 REQUIRED)
include_directories(SYSTEM ${GLM_INCLUDE_DIRS})
add_definitions(-DGLM_FORCE_RADIANS) # Only needed for older GLM versions

# DbgHelp
if(MSVC AND BUILD_CRASHHANDLER)
	find_package(DbgHelp ${OPTIONAL_DEPENDENCY})
endif()

# Qt
set(ARX_HAVE_CRASHREPORTER 0)
if(BUILD_CRASHREPORTER OR BUILD_PROFILER)
	
	find_package(Qt 4.7 COMPONENTS Core Concurrent Gui Widgets ${OPTIONAL_DEPENDENCY})
	
	if(HAVE_QT4 AND NOT MSVC)
		check_link_library(QtCore QtCore_LIBRARIES)
		check_link_library(QtConcurrent QtConcurrent_LIBRARIES)
		check_link_library(QtGui QtGui_LIBRARIES)
		check_link_library(QtWidgets QtWidgets_LIBRARIES)
	endif()
	
	# Needed by the crash reporter
	
	if(HAVE_QT AND BUILD_CRASHREPORTER)
		if(NOT WIN32 OR NOT USE_WINHTTP)
			find_package(CURL 7.20.0 ${OPTIONAL_DEPENDENCY})
			if(CURL_FOUND AND NOT MSVC)
				check_link_library(CURL CURL_LIBRARIES)
			endif()
		endif()
		if((WIN32 AND USE_WINHTTP) OR CURL_FOUND)
			if(NOT MSVC OR DBGHELP_FOUND)
				set(ARX_HAVE_CRASHREPORTER 1)
			endif()
		endif()
	endif()
	
	if(HAVE_QT AND BUILD_PROFILER)
		set(ARX_HAVE_PROFILER 1)
	endif()
	
endif()

find_package(Doxygen)

find_package(PythonInterp)

# Check for consistent configuration between SDL and libepoxy/GLEW
if(ARX_HAVE_SDL AND ARX_HAVE_OPENGL AND NOT WIN32 AND NOT APPLE)
	set(old_CMAKE_REQUIRED_INCLUDES)
	if(ARX_HAVE_SDL2)
		set(CMAKE_REQUIRED_INCLUDES ${SDL2_INCLUDE_DIR})
	else()
		set(CMAKE_REQUIRED_INCLUDES ${SDL_INCLUDE_DIR})
	endif()
	check_symbol_exists(SDL_VIDEO_DRIVER_X11 "SDL_config.h" SDL_HAVE_VIDEO_X11)
	check_symbol_exists(SDL_VIDEO_DRIVER_WAYLAND "SDL_config.h" SDL_HAVE_VIDEO_WAYLAND)
	if(EPOXY_FOUND)
		set(CMAKE_REQUIRED_INCLUDES ${Epoxy_INCLUDE_DIR})
	else()
		set(CMAKE_REQUIRED_INCLUDES ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})
	endif()
	if(SDL_HAVE_VIDEO_X11)
		if(EPOXY_FOUND)
			check_include_files("epoxy/glx.h" ARX_HAVE_GLX)
		else()
			check_include_files("GL/glxew.h" ARX_HAVE_GLX)
		endif()
	endif()
	if(SDL_HAVE_VIDEO_WAYLAND)
		if(EPOXY_FOUND)
			check_include_files("epoxy/egl.h" ARX_HAVE_EGL)
		else()
			check_include_files("GL/eglew.h" ARX_HAVE_EGL)
		endif()
	endif()
	set(CMAKE_REQUIRED_INCLUDES ${old_CMAKE_REQUIRED_INCLUDES})
endif()


# Check for optional functionality and system configuration

if(NOT CXX_STD_VERSION LESS 2011)
	enable_cxx_version(${CXX_STD_VERSION})
	set(ARX_HAVE_CXX11_LONG_LONG 1) # everyone has had this for ages
	check_cxx11("alignas"                ARX_HAVE_CXX11_ALIGNAS            1900)
	check_cxx11("alignof"                ARX_HAVE_CXX11_ALIGNOF            1900)
	check_cxx11("auto"                   ARX_HAVE_CXX11_AUTO               1600)
	check_cxx11("decltype"               ARX_HAVE_CXX11_DECLTYPE           1900)
	check_cxx11("std::atomic"            ARX_HAVE_CXX11_ATOMIC             1700)
	check_cxx11("std::forward"           ARX_HAVE_CXX11_FORWARD            1800)
	check_cxx11("std::integral_constant" ARX_HAVE_CXX11_INTEGRAL_CONSTANT  1500)
	check_cxx11("std::max_align_t"       ARX_HAVE_CXX11_MAX_ALIGN_T        1700)
	check_cxx11("noexcept"               ARX_HAVE_CXX11_NOEXCEPT           1900)
	check_cxx11("static_assert"          ARX_HAVE_CXX11_STATIC_ASSERT      1600)
	check_cxx11("variadic templates"     ARX_HAVE_CXX11_VARIADIC_TEMPLATES 1800)
	check_cxx11("thread_local"           ARX_HAVE_CXX11_THREADLOCAL        1900)
	check_cxx11("default"                ARX_HAVE_CXX11_DEFAULT            1800)
	check_cxx11("final"                  ARX_HAVE_CXX11_FINAL              1700)
	check_cxx17("[[nodiscard]]"          ARX_HAVE_CXX17_NODISCARD          1911)
	if(WIN32)
		check_cxx17("fstream(wchar*)" ARX_HAVE_CXX17_FSTREAM_WCHAR ALWAYS)
	endif()
endif()

if(MSVC)
	
	# Compiler builtins & language extensions
	set(ARX_HAVE_ASSUME 1)
	set(ARX_HAVE_DECLSPEC_ALIGN 1)
	set(ARX_HAVE_MSVC_ALIGNOF 1)
	set(ARX_HAVE_DECLSPEC_THREADLOCAL 1)
	
	# Standard library features
	set(ARX_HAVE_ALIGNED_MALLOC 1)
	set(ARX_HAVE_ALIGNED_FREE 1)
	
	set(ARX_HAVE_XMMINTRIN 1)
	set(ARX_HAVE_PMMINTRIN 1)
	
else(MSVC)
	
	if(USE_STATIC_LIBS)
		add_ldflag("-static-libstdc++")
		add_ldflag("-static-libgcc")
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
		)
	endif()
	if(USE_NATIVE_FS AND NOT WIN32)
		list(APPEND libc_features -D_FILE_OFFSET_BITS=64)
	endif()
	string(REPLACE ";" " " CMAKE_REQUIRED_DEFINITIONS "${libc_features}")
	add_definitions(${libc_features})
	
	# Check compiler builtins & language extensions
	# These are a symbols, so we can't use check_symbol_exists
	if(NOT ARX_HAVE_CXX11_ALIGNAS)
		check_builtin(ARX_HAVE_ATTRIBUTE_ALIGNED      "__attribute__((aligned(n)))")
	endif()
	check_builtin(ARX_HAVE_ATTRIBUTE_ALLOC_ALIGN    "__attribute__((alloc_align(i)))")
	check_builtin(ARX_HAVE_ATTRIBUTE_ALLOC_SIZE     "__attribute__((alloc_size(i)))")
	check_builtin(ARX_HAVE_ATTRIBUTE_ALWAYS_INLINE  "__attribute__((always_inline))")
	check_builtin(ARX_HAVE_ATTRIBUTE_ASSUME_ALIGNED "__attribute__((assume_aligned(n)))")
	check_builtin(ARX_HAVE_ATTRIBUTE_FORMAT_PRINTF  "__attribute__((format(printf, i, j)))")
	check_builtin(ARX_HAVE_ATTRIBUTE_MALLOC         "__attribute__((malloc))")
	if(NOT ARX_HAVE_CXX17_NODISCARD)
		check_builtin(ARX_HAVE_ATTRIBUTE_WARN_UNUSED_RESULT "__attribute__((warn_unused_result))")
	endif()
	check_builtin(ARX_HAVE_BUILTIN_ASSUME           "__builtin_assume")
	if(NOT ARX_HAVE_BUILTIN_ASSUME)
		check_builtin(ARX_HAVE_ASSUME                 "__assume")
	endif()
	check_builtin(ARX_HAVE_BUILTIN_ASSUME_ALIGNED   "__builtin_assume_aligned")
	check_builtin(ARX_HAVE_BUILTIN_TRAP             "__builtin_trap")
	check_builtin(ARX_HAVE_BUILTIN_UNREACHABLE      "__builtin_unreachable")
	if(NOT ARX_HAVE_CXX11_ALIGNOF)
		check_builtin(ARX_HAVE_GCC_ALIGNOF            "__alignof__")
	endif()
	if(NOT ARX_HAVE_CXX11_THREADLOCAL)
		check_builtin(ARX_HAVE_GCC_THREADLOCAL        "__thread")
	endif()
	if(NOT ARX_HAVE_CXX11_DECLTYPE)
		check_builtin(ARX_HAVE_GCC_TYPEOF             "__typeof__")
	endif()
	
	if(WIN32)
		check_symbol_exists(_aligned_malloc "malloc.h" ARX_HAVE_ALIGNED_MALLOC)
		check_symbol_exists(_aligned_free "malloc.h" ARX_HAVE_ALIGNED_FREE)
	endif()
	if(NOT ARX_HAVE_ALIGNED_MALLOC OR NOT ARX_HAVE_ALIGNED_FREE)
		check_symbol_exists(aligned_alloc "stdlib.h" ARX_HAVE_ALIGNED_ALLOC)
		if(NOT ARX_HAVE_ALIGNED_ALLOC)
			check_symbol_exists(posix_memalign "stdlib.h" ARX_HAVE_POSIX_MEMALIGN)
		endif()
	endif()
	
	set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_DL_LIBS})
	check_symbol_exists(dlsym "dlfcn.h" ARX_HAVE_DLSYM)
	unset(CMAKE_REQUIRED_LIBRARIES)
	
	check_symbol_exists(nanosleep "time.h" ARX_HAVE_NANOSLEEP)
	
	set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
	check_symbol_exists(pthread_setname_np "pthread.h" ARX_HAVE_PTHREAD_SETNAME_NP)
	if(NOT ARX_HAVE_PTHREAD_SETNAME_NP)
		check_symbol_exists(pthread_set_name_np "pthread_np.h" ARX_HAVE_PTHREAD_SET_NAME_NP)
	endif()
	check_symbol_exists(prctl "sys/prctl.h" ARX_HAVE_PRCTL)
	unset(CMAKE_REQUIRED_LIBRARIES)
	
	check_symbol_exists(uname "sys/utsname.h" ARX_HAVE_UNAME)
	check_symbol_exists(getrusage "sys/resource.h" ARX_HAVE_GETRUSAGE)
	
	check_symbol_exists(getexecname "stdlib.h" ARX_HAVE_GETEXECNAME)
	check_symbol_exists(setenv "stdlib.h" ARX_HAVE_SETENV)
	check_symbol_exists(unsetenv "stdlib.h" ARX_HAVE_UNSETENV)
	
	check_symbol_exists(__get_cpuid "cpuid.h" ARX_HAVE_GET_CPUID)
	if(ARX_HAVE_GET_CPUID)
		check_symbol_exists(__get_cpuid_max "cpuid.h" ARX_HAVE_GET_CPUID_MAX)
	endif()
	
	# CMake has trouble detecting _MM_SET_FLUSH_ZERO_MODE with GCC, check for the includes instead
	check_include_files("xmmintrin.h" ARX_HAVE_XMMINTRIN)
	check_include_files("pmmintrin.h" ARX_HAVE_PMMINTRIN)
	# _fxsave is unreliable with GCC, use the builtin directly
	check_builtin(ARX_HAVE_BUILTIN_IA32_FXSAVE "__builtin_ia32_fxsave")
	
	find_library(LIBRT_LIBRARY rt)
	
	if(LIBRT_LIBRARY)
		check_link_library(librt LIBRT_LIBRARY)
		set(CMAKE_REQUIRED_LIBRARIES "${LIBRT_LIBRARY}")
	endif()
	check_symbol_exists(clock_gettime "time.h" ARX_HAVE_CLOCK_GETTIME)
	if(LIBRT_LIBRARY)
		unset(CMAKE_REQUIRED_LIBRARIES)
	endif()
	if(NOT ARX_HAVE_CLOCK_GETTIME AND NOT WIN32)
		check_include_files("mach/clock.h;mach/clock_types.h;mach/mach_host.h"
		                    ARX_HAVE_MACH_CLOCK)
	endif()
	
	check_symbol_exists(open "fcntl.h" ARX_HAVE_OPEN)
	check_symbol_exists(O_CLOEXEC "fcntl.h" ARX_HAVE_O_CLOEXEC)
	check_symbol_exists(fcntl "fcntl.h" ARX_HAVE_FCNTL)
	
	check_symbol_exists(chdir "unistd.h" ARX_HAVE_CHDIR)
	check_symbol_exists(fork "unistd.h" ARX_HAVE_FORK)
	check_symbol_exists(readlink "unistd.h" ARX_HAVE_READLINK)
	check_symbol_exists(dup2 "unistd.h" ARX_HAVE_DUP2)
	check_symbol_exists(pipe "unistd.h" ARX_HAVE_PIPE)
	check_symbol_exists(read "unistd.h" ARX_HAVE_READ)
	check_symbol_exists(close "unistd.h" ARX_HAVE_CLOSE)
	check_symbol_exists(setpgid "unistd.h" ARX_HAVE_SETPGID)
	check_symbol_exists(execvp "unistd.h" ARX_HAVE_EXECVP)
	check_symbol_exists(posix_spawnp "spawn.h" ARX_HAVE_POSIX_SPAWNP)
	if(NOT WIN32)
		check_symbol_exists(isatty "unistd.h" ARX_HAVE_ISATTY)
	endif()
	
	check_symbol_exists(waitpid "sys/wait.h" ARX_HAVE_WAITPID)
	
	check_symbol_exists(sched_getscheduler "sched.h" ARX_HAVE_SCHED_GETSCHEDULER)
	
	check_symbol_exists(kill "signal.h" ARX_HAVE_KILL)
	
	check_symbol_exists(backtrace "execinfo.h" ARX_HAVE_BACKTRACE)
	
	check_include_file("sys/types.h" ARX_HAVE_SYS_TYPES_H)
	if(CMAKE_USE_PTHREADS_INIT AND ARX_HAVE_SYS_TYPES_H)
		set(ARX_HAVE_PTHREADS 1)
	endif()
	
	check_symbol_exists(getpid "unistd.h" ARX_HAVE_GETPID)
	check_symbol_exists(getgid "unistd.h" ARX_HAVE_GETGID)
	check_symbol_exists(getuid "unistd.h" ARX_HAVE_GETUID)
	
	check_symbol_exists(sysconf "unistd.h" ARX_HAVE_SYSCONF)
	
	check_symbol_exists(confstr "unistd.h" ARX_HAVE_CONFSTR)
	
	check_symbol_exists(sigaction "signal.h" ARX_HAVE_SIGACTION)
	
	check_symbol_exists(setrlimit "sys/resource.h" ARX_HAVE_SETRLIMIT)
	
	if(CMAKE_SYSTEM_NAME MATCHES "BSD")
		check_symbol_exists(sysctl "sys/types.h;sys/sysctl.h" ARX_HAVE_SYSCTL)
		check_symbol_exists(sysctlbyname "sys/types.h;sys/sysctl.h" ARX_HAVE_SYSCTLBYNAME)
	endif()
	
	if(CMAKE_CXX_COMPILER_ID STREQUAL "PathScale")
		# EKOPath recognizes these but then fails to link
	else()
		check_builtin(INNOEXTRACT_HAVE_BUILTIN_BSWAP16 "__builtin_bswap16(0)")
		check_builtin(INNOEXTRACT_HAVE_BUILTIN_BSWAP32 "__builtin_bswap32(0)")
		check_builtin(INNOEXTRACT_HAVE_BUILTIN_BSWAP64 "__builtin_bswap64(0)")
	endif()
	if(NOT INNOEXTRACT_HAVE_BUILTIN_BSWAP16)
		check_symbol_exists(bswap_16 "byteswap.h" INNOEXTRACT_HAVE_BSWAP_16)
	endif()
	if(NOT INNOEXTRACT_HAVE_BUILTIN_BSWAP32)
		check_symbol_exists(bswap_32 "byteswap.h" INNOEXTRACT_HAVE_BSWAP_32)
	endif()
	if(NOT INNOEXTRACT_HAVE_BUILTIN_BSWAP64)
		check_symbol_exists(bswap_64 "byteswap.h" INNOEXTRACT_HAVE_BSWAP_64)
	endif()
	
	if(USE_NATIVE_FS AND NOT WIN32)
		
		# Whether multiple cuncurrent readdir calls with *different* directory streams are threadsafe
		set(READDIR_IS_THREADSAFE 0)
		if(HAVE_GLIBC)
			# TODO Find a more portable check for this - from the glibc documentation:
			# POSIX-1.2008 does not guarantee that readdir is thread-safe, even when access to the same
			# dirstream is serialized. But in current implementations (including the GNU C Library),
			# it is safe to call readdir concurrently on different dirstreams, so there is no need to
			# use readdir_r in most multi-threaded programs.
			# It is expected that future versions of POSIX will obsolete readdir_r and mandate the level
			# of thread safety for readdir which is provided by the GNU C Library and other
			# implementations today.
			set(READDIR_IS_THREADSAFE 1)
		endif()
		
		check_include_file("sys/stat.h" ARX_HAVE_SYS_STAT_H)
		check_include_file("sys/errno.h" ARX_HAVE_SYS_ERRNO_H)
		check_include_file("dirent.h" ARX_HAVE_DIRENT_H)
		
		check_symbol_exists(dirfd "dirent.h" ARX_HAVE_DIRFD)
		if(ARX_HAVE_DIRFD)
			check_symbol_exists(fstatat "sys/stat.h" ARX_HAVE_FSTATAT)
		endif()
		
		# readdir_r is complicated to use and is deprecated in glibc 2.24
		# Prefer readdir if it is know to be threadsafe
		if(READDIR_IS_THREADSAFE)
			check_symbol_exists(readdir "dirent.h" ARX_HAVE_THREADSAFE_READDIR)
		endif()
		if(NOT ARX_HAVE_THREADSAFE_READDIR)
			check_symbol_exists(readdir_r "dirent.h" ARX_HAVE_READDIR_R)
			check_symbol_exists(_PC_NAME_MAX "unistd.h" ARX_HAVE_PC_NAME_MAX)
			check_symbol_exists(NAME_MAX "dirent.h" ARX_HAVE_NAME_MAX)
			if(ARX_HAVE_DIRFD AND ARX_HAVE_PC_NAME_MAX)
				check_symbol_exists(fpathconf "unistd.h" ARX_HAVE_FPATHCONF)
			endif()
		endif()
		
		check_symbol_exists(_PC_CASE_SENSITIVE "unistd.h" ARX_HAVE_PC_CASE_SENSITIVE)
		if(ARX_HAVE_PC_NAME_MAX OR ARX_HAVE_PC_CASE_SENSITIVE)
			check_symbol_exists(pathconf "unistd.h" ARX_HAVE_PATHCONF)
		endif()
		
		check_symbol_exists(getcwd "unistd.h" ARX_HAVE_GETCWD)
		
		if(ARX_HAVE_SYS_STAT_H AND ARX_HAVE_SYS_ERRNO_H AND ARX_HAVE_DIRENT_H
		   AND ARX_HAVE_GETCWD
		   AND (ARX_HAVE_THREADSAFE_READDIR
		        OR (ARX_HAVE_READDIR_R
		            AND (ARX_HAVE_PC_NAME_MAX
		                   AND ((ARX_HAVE_DIRFD AND ARX_HAVE_FPATHCONF) OR ARX_HAVE_PATHCONF)
			               OR ARX_HAVE_NAME_MAX)
		           )
		       )
		  )
			set(ARX_HAVE_POSIX_FILESYSTEM 1)
		elseif(STRICT_USE)
			message(FATAL_ERROR "Missing dependencies for native filesystem backend.")
		endif()
		
	endif()
	
endif(MSVC)


# Sources

set(AI_SOURCES
	src/ai/Anchors.cpp
	src/ai/PathFinder.cpp
	src/ai/PathFinderManager.cpp
	src/ai/Paths.cpp
)

set(ANIMATION_SOURCES
	src/animation/Animation.cpp
	src/animation/AnimationRender.cpp
	src/animation/Skeleton.cpp
)

set(CINEMATIC_SOURCES
	src/cinematic/Cinematic.cpp
	src/cinematic/CinematicController.cpp
	src/cinematic/CinematicKeyframer.cpp
	src/cinematic/CinematicLoad.cpp
	src/cinematic/CinematicTexture.cpp
	src/cinematic/CinematicEffects.cpp
	src/cinematic/CinematicSound.cpp
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

set(CORE_SOURCES
	src/core/Application.cpp
	src/core/ArxGame.cpp
	src/core/Benchmark.cpp
	src/core/Config.cpp
	src/core/Core.cpp
	src/core/GameTime.cpp
	src/core/Localisation.cpp
	src/core/SaveGame.cpp
	src/core/Startup.cpp
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
	src/game/Missile.cpp
	src/game/NPC.cpp
	src/game/Player.cpp
	src/game/Spells.cpp
	src/game/effect/Quake.cpp
	src/game/effect/ParticleSystems.cpp
	src/game/magic/Precast.cpp
	src/game/magic/Rune.cpp
	src/game/magic/RuneDraw.cpp
	src/game/magic/Spell.cpp
	src/game/magic/SpellData.cpp
	src/game/magic/SpellRecognition.cpp
	src/game/magic/spells/SpellsLvl01.cpp
	src/game/magic/spells/SpellsLvl02.cpp
	src/game/magic/spells/SpellsLvl03.cpp
	src/game/magic/spells/SpellsLvl04.cpp
	src/game/magic/spells/SpellsLvl05.cpp
	src/game/magic/spells/SpellsLvl06.cpp
	src/game/magic/spells/SpellsLvl07.cpp
	src/game/magic/spells/SpellsLvl08.cpp
	src/game/magic/spells/SpellsLvl09.cpp
	src/game/magic/spells/SpellsLvl10.cpp
	src/game/npc/Dismemberment.cpp
	src/game/spell/FlyingEye.cpp
	src/game/spell/Cheat.cpp
)

set(GRAPHICS_SOURCES
	src/graphics/Draw.cpp
	src/graphics/DrawLine.cpp
	src/graphics/DrawDebug.cpp
	src/graphics/GlobalFog.cpp
	src/graphics/Math.cpp
	src/graphics/Raycast.cpp
	src/graphics/Renderer.cpp
	src/graphics/RenderBatcher.cpp
	src/graphics/data/FTL.cpp
	src/graphics/data/Mesh.cpp
	src/graphics/data/MeshManipulation.cpp
	src/graphics/data/TextureContainer.cpp
	src/graphics/effects/BlobShadow.cpp
	src/graphics/effects/Cabal.cpp
	src/graphics/effects/Fade.cpp
	src/graphics/effects/Field.cpp
	src/graphics/effects/Fissure.cpp
	src/graphics/effects/FloatingStones.cpp
	src/graphics/effects/Fog.cpp
	src/graphics/effects/SpellEffects.cpp
	src/graphics/effects/Halo.cpp
	src/graphics/effects/LightFlare.cpp
	src/graphics/effects/Lightning.cpp
	src/graphics/effects/MagicMissile.cpp
	src/graphics/effects/PolyBoom.cpp
	src/graphics/effects/RotatingCone.cpp
	src/graphics/effects/Trail.cpp
	src/graphics/font/Font.cpp
	src/graphics/font/FontCache.cpp
	src/graphics/image/Image.cpp
	src/graphics/image/ImageColorKey.cpp
	src/graphics/image/ImageSave.cpp
	src/graphics/image/stb_image.cpp
	src/graphics/particle/Particle.cpp
	src/graphics/particle/ParticleEffects.cpp
	src/graphics/particle/ParticleManager.cpp
	src/graphics/particle/ParticleSystem.cpp
	src/graphics/particle/MagicFlare.cpp
	src/graphics/particle/Spark.cpp
	src/graphics/spells/Spells05.cpp
	src/graphics/texture/PackedTexture.cpp
	src/graphics/texture/Texture.cpp
	src/graphics/texture/TextureStage.cpp
)

set(GRAPHICS_OPENGL_SOURCES
	src/graphics/opengl/GLDebug.cpp
	src/graphics/opengl/GLTexture.cpp
	src/graphics/opengl/GLTextureStage.cpp
	src/graphics/opengl/GLVertexBuffer.cpp
	src/graphics/opengl/OpenGLRenderer.cpp
)

set(GUI_SOURCES
	src/gui/CharacterCreation.cpp
	src/gui/CinematicBorder.cpp
	src/gui/Console.cpp
	src/gui/Credits.cpp
	src/gui/Cursor.cpp
	src/gui/Hud.cpp
	src/gui/Interface.cpp
	src/gui/LoadLevelScreen.cpp
	src/gui/Logo.cpp
	src/gui/MainMenu.cpp
	src/gui/Menu.cpp
	src/gui/MenuPublic.cpp
	src/gui/MenuWidgets.cpp
	src/gui/MiniMap.cpp
	src/gui/Note.cpp
	src/gui/Notification.cpp
	src/gui/Speech.cpp
	src/gui/Text.cpp
	src/gui/TextManager.cpp
	src/gui/book/Book.cpp
	src/gui/book/Necklace.cpp
	src/gui/debug/DebugHud.cpp
	src/gui/debug/DebugHudAudio.cpp
	src/gui/debug/DebugKeys.cpp
	src/gui/debug/DebugPanel.cpp
	src/gui/hud/HudCommon.cpp
	src/gui/hud/PlayerInventory.cpp
	src/gui/hud/SecondaryInventory.cpp
	src/gui/menu/MenuCursor.cpp
	src/gui/menu/MenuFader.cpp
	src/gui/menu/MenuPage.cpp
	src/gui/widget/ButtonWidget.cpp
	src/gui/widget/CheckboxWidget.cpp
	src/gui/widget/CycleTextWidget.cpp
	src/gui/widget/KeybindWidget.cpp
	src/gui/widget/PanelWidget.cpp
	src/gui/widget/SaveSlotWidget.cpp
	src/gui/widget/SliderWidget.cpp
	src/gui/widget/TextInputWidget.cpp
	src/gui/widget/TextWidget.cpp
	src/gui/widget/Widget.cpp
	src/gui/widget/WidgetContainer.cpp
)

set(INPUT_SOURCES
	src/input/Input.cpp
	src/input/TextInput.cpp
)
set(INPUT_SDL1_SOURCES src/input/SDL1InputBackend.cpp)
set(INPUT_SDL2_SOURCES src/input/SDL2InputBackend.cpp)

set(IO_SOURCES
	src/io/IniReader.cpp
	src/io/IniSection.cpp
	src/io/IniWriter.cpp
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
	src/physics/Attractors.cpp
	src/physics/Collisions.cpp
	src/physics/CollisionShapes.cpp
	src/physics/Projectile.cpp
	src/physics/Physics.cpp
)

# Basic platform abstraction sources
set(PLATFORM_SOURCES
	src/platform/Alignment.cpp
	src/platform/Environment.cpp
	src/platform/Lock.cpp
	src/platform/OS.cpp
	src/platform/Platform.cpp
	src/platform/Process.cpp
	src/platform/ProgramOptions.cpp
	src/platform/Time.cpp
)
set(PLATFORM_CONSOLE_SOURCES)
set(PLATFORM_GUI_SOURCES)
if(WIN32)
	list(APPEND PLATFORM_SOURCES src/platform/WindowsUtils.cpp)
	list(APPEND PLATFORM_CONSOLE_SOURCES src/platform/WindowsMainConsole.cpp)
	list(APPEND PLATFORM_GUI_SOURCES src/platform/WindowsMainGUI.cpp)
endif()

# Extra platform abstraction - depends on the crash handler or SDL
set(PLATFORM_EXTRA_SOURCES
	src/platform/Dialog.cpp
	src/platform/Thread.cpp
)
if(MACOS)
	list(APPEND PLATFORM_EXTRA_SOURCES src/platform/Dialog.mm)
endif()

# Crash handler sources
set(PLATFORM_CRASHHANDLER_SOURCES src/platform/CrashHandler.cpp)
set(PLATFORM_CRASHHANDLER_IMPL_SOURCES
	src/platform/crashhandler/CrashHandlerImpl.cpp
	src/platform/crashhandler/CrashProcessorImpl.cpp
)
set(PLATFORM_CRASHHANDLER_POSIX_SOURCES
	src/platform/crashhandler/CrashHandlerPOSIX.cpp
	src/platform/crashhandler/CrashProcessorPOSIX.cpp
)
set(PLATFORM_CRASHHANDLER_WINDOWS_SOURCES
	src/platform/crashhandler/CrashHandlerWindows.cpp
	src/platform/crashhandler/CrashProcessorWindows.cpp
)

# Profiler sources
set(PLATFORM_PROFILER_SOURCES src/platform/profiler/Profiler.cpp)

set(SCENE_SOURCES
	src/scene/Background.cpp
	src/scene/ChangeLevel.cpp
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

set(UTIL_CMDLINE_SOURCES
	src/util/cmdline/Parser.cpp
	src/util/cmdline/CommandLine.cpp
)

set(UTIL_MD5_SOURCES
	src/util/MD5.cpp
)

set(WINDOW_SOURCES
	src/window/RenderWindow.cpp
	src/window/Window.cpp
)
set(WINDOW_SDL1_SOURCES src/window/SDL1Window.cpp)
set(WINDOW_SDL2_SOURCES src/window/SDL2Window.cpp)
if(NOT WIN32 AND NOT MACOS)
	list(APPEND WINDOW_SDL2_SOURCES src/window/SDL2X11Util.c)
endif()

# TODO manually specify theese like we do for the tools
file(GLOB_RECURSE ALL_INCLUDES RELATIVE ${PROJECT_SOURCE_DIR} src/*.h)

include_directories(src ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/tools)


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
	set(old_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
	set(CMAKE_REQUIRED_INCLUDES ${OPENAL_INCLUDE_DIR})
	check_symbol_exists(ALC_SOFT_HRTF "alext.h" ARX_HAVE_OPENAL_HRTF)
	set(CMAKE_REQUIRED_INCLUDES ${old_CMAKE_REQUIRED_INCLUDES})
endif()

# Graphics
if(ARX_HAVE_OPENGL)
	list(APPEND GRAPHICS_SOURCES ${GRAPHICS_OPENGL_SOURCES})
	if(EPOXY_FOUND)
		list(APPEND ARX_LIBRARIES ${Epoxy_LIBRARIES})
		include_directories(SYSTEM ${Epoxy_INCLUDE_DIR})
	else()
		list(APPEND ARX_LIBRARIES ${GLEW_LIBRARIES} ${OPENGL_gl_LIBRARY})
		include_directories(SYSTEM ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})
		add_definitions(${GLEW_DEFINITIONS})
	endif()
else()
	# SDL backends require OpenGL
	unset(ARX_HAVE_SDL)
	unset(ARX_HAVE_SDL1)
	unset(ARX_HAVE_SDL2)
endif()

# Windowing / input
if(ARX_HAVE_SDL)
	if(ARX_HAVE_SDL2)
		list(APPEND WINDOW_SOURCES ${WINDOW_SDL2_SOURCES})
		list(APPEND INPUT_SOURCES ${INPUT_SDL2_SOURCES})
		list(APPEND ARX_LIBRARIES ${SDL2_LIBRARIES})
		include_directories(SYSTEM ${SDL2_INCLUDE_DIR})
	elseif(ARX_HAVE_SDL1)
		list(APPEND WINDOW_SOURCES ${WINDOW_SDL1_SOURCES})
		list(APPEND INPUT_SOURCES ${INPUT_SDL1_SOURCES})
		list(APPEND ARX_LIBRARIES ${SDL_LIBRARY})
		include_directories(SYSTEM ${SDL_INCLUDE_DIR})
	endif()
	add_definitions(-DNO_SDL_GLEXT) # we use libepoxy/GLEW for that
endif()

# Logging
if(ARX_HAVE_ISATTY)
	list(APPEND IO_LOGGER_SOURCES ${IO_LOGGER_POSIX_SOURCES})
endif()
if(WIN32)
	list(APPEND IO_LOGGER_SOURCES ${IO_LOGGER_WINDOWS_SOURCES})
endif()
list(APPEND IO_SOURCES ${IO_LOGGER_SOURCES} ${IO_LOGGER_EXTRA_SOURCES})

list(APPEND IO_SOURCES ${IO_RESOURCE_SOURCES})

# Filesystem
if(ARX_HAVE_POSIX_FILESYSTEM)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_POSIX_SOURCES})
elseif(USE_NATIVE_FS AND WIN32)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_WINDOWS_SOURCES})
	set(ARX_HAVE_WIN32_FILESYSTEM 1)
else()
	
	find_package(Boost 1.48 REQUIRED COMPONENTS filesystem system)
	set(ARX_HAVE_BOOST_FILESYSTEM_V3 1)
	add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
	add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)
	list(APPEND IO_FILESYSTEM_SOURCES ${IO_FILESYSTEM_BOOST_SOURCES})
	list(APPEND BASE_LIBRARIES ${Boost_LIBRARIES})
	
	# BOOST_SCOPED_ENUM is implemented differently if compiled with C++11 support
	# This means the Boost.Filesystem ABI depends on what C++ version was used
	# to compile the library - check for that!
	check_compile(boost_filesystem_abi_matches
		"${CXX_CHECK_DIR}/boostfs-cxx11-scoped_enum.cpp"
		"scoped enum" "Boost ABI"
		"${Boost_LIBRARIES}"
		"mismatch, fixing"
	)
	if(NOT boost_filesystem_abi_matches)
		add_definitions(
			# Different names for different Boost versions :/
			-DBOOST_NO_SCOPED_ENUMS
			-DBOOST_NO_CXX11_SCOPED_ENUMS
		)
	endif()
	
endif()
list(APPEND IO_SOURCES ${IO_FILESYSTEM_SOURCES})

# Crash reporter
if(BUILD_CRASHHANDLER)
	if(ARX_HAVE_EXECVP AND ARX_HAVE_FORK AND ARX_HAVE_KILL)
		list(APPEND PLATFORM_CRASHHANDLER_SOURCES ${PLATFORM_CRASHHANDLER_IMPL_SOURCES})
		list(APPEND PLATFORM_CRASHHANDLER_SOURCES ${PLATFORM_CRASHHANDLER_POSIX_SOURCES})
		set(ARX_HAVE_CRASHHANDLER_POSIX 1)
	elseif(MSVC AND DBGHELP_FOUND)
		list(APPEND PLATFORM_CRASHHANDLER_SOURCES ${PLATFORM_CRASHHANDLER_IMPL_SOURCES})
		list(APPEND PLATFORM_CRASHHANDLER_SOURCES ${PLATFORM_CRASHHANDLER_WINDOWS_SOURCES})
		set(ARX_HAVE_CRASHHANDLER_WINDOWS 1)
		list(APPEND ARX_LIBRARIES ${DBGHELP_LIBRARIES} psapi)
		include_directories(SYSTEM ${DBGHELP_INCLUDE_DIR})
	else()
		# Don't bother building arxcrashreporter if it will never be used.
		set(ARX_HAVE_CRASHREPORTER 0)
	endif()
endif()

if(ARX_HAVE_DLSYM)
	list(APPEND ARX_LIBRARIES ${CMAKE_DL_LIBS})
endif()

if(LIBRT_LIBRARY AND (ARX_HAVE_CLOCK_GETTIME OR ARX_HAVE_CRASHHANDLER_POSIX))
	# Needed for clock_gettime and boost::interprocess on some system.
	list(APPEND BASE_LIBRARIES ${LIBRT_LIBRARY})
endif()

list(APPEND ARX_LIBRARIES ${BASE_LIBRARIES})

if(NOT MSVC)
	check_link_library(Boost Boost_LIBRARIES)
endif()

if(BUILD_PROFILER_INSTRUMENT)
	if(NOT ARX_HAVE_CXX11_ATOMIC)
		message(WARNING "Missing c++11 atomic, profiler instrumentation disabled")
		set(BUILD_PROFILER_INSTRUMENT 0)
	endif()
endif()

set(ARX_SOURCES
	${AI_SOURCES}
	${ANIMATION_SOURCES}
	${CINEMATIC_SOURCES}
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
	${PLATFORM_GUI_SOURCES}
	${PLATFORM_EXTRA_SOURCES}
	${PLATFORM_CRASHHANDLER_SOURCES}
	${PLATFORM_PROFILER_SOURCES}
	${SCENE_SOURCES}
	${SCRIPT_SOURCES}
	${UTIL_SOURCES}
	${UTIL_CMDLINE_SOURCES}
	${WINDOW_SOURCES}
)

# Using include() for now as add_subdirectory causes problems with uses in qrc files
include(data/CMakeLists.txt)

# Set the icon for the Windows executable by adding this resource file to the sources
if(WIN32)
	list(APPEND ARX_SOURCES ${arx-libertatis-icon.rc})
endif()


# Prepare sources

# Create source groups
list(APPEND ALL_FILES_FOR_GROUPS ${ALL_INCLUDES} ${ARX_SOURCES})
create_source_groups(ALL_FILES_FOR_GROUPS)

configure_file("src/Configure.h.in" "Configure.h" ESCAPE_QUOTES)

configure_file("src/io/fs/PathConstants.h.in" "io/fs/PathConstants.h"
               ESCAPE_QUOTES)

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/platform")
set(PLATFORM_CONFIG_H "platform/PlatformConfig.h")
configure_file("src/${PLATFORM_CONFIG_H}.in" "${PLATFORM_CONFIG_H}" ESCAPE_QUOTES)

set(VERSION_TEMPLATE "src/core/Version.cpp.in")
set(VERSION_FILE     "${PROJECT_BINARY_DIR}/core/Version.cpp")
set(VERSION_SOURCES  VERSION "VERSION" AUTHORS "AUTHORS" COPYING "COPYING")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/core")
version_file("${VERSION_TEMPLATE}" "${VERSION_FILE}" "${VERSION_SOURCES}" ".git")
list(APPEND ARX_SOURCES ${VERSION_FILE})

if(WIN32)
	set(VERSION_RC_TEMPLATE "src/core/Version.rc.in")
	function(create_version_resource exe name)
		set(rc_file "${PROJECT_BINARY_DIR}/${exe}-version.rc")
		set(rc_sources VERSION "VERSION" COPYING "COPYING")
		if(ARGC GREATER 2)
			list(APPEND rc_sources ${ARGV2} "${ARGV2}")
		endif()
		set(rc_defines "-DEXE=${exe}" "-DNAME=${name}")
		set(${exe}-version.rc "${rc_file}" PARENT_SCOPE)
		version_file("${VERSION_RC_TEMPLATE}"
		             "${rc_file}" "${rc_sources}" ".git" "${rc_defines}")
	endfunction()
	set(APPLICATION_MANIFEST_TEMPLATE "src/core/Version.manifest.in")
	function(create_application_manifest exe)
		set(manifest_file "${PROJECT_BINARY_DIR}/${exe}.manifest")
		set(manifest_sources VERSION "VERSION")
		set(manifest_defines "-DEXE=${exe}")
		if(ARGC GREATER 1 AND ARGV1 STREQUAL "dpiaware")
			list(APPEND manifest_defines "-DDPIAWARE=true")
		else()
			list(APPEND manifest_defines "-DDPIAWARE=false")
		endif()
		version_file("${APPLICATION_MANIFEST_TEMPLATE}"
		             "${manifest_file}" "${manifest_sources}" ".git" "${manifest_defines}")
		set(${exe}.manifest "${manifest_file}" PARENT_SCOPE)
	endfunction()
	create_version_resource(arx "${VERSION_NAME}")
	create_application_manifest(arx dpiaware)
	list(APPEND ARX_SOURCES ${arx-version.rc} ${arx.manifest})
endif()


# Add executables

add_executable_shared(arx "${ARX_SOURCES}" "${ARX_LIBRARIES}" "${ALL_INCLUDES}")
set_binary_type(arx WIN32)
set_binary_installdir(arx "${GAMESBINDIR}")

if(Qt_VERSION VERSION_LESS 5.6)
	set(qt_dpiaware)
else()
	set(qt_dpiaware dpiaware)
endif()

set(BASE_SOURCES
	${PLATFORM_SOURCES}
	${IO_FILESYSTEM_SOURCES}
	${IO_LOGGER_SOURCES}
	${UTIL_SOURCES}
	${VERSION_FILE}
)

if(ARX_HAVE_CRASHREPORTER)
	
	set(arxcrashreporter_Qt_SOURCES
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
	)
	
	set(arxcrashreporter_SOURCES
		tools/crashreporter/tbg/HTTPClient.h
		${BASE_SOURCES}
	)
	
	if(WIN32 AND USE_WINHTTP)
		list(APPEND arxcrashreporter_SOURCES tools/crashreporter/tbg/HTTPClientWindows.cpp)
	else()
		list(APPEND arxcrashreporter_SOURCES tools/crashreporter/tbg/HTTPClientCURL.cpp)
	endif()
	
	if(WIN32)
		list(APPEND arxcrashreporter_SOURCES ${arx-libertatis-icon.rc})
		create_version_resource(arxcrashreporter "${VERSION_NAME} Crash Reporter")
		create_application_manifest(arxcrashreporter ${qt_dpiaware})
		list(APPEND arxcrashreporter_SOURCES ${arxcrashreporter-version.rc} ${arxcrashreporter.manifest})
	endif()
	
	set(arxcrashreporter_MANUAL_SOURCES
		${arxcrashreporter_SOURCES}
		${arxcrashreporter_Qt_SOURCES}
	)
	
	create_source_groups(arxcrashreporter_MANUAL_SOURCES)
	
	list(APPEND arxcrashreporter_Qt_SOURCES ${QtGui_SOURCES})
	qt_wrap_ui(arxcrashreporter_Qt_SOURCES
		tools/crashreporter/ui/ErrorReportDialog.ui
	)
	qt_wrap_cpp(arxcrashreporter_Qt_SOURCES
		tools/crashreporter/ui/ErrorReportDialog.h
		tools/crashreporter/qhexedit/QHexEdit.h
		tools/crashreporter/qhexedit/QHexEditPrivate.h
	)
	
	file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/tools/crashreporter/resources")
	if(HAVE_QT4 AND "${arx-libertatis-logo-58.png}" STREQUAL "")
		# Create a dummy file to prevent a RCC warning
		set(arx-libertatis-logo-58.png "${PROJECT_BINARY_DIR}/arx-libertatis-logo-58.png")
		file(WRITE ${arx-libertatis-logo-58.png} "")
	endif()
	configure_file(
		tools/crashreporter/resources/CrashReporter.qrc.in
		${PROJECT_BINARY_DIR}/tools/crashreporter/resources/CrashReporter.qrc
	)
	qt_add_resources(arxcrashreporter_Qt_SOURCES
		${PROJECT_BINARY_DIR}/tools/crashreporter/resources/CrashReporter.qrc
	)
	
	list(APPEND arxcrashreporter_SOURCES ${arxcrashreporter_Qt_SOURCES})
	
	add_source_flags("${arxcrashreporter_Qt_SOURCES}"
		"${QtCore_EXECUTABLE_COMPILE_FLAGS}"
	)
	
	add_source_definitions("${arxcrashreporter_Qt_SOURCES}"
		${QtCore_DEFINITIONS}
		${QtConcurrent_DEFINITIONS}
		${QtGui_DEFINITIONS}
		${QtWidgets_DEFINITIONS}
		-DQT_NO_DEBUG
	)
	
	set(arxcrashreporter_INCLUDES
		${QtCore_INCLUDE_DIRS}
		${QtConcurrent_INCLUDE_DIRS}
		${QtGui_INCLUDE_DIRS}
		${QtWidgets_INCLUDE_DIRS}
	)
	
	if(NOT WIN32 OR NOT USE_WINHTTP)
		list(APPEND arxcrashreporter_INCLUDES ${CURL_INCLUDE_DIRS})
	endif()
	
	set(arxcrashreporter_LIBRARIES
		${BASE_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		${QtCore_LIBRARIES}
		${QtConcurrent_LIBRARIES}
		${QtGui_LIBRARIES}
		${QtWidgets_LIBRARIES}
		${QtCore_QTMAIN_LIBRARIES}
	)
	
	if(WIN32 AND USE_WINHTTP)
		list(APPEND arxcrashreporter_LIBRARIES winhttp wininet)
	else()
		list(APPEND arxcrashreporter_LIBRARIES ${CURL_LIBRARIES})
	endif()
	
	add_executable_shared(arxcrashreporter
	                      "${arxcrashreporter_SOURCES}" "${arxcrashreporter_LIBRARIES}")
	set_binary_type(arxcrashreporter WIN32)
	set_binary_installdir(arxcrashreporter "${CMAKE_INSTALL_LIBEXECDIR}")
	add_binary_includes(arxcrashreporter ${arxcrashreporter_INCLUDES})
	
endif()

if(ARX_HAVE_PROFILER)
	
	set(arxprofiler_Qt_SOURCES
		tools/profiler/ProfilerMain.cpp
		tools/profiler/ui/ArxProfiler.h
		tools/profiler/ui/ArxProfiler.cpp
	)
	
	set(arxprofiler_SOURCES
		src/platform/profiler/ProfilerDataFormat.h
		${BASE_SOURCES}
	)
	
	if(WIN32)
		list(APPEND arxprofiler_SOURCES ${arx-libertatis-icon.rc})
		create_version_resource(arxprofiler "${VERSION_NAME} Profiler")
		create_application_manifest(arxprofiler ${qt_dpiaware})
		list(APPEND arxprofiler_SOURCES ${arxprofiler-version.rc} ${arxprofiler.manifest})
	endif()
	
	set(arxprofiler_MANUAL_SOURCES
		${arxprofiler_SOURCES}
		${arxprofiler_Qt_SOURCES}
	)
	
	create_source_groups(arxprofiler_SOURCES)
	
	list(APPEND arxprofiler_Qt_SOURCES ${QtGui_SOURCES})
	qt_wrap_ui(arxprofiler_Qt_SOURCES
		tools/profiler/ui/ArxProfiler.ui
	)
	qt_wrap_cpp(arxprofiler_Qt_SOURCES
		tools/profiler/ui/ArxProfiler.h
	)
	
	list(APPEND arxprofiler_SOURCES ${arxprofiler_Qt_SOURCES})
	
	add_source_flags("${arxprofiler_Qt_SOURCES}"
		"${QtCore_EXECUTABLE_COMPILE_FLAGS}"
	)
	
	add_source_definitions("${arxprofiler_Qt_SOURCES}"
		${QtCore_DEFINITIONS}
		${QtGui_DEFINITIONS}
		${QtWidgets_DEFINITIONS}
		-DQT_NO_DEBUG
	)
	
	set(arxprofiler_INCLUDES
		${QtCore_INCLUDE_DIRS}
		${QtGui_INCLUDE_DIRS}
		${QtWidgets_INCLUDE_DIRS}
	)
	
	set(arxprofiler_LIBRARIES
		${BASE_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		${QtCore_LIBRARIES}
		${QtGui_LIBRARIES}
		${QtWidgets_LIBRARIES}
		${QtCore_QTMAIN_LIBRARIES}
	)
	
	add_executable_shared(arxprofiler
	                      "${arxprofiler_SOURCES}" "${arxprofiler_LIBRARIES}")
	set_binary_type(arxprofiler WIN32)
	set_binary_installdir(arxprofiler "${CMAKE_INSTALL_BINDIR}")
	add_binary_includes(arxprofiler ${arxprofiler_INCLUDES})
	
endif()

if(BUILD_TOOLS)
	
	set(arxsavetool_SOURCES
		${BASE_SOURCES}
		${PLATFORM_CONSOLE_SOURCES}
		${IO_RESOURCE_SOURCES}
		${UTIL_CMDLINE_SOURCES}
		src/core/Localisation.cpp
		src/io/SaveBlock.cpp
		src/io/IniReader.cpp
		src/io/IniSection.cpp
		tools/savetool/SaveFix.h
		tools/savetool/SaveFix.cpp
		tools/savetool/SaveRename.cpp
		tools/savetool/SaveTool.cpp
		tools/savetool/SaveView.h
		tools/savetool/SaveView.cpp
	)
	
	if(WIN32)
		create_version_resource(arxsavetool "${VERSION_NAME} Save File Tool")
		create_application_manifest(arxsavetool)
		list(APPEND arxsavetool_SOURCES ${arxsavetool-version.rc} ${arxsavetool.manifest})
	endif()
	
	set(arxsavetool_LIBRARIES ${BASE_LIBRARIES} ${ZLIB_LIBRARIES})
	
	add_executable_shared(arxsavetool "${arxsavetool_SOURCES}" "${arxsavetool_LIBRARIES}")
	
	set(arxunpak_SOURCES
		${BASE_SOURCES}
		${PLATFORM_CONSOLE_SOURCES}
		${IO_RESOURCE_SOURCES}
		${UTIL_CMDLINE_SOURCES}
		${UTIL_MD5_SOURCES}
		tools/unpak/UnPak.cpp
	)
	
	if(WIN32)
		create_version_resource(arxunpak "${VERSION_NAME} PAK File Unpacker")
		create_application_manifest(arxunpak)
		list(APPEND arxunpak_SOURCES ${arxunpak-version.rc} ${arxunpak.manifest})
	endif()
	
	set(arxunpak_LIBRARIES ${BASE_LIBRARIES})
	
	add_executable_shared(arxunpak "${arxunpak_SOURCES}" "${arxunpak_LIBRARIES}")
	
endif()

if(BUILD_IO_LIBRARY)
	
	set(ArxIO_SOURCES
		${BASE_SOURCES}
		src/io/Blast.cpp
		src/lib/ArxIO.cpp
	)
	if(WIN32)
		list(APPEND ArxIO_SOURCES src/platform/WindowsUtils.cpp)
	endif()
	
	set(ArxIO_LIBRARIES ${BASE_LIBRARIES} ${ZLIB_LIBRARIES})
	
	add_library_shared(ArxIO "${ArxIO_SOURCES}" "${ArxIO_LIBRARIES}")
	set_binary_public_headers(ArxIO src/lib/ArxIO.h)
	set_binary_version(ArxIO "${VERSION_NUMBER}" "${IO_LIBRARY_ABI_VERSION}")
	
endif()

if(INSTALL_BLENDER_PLUGIN)
	install(DIRECTORY plugins/blender/arx_addon/ DESTINATION "${INSTALL_BLENDER_PLUGINDIR}")
endif()


# Build and link executables

if(UNITY_BUILD)
	unity_build()
else()
	shared_build()
endif()


# Custom make targets

if(DOXYGEN_EXECUTABLE)
	set(DOXYGEN_OUTPUT_DIR "${PROJECT_BINARY_DIR}/doc")
	configure_file("scripts/Doxyfile" "Doxyfile" ESCAPE_QUOTES)
	add_custom_target(doc
		#build the documentation
		COMMAND "${CMAKE_COMMAND}" -E make_directory "${DOXYGEN_OUTPUT_DIR}"
		COMMAND "${CMAKE_COMMAND}" -E chdir ${PROJECT_SOURCE_DIR}
			${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
		COMMENT "Building doxygen documentation."
		VERBATIM
	)
endif()

file(GLOB_RECURSE STYLE_CHECKED_SOURCES RELATIVE ${PROJECT_SOURCE_DIR}
	src/*.cpp
	src/*.c
	src/*.h
	tools/*.cpp
	tools/*.c
	tools/*.h
)
list(APPEND STYLE_CHECKED_SOURCES
	${ARX_SOURCES}
	${ALL_INCLUDES}
	${arxsavetool_SOURCES}
	${arxunpak_SOURCES}
	${arxcrashreporter_MANUAL_SOURCES}
	${arxprofiler_MANUAL_SOURCES}
	${ArxIO_SOURCES}
)
list(REMOVE_ITEM STYLE_CHECKED_SOURCES
	src/graphics/image/stb_image.h
	src/graphics/image/stb_image.cpp
)

add_style_check_target(style "${STYLE_CHECKED_SOURCES}" ARX)

if(BUILD_TESTS)
	add_subdirectory(tests "${PROJECT_BINARY_DIR}/test")
endif()


# Install Arx Libertatis data files
# TODO these should be zipped up once we support that
install(DIRECTORY data/core/ DESTINATION "${INSTALL_DATADIR}")
# Also tell arx where to find the not yet installed data
# This is required to run arx from the build directory without installing
set(arx_core_dir "${PROJECT_SOURCE_DIR}/data/core")
get_filename_component(arx_core_dir "${arx_core_dir}" ABSOLUTE)
file(TO_NATIVE_PATH "${arx_core_dir}" arx_core_dir)
if(CMAKE_VERSION VERSION_LESS 2.8.12)
	foreach(config IN LISTS BUILD_TYPES)
		get_property(arx_location TARGET arx PROPERTY LOCATION_${config})
		get_filename_component(arx_location "${arx_location}" PATH)
		file(WRITE "${arx_location}/data.dirs" "${arx_core_dir}\n")
	endforeach()
else()
	file(GENERATE OUTPUT "$<TARGET_FILE_DIR:arx>/data.dirs" CONTENT "${arx_core_dir}\n")
endif()

# Install the data install script
if(INSTALL_SCRIPTS AND NOT WIN32)
	install(PROGRAMS scripts/arx-install-data DESTINATION "${SCRIPTDIR}")
endif()

# Install icon and desktop entry
if(NOT WIN32 AND NOT MACOS)
	
	find_program(DESKTOP_FILE_VALIDATE desktop-file-validate)
	
	if(DESKTOP_FILE_VALIDATE)
		get_filename_component(ABS_DESKTOP_FILE "data/icons/${ICON_NAME}.desktop" ABSOLUTE)
		add_custom_target(
			validate_desktop_file ALL
			"${DESKTOP_FILE_VALIDATE}" "${ABS_DESKTOP_FILE}" VERBATIM
			DEPENDS "${ABS_DESKTOP_FILE}"
		)
	endif()
	
	install(FILES "data/icons/${ICON_NAME}.desktop" DESTINATION "${APPDIR}" OPTIONAL)
	
endif()

# Install man pages
install(FILES data/man/arx.6 DESTINATION "${CMAKE_INSTALL_MANDIR}/man6" OPTIONAL)
if(BUILD_TOOLS)
	install(FILES data/man/arxsavetool.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
	        OPTIONAL)
	install(FILES data/man/arxunpak.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
	        OPTIONAL)
endif()
if(INSTALL_SCRIPTS AND NOT WIN32)
	install(FILES data/man/arx-install-data.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
	        OPTIONAL)
endif()


# Print a configuration summary

message("")
message("Configuration:")
set(separator "")
set(build_type "")
foreach(config IN LISTS BUILD_TYPES)
	set(build_type "${build_type}${separator} ${config}")
	set(separator ",")
endforeach()
if(DEBUG AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	set(build_type "${build_type} with debug output")
elseif(NOT DEBUG AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	set(build_type "${build_type} without debug output")
endif()
if(UNITY_BUILD)
	set(build_type "${build_type} (unity build)")
endif()
message(" - Build type:${build_type}")
print_configuration("Filesystem backend" FIRST
	ARX_HAVE_POSIX_FILESYSTEM "POSIX"
	ARX_HAVE_WIN32_FILESYSTEM "Win32"
	1                         "Boost"
)
print_configuration("Renderer"
	ARX_HAVE_EPOXY "OpenGL (libepoxy)"
	ARX_HAVE_EPOXY "OpenGL ES (libepoxy)"
	ARX_HAVE_GLEW  "OpenGL (GLEW)"
)
print_configuration("Audio features"
	ARX_HAVE_OPENAL_EFX  "EFX"
	ARX_HAVE_OPENAL_HRTF "HRTF"
)
print_configuration("Windowing"
	ARX_HAVE_SDL2 "SDL 2"
	ARX_HAVE_SDL1 "SDL 1.2"
	HAVE_QT5      "Qt 5"
	HAVE_QT4      "Qt 4"
)
print_configuration("Features"
	ARX_HAVE_CRASHHANDLER_POSIX   "POSIX crash handler"
	ARX_HAVE_CRASHHANDLER_WINDOWS "Win32 crash handler"
	BUILD_PROFILER_INSTRUMENT     "profiler instrumentation"
)
print_configuration("Tools"
	BUILD_TOOLS            "savetool"
	BUILD_TOOLS            "unpak"
	ARX_HAVE_CRASHREPORTER "crash reporter"
	ARX_HAVE_PROFILER      "profiler"
)
message("")


# Detect configuration errors

if(NOT (ARX_HAVE_PTHREADS OR WIN32))
	message(SEND_ERROR "No supported thread libraries found.")
endif()
if(NOT (ARX_HAVE_OPENGL))
	message(SEND_ERROR "No renderer available - need libepoxy or OpenGL and GLEW")
endif()
if(NOT (ARX_HAVE_OPENAL))
	message(WARNING "No audio backend enabled - need OpenAL")
endif()
if(NOT (ARX_HAVE_SDL))
	message(SEND_ERROR "No windowing backend available - need SDL 2 or 1.2")
endif()
if(NOT (ARX_HAVE_NANOSLEEP OR WIN32))
	message(SEND_ERROR "Need either nanosleep or WIN32.")
endif()
if(NOT (ARX_HAVE_CLOCK_GETTIME OR WIN32 OR ARX_HAVE_MACH_CLOCK))
	message(SEND_ERROR "Need either clock_gettime or WIN32 or mach clock_get_time.")
endif()
if(WIN32 AND NOT ARX_HAVE_CXX17_FSTREAM_WCHAR)
	message(WARNING "Missing support for Unicode file paths")
endif()

# Check for consistent configuration between SDL and libepoxy/GLEW
if(ARX_HAVE_SDL AND ARX_HAVE_OPENGL AND NOT WIN32 AND NOT APPLE)
	if(USE_X11 AND NOT SDL_HAVE_VIDEO_X11)
		message(WARNING "SDL is not configured with X11 support")
	endif()
	if(USE_WAYLAND AND NOT SDL_HAVE_VIDEO_WAYLAND)
		message(WARNING "SDL is not configured with Wayland support")
	endif()
	if(EPOXY_FOUND)
		set(wrangler "libepoxy")
	else()
		set(wrangler "GLEW")
	endif()
	if(SDL_HAVE_VIDEO_X11 AND NOT ARX_HAVE_GLX)
		message(WARNING "SDL is configured with X11 support but ${wrangler} is missing GLX support")
	endif()
	if(SDL_HAVE_VIDEO_WAYLAND AND NOT ARX_HAVE_EGL)
		message(WARNING "SDL is configured with Wayland support but ${wrangler} is missing EGL support")
	endif()
endif()
