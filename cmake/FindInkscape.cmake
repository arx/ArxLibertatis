
# Try to find the Inkscape command-line SVG rasterizer
# Once done this will define
#
# Inkscape_FOUND
# Inkscape_EXECUTABLE   Where to find Inkscape
# Inkscape_EXPORT       Option to specify the destination file
# Inkscape_GUI          Option to disable the GUI if needed

find_program(
	Inkscape_EXECUTABLE
	NAMES inkscape
	DOC "Inkscape command-line SVG rasterizer"
)

execute_process(COMMAND ${Inkscape_EXECUTABLE} "--help" OUTPUT_VARIABLE _Inkscape_HELP ERROR_QUIET)

if(_Inkscape_HELP MATCHES "--without-gui")
	set(Inkscape_GUI "--without-gui" CACHE STRING "Inkscape option to disable the GUI if needed")
endif()

if(NOT DEFINED Inkscape_EXPORT)
	foreach(option IN ITEMS "--export-file" "--export-filename" "--export-png")
		if(_Inkscape_HELP MATCHES "${option}=")
			set(Inkscape_EXPORT "${option}" CACHE STRING "Inkscape option to specify the export filename")
			break()
		endif()
	endforeach()
	if(NOT DEFINED Inkscape_EXPORT)
		message(WARNING "Could not determine Inkscape export file option, assuming --export-file")
		set(Inkscape_EXPORT "--export-file" CACHE STRING "Inkscape option to specify the export filename")
	endif()
endif()

# handle the QUIETLY and REQUIRED arguments and set Inkscape_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Inkscape REQUIRED_VARS Inkscape_EXECUTABLE)
