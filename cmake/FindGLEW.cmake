
# Look for the header file.
find_path(GLEW_INCLUDE_DIR NAMES GL/glew.h)
mark_as_advanced(GLEW_INCLUDE_DIR)

# Look for the library.
find_library(GLEW_LIBRARY NAMES 
	GLEW
)
mark_as_advanced(GLEW_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set CURL_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW DEFAULT_MSG GLEW_LIBRARY GLEW_INCLUDE_DIR)

if(GLEW_FOUND)
	set(GLEW_LIBRARIES ${GLEW_LIBRARY})
endif(GLEW_FOUND)
