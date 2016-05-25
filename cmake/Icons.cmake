
# Where to look for source dirs
set(ICON_SOURCE_DIRS "${CMAKE_SOURCE_DIR}")

set(ICON_RESOURCE_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/Icon.rc.in")
set(ICON_RESOURCE_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/ConfigureFileScript.cmake")

set(Inkscape_OPTIONS)
set(ImageMagick_OPTIONS -filter lanczos -colorspace sRGB)
set(OptiPNG_OPTIONS -quiet)
set(iconutil_OPTIONS)

# Add an icon generated from source icons
#
# Usage: add_icon([TARGET] name
#   source.svg source_size_spec [source_render_size] [source2.svg …]
#   [ICO ico_size_specs]
#   [ICNS icns_size_specs]
#   [ICONSET overview_size_specs]
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
# ICO                 Generate a Windows .ico file containing the following sizes
#                     The path to the .ico file will be stored in ${${name}.ico}.
#                     The path to a resource script will be stored in ${${name}-icon.rc}.
# ICNS                Generate a Mac OS X .icns file containing the following sizes
#                     The path to the .icns file will be stored in ${${name}.icns}.
# ICONSET             Generate a set of .png icons for use with the icon theme spec
#                     The path to the icon files will be stored in ${${name}-iconset}.
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
#  ${height}@${bits}bit     Generate an icon with display and pixel resolution ${height},
#                           but with only ${bits} bits used for the color/alpha channels.
#                           The bit depths is only used for ICO icons and ignored
#                           everywhere else.
#
# The order of the sizes only matters for ICO, PNG and OVERVIEW icons
#  For ICO icons it determines the order in the .ico file and should be sorted from
#    smallest to largest. For each dimension the size specs with higher bit depths
#    should be listed first.
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
#                     ICO icons are never installed
#                     ICNS icons are installed to ${ICONDIR}
#                     ICONSET icons are installed to ${ICONTHEMEDIR}/${size}x${size}/apps
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
#                      or more of "ico", "icns", "iconset", "png" and "overview".
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
# iconutil_OPTIONS     Additional command-line options to pass to iconutil / icnsutil.
#
function(add_icon name)
	
	set(MODE_DEFAULT  0)
	set(MODE_SOURCE   1)
	set(MODE_ICO      2)
	set(MODE_ICNS     3)
	set(MODE_ICONSET  4)
	set(MODE_PNG      5)
	set(MODE_OVERVIEW 6)
	set(MODE_NAME     7)
	
	set(mode ${MODE_DEFAULT})
	
	# Parse arguments
	
	set(target ${name}-icon)
	if(name STREQUAL "TARGET")
		set(mode ${MODE_NAME})
	endif()
	
	set(current_source)
	
	set(ico_sizes)
	set(icns_sizes)
	set(iconset_sizes)
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
		elseif(arg STREQUAL "ICO")
			_add_icon_source()
			set(mode ${MODE_ICO})
		elseif(arg STREQUAL "ICNS")
			_add_icon_source()
			set(mode ${MODE_ICNS})
		elseif(arg STREQUAL "ICONSET")
			_add_icon_source()
			set(mode ${MODE_ICONSET})
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
		elseif(mode EQUAL MODE_ICO)
			list(APPEND ico_sizes ${arg})
		elseif(mode EQUAL MODE_ICNS)
			list(APPEND icns_sizes ${arg})
		elseif(mode EQUAL MODE_ICONSET)
			list(APPEND iconset_sizes ${arg})
		elseif(mode EQUAL MODE_PNG)
			list(APPEND png_sizes ${arg})
		elseif(mode EQUAL MODE_OVERVIEW)
			list(APPEND overview_sizes ${arg})
		else()
			message(FATAL_ERROR "Unexpected icon argument: ${arg}")
		endif()
		
	endforeach()
	
	_add_icon_source()
	
	# Clean output variables
	
	foreach(size IN LISTS source_sizes ico_sizes icns_sizes iconset_sizes ico_sizes
	                      png_sizes overview_sizes)
		_remove_icon_bits(size ${size})
		unset(${name}-${size}.png CACHE)
	endforeach()
	if(ico_sizes)
		unset(${name}.ico CACHE)
		unset(${target}.rc CACHE)
	endif()
	if(icns_sizes)
		unset(${name}.icns CACHE)
	endif()
	if(iconset_sizes)
		unset(${name}-iconset CACHE)
	endif()
	if(png_sizes)
		unset(${name}.png CACHE)
		unset(${name}-png CACHE)
	endif()
	if(overview_sizes)
		unset(${target}-overview ${overview_file} CACHE)
	endif()
	
	# Generate icons
	
	set(generated_sizes)
	
	if(NOT ico_sizes AND NOT icns_sizes AND NOT iconset_sizes AND NOT overview_sizes)
		if(NOT png_sizes)
			set(png_sizes ${source_sizes})
		endif()
		set(all_types 1)
	endif()
	if((all_types AND NOT ICON_TYPE STREQUAL "none") OR ICON_TYPE STREQUAL "all")
		set(ICON_TYPE "ico" "icns" "iconset" "png" "overview")
	endif()
	
	set(outputs)
	
	foreach(type IN LISTS ICON_TYPE)
		
		if(type STREQUAL "ico")
			
			# Windows icon
			
			if(ico_sizes)
				
				_find_icon(ico_file "${name}.ico")
				
				if(NOT ico_file)
					
					find_package(ImageMagick COMPONENTS convert REQUIRED)
					
					set(ico_file "${ICON_BINARY_DIR}/${name}.ico")
					set(ico_command "${ImageMagick_convert_EXECUTABLE}" ${ImageMagick_OPTIONS})
					set(ico_depends "${ImageMagick_convert_EXECUTABLE}")
					
					foreach(size IN LISTS ico_sizes)
						
						_add_icon_size(source ${size})
						if(source)
							
							_get_icon_bits(bits ${size})
							if(bits EQUAL 32)
								list(APPEND ico_command "${source}")
							else()
								math(EXPR colors "2 << ${bits}")
								list(APPEND ico_command
									"("
										"${source}"
										-channel alpha
										-threshold "50%"
										+channel
										-colors "${colors}"
									")"
								)
							endif()
							
							list(APPEND ico_depends "${source}")
							
						endif()
						
					endforeach()
					
					list(APPEND ico_command "${ico_file}")
					
					add_custom_command(
						OUTPUT "${ico_file}"
						COMMAND "${CMAKE_COMMAND}" -E make_directory "${ICON_BINARY_DIR}"
						COMMAND ${ico_command}
						DEPENDS ${ico_depends}
						COMMENT "Generating ${name}.ico"
						VERBATIM
					)
					
				endif()
				
				# No need to install the .ico as it is typically linked into the executable
				# Provide a resource script to do just that
				set(rc_file "${CMAKE_CURRENT_BINARY_DIR}/${target}.rc")
				add_custom_command(
					OUTPUT "${rc_file}"
					COMMAND ${CMAKE_COMMAND}
						"-Dname=${name}" "-Dicon=${ico_file}"
						"-Dtemplate=${ICON_RESOURCE_TEMPLATE}" "-Doutput=${rc_file}"
						-P "${ICON_RESOURCE_SCRIPT}"
					DEPENDS "${ICON_RESOURCE_TEMPLATE}" "${ICON_RESOURCE_SCRIPT}"
					MAIN_DEPENDENCY ${ico_file}
					COMMENT ""
					VERBATIM
				)
				
				list(APPEND outputs ${ico_file} ${rc_file})
				
				set(${name}.ico ${ico_file} CACHE INTERNAL "")
				set(${target}.rc ${rc_file} CACHE INTERNAL "")
				
			endif()
			
		elseif(type STREQUAL "icns")
			
			# Mac OS X icon
			
			if(icns_sizes)
				
				_find_icon(icns_file "${name}.icns")
				
				if(NOT icns_file AND icns_sizes)
					
					find_package(iconutil REQUIRED)
					
					set(icns_file "${ICON_BINARY_DIR}/${name}.icns")
					set(iconset_dir "${CMAKE_CURRENT_BINARY_DIR}/${name}.iconset")
					set(iconset_commands)
					set(icns_depends "${iconutil_EXECUTABLE}" )
					
					foreach(size IN LISTS icns_sizes)
						
						_add_icon_size(source ${size})
						if(source)
							
							_get_icon_size(ssize ${size})
							list(APPEND iconset_commands
								COMMAND "${CMAKE_COMMAND}" -E copy
									"${source}" "${iconset_dir}/icon_${ssize}x${size}.png"
							)
							
							list(APPEND icns_depends "${source}")
							
						endif()
						
					endforeach()
					
					list(APPEND icns_command "${icns_file}")
					
					add_custom_command(
						OUTPUT "${icns_file}"
						COMMAND "${CMAKE_COMMAND}" -E remove_directory "${iconset_dir}"
						COMMAND "${CMAKE_COMMAND}" -E make_directory "${iconset_dir}"
						${iconset_commands}
						COMMAND "${CMAKE_COMMAND}" -E make_directory "${ICON_BINARY_DIR}"
						COMMAND "${iconutil_EXECUTABLE}" ${iconutil_OPTIONS}
							-c icns -o "${icns_file}" "${iconset_dir}"
						DEPENDS ${icns_depends}
						COMMENT "Generating ${name}.icns"
						VERBATIM
					)
					
				endif()
				
				if(install)
					install(
						FILES ${icns_file}
						DESTINATION "${ICONDIR}"
						COMPONENT icons
						OPTIONAL
					)
				endif()
				
				list(APPEND outputs ${icns_file})
				
				set(${name}.icns ${icns_file} CACHE INTERNAL "")
				
			endif()
			
		elseif(type STREQUAL "iconset")
			
			# Linux icon
			# https://specifications.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
			
			if(iconset_sizes)
				
				set(iconset_files)
				
				foreach(size IN LISTS iconset_sizes)
					
					_add_icon_size(source ${size})
					if(source)
						
						list(APPEND iconset_files ${source})
						
						if(install)
							_get_icon_size(ssize ${size})
							install(
								FILES ${source}
								DESTINATION "${ICONTHEMEDIR}/${ssize}x${size}/apps"
								COMPONENT icons
								RENAME "${name}.png"
								OPTIONAL
							)
						endif()
						
					endif()
					
				endforeach()
				
				list(APPEND outputs ${iconset_files})
				
				set(${name}-iconset ${iconset_files} CACHE INTERNAL "")
				
			endif()
			
		elseif(type STREQUAL "png")
			
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
	
	if(spec MATCHES "^([0-9]+)(\\@[0-9]+(bit|x))?$")
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
	elseif(spec MATCHES "^([0-9]+)(\\@[0-9]+bit)?$")
		set(${var} ${CMAKE_MATCH_1} PARENT_SCOPE)
	else()
		message(FATAL_ERROR "Unknown icon size spec: ${spec}")
	endif()
	
endfunction()

# Get the bit depth from an icon size spec
#
# Params:
#  var   Variable to receive the bit depth
#  spec  Icon size spec
function(_get_icon_bits var spec)
	
	if(spec MATCHES "^([0-9]+)\\@([0-9])+bit$")
		set(${var} ${CMAKE_MATCH_2} PARENT_SCOPE)
	elseif(spec MATCHES "^([0-9]+)(\\@[0-9]+x)?$")
		set(${var} 32 PARENT_SCOPE)
	else()
		message(FATAL_ERROR "Unknown icon size spec: ${spec}")
	endif()
	
endfunction()

# Remove bit depth iformation from an icon size spec
#
# Params:
#  var   Variable to receive the modified size spec
#  spec  Icon size spec
function(_remove_icon_bits var spec)
	
	if(spec MATCHES "^([0-9]+)(\\@[0-9]+bit)$")
		set(${var} ${CMAKE_MATCH_1} PARENT_SCOPE)
	elseif(spec MATCHES "^([0-9]+)(\\@([0-9]+)x)?$")
		set(${var} ${spec} PARENT_SCOPE)
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
	
	_remove_icon_bits(size ${size})
	
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
