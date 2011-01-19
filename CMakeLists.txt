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
set(CMAKE_CXX_FLAGS "-fpermissive -w -m32")

#Add sources
set(SOURCE_DIRS
	ArxCommon
	Athena
    danae
    DANAE_Debugger
    eerie
    hermes
    Mercury
    minos
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
target_link_libraries(arx dsound dxguid gdi32 shell32 dinput comdlg32 ole32 comctl32 ddraw jpeg z)

add_custom_target(remake
        COMMAND make clean && rm CMakeCache.txt && cmake . -G\"Unix Makefiles\" && make -j`getconf _NPROCESSORS_ONLN`
)
