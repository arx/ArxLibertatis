
set(VERSION_STRING_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/VersionScript.cmake")

# Create a rule to generate a version string at compile time.
#
# An optional fifth argument can be used to add additional cmake defines.
#
# SRC is processed using the configure_file() cmake command
# at build to produce DST with the following variable available:
#
# VERSION_SOURCES:
#  List of (${var} ${file}) pairs.
#
# for each variable ${var}
# - ${var}: The contents of the associated file
# - ${var}_COUNT: Number of lines in the associated file
# - ${var}_${i}: The ${i}-th line of the associated file
# - ${var}_${i}_SHORTNAME: The first component of the ${i}-th line of the associated file
# - ${var}_${i}_STRING: Everything except the first component of the ${i}-th line of the associated file
# - ${var}_${i}_NAME: Everything except the last component of the ${i}-th line of the associated file
# - ${var}_${i}_NUMBER: The last component of the ${i}-th line of the associated file
# - ${var}_HEAD: The first paragraph of the associated file
# - ${var}_TAIL: The remaining paragraphs of the associated file
#
# - GIT_COMMIT: The current git commit. (not defined if there is no GIT_DIR directory)
# - GIT_COMMIT_PREFIX_${i}: The first ${i} characters of GIT_COMMIT (i=0..39)
# For the exact syntax of SRC see the documentation of the configure_file() cmake command.
# The version file is regenerated whenever VERSION_FILE or the current commit changes.
function(version_file SRC DST VERSION_SOURCES GIT_DIR)
	
	set(mode "variable")
	
	set(args)
	set(dependencies "${VERSION_STRING_SCRIPT}")
	
	foreach(arg IN LISTS VERSION_SOURCES)
		
		if(mode STREQUAL "variable")
			set(mode "file")
		else()
			get_filename_component(arg "${arg}" ABSOLUTE)
			list(APPEND dependencies ${arg})
			set(mode "variable")
		endif()
		
		list(APPEND args ${arg})
		
	endforeach()
	
	get_filename_component(abs_src "${SRC}" ABSOLUTE)
	get_filename_component(abs_dst "${DST}" ABSOLUTE)
	get_filename_component(abs_git_dir "${GIT_DIR}" ABSOLUTE)
	
	set(defines)
	if(${ARGC} GREATER 4)
		set(defines ${ARGV4})
	endif()
	
	if(EXISTS "${abs_git_dir}")
		find_program(GIT_COMMAND git)
		if(GIT_COMMAND)
			list(APPEND dependencies "${GIT_COMMAND}")
		endif()
		if(EXISTS "${abs_git_dir}/HEAD")
			list(APPEND dependencies "${abs_git_dir}/HEAD")
		endif()
		if(EXISTS "${abs_git_dir}/packed-refs")
			list(APPEND dependencies "${abs_git_dir}/packed-refs")
		endif()
		if(EXISTS "${abs_git_dir}/logs/HEAD")
			list(APPEND dependencies "${abs_git_dir}/logs/HEAD")
		endif()
	endif()
	
	add_custom_command(
		OUTPUT
			"${abs_dst}"
		COMMAND
			${CMAKE_COMMAND}
			"-DINPUT=${abs_src}"
			"-DOUTPUT=${abs_dst}"
			"-DVERSION_SOURCES=${args}"
			"-DGIT_DIR=${abs_git_dir}"
			"-DGIT_COMMAND=${GIT_COMMAND}"
			${defines}
			-P "${VERSION_STRING_SCRIPT}"
		MAIN_DEPENDENCY
			"${abs_src}"
		DEPENDS
			${dependencies}
		COMMENT ""
		VERBATIM
	)
	
endfunction(version_file)
