
# Try to find the SDL2 library and include path.
# Once done this will define
#
# SDL2_FOUND
# SDL2_INCLUDE_DIR   Where to find SDL.h
# SDL2_LIBRARY       The libSDL2 library only
# SDL2_LIBRARIES     All libraries to link against for SDL2
#
# Typical usage could be something like:
#   find_package(SDL2 REQUIRED)
#   include_directories(SYSTEM ${SDL2_INCLUDE_DIR})
#   ...
#   target_link_libraries(myexe ${SDL2_LIBRARIES})
#
# The following additional options can be defined before the find_package() call:
# SDL2_USE_STATIC_LIBS  Statically link against libsdl2 (default: OFF)
# SDL2_BUILDING_LIBRARY Don't link against SDL2main - this means the user is responsible
#                       for abstracting their own main() from OS-specific entry points.
#
# For OS X, this module will automatically add the -framework Cocoa on your behalf.
#
# $SDL2DIR is an environment variable that would
# correspond to the ./configure --prefix=$SDL2DIR
# used in building SDL2.
#
#
# Modified by Eric Wing.
# Added code to assist with automated building by using environmental variables
# and providing a more controlled/consistent search behavior.
# Added new modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).
# Also corrected the header search path to follow "proper" SDL guidelines.
# Added a search for SDL2main which is needed by some platforms.
# Added a search for threads which is needed by some platforms.
# Added needed compile switches for MinGW.
#
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of
# SDL2_LIBRARY to override this selection or set the CMake environment
# CMAKE_INCLUDE_PATH to modify the search paths.
#
# Note that the header path has changed from SDL2/SDL.h to just SDL.h
# This needed to change because "proper" SDL convention
# is #include "SDL.h", not <SDL2/SDL.h>. This is done for portability
# reasons because not all systems place things in SDL2/ (see FreeBSD).

#=============================================================================
# Copyright 2003-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)


if(UNIX)
	find_package(PkgConfig QUIET)
	pkg_check_modules(_PC_SDL2 sdl2)
endif()


include(UseStaticLibs)
use_static_libs(SDL2 _PC_SDL2)


set(_SDL2_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)


find_path(SDL2_INCLUDE_DIR SDL.h
	HINTS
		$ENV{SDL2DIR}
		${_PC_SDL2_INCLUDE_DIRS}
	PATH_SUFFIXES include/SDL2 include
	PATHS ${_SDL2_SEARCH_PATHS}
	DOC "The directory where SDL.h resides"
)
mark_as_advanced(SDL2_INCLUDE_DIR)


# Prefer libraries in the same prefix as the include files
string(REGEX REPLACE "(.*)/include(/SDL)?/?" "\\1" _SDL2_BASE_DIR ${SDL2_INCLUDE_DIR})

find_library(SDL2_LIBRARY
	NAMES SDL2
	HINTS
		$ENV{SDL2DIR}
		${_PC_SDL2_LIBRARY_DIRS}
		"${_SDL2_BASE_DIR}/lib"
	PATH_SUFFIXES lib64 lib
	PATHS ${SDL2_SEARCH_PATHS}
	DOC "The SDL2 library"
)
mark_as_advanced(SDL2_LIBRARY)


if(NOT SDL2_BUILDING_LIBRARY AND NOT ${SDL2_INCLUDE_DIR} MATCHES ".framework")
	# This is mainly for Windows. Unix platforms provide SDL2main for compatibility
	# even though they don't necessarily need it.
	find_library(SDL2MAIN_LIBRARY
		NAMES SDL2main
		HINTS
			$ENV{SDL2DIR}
			${_PC_SDL2_LIBRARY_DIRS}
			"${_SDL2_BASE_DIR}/lib"
		PATH_SUFFIXES lib64 lib
		PATHS ${SDL2_SEARCH_PATHS}
		DOC "The SDL2main library"
	)
	mark_as_advanced(SDL2MAIN_LIBRARY)
else()
	unset(SDL2MAIN_LIBRARY)
endif()


use_static_libs_restore()


# SDL2 may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
if(NOT APPLE)
	find_package(Threads)
endif()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2 DEFAULT_MSG SDL2_LIBRARY SDL2_INCLUDE_DIR)


IF(SDL2_LIBRARY)
	
	set(SDL2_LIBRARIES ${SDL2_LIBRARY})
	
	# For SDL2main
	if(NOT SDL2_BUILDING_LIBRARY AND SDL2MAIN_LIBRARY)
			list(APPEND SDL2_LIBRARIES ${SDL2MAIN_LIBRARY})
	endif()
	
	# For OS X, SDL2 uses Cocoa as a backend so it must link to Cocoa.
	if(APPLE)
		list(APPEND SDL2_LIBRARIES "-framework Cocoa")
	endif(APPLE)
	
	# For threads, as mentioned Apple doesn't need this.
	# In fact, there seems to be a problem if I used the Threads package
	# and try using this line, so I'm just skipping it entirely for OS X.
	if(NOT APPLE)
		list(APPEND SDL2_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
	endif(NOT APPLE)
	
endif()
