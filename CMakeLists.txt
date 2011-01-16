cmake_minimum_required(VERSION 2.8)
project(ArxFatalis)

set(SRC_DIR Sources)

include_directories(
	Sources/Include
	Sources/Include/OtherLibs
	/usr/include/wine/windows
	#/usr/include/wine/msvcrt
)

set(CMAKE_CXX_FLAGS "-fpermissive -w") #Debug

#Add liblub sources
set(SOURCE_DIRS
        DANAE
)
foreach(SOURCE_DIR ${SOURCE_DIRS})
    file(GLOB GLOB_RESULT ${SRC_DIR}/${SOURCE_DIR}/*.cpp)
    set(SOURCES ${SOURCES} ${GLOB_RESULT})
endforeach(SOURCE_DIR)


#add_library()
add_executable(arx ${SOURCES})
#add_executable(arx Sources/DANAE/ARX_CParticleSystem.cpp)

#link
#target_link_libraries()
