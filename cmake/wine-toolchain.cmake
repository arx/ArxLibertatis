
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux-wine)

# Look for wine compilers
find_program(Wine_GCC winegcc)
mark_as_advanced(Wine_GCC)
find_program(Wine_GXX wineg++)
mark_as_advanced(Wine_GXX)

if((NOT Wine_GCC) OR (NOT Wine_GXX))
	message(FATAL_ERROR "wine compiler not found (found gcc=\"${Wine_GCC}\", g++=\"${Wine_GXX}\")")
endif()

# which compilers to use for C and C++
set(CMAKE_C_COMPILER "${Wine_GCC}")
set(CMAKE_CXX_COMPILER "${Wine_GXX}")

set(CMAKE_EXECUTABLE_SUFFIX ".exe")
