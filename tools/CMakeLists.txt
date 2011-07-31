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
find_package(Boost REQUIRED)

if(MSVC)
	set(HAVE_WINAPI 1)
else(MSVC)
	
	if(NOT CMAKE_TOOLCHAIN_FILE)
		find_program(GXX_BINARY g++)
		set(CMAKE_CXX_COMPILER "${GXX_BINARY}")
		set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wformat=2 -Wundef -Wpointer-arith -Wlogical-op -Wcast-qual -O2 -fno-strict-aliasing -ggdb")
	endif()
	
	if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
		set(HAVE_WINAPI 1)
	else()
		set(HAVE_WINAPI 0)
	endif()
	
	if(ARX_FORCE_32BIT)
		set(CMAKE_C_FLAGS "-m32 ${CMAKE_C_FLAGS}")
		set(CMAKE_CXX_FLAGS "-m32 ${CMAKE_CXX_FLAGS}")
	endif()
	
	check_include_files("sys/stat.h;sys/errno.h;dirent.h" HAVE_POSIX_FILESYSTEM)
	
	check_link_library(ZLIB ZLIB_LIBRARIES)
endif(MSVC)

if(HAVE_POSIX_FILESYSTEM)
	set(FILESYSTEM_SRC "../src/io/FilesystemPOSIX.cpp")
elseif(HAVE_WINAPI)
	add_definitions(-DHAVE_WINDOWS_FILESYSTEM)
	set(FILESYSTEM_SRC "../src/io/FilesystemWindows.cpp")
elseif((Boost_MAJOR_VERSION GREATER 1) OR (NOT Boost_MINOR_VERSION LESS 44))
	find_package(Boost 1.44 REQUIRED COMPONENTS filesystem system)
	set(HAVE_BOOST_FILESYSTEM_V3 1)
	add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
	set(FILESYSTEM_SRC "../src/io/FilesystemBoost.cpp")
else()
	message(FATAL_ERROR "You need either Boost >= 1.44 or windows api or sys/stat.h, sys/errno.h and dirent.h; Found boost version ${Boost_VERSION}")
endif()

if(NOT MSVC)
	check_link_library(Boost Boost_LIBRARIES)
endif()

include_directories(../src)
include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})

# TODO use this for the game too once we get rid of wine (for non-unity-builds)
add_library(tools_common STATIC
	../src/io/Filesystem.cpp
	${FILESYSTEM_SRC}
	../src/io/FilePath.cpp
	../src/io/FileStream.cpp
	../src/io/PakEntry.cpp
	../src/io/PakReader.cpp
	../src/io/Logger.cpp
	../src/io/Blast.cpp
	../src/platform/String.cpp
	../src/platform/Platform.cpp
)

target_link_libraries(tools_common ${Boost_LIBRARIES})

add_executable(unpaker unpak/unpak.cpp)

add_executable(savetool
	savetool/savetool.cpp
	savetool/SaveFix.cpp
	savetool/SaveView.cpp
	../src/io/SaveBlock.cpp
	../src/core/Localisation.cpp
	../src/io/IniReader.cpp
	../src/io/IniSection.cpp
)

target_link_libraries(savetool tools_common ${ZLIB_LIBRARIES})
target_link_libraries(unpaker tools_common)
