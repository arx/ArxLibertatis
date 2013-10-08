
# Helper function to create and install an application icon file
#
# Usage: add_icon(name src-path src-sizes...)
#
#  name       Output icon name. The generated icon is placed under the ICONDIR directory
#  src-path   Prefix for input icon files, relative to source directory
#  src-sizes  One or more sizes in which the source icon is available:
#              [size]      [source image]
#              scalable    src-path.svg
#              <d>         src-path_<d>x<d>.png      where <d> is an integer
#              <d>@2x      src-path_<d>x<d>@2x.png   where <d> is an integer
#
# Images are automatically rasterized, scaled and bundled into platform-specific
# icon formats as needed.
#
# Supported icon types:
#  "ico"   (default on Windows)
#  "icns"  (default on Mac OS X)
#  "fdo"   (freedesktop.org icon tree: <d>x<d>/apps/<icon>.png, default everywhere else)
#  "first" (use the first matching size in ICON_SIZES, ie. largest icon only)
#  "none"  (don't install any icons)
# The icon type can be set using the ICON_TYPE variable.
#
# For all icon types except "icns", the generated sizes can be set using the ICON_SIZES
# list variable. By default an icon type-specific set of icon sizes is used.
# Valid sizes:
#   scalable     Install source .svg icons - cannot be used with the "ico" icon type
#   <d>          Install icon with size <d>x<d>, resize a larger icon if needed
#   <d>=         Install icon with size <d>x<d>, but only if in the source set
# Icons are never upscaled and sizes in ICON_SIZES larger than the source are ignored.
#


if(WIN32)
	set(DEFAULT_ICON_TYPE "ico")
elseif(MACOSX OR CREATE_MAC_APP_BUNDLE)
	set(DEFAULT_ICON_TYPE "icns")
else()
	set(DEFAULT_ICON_TYPE "fdo")
endif()
set(ICON_TYPE "${DEFAULT_ICON_TYPE}" CACHE STRING "Icon type to create")
set(ICON_SIZES "" CACHE STRING "Icon sizes to create")

function(icon_file PREFIX SIZE VAR)
	string(REGEX REPLACE "^${scalable}$" ".svg" outfile "${SIZE}")
	string(REGEX REPLACE "^([0-9]+)((\\@2x)?)$" "_\\1x\\1\\2.png" outfile "${outfile}")
	get_filename_component(outfile "${PREFIX}${outfile}" ABSOLUTE)
	set(${VAR} "${outfile}" PARENT_SCOPE)
endfunction(icon_file)

function(add_icon NAME SRC)
	
	set(insizes)
	math(EXPR end "${ARGC} - 1")
	foreach(i RANGE 2 ${end})
		set(size "${ARGV${i}}")
		if("${size}" MATCHES "^[0-9]+=?$")
			list(APPEND insizes ${size})
		elseif("${size}" STREQUAL "scalable")
			list(APPEND insizes ${scalable})
		else()
			message(FATAL_ERROR "Bad icon size: ${size}")
		endif()
	endforeach()
	
	set(prefix "${CMAKE_BINARY_DIR}/${NAME}")
	
	set(scalable 9999)
	
	set(copy 0)
	
	if("${ICON_TYPE}" STREQUAL "ico")
		
		if(NOT USE_IMAGEMAGICK)
			return()
		endif()
		find_package(ImageMagick COMPONENTS convert ${OPTIONAL_DEPENDENCY})
		if(NOT ImageMagick_convert_FOUND)
			return()
		endif()
		
		set(output "${prefix}.ico")
		set(outsizes 1024= 512= 256 128= 64= 48 32 24= 16 8=)
		
	elseif("${ICON_TYPE}" STREQUAL "icns")
		
		if(NOT USE_ICNSUTIL AND NOT USE_ICONUTIL AND NOT USE_PNG2ICNS)
			return()
		endif()
		unset(ICONUTIL CACHE)
		unset(PNG2ICNS CACHE)
		if(USE_ICNSUTIL)
			find_program(ICONUTIL icnsutil)
		endif()
		if(NOT ICONUTIL AND USE_ICONUTIL)
			find_program(ICONUTIL iconutil)
		endif()
		if(NOT ICONUTIL AND USE_PNG2ICNS)
			find_program(PNG2ICNS png2icns)
		endif()
		if(NOT ICONUTIL AND NOT PNG2ICNS)
			if(STRICT_USE)
				message(FATAL_ERROR "Missing tool to create Mac .icns icons")
			endif()
			return()
		endif()
		
		set(output "${prefix}.icns")
		if(PNG2ICNS)
			set(outsizes 1024 512 256 128 32 16)
		else()
			get_filename_component(iconset "${prefix}.iconset" ABSOLUTE)
			set(prefix "${iconset}/icon")
			set(copy 1)
			set(outsizes 512@2x 512 256@2x 256 128@2x 128 32@2x 32 16@2x 16)
			make_directory("${iconset}")
		endif()
		
	elseif("${ICON_TYPE}" STREQUAL "fdo")
		
		unset(output)
		set(outsizes ${scalable} 1024= 512= 256= 192= 128= 96= 80= 72= 64= 48 42= 36= 32= 24= 22= 16= 8=)
		
	elseif("${ICON_TYPE}" STREQUAL "first")
		
		unset(output)
		set(outsizes ${scalable} 1024= 512= 256= 192= 128= 96= 80= 72= 64= 48= 42= 36= 32= 24= 22= 16= 8=)
		
	elseif("${ICON_TYPE}" STREQUAL "none")
		
		return()
		
	else()
		
		message(FATAL_ERROR "Uknown icon type: ${ICON_TYPE}")
		
	endif()
	
	if(NOT "${ICON_SIZES}" STREQUAL "" AND NOT "${ICON_TYPE}" STREQUAL "icns")
		set(outsizes)
		foreach(size IN LISTS ICON_SIZES)
			if("${size}" MATCHES "^[0-9]+=?$")
				list(APPEND outsizes ${size})
			elseif("${size}" STREQUAL "scalable" AND NOT "${ICON_TYPE}" STREQUAL "ico")
				list(APPEND outsizes ${scalable})
			else()
				message(FATAL_ERROR "Bad icon size: ${size}")
			endif()
		endforeach()
	endif()
	
	set(files)
	
	foreach(outres IN LISTS outsizes)
		
		# Parse output size specification
		# - size      required size
		# - size=     optional size (only used if in original input)
		# - size@2x   sub-pixel size (use size@2x from input if available, 2 * size otherwise)
		set(create 1)
		if("${outres}" MATCHES "\\=$")
			string(REGEX REPLACE "\\=$" "" outres "${outres}")
			set(outsize "${outres}")
			set(create 0)
		elseif("${outres}" MATCHES "\\@2x$")
			string(REGEX REPLACE "\\@2x$" "" outsize "${outres}")
			math(EXPR outsize "${outsize} * 2")
		else()
			set(outsize "${outres}")
		endif()
		
		# Find a matching size or the next largest
		unset(insize)
		foreach(size IN LISTS insizes)
			string(REGEX REPLACE "\\=$" "" res "${size}")
			if("${outres}" STREQUAL "${size}")
				set(insize "${size}")
				break()
			elseif(NOT ${res} LESS ${outsize})
				if(NOT DEFINED insize)
					set(insize ${size})
				elseif(${res} LESS ${insize})
					set(insize ${size})
				endif()
			endif()
		endforeach()
		
		# Check if we already created a file of the required size
		list(FIND outsizes "${outsize}" realoutsize)
		
		icon_file("${prefix}" "${outres}" outfile)
		
		if(NOT DEFINED insize)
			# input size is larger than output size
		else()
			
			unset(infile)
			if("${insize}" STREQUAL "${outsize}" OR "${insize}" STREQUAL "${outsize}")
				icon_file("${SRC}" "${insize}" infile)
			elseif(NOT "${outsize}" STREQUAL "${outres}" AND NOT ${realoutsize} EQUAL -1)
				icon_file("${prefix}" "${outsize}" infile)
			endif()
			
			if(DEFINED infile)
				
				# We already have a file with the exact required dimensions
				
				if(${copy})
					# Copy if we really need to
					add_custom_command(
						OUTPUT "${outfile}"
						COMMAND cmake -E copy "${infile}" "${outfile}"
						MAIN_DEPENDENCY "${infile}"
						COMMENT ""
						VERBATIM
					)
					list(APPEND files "${outfile}")
				else()
					# Otherwise use the input file directly
					list(APPEND files "${infile}")
				endif()
				
			elseif(${create} AND USE_IMAGEMAGICK)
				
				# We don't have a matching input, but the output is non-optional
				# Scale the next-largest input to fit
				
				icon_file("${SRC}" "${insize}" infile)
				find_package(ImageMagick COMPONENTS convert ${OPTIONAL_DEPENDENCY})
				if(ImageMagick_convert_FOUND)
					add_custom_command(
						OUTPUT "${outfile}"
						COMMAND "${ImageMagick_convert_EXECUTABLE}"
							"${infile}" -resize ${outsize}x${outsize} "${outfile}"
						MAIN_DEPENDENCY "${infile}"
						COMMENT "Scaling ${infile} to ${outsize}x${outsize}"
						VERBATIM
					)
					list(APPEND files "${outfile}")
				endif()
				
			else()
				
				# Skipping optional output size that is not available as input
				
			endif()
			
		endif()
		
	endforeach()
	
	list(LENGTH files count)
	if(${count} EQUAL 0)
		return()
	endif()
	
	# Combine and install icon files
	if("${ICON_TYPE}" STREQUAL "ico")
		
		add_custom_command(
			OUTPUT "${output}"
			COMMAND "${ImageMagick_convert_EXECUTABLE}" ${files} "${output}"
			DEPENDS ${files}
			COMMENT "Combining ${output}"
			VERBATIM
		)
		install(FILES "${output}" DESTINATION "${ICONDIR}" OPTIONAL)
		
	elseif("${ICON_TYPE}" STREQUAL "icns")
		
		if(PNG2ICNS)
			add_custom_command(
				OUTPUT "${output}"
				COMMAND "${PNG2ICNS}" "${output}" ${files}
				DEPENDS ${files}
				COMMENT "Combining ${output}"
				VERBATIM
			)
		else()
			add_custom_command(
				OUTPUT "${output}"
				COMMAND "${ICONUTIL}" -c icns -o "${output}" "${iconset}"
				DEPENDS ${files}
				COMMENT "Combining ${output}"
				VERBATIM
			)
		endif()
		install(FILES "${output}" DESTINATION "${ICONDIR}" OPTIONAL)
		
	elseif("${ICON_TYPE}" STREQUAL "fdo")
		
		foreach(file IN LISTS files)
			string(REGEX REPLACE "^.*\\.svg$" "scalable" size "${file}")
			string(REGEX REPLACE "^.*_([0-9]+x[0-9]+)\\.png$" "\\1" size "${size}")
			string(REGEX REPLACE "^.*(\\.[^.]+)$" "${NAME}\\1" filename "${file}")
			install(
				FILES "${file}"
				DESTINATION "${ICONDIR}/${size}/apps"
				RENAME "${filename}"
				OPTIONAL
			)
		endforeach()
		
	elseif("${ICON_TYPE}" STREQUAL "first")
		
		foreach(file IN LISTS files)
			string(REGEX REPLACE "^.*(\\.[^.]+)$" "${NAME}\\1" filename "${file}")
			install(FILES "${file}" DESTINATION "${ICONDIR}" RENAME "${filename}" OPTIONAL)
			set(output "${file}")
			unset(files)
			break()
		endforeach()
		
	endif()
	
	# Add icon to the default target
	add_custom_target(${NAME}-icon ALL DEPENDS ${output} ${files})
	
endfunction(add_icon)
