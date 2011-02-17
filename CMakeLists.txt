cmake_minimum_required(VERSION 2.8)
project(ArxFatalis)

set(INC_DIR include)
set(SRC_DIR src)

include_directories(
	include
IF(UNIX)
	#include wine
	/usr/include/wine/windows
ENDIF()
)

IF (MSVC)
	# Set optimal options for msvc:
	
	# Disable deprecation warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
	
	# Source groups
	set(SOURCE_GROUPS athena danae debugger eerie hermes minos)
	foreach(SOURCE_GROUP ${SOURCE_GROUPS})
		file(GLOB_RECURSE SOURCES ${SRC_DIR}/${SOURCE_GROUP}/*.cpp)
		SOURCE_GROUP("src\\\\${SOURCE_GROUP}" FILES ${SOURCES})
	endforeach(SOURCE_GROUP)
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
add_definitions(-DNOMINMAX)			# Prevent <windows.h> from defining min/max macros that can clash with std::min/std::max

#glob all sources and includes
file(GLOB_RECURSE ALL_INCLUDES ${INC_DIR}/*.h)
SOURCE_GROUP("inc" FILES ${ALL_INCLUDES})

file(GLOB_RECURSE ALL_SOURCES ${SRC_DIR}/*.cpp)

#build - WIN32 is only here to specify that on Windows, the app is not a console application, but a windows app (with WinMain)
add_executable(arx WIN32 ${ALL_SOURCES} ${ALL_INCLUDES})

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

add_subdirectory(tools)


