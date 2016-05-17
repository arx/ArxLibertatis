
# Try to find the OptiPNG PNG compression optimizer
# Once done this will define
#
# OptiPNG_FOUND
# OptiPNG_EXECUTABLE   Where to find OptiPNG

find_program(
	OptiPNG_EXECUTABLE
	NAMES optipng
	DOC "OptiPNG PNG compression optimizer"
)

# handle the QUIETLY and REQUIRED arguments and set OptiPNG_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OptiPNG REQUIRED_VARS OptiPNG_EXECUTABLE)
