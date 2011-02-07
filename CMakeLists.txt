cmake_minimum_required(VERSION 2.8)
project(ArxFatalis)

option(ARX_BUILD_TESTS ON)
option(ARX_BUILD_TOOLS ON)
option(ARX_MINGW OFF)

set(SRC_DIR src)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

include_directories(${SRC_DIR})

IF (MSVC)
	# Set optimal options for msvc:
	
	# Disable deprecation warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	add_definitions(/wd4995)
	
	# TEMP - disable warning caused by the F2L removal
	add_definitions(/wd4244)    # conversion from 'float' to 'long', possible loss of data
	
	# Source groups
	set(SOURCE_GROUPS
		ai animation audio core game graphics gui io physics scene scripting window
	)

	foreach(SOURCE_GROUP ${SOURCE_GROUPS})
		file(GLOB_RECURSE SOURCES ${SRC_DIR}/${SOURCE_GROUP}/*.*)
		SOURCE_GROUP("${SOURCE_GROUP}" FILES ${SOURCES})
	endforeach(SOURCE_GROUP)
	#glob includes
	file(GLOB_RECURSE ALL_INCLUDES ${SRC_DIR}/*.h)
	SOURCE_GROUP("inc" FILES ${ALL_INCLUDES})
ELSE (MSVC)
	# Set optimal options for gcc:
	#wine compiler
	set(CMAKE_CXX_COMPILER wineg++)
	set(CMAKE_C_COMPILER winegcc)

	#set 32bit
	set(CMAKE_CXX_FLAGS "-m32")

	#Debug
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
	
	#use posix threads
	add_definitions(-DHAVE_PTHREADS)
	set(THREAD_LIBS pthread rt)
	
ENDIF (MSVC)

if(ARX_MINGW)
	set(CMAKE_CXX_COMPILER i486-mingw32-g++)
	set(CMAKE_C_COMPILER i486-mingw32-gcc)
endif()

# Preprocessor definitions
add_definitions(-D_DEBUG)
add_definitions(-DNOMINMAX)

#glob sources
file(GLOB_RECURSE ALL_SOURCES ${SRC_DIR}/*.cpp)

#build
add_executable(arx ${ALL_SOURCES} ${ALL_INCLUDES})

#link
target_link_libraries(arx dsound dxguid gdi32 shell32 dinput comdlg32 ole32 comctl32 ddraw jpeg z sfml-system ${THREAD_LIBS})

add_custom_target(remake
	#clean and compile with 1 thread per core
	COMMAND make clean && rm CMakeCache.txt && cmake ${CMAKE_SOURCE_DIR} -G\"Unix Makefiles\" && make -j`getconf _NPROCESSORS_ONLN`
)

add_custom_target(doc
	#build the documentation
	COMMAND cd ${CMAKE_SOURCE_DIR} && doxygen doc/Doxyfile
)

#find python2
IF(EXISTS "/usr/bin/python2")
	SET(PYTHON_BIN "python2")
ELSE()
	SET(PYTHON_BIN "python")
ENDIF()
SET(STYLE_FILTER -readability/casting,-build/header_guard,-build/include,-readability/streams,-whitespace,-runtime/int)
add_custom_target(style
	COMMAND ${PYTHON_BIN} ${CMAKE_SOURCE_DIR}/scripts/cpplint.py --filter=${STYLE_FILTER} ${ALL_SOURCES} ${ALL_INCLUDES}
)

if(ARX_BUILD_TESTS)
	add_subdirectory(tests ${CMAKE_SOURCE_DIR}/bin/tests)
endif()

if(ARX_BUILD_TOOLS)
	add_subdirectory(tools ${CMAKE_SOURCE_DIR}/bin/tools)
endif()
