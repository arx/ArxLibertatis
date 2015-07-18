cmake_minimum_required(VERSION 2.8)
# compile tests tool

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_FLAGS "-m64 -std=c++11")
set(ENABLE_TESTING TRUE)
#set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Woverloaded-virtual")

# TODO this is required because the debug log has too many dependencies
add_definitions(-UARX_DEBUG)

include_directories(
	../src
	../
)

add_executable(arxtest
	testMain.cpp
	
	../src/graphics/Math.cpp
	../src/graphics/Color.h
	../src/graphics/Renderer.cpp
	../src/game/Camera.cpp
	../src/util/String.cpp
	
	graphics/ColorTest.cpp
	
# TODO the logger should not be required for using the ini reader
#	../src/platform/Platform.h
#	../src/platform/Platform.cpp
#	../src/platform/Lock.h
#	../src/platform/Lock.cpp
#	../src/platform/Environment.h
#	../src/platform/Environment.cpp
#	../src/io/log/LogBackend.h
#	../src/io/log/LogBackend.cpp
#	../src/io/log/ConsoleLogger.h
#	../src/io/log/ConsoleLogger.cpp
#	../src/io/log/Logger.h
#	../src/io/log/Logger.cpp
#	../src/io/IniReader.cpp
	../src/io/IniSection.cpp
	io/IniTest.h
	io/IniTest.cpp
	
	math/AssertionTraits.h
	math/LegacyMath.h
	math/LegacyMathTest.cpp
	util/StringTest.cpp
)

target_link_libraries(arxtest cppunit)
