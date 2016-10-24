
include(Icons)

set(ICON_BINARY_DIR "${PROJECT_BINARY_DIR}/data.generated/icons")
set(ICON_SOURCE_DIRS "${PROJECT_SOURCE_DIR}/data/icons")
set(data_sources)
foreach(dir IN LISTS DATA_FILES)
	get_filename_component(abs_dir "${dir}" ABSOLUTE)
	list(APPEND data_sources "${abs_dir}")
	if(NOT IS_DIRECTORY "${abs_dir}")
		message(WARNING "DATA_FILES points to non-existent directory '${abs_dir}'")
	endif()
endforeach()
list(APPEND data_sources "${PROJECT_BINARY_DIR}/arx-libertatis-data-${DATA_VERSION}")
list(APPEND data_sources "${PROJECT_BINARY_DIR}/arx-libertatis-data")
list(REMOVE_DUPLICATES data_sources)
foreach(dir IN LISTS data_sources)
	if(IS_DIRECTORY "${dir}")
		if(EXISTS "${dir}/VERSION")
			parse_version_file(FOUND_DATA_VERSION "${dir}/VERSION")
			if(NOT FOUND_DATA_VERSION_NUMBER EQUAL DATA_VERSION)
				message(WARNING "Found data version ${FOUND_DATA_VERSION_NUMBER} in ${dir},"
				                 " but we need version ${DATA_VERSION}")
			endif()
		else()
			message(WARNING "Missing VERSION file in data dir ${dir}")
		endif()
		message("Using pre-built data from ${dir}")
		if(IS_DIRECTORY "${dir}/icons")
			list(APPEND ICON_SOURCE_DIRS "${dir}/icons")
		endif()
	endif()
endforeach()

# Sizes in the Windows .ico file - sorted smaller → larger, high bits → low bits
set(ico_sizes
	16
	16@8bit
	16@4bit
	20
	24
	24@8bit
	24@4bit
	32
	32@8bit
	32@4bit
	40
	48
	48@8bit
	64
	128
	256
)

# Sizes in the Mac OS X .icns file
set(icns_sizes
	16
	16@2x
	32
	32@2x
	128
	128@2x
	256
	256@2x
	512
	512@2x
)

# Sizes in the Linux themable icon set
set(iconset_sizes
	16
	22
	24
	32
	48
	64
	96
	128
	192
	256
	512
)

# Sizes for the portable PNG icon - first is the default size
set(png_sizes
	128
	32
	24
	22
	16
)

# Montage for comparing different icon sizes
set(overview_sizes
	16
	20
	22
	24
	32
	40
	48
	64
	96
	128
	192
	256
	512
)

add_icon(arx-libertatis
	arx-libertatis.svg    1024
	arx-libertatis-32.svg 32@2x 512
	arx-libertatis-16.svg 16@2x
	ICO ${ico_sizes}
	ICNS ${icns_sizes}
	ICONSET ${iconset_sizes}
	PNG ${png_sizes}
	OVERVIEW ${overview_sizes}
	INSTALL
)

add_icon(TARGET arx-libertatis-logo arx-libertatis-logo.svg 58 512)

add_custom_target(data DEPENDS
	arx-libertatis-icon
	arx-libertatis-logo
)
