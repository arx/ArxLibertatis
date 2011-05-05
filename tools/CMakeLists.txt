cmake_minimum_required(VERSION 2.8)
# compile tools

find_package(ZLIB REQUIRED)

if(MSVC)
else(MSVC)
	set(CMAKE_CXX_COMPILER g++)
	set(CMAKE_C_COMPILER gcc)
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wformat=2 -Wundef -Wpointer-arith -Wlogical-op -Wcast-qual -O2 -fno-strict-	aliasing -ggdb")
endif(MSVC)

include_directories(
	../src
)

add_executable(unpaker 
	unpak/unpak.cpp
	savetool/Filesystem_C.cpp
	../src/io/PakEntry.cpp
	../src/io/PakReader.cpp
	../src/io/HashMap.cpp
	../src/io/Logger.cpp
	../src/io/Blast.cpp
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
	../src/io/PakManager.cpp
	../src/platform/String.cpp
)

target_link_libraries(savetool ${ZLIB_LIBRARIES})
