cmake_minimum_required(VERSION 2.8)
project(ArxFatalis)


set(SRC_DIR src)

include_directories(
	include
	/usr/include/wine/windows
)

#wine compiler
set(CMAKE_CXX_COMPILER /usr/bin/wineg++)
set(CMAKE_C_COMPILER /usr/bin/winegcc)

#set temporally less gcc problems
set(CMAKE_CXX_FLAGS "-fpermissive -w -m32")

#Debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")

add_definitions(-D_DEBUG)

file(GLOB_RECURSE SOURCES ${SRC_DIR}/*.cpp)

#LIST(REMOVE_ITEM SOURCES 
#)

#build
add_executable(arx ${SOURCES})

#link
target_link_libraries(arx dsound dxguid gdi32 shell32 dinput comdlg32 ole32 comctl32 ddraw jpeg z)

add_custom_target(remake
        COMMAND make clean && rm CMakeCache.txt && cmake . -G\"Unix Makefiles\" && make -j`getconf _NPROCESSORS_ONLN`
)



# compile unpak tool

set(UNPAK_SRC_DIR tools/unpak)

include_directories(
	include
)

# normal build TODO
#set(CMAKE_CXX_COMPILER g++)
#set(CMAKE_C_COMPILER gcc)
#set(CMAKE_CXX_FLAGS "-fpermissive -w")

add_executable(unpak tools/unpak/unpak.cpp src/hermes/PakEntry.cpp src/hermes/PakReader.cpp src/hermes/HashMap.cpp src/hermes/blast.cpp)

# target_link_libraries(unpak z)
