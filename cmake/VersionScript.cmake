
cmake_minimum_required(VERSION 2.8)

# CMake script that reads a VERSION file and the current git history and the calls configure_file().
# This is used by version_file() in VersionString.cmake

if((NOT DEFINED INPUT) OR (NOT DEFINED OUTPUT) OR (NOT DEFINED VERSION_SOURCES) OR (NOT DEFINED GIT_DIR))
	message(SEND_ERROR "Invalid arguments.")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/VersionString.cmake")

# configure_file doesn't handle newlines correctly - pre-escape variables
macro(_version_escape var string)
	# Escape the escape character and quotes
	string(REGEX REPLACE "([\\\\\"])" "\\\\\\1" ${var} "${string}")
	# Pull newlines out of string
	string(REGEX REPLACE "\n" "\\\\n\"\n\t\"" ${var} "${${var}}")
endmacro()

set(var "")
foreach(arg IN LISTS VERSION_SOURCES)
	if(var STREQUAL "")
		set(var ${arg})
	else()
		parse_version_file(${var} "${arg}" ON)
		set(var "")
	endif()
endforeach()

# Check for a git directory and fill in the git commit hash if one exists.
unset(GIT_COMMIT)
if(NOT GIT_DIR STREQUAL "")
	
	unset(git_head)
	
	if(GIT_COMMAND)
		execute_process(
			COMMAND "${GIT_COMMAND}" "--git-dir=${GIT_DIR}" "rev-parse" "HEAD"
			RESULT_VARIABLE result
			OUTPUT_VARIABLE git_head
		)
		if(NOT "${result}" EQUAL 0)
			unset(git_head)
		endif()
	endif()
	
	if(NOT git_head AND EXISTS "${GIT_DIR}/HEAD")
		
		file(READ "${GIT_DIR}/HEAD" git_head)
		
		if(git_head MATCHES "^[ \t\r\n]*ref\\:(.*)$")
			
			# Remove the first for characters from git_head to get git_ref.
			# We can't use a length of -1 for string(SUBSTRING) as cmake < 2.8.5 doesn't support it.
			string(LENGTH "${git_head}" git_head_length)
			math(EXPR git_ref_length "${git_head_length} - 4")
			string(SUBSTRING "${git_head}" 4 ${git_ref_length} git_ref)
			string(STRIP "${git_ref}" git_ref)
			
			unset(git_head)
			if(EXISTS "${GIT_DIR}/${git_ref}")
				file(READ "${GIT_DIR}/${git_ref}" git_head)
			elseif(EXISTS "${GIT_DIR}/packed-refs")
				file(READ "${GIT_DIR}/packed-refs" git_refs)
				string(REGEX REPLACE "[^0-9A-Za-z]" "\\\\\\0" git_ref "${git_ref}")
				string(REGEX MATCH "[^\r\n]* ${git_ref}( [^\r\n])?" git_head "${git_refs}")
			endif()
			
		endif()
		
	endif()
	
	# Create variables for all prefixes of the git comit ID.
	string(REGEX MATCH "[0-9A-Za-z]+" git_commit "${git_head}")
	string(LENGTH "${git_commit}" git_commit_length)
	if(NOT git_commit_length LESS 40)
		string(TOLOWER "${git_commit}" GIT_COMMIT)
		foreach(i RANGE 20)
			string(SUBSTRING "${GIT_COMMIT}" 0 ${i} GIT_COMMIT_PREFIX_${i})
			set(GIT_SUFFIX_${i} " + ${GIT_COMMIT_PREFIX_${i}}")
		endforeach()
	else()
		message(WARNING "Git repository detected, but could not determine HEAD")
	endif()
	
endif()

configure_file("${INPUT}" "${OUTPUT}")
