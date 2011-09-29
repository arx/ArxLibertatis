
# Try to find GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_DIR
# GLEW_LIBRARIES
#
# Adapted from http://nvidia-texture-tools.googlecode.com/svn-history/r96/trunk/cmake/FindGLEW.cmake

if(WIN32)
	find_path(GLEW_INCLUDE_DIR GL/glew.h
		$ENV{PROGRAMFILES}/GLEW/include
		DOC "The directory where GL/glew.h resides")
	find_library(GLEW_LIBRARY
		NAMES glew GLEW glew32 glew32s glew64 glew64s
		PATHS
		$ENV{PROGRAMFILES}/GLEW/lib
		DOC "The GLEW library")
else(WIN32)
	find_path(GLEW_INCLUDE_DIR GL/glew.h
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		DOC "The directory where GL/glew.h resides")
	find_library(GLEW_LIBRARY
		NAMES GLEW glew
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The GLEW library")
endif(WIN32)

mark_as_advanced(GLEW_INCLUDE_DIR)
mark_as_advanced(GLEW_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set GLEW_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW DEFAULT_MSG GLEW_LIBRARY GLEW_INCLUDE_DIR)

if(GLEW_FOUND)
	set(GLEW_LIBRARIES ${GLEW_LIBRARY})
endif(GLEW_FOUND)
