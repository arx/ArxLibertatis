cmake_minimum_required(VERSION 2.8)
# compile tools

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(UnityBuild)
include(CompileCheck)

find_package(ZLIB REQUIRED)

unset(Boost_LIBRARIES)

if(MSVC)
	# Link statically with Boost under Windows
	set(Boost_USE_STATIC_LIBS ON)
endif(MSVC)
find_package(Boost 1.37 REQUIRED COMPONENTS filesystem system)

if(NOT MSVC AND NOT CMAKE_TOOLCHAIN_FILE)
	find_program(GXX_BINARY g++)
	set(CMAKE_CXX_COMPILER "${GXX_BINARY}")
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wformat=2 -Wundef -Wpointer-arith -Wlogical-op -Wcast-qual -O2 -fno-strict-aliasing -ggdb")
endif()

if(NOT MSVC)
	if(ARX_FORCE_32BIT)
		set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
		set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
	endif()
	check_link_library(ZLIB ZLIB_LIBRARIES)
	check_link_library(Boost Boost_LIBRARIES)
endif()

include_directories(../src)
include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})

add_executable(unpaker 
	unpak/unpak.cpp
	../src/io/PakEntry.cpp
	../src/io/PakReader.cpp
	../src/io/Logger.cpp
	../src/io/Blast.cpp
	../src/io/Filesystem.cpp
	../src/io/FilePath.cpp
	../src/io/FileStream.cpp
	../src/platform/String.cpp
	../src/platform/Platform.cpp
)

add_executable(savetool
	savetool/savetool.cpp
	../src/io/Logger.cpp
	../src/io/Blast.cpp
	../src/io/SaveBlock.cpp
	../src/io/PakEntry.cpp
	../src/io/PakReader.cpp
	../src/io/Filesystem.cpp
	../src/io/FilePath.cpp
	../src/io/FileStream.cpp
	../src/platform/String.cpp
	../src/platform/Platform.cpp
)

target_link_libraries(savetool ${ZLIB_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(unpaker ${Boost_LIBRARIES})
