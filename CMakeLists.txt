cmake_minimum_required(VERSION 2.8)
project(ArxFatalis)

set(INC_DIR include)
set(SRC_DIR src)

include_directories(
	include
	/usr/include/wine/windows
)

IF (MSVC)
	# Set optimal options for msvc:
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ELSE (MSVC)
	# Set optimal options for gcc:
	#wine compiler
	set(CMAKE_CXX_COMPILER /usr/bin/wineg++)
	set(CMAKE_C_COMPILER /usr/bin/winegcc)

	#set temporally less gcc problems
	set(CMAKE_CXX_FLAGS "-m32 -w")

	#Debug
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
ENDIF (MSVC)

# Preprocessor definitions
add_definitions(-D_DEBUG)

# Source groups
file(GLOB_RECURSE SOURCES_ATHENA ${SRC_DIR}/athena/*.cpp)
SOURCE_GROUP("src\\\\athena" FILES ${SOURCES_ATHENA})

file(GLOB_RECURSE SOURCES_DANEA ${SRC_DIR}/danae/*.cpp)
SOURCE_GROUP("src\\\\danae" FILES ${SOURCES_DANEA})

file(GLOB_RECURSE SOURCES_DEBUGGER ${SRC_DIR}/debugger/*.cpp)
SOURCE_GROUP("src\\\\debugger" FILES ${SOURCES_DEBUGGER})

file(GLOB_RECURSE SOURCES_EERIE ${SRC_DIR}/eerie/*.cpp)
SOURCE_GROUP("src\\\\eerie" FILES ${SOURCES_EERIE})

file(GLOB_RECURSE SOURCES_HERMES ${SRC_DIR}/hermes/*.cpp)
SOURCE_GROUP("src\\\\hermes" FILES ${SOURCES_HERMES})

file(GLOB_RECURSE SOURCES_MERCURY ${SRC_DIR}/mercury/*.cpp)
SOURCE_GROUP("src\\\\mercury" FILES ${SOURCES_MERCURY})

file(GLOB_RECURSE SOURCES_MINOS	${SRC_DIR}/minos/*.cpp)
SOURCE_GROUP("src\\\\minos" FILES ${SOURCES_MINOS})

file(GLOB_RECURSE ALL_INCLUDES ${INC_DIR}/*.h)
SOURCE_GROUP("inc" FILES ${ALL_INCLUDES})

file(GLOB_RECURSE ALL_SOURCES ${SRC_DIR}/*.cpp)

#LIST(REMOVE_ITEM ALL_SOURCES 
#)

#build
add_executable(arx ${ALL_SOURCES} ${ALL_INCLUDES})

#link
target_link_libraries(arx dsound dxguid gdi32 shell32 dinput comdlg32 ole32 comctl32 ddraw jpeg z)

add_custom_target(remake
        COMMAND make clean && rm CMakeCache.txt && cmake . -G\"Unix Makefiles\" && make -j`getconf _NPROCESSORS_ONLN`
)

add_subdirectory(tools)


