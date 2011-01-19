cmake_minimum_required(VERSION 2.8)
project(ArxFatalis)

set(SRC_DIR Sources)

include_directories(
	Sources/Include
	Sources/Include/OtherLibs
	/usr/include/wine/windows
	#/usr/include/wine/msvcrt
)

#wine compiler
set(CMAKE_CXX_COMPILER /usr/bin/wineg++)
set(CMAKE_C_COMPILER /usr/bin/winegcc)

#set temporally less gcc problems
set(CMAKE_CXX_FLAGS "-fpermissive -w")

#Add sources
set(SOURCE_DIRS
	ArxCommon
	#Athena
    DANAE
    DANAE_Debugger
    EERIE
    HERMES
    Mercury
    MINOS
)
foreach(SOURCE_DIR ${SOURCE_DIRS})
    file(GLOB GLOB_RESULT ${SRC_DIR}/${SOURCE_DIR}/*.cpp)
    set(SOURCES ${SOURCES} ${GLOB_RESULT})
endforeach(SOURCE_DIR)

#LIST(REMOVE_ITEM SOURCES 
#)

#build
add_executable(arx ${SOURCES})

#link
target_link_libraries(arx wine jpeg)
