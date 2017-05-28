
# Try to find the XCode iconutil tool or compatible icnsutil re-implementation
# Once done this will define
#
# iconutil_FOUND
# iconutil_EXECUTABLE   Where to find iconutil/icnsutil
#
# For non-macOS platforms, icnsutil can be found at https://github.com/pornel/libicns

set(_iconutil_EXECUTABLE_NAMES icnsutil)
if(APPLE)
	set(_iconutil_EXECUTABLE_NAMES iconutil ${_iconutil_EXECUTABLE_NAMES})
endif()

find_program(
	iconutil_EXECUTABLE
	NAMES ${_iconutil_EXECUTABLE_NAMES}
	DOC "XCode iconutil tool or compatible re-implementation"
)

# handle the QUIETLY and REQUIRED arguments and set iconutil_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(iconutil REQUIRED_VARS iconutil_EXECUTABLE)
