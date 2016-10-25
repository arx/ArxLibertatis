
# Try to find DbgHelp library and include path.
# Once done this will define
#
# DBGHELP_FOUND
# DBGHELP_INCLUDE_DIR
# DBGHELP_LIBRARIES


find_library(DBGHELP_LIBRARY dbghelp.lib)
if(DBGHELP_LIBRARY)
	find_path(DBGHELP_INCLUDE_DIR dbghelp.h)
elseif(MSVC)
	set(DBGHELP_LIBRARY dbghelp)
	set(DBGHELP_INCLUDE_DIR)
endif()

# Handle the REQUIRED argument and set DBGHELP_FOUND
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DbgHelp DEFAULT_MSG DBGHELP_LIBRARY)

mark_as_advanced(DBGHELP_INCLUDE_DIR)
mark_as_advanced(DBGHELP_LIBRARY)

if(DBGHELP_FOUND)
	set(DBGHELP_LIBRARIES ${DBGHELP_LIBRARY})
endif(DBGHELP_FOUND)
