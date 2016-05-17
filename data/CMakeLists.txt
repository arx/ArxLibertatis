
include(Icons)

set(ICON_BINARY_DIR "${CMAKE_BINARY_DIR}/data.generated/icons")
set(ICON_SOURCE_DIRS "${CMAKE_SOURCE_DIR}/data/icons")
set(data_sources)
foreach(dir IN LISTS DATA_FILES)
	list(APPEND data_sources "${dir}")
	if(NOT IS_DIRECTORY "${dir}")
		message(WARNING "DATA_FILES points to non-existent directory '${dir}'")
	endif()
endforeach()
list(APPEND data_sources "${CMAKE_BINARY_DIR}/arx-libertatis-data-${DATA_VERSION}")
list(APPEND data_sources "${CMAKE_BINARY_DIR}/arx-libertatis-data")
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

# Sizes for the portable PNG icon - first is the default size
set(png_sizes
	128
	32
	24
	22
	16
)

add_icon(arx-libertatis
	arx-libertatis.svg    1024
	arx-libertatis-32.svg 32@2x 512
	arx-libertatis-16.svg 16@2x
	PNG ${png_sizes}
)

add_custom_target(data DEPENDS
	arx-libertatis-icon
)
