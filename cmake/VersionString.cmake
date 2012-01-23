
# Create a rule to generate a version string at compile time.
#
# SRC is processed using the configure_file() cmake command
# at build to produce DST with the following variable available:
#
# - BASE_VERSION: The contents of the file specified by VERSION_FILE.
# - GIT_COMMIT: The current git commit. This variable is not defined if there is no GIT_DIR directory.
# - SHORT_GIT_COMMIT: The first 10 characters of the git commit.
# For the exact syntax of SRC see the documentation of the configure_file() cmake command.
#
# The version file is regenerated whenever VERSION_FILE or the current commit changes.
function(version_file SRC DST VERSION_FILE GIT_DIR)
	
	get_filename_component(ABS_SRC "${SRC}" ABSOLUTE)
	get_filename_component(ABS_DST "${DST}" ABSOLUTE)
	get_filename_component(ABS_VERSION_FILE "${VERSION_FILE}" ABSOLUTE)
	get_filename_component(ABS_GIT_DIR "${GIT_DIR}" ABSOLUTE)
	
	set(dependencies "${VERSION_FILE}" "${CMAKE_MODULE_PATH}/VersionScript.cmake")
	
	if(EXISTS "${GIT_DIR}/HEAD")
		list(APPEND dependencies "${GIT_DIR}/HEAD")
	endif()
	
	if(EXISTS "${GIT_DIR}/logs/HEAD")
		list(APPEND dependencies "${GIT_DIR}/logs/HEAD")
	endif()
	
	add_custom_command(
		OUTPUT
			"${DST}"
		COMMAND
			${CMAKE_COMMAND}
			"-DINPUT=${ABS_SRC}"
			"-DOUTPUT=${ABS_DST}"
			"-DVERSION_FILE=${ABS_VERSION_FILE}"
			"-DGIT_DIR=${ABS_GIT_DIR}"
			-P "${CMAKE_MODULE_PATH}/VersionScript.cmake"
		MAIN_DEPENDENCY
			"${SRC}"
		DEPENDS
			${dependencies}
		COMMENT ""
		VERBATIM
	)
	
endfunction(version_file)
