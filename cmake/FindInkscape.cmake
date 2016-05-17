
# Try to find the Inkscape command-line SVG rasterizer
# Once done this will define
#
# Inkscape_FOUND
# Inkscape_EXECUTABLE   Where to find Inkscape

find_program(
	Inkscape_EXECUTABLE
	NAMES inkscape
	DOC "Inkscape command-line SVG rasterizer"
)

# handle the QUIETLY and REQUIRED arguments and set Inkscape_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Inkscape REQUIRED_VARS Inkscape_EXECUTABLE)
