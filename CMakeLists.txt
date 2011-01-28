cmake_minimum_required(VERSION 2.8)
project(ArxFatalis)

set(INC_DIR include)
set(SRC_DIR src)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

option(ARX_BUILD_TESTS ON)
option(ARX_BUILD_TOOLS ON)

include_directories(
	src
IF(UNIX)
	#include wine
	/usr/include/wine/windows
ENDIF()
)

IF (MSVC)
	# Set optimal options for msvc:
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	# Source groups
	set(SOURCE_GROUPS
		    athena danae debugger eerie hermes minos
	)

	foreach(SOURCE_GROUP ${SOURCE_GROUPS})
		file(GLOB_RECURSE SOURCES ${SRC_DIR}/${SOURCE_GROUP}/*.cpp)
		SOURCE_GROUP("src\\\\${SOURCE_GROUP}" FILES ${SOURCES})
	endforeach(SOURCE_GROUP)
	#glob includes
	file(GLOB_RECURSE ALL_INCLUDES ${INC_DIR}/*.h)
	SOURCE_GROUP("inc" FILES ${ALL_INCLUDES})
ELSE (MSVC)
	# Set optimal options for gcc:
	#wine compiler
	set(CMAKE_CXX_COMPILER /usr/bin/wineg++)
	set(CMAKE_C_COMPILER /usr/bin/winegcc)

	#set 32bit
	set(CMAKE_CXX_FLAGS "-m32")

	#Debug
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
ENDIF (MSVC)

# Preprocessor definitions
add_definitions(-D_DEBUG)

#glob sources
file(GLOB_RECURSE ALL_SOURCES ${SRC_DIR}/*.cpp)

#build
add_executable(arx ${ALL_SOURCES} ${ALL_INCLUDES})

#link
target_link_libraries(arx dsound dxguid gdi32 shell32 dinput comdlg32 ole32 comctl32 ddraw jpeg z)

add_custom_target(remake
		#clean and compile with 1 thread per core
        COMMAND make clean && rm CMakeCache.txt && cmake . -G\"Unix Makefiles\" && make -j`getconf _NPROCESSORS_ONLN`
)

add_custom_target(doc
		#build the documentation
        COMMAND doxygen doc/Doxyfile
)

#find python2
IF(EXISTS "/usr/bin/python2")
	SET(PYTHON_BIN "python2")
ELSE()
	SET(PYTHON_BIN "python")
ENDIF()
SET(STYLE_FILTER -readability/casting,-build/header_guard,-build/include,-readability/streams,-whitespace,-runtime/int)
add_custom_target(style
        COMMAND ${PYTHON_BIN} scripts/cpplint.py --filter=${STYLE_FILTER} ${ALL_SOURCES} ${ALL_INCLUDES}
)

if(ARX_BUILD_TESTS)
	add_subdirectory(tests ${CMAKE_SOURCE_DIR}/bin/tests)
endif()

if(ARX_BUILD_TOOLS)
	add_subdirectory(tools ${CMAKE_SOURCE_DIR}/bin/tools)
endif()
