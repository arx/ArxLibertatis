
# Where to look for source dirs
set(ICON_SOURCE_DIRS "${CMAKE_SOURCE_DIR}")

set(ICON_RESOURCE_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/Icon.rc.in")
set(ICON_RESOURCE_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/ConfigureFileScript.cmake")

set(Inkscape_OPTIONS)
set(ImageMagick_OPTIONS -filter lanczos -colorspace sRGB)
set(OptiPNG_OPTIONS -quiet)

# Add an icon generated from source icons
#
# Usage: add_icon([TARGET] name
#   source.svg source_size_spec [source_render_size] [source2.svg …]
#   [PNG png_size_specs]
#   [OVERVIEW overview_size_specs]
#   [ALL]
#   [INSTALL]
# )
#
# TARGET              By default a target called ${name}-icon will be created that
#                     depends on all the output files. With this option the target
#                     will be called just ${name}.
#
# source.svg          The .svg file to generate the icon from.
#                     Multiple .svg files with their own size_specs can be added.
# source_size_spec    The icon size to use the preceding .svg for
# source_render_size  The resulution to rasterize the .svg at
#                     If this differs from the source_size_spec the result will be scaled.
#
# PNG                 Generate a set of portable .png icons
#                     The path to the icon files will be stored in ${${name}-png}.
#                     The path to the first icon will be stored in ${${name}.png}.
# OVERVIEW            Generate a montage of different icon sizes
#                     The path to the file will be stored in ${${name}-icon-overview}.
#
# The icon size specs for both the source files and the generated files
# can be on of the following:
#  ${height}                Generate an icon with display and pixel resolution ${height}.
#  ${height}@${multiplier}  Generate an icon with display resolution ${height} and
#                           pixel resolution ${height} * ${multiplier}.
#
# The order of the sizes matters for PNG and OVERVIEW icons
#  For PNG icons the first size is used as the default size.
#    The remaining order is ignored.
#  For OVERVIEWs the order determines the order of the icons in the overview
#  For all other icon types the order is ignored.
#
# ALL                 By default only those icon types listed in ICON_TYPE will be built,
#                     unless ICON_TYPE is "all" or only sizes for the PNG type have been
#                     defined.
#                     When using this option, all icon types with defined sizes are built.
#
# INSTALL             Install the generated icons to standard system locations.
#                     PNG icons are installed to ${ICONDIR}
#                     OVERVIEWs are never installed
#
# Variables used by this function:
#
# ICON_SOURCE_DIRS     Relative .svg sources are resolved to paths in this list.
#                      Additionally, needed output or intermediate icon files are found
#                      in the paths in this list, they are used instead of re-generating
#                      those files.
#
# ICON_TYPE            The icon type(s) to generate. Can be "none", "all" or a list of one
#                      or more of "png" and "overview".
#                      Ignored if only sizes for the PNG type are specified or if the
#                      ALL option is used.
#
# Inkscape_OPTIONS     Additional command-line options to pass to inkscape.
#
# ImageMagick_OPTIONS  Additional command-line options to pass to the Image Magick
#                      convert, mogrify and montage commands.
#
# OptiPNG_OPTIONS      Additional command-line options to pass to optipng.
#
function(add_icon name)
	
	set(MODE_DEFAULT  0)
	set(MODE_SOURCE   1)
	set(MODE_PNG      2)
	set(MODE_OVERVIEW 3)
	set(MODE_NAME     4)
	
	set(mode ${MODE_DEFAULT})
	
	# Parse arguments
	
	set(target ${name}-icon)
	if(name STREQUAL "TARGET")
		set(mode ${MODE_NAME})
	endif()
	
	set(current_source)
	
	set(png_sizes)
	set(overview_sizes)
	
	set(install 0)
	set(all_types 0)
	
	set(source_sizes)
	
	foreach(arg IN LISTS ARGN)
		
		if(mode EQUAL MODE_NAME)
			set(name "${arg}")
			set(target "${arg}")
			set(mode ${MODE_DEFAULT})
		elseif((mode EQUAL MODE_DEFAULT OR mode EQUAL MODE_SOURCE) AND arg MATCHES "\\.svg$")
			_add_icon_source()
			set(current_source ${arg})
			set(current_source_size)
			set(current_source_render_size)
			set(mode ${MODE_SOURCE})
		elseif(arg STREQUAL "PNG")
			_add_icon_source()
			set(mode ${MODE_PNG})
		elseif(arg STREQUAL "OVERVIEW")
			_add_icon_source()
			set(mode ${MODE_OVERVIEW})
		elseif(arg STREQUAL "INSTALL")
			_add_icon_source()
			set(install 1)
			set(mode ${MODE_DEFAULT})
		elseif(arg STREQUAL "ALL")
			_add_icon_source()
			set(all_types 1)
			set(mode ${MODE_DEFAULT})
		elseif(mode EQUAL MODE_SOURCE)
			if(NOT current_source_size)
				set(current_source_size ${arg})
			elseif(NOT current_source_render_size)
				set(current_source_render_size ${arg})
			else()
				message(FATAL_ERROR "Unexpected icon argument: ${arg}")
			endif()
		elseif(mode EQUAL MODE_PNG)
			list(APPEND png_sizes ${arg})
		elseif(mode EQUAL MODE_OVERVIEW)
			list(APPEND overview_sizes ${arg})
		else()
			message(FATAL_ERROR "Unexpected icon argument: ${arg}")
		endif()
		
	endforeach()
	
	# Clean output variables
	
	foreach(size IN LISTS source_sizes png_sizes)
		unset(${name}-${size}.png CACHE)
	endforeach()
	if(png_sizes)
		unset(${name}.png CACHE)
		unset(${name}-png CACHE)
	endif()
	
	# Generate icons
	
	set(generated_sizes)
	
	if(NOT png_sizes)
		set(png_sizes ${source_sizes})
	endif()
	set(all_types 1)
	if((all_types AND NOT ICON_TYPE STREQUAL "none") OR ICON_TYPE STREQUAL "all")
		set(ICON_TYPE "png")
	endif()
	
	set(outputs)
	
	foreach(type IN LISTS ICON_TYPE)
		
		if(type STREQUAL "png")
			
			# Portable icon
			
			if(png_sizes)
				
				set(png_file)
				set(png_files)
				
				foreach(size IN LISTS png_sizes)
					
					_add_icon_size(source ${size})
					if(source)
						
						if(NOT png_file)
							set(png_file ${source})
							set(install_name "${name}.png")
						else()
							set(install_name "${name}_${size}.png")
						endif()
						list(APPEND png_files ${source})
						
						if(install)
							install(
								FILES ${source}
								DESTINATION "${ICONDIR}"
								COMPONENT icons
								RENAME "${install_name}"
								OPTIONAL
							)
						endif()
						
					endif()
					
				endforeach()
				
				list(APPEND outputs ${png_files})
				
				set(${name}.png ${png_file} CACHE INTERNAL "")
				set(${name}-png ${png_files} CACHE INTERNAL "")
				
			endif()
			
		elseif(type STREQUAL "overview")
			
			# Icon size comparison chart
			
			if(overview_sizes)
				
				_find_icon(overview_file "${name}-overview.png")
				
				if(NOT overview_file)
					
					find_package(ImageMagick COMPONENTS montage REQUIRED)
					
					set(overview_file "${ICON_BINARY_DIR}/${name}-overview.png")
					set(overview_command "${ImageMagick_montage_EXECUTABLE}" ${ImageMagick_OPTIONS})
					set(overview_depends "${ImageMagick_montage_EXECUTABLE}")
					
					foreach(size IN LISTS overview_sizes)
						
						_add_icon_size(source ${size})
						if(source)
							
							list(APPEND overview_command
								"("
									-bordercolor black -border 20x20
									-background black -alpha remove
									"${source}"
								")"
								"("
									-bordercolor gray -border 20x20
									-background gray -alpha remove
									"${source}"
								")"
								"("
									-bordercolor white -border 20x20
									-background white -alpha remove
									"${source}"
								")"
							)
							list(APPEND overview_depends "${source}")
							
						endif()
						
					endforeach()
					
					list(APPEND overview_command
						-tile 3x
						-geometry +0+0
						-background gray
						"${overview_file}"
					)
					
					add_custom_command(
						OUTPUT "${overview_file}"
						COMMAND "${CMAKE_COMMAND}" -E make_directory "${ICON_BINARY_DIR}"
						COMMAND ${overview_command}
						DEPENDS ${overview_depends}
						COMMENT "Generating ${name}-overview.png"
						VERBATIM
					)
					
				endif()
				
				list(APPEND outputs ${overview_file})
				
				set(${target}-overview ${overview_file} CACHE INTERNAL "")
				
			endif()
			
		elseif(NOT type STREQUAL "none")
			message(FATAL_ERROR "Unknown icon type: '${type}'")
		endif()
		
	endforeach()
	
	if(install)
		add_custom_target(${target} ALL DEPENDS ${outputs})
	else()
		add_custom_target(${target} DEPENDS ${outputs})
	endif()
	
endfunction()

# Helper functions

# Parse an icon source definition
macro(_add_icon_source)
	
	if(current_source)
		
		if(NOT current_source_size)
			message(FATAL_ERROR "Missing size for source: ${current_source}")
		endif()
		
		if(NOT current_source_render_size)
			_get_icon_resolution(current_source_render_size "${current_source_size}")
		endif()
		
		set(source_file_${current_source_size} ${current_source})
		set(source_render_size_${current_source_size} ${current_source_render_size})
		list(APPEND source_sizes ${current_source_size})
		
		set(current_source)
		set(mode ${MODE_DEFAULT})
		
	endif()
	
endmacro()

# Find a source or prebuilt icon in ICON_SOURCE_DIRS
#
# Params:
#  var   Variable to receive the path to the icon file.
#  name  Icon filename
function(_find_icon var name)
	
	if(IS_ABSOLUTE name)
		if(EXISTS name)
			set(${var} "${name}" PARENT_SCOPE)
		else()
			set(${var} PARENT_SCOPE)
		endif()
		return()
	endif()
	
	foreach(dir IN LISTS ICON_SOURCE_DIRS)
		if(EXISTS "${dir}/${name}")
			set(${var} "${dir}/${name}" PARENT_SCOPE)
			return()
		endif()
	endforeach()
	
	set(${var} PARENT_SCOPE)
	
endfunction()

# Get the display size from an icon size spec
#
# Params:
#  var   Variable to receive the display size
#  spec  Icon size spec
function(_get_icon_size var spec)
	
	if(spec MATCHES "^([0-9]+)(\\@[0-9]+x)?$")
		set(${var} ${CMAKE_MATCH_1} PARENT_SCOPE)
	else()
		message(FATAL_ERROR "Unknown icon size spec: ${spec}")
	endif()
	
endfunction()

# Get the pixel size from an icon size spec
#
# Params:
#  var   Variable to receive the pixel size
#  spec  Icon size spec
function(_get_icon_resolution var spec)
	
	if(spec MATCHES "^([0-9]+)\\@([0-9]+)x$")
		math(EXPR size "${CMAKE_MATCH_1} * ${CMAKE_MATCH_2}")
		set(${var} ${size} PARENT_SCOPE)
	elseif(spec MATCHES "^([0-9]+)$")
		set(${var} ${CMAKE_MATCH_1} PARENT_SCOPE)
	else()
		message(FATAL_ERROR "Unknown icon size spec: ${spec}")
	endif()
	
endfunction()

#  Add an optional OptiPNG step to a generated .png file
#
# Params:
#  file  The file to add the optimization step to
function(_optimize_icon file)
	
	if(NOT OPTIMIZE_ICONS)
		return()
	endif()
	
	if(STRICT_USE)
		set(OPTIONAL_DEPENDENCY REQUIRED)
	else()
		set(OPTIONAL_DEPENDENCY)
	endif()
	
	find_package(OptiPNG ${OPTIONAL_DEPENDENCY})
	
	if(OptiPNG_FOUND)
		
		add_custom_command(
			OUTPUT "${file}"
			COMMAND "${OptiPNG_EXECUTABLE}" ${OptiPNG_OPTIONS} "${file}"
			DEPENDS "${OptiPNG_EXECUTABLE}"
			VERBATIM
			APPEND
		)
		
	endif()
	
endfunction()

# Get a .png file for a specific icon size
#
# Creates the file from scalable or larger versions if needed.
#
# Params:
#  var   Variable to receive the .png file name or null
#  spec  Icon size to get or create
function(_add_icon_size var size)
	
	set(filename "${name}-${size}.png")
	set(file "${ICON_BINARY_DIR}/${filename}")
	
	# Return early if we have already generated the file
	list(FIND generated_sizes ${size} already_generated)
	if(already_generated GREATER -1)
		set(${var} ${file} PARENT_SCOPE)
		return()
	endif()
	
	# Use prebuilt files if available
	_find_icon(prebuilt_file ${filename})
	if(prebuilt_file)
		set(${var} ${prebuilt_file} PARENT_SCOPE)
		set(${name}-${size}.png "${prebuilt_file}" CACHE INTERNAL "")
		return()
	endif()
	
	# Look for the source file with the next larger display size
	
	_get_icon_size(ssize ${size})
	_get_icon_resolution(res ${size})
	
	set(match)
	
	foreach(source IN LISTS source_sizes)
		
		if(source STREQUAL size)
			# Exact natch
			set(match ${source})
			break()
		endif()
		
		_get_icon_size(source_ssize ${source})
		_get_icon_resolution(source_res ${source})
		
		if(NOT source_ssize LESS ssize AND NOT source_res LESS res)
			
			if(NOT match)
				# First match
				set(match ${source})
			else()
				
				_get_icon_size(last_ssize ${match})
				if(source_ssize LESS last_ssize)
					# Closer match
					set(match ${source})
				elseif(source_ssize EQUAL last_ssize)
					_get_icon_resolution(last_res ${match})
					if(source_res EQUAL res)
						# Closer match
						set(match ${source})
					elseif(source_res GREATER last_res AND NOT last_res EQUAL res)
						# Closer match
						set(match ${source})
					endif()
				endif()
				
			endif()
			
		endif()
		
	endforeach()
	
	if(NOT match)
		message(WARNING "Could not generate size ${_add_size} for icon ${name}!")
		return()
	endif()
	
	_get_icon_size(source_ssize ${match})
	_get_icon_resolution(source_res ${match})
	
	# If the source display size is larger, drop the @nx suffix
	# and use the multiplied size instead to avoid duplicated rescales
	if(res GREATER size AND NOT source_ssize LESS res)
		_add_icon_size(path ${res})
		set(generated_sizes ${generated_sizes} PARENT_SCOPE)
		set(${var} ${path} PARENT_SCOPE)
		return()
	endif()
	
	# Rasterize the source file
	set(source_filename "${name}-${match}.png")
	set(source_file "${ICON_BINARY_DIR}/${source_filename}")
	list(FIND generated_sizes ${match} already_generated)
	if(NOT already_generated GREATER -1)
		
		find_package(Inkscape REQUIRED)
		
		_find_icon(source_svg ${source_file_${match}})
		if(NOT source_svg)
			message(WARNING "Could not find icon source ${source_file_${match}}!")
		endif()
		
		set(render_size ${source_render_size_${match}})
		
		add_custom_command(
			OUTPUT "${source_file}"
			COMMAND "${CMAKE_COMMAND}" -E make_directory "${ICON_BINARY_DIR}"
			COMMAND "${Inkscape_EXECUTABLE}" -z ${Inkscape_OPTIONS}
				-f "${source_svg}" -h "${render_size}" -e "${source_file}"
			MAIN_DEPENDENCY "${source_svg}"
			DEPENDS "${Inkscape_EXECUTABLE}"
			COMMENT "Rendering ${source_file_${match}} to ${source_filename}"
			VERBATIM
		)
		
		# We let the user override the rasterize size
		# If they have done so we need to scale the result to the source pixel size
		
		if(NOT render_size EQUAL source_res)
			
			find_package(ImageMagick COMPONENTS mogrify REQUIRED)
			
			add_custom_command(
				OUTPUT "${source_file}"
				COMMAND "${ImageMagick_mogrify_EXECUTABLE}" ${ImageMagick_OPTIONS}
					-resize "x${source_res}" "${source_file}"
				DEPENDS "${ImageMagick_mogrify_EXECUTABLE}"
				VERBATIM
				APPEND
			)
			
		endif()
		
		_optimize_icon(${source_file})
		
		list(APPEND generated_sizes ${match})
		set(${name}-${match}.png "${source_file}" CACHE INTERNAL "")
		
	endif()
	
	if(source_res EQUAL res)
		set(${var} ${source_file} PARENT_SCOPE)
	else()
		
		# Scale the source file to the desired size
		
		find_package(ImageMagick COMPONENTS convert REQUIRED)
		
		add_custom_command(
			OUTPUT "${file}"
			COMMAND "${CMAKE_COMMAND}" -E make_directory "${ICON_BINARY_DIR}"
			COMMAND "${ImageMagick_convert_EXECUTABLE}" ${ImageMagick_OPTIONS}
				"${source_file}" -resize "x${res}" "${file}"
			MAIN_DEPENDENCY "${source_file}"
			DEPENDS "${ImageMagick_convert_EXECUTABLE}"
			COMMENT "Scaling ${source_filename} to ${filename}"
			VERBATIM
		)
		
		_optimize_icon(${file})
		
		list(APPEND generated_sizes ${size})
		set(${name}-${size}.png "${file}" CACHE INTERNAL "")
		
		set(${var} ${file} PARENT_SCOPE)
		
	endif()
	
	set(generated_sizes ${generated_sizes} PARENT_SCOPE)
	
endfunction()
