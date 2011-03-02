
# Look for wine compilers
find_program(Wine_GCC winegcc)
mark_as_advanced(Wine_GCC)
find_program(Wine_GXX wineg++)
mark_as_advanced(Wine_GXX)

# handle the QUIETLY and REQUIRED arguments and set CURL_FOUND to TRUE if 
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Wine DEFAULT_MSG Wine_GXX Wine_GCC)
