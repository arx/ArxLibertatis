cmake_minimum_required(VERSION 2.8)
# compile tools

find_package(ZLIB REQUIRED)

if(NOT MSVC AND NOT CMAKE_TOOLCHAIN_FILE)
	set(CMAKE_CXX_COMPILER g++)
	set(CMAKE_C_COMPILER gcc)
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wformat=2 -Wundef -Wpointer-arith -Wlogical-op -Wcast-qual -O2 -fno-strict-aliasing -ggdb")
endif()

include_directories(
	../src
)

add_executable(unpaker 
	unpak/unpak.cpp
	savetool/Filesystem_C.cpp
	../src/io/PakEntry.cpp
	../src/io/PakReader.cpp
	../src/io/Logger.cpp
	../src/io/Blast.cpp
	../src/platform/String.cpp
	../src/platform/Platform.cpp
)

add_executable(savetool
	savetool/savetool.cpp
	savetool/Filesystem_C.cpp
	../src/io/Logger.cpp
	../src/io/Blast.cpp
	../src/io/SaveBlock.cpp
	../src/io/HashMap.cpp
	../src/io/FilePath.cpp
	../src/io/PakEntry.cpp
	../src/io/PakReader.cpp
	../src/platform/String.cpp
	../src/platform/Platform.cpp
)

target_link_libraries(savetool ${ZLIB_LIBRARIES})
