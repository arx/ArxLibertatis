
find_package(PythonInterp)

set(STYLE_FILTER)

# Complains about any c-style cast -> too annoying.
set(STYLE_FILTER ${STYLE_FILTER},-readability/casting)

# Insists on including evrything in the .cpp file even if it is included in the header.
# This behaviour conflicts with orther tools.
set(STYLE_FILTER ${STYLE_FILTER},-build/include_what_you_use)

# Too many false positives and not very helpful error messages.
set(STYLE_FILTER ${STYLE_FILTER},-build/include_order)

# No thanks.
set(STYLE_FILTER ${STYLE_FILTER},-readability/streams)

# Ugh!
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/tab)

# Yes it is!
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/blank_line)

# Suggessts excessive indentation.
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/labels)

# Why?
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/todo)
set(STYLE_FILTER ${STYLE_FILTER},-readability/todo)

# "For a static/global string constant, use a C style string"
set(STYLE_FILTER ${STYLE_FILTER},-runtime/string)

# Having classes with virtual methods but without a virtual destructor is not always wrong!
set(STYLE_FILTER ${STYLE_FILTER},-runtime/virtual)

# TODO consider enabling these and fixing the warnings

# Very noisy and perhaps a matter of taste.
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/braces)
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/parens)
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/newline)

# Complains about using short, long, etc.
set(STYLE_FILTER ${STYLE_FILTER},-runtime/int)

# Complains about non-const references as parameters
set(STYLE_FILTER ${STYLE_FILTER},-runtime/references)

# TODO enable these!

# Very noisy but should be fixed.
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/operators)
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/comma)
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/comments)
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/end_of_line)
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/align_tab)
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/line_length)
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/semicolon)
set(STYLE_FILTER ${STYLE_FILTER},-whitespace/ident_space)

# Unsafe functions.
set(STYLE_FILTER ${STYLE_FILTER},-runtime/printf)
set(STYLE_FILTER ${STYLE_FILTER},-runtime/threadsafe_fn)

# Very much known...
set(STYLE_FILTER ${STYLE_FILTER},-readability/fn_size)

# Has false positives and is already covered by -pedantic or -Wvla
set(STYLE_FILTER ${STYLE_FILTER},-runtime/arrays)

set(STYLE_CHECK_SCRIPT "${PROJECT_SOURCE_DIR}/scripts/cpplint.py")

# Add a target that runs cpplint.py
#
# Parameters:
# - TARGET_NAME the name of the target to add
# - SOURCES_LIST a complete list of source and include files to check
function(add_style_check_target TARGET_NAME SOURCES_LIST PROJECT)
	
	if(NOT PYTHONINTERP_FOUND)
		return()
	endif()
	
	list(SORT SOURCES_LIST)
	list(REMOVE_DUPLICATES SOURCES_LIST)
	
	add_custom_target(${TARGET_NAME}
		COMMAND "${CMAKE_COMMAND}" -E chdir
			"${PROJECT_SOURCE_DIR}"
			"${PYTHON_EXECUTABLE}"
			"${STYLE_CHECK_SCRIPT}"
			"--filter=${STYLE_FILTER}"
			"--project=${PROJECT}"
			${SOURCES_LIST}
		DEPENDS ${SOURCES_LIST} ${STYLE_CHECK_SCRIPT}
		COMMENT "Checking code style."
		VERBATIM
	)
	
endfunction(add_style_check_target)
