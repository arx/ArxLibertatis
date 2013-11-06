
# Try to find the GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_DIR   Where to find GL/glew.h
# GLEW_LIBRARIES     The glew library
# GLEW_DEFINITIONS   Definitions to use when compiling code that uses glew
#
# Typical usage could be something like:
#   find_package(GLEW REQUIRED)
#   include_directories(SYSTEM ${GLEW_INCLUDE_DIR})
#   add_definitions(${GLEW_DEFINITIONS})
#   ...
#   target_link_libraries(myexe ${GLEW_LIBRARIES})
#
# The following additional options can be defined before the find_package() call:
# GLEW_USE_STATIC_LIBS  Statically link against glew (default: OFF)
#
# Adapted from:
#	http://nvidia-texture-tools.googlecode.com/svn-history/r96/trunk/cmake/FindGLEW.cmake

if(UNIX)
	find_package(PkgConfig QUIET)
	pkg_check_modules(_PC_GLEW glew)
endif()

include(UseStaticLibs)
use_static_libs(GLEW _PC_GLEW)

find_path(GLEW_INCLUDE_DIR GL/glew.h
	HINTS
		${_PC_GLEW_INCLUDE_DIRS}
	PATHS
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		$ENV{PROGRAMFILES}/GLEW/include
	DOC "The directory where GL/glew.h resides"
)
mark_as_advanced(GLEW_INCLUDE_DIR)

set(_glew_library_names GLEW glew)

if(WIN32)
	list(APPEND _glew_library_names glew32 glew64)
	if(GLEW_USE_STATIC_LIBS)
		set(_glew_library_names glew32s glew64s ${_glew_library_names})
	endif()
endif()

# Prefer libraries in the same prefix as the include files
string(REGEX REPLACE "(.*)/include/?" "\\1" GLEW_BASE_DIR ${GLEW_INCLUDE_DIR})

find_library(GLEW_LIBRARY
	NAMES ${_glew_library_names}
	HINTS
		${_PC_GLEW_LIBRARY_DIRS}
		"${GLEW_BASE_DIR}/lib"
	PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		$ENV{PROGRAMFILES}/GLEW/lib
	DOC "The GLEW library"
)
mark_as_advanced(GLEW_LIBRARY)

use_static_libs_restore()

set(GLEW_DEFINITIONS -DGL_GLEXT_PROTOTYPES)
if(WIN32 AND GLEW_USE_STATIC_LIBS)
	list(APPEND GLEW_DEFINITIONS -DGLEW_STATIC)
endif()

# handle the QUIETLY and REQUIRED arguments and set GLEW_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW DEFAULT_MSG GLEW_LIBRARY GLEW_INCLUDE_DIR)

if(GLEW_FOUND)
	set(GLEW_LIBRARIES ${GLEW_LIBRARY})
endif(GLEW_FOUND)
