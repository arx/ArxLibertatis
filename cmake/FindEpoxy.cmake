
# Try to find the libepoxy library and include path.
# Once done this will define
#
# EPOXY_FOUND
# Epoxy_INCLUDE_DIR   Where to find epoxy/gl.h
# Epoxy_LIBRARIES     The epoxy library
#
# Typical usage could be something like:
#   find_package(Epoxy REQUIRED)
#   include_directories(SYSTEM ${Epoxy_INCLUDE_DIR})
#   ...
#   target_link_libraries(myexe ${Epoxy_LIBRARIES})
#
# The following additional options can be defined before the find_package() call:
# Epoxy_USE_STATIC_LIBS  Statically link against libepoxy (default: OFF)

if(UNIX)
	find_package(PkgConfig QUIET)
	pkg_check_modules(_PC_Epoxy epoxy)
endif()

include(UseStaticLibs)
use_static_libs(Epoxy _PC_Epoxy)

find_path(Epoxy_INCLUDE_DIR epoxy/gl.h
	HINTS
		${_PC_Epoxy_INCLUDE_DIRS}
	PATHS
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
	DOC "The directory where epoxy/gl.h resides"
)
mark_as_advanced(Epoxy_INCLUDE_DIR)

# Prefer libraries in the same prefix as the include files
string(REGEX REPLACE "(.*)/include/?" "\\1" Epoxy_BASE_DIR "${Epoxy_INCLUDE_DIR}")

find_library(Epoxy_LIBRARY
	NAMES epoxy libepoxy
	HINTS
		${_PC_Epoxy_LIBRARY_DIRS}
		"${Epoxy_BASE_DIR}/lib"
	PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		$ENV{PROGRAMFILES}/epoxy/lib
	DOC "The epoxy library"
)
mark_as_advanced(Epoxy_LIBRARY)

use_static_libs_restore()

# handle the QUIETLY and REQUIRED arguments and set EPOXY_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Epoxy DEFAULT_MSG Epoxy_LIBRARY Epoxy_INCLUDE_DIR)

if(EPOXY_FOUND)
	set(Epoxy_LIBRARIES ${Epoxy_LIBRARY})
endif(EPOXY_FOUND)

has_static_libs(Epoxy Epoxy_LIBRARIES)
if(Epoxy_HAS_STATIC_LIBS)
	if(_PC_Epoxy_FOUND)
		foreach(lib IN LISTS _PC_Epoxy_STATIC_LIBRARIES)
			if(lib MATCHES "(^|^\-l|/|\\\\)(lib)?epoxy")
				# The main lib we linked above
			elseif(lib MATCHES "(^|^\-l|/|\\\\)(lib)?(ld|dl|dld)")
				list(APPEND Epoxy_LIBRARIES ${CMAKE_DL_LIBS})
			elseif(lib MATCHES "(^|^\-l?|/|\\\\)(lib)?(p?threads?)")
				find_package(Threads REQUIRED)
				if(CMAKE_THREAD_LIBS_INIT)
					list(APPEND Epoxy_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
				endif()
			else()
				message(WARNING "Epoxy: linking unknown library \"${lib}\"")
				list(APPEND Epoxy_LIBRARIES ${lib})
			endif()
		endforeach()
	elseif(NOT WIN32)
		if(CMAKE_DL_LIBS)
			list(APPEND Epoxy_LIBRARIES ${CMAKE_DL_LIBS})
		endif()
		find_package(Threads REQUIRED)
		if(CMAKE_THREAD_LIBS_INIT)
			list(APPEND Epoxy_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
		endif()
	endif()
endif()
