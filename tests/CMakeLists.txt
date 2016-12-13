cmake_minimum_required(VERSION 2.8)
# compile tests tool

suboption(CppUnit_USE_STATIC_LIBS "Statically link CppUnit" BOOL ${USE_STATIC_LIBS})

find_package(CppUnit REQUIRED)

# CppUnit requires RTTI
string(REGEX REPLACE "-fno-rtti" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

include_directories(SYSTEM ${CppUnit_INCLUDE_DIR})

# TODO this is required because the debug log has too many dependencies
add_definitions(-UARX_DEBUG)

include_directories(
	../src
	../
)

add_executable(arxtest
	TestMain.cpp
	
	../src/graphics/Math.cpp
	../src/graphics/Color.h
	../src/graphics/Renderer.cpp
	../src/game/Camera.cpp
	../src/util/String.cpp
	
	graphics/ColorTest.cpp
	
	../src/io/fs/FilePath.cpp
	io/fs/FilePathTest.cpp
	
	../src/io/resource/ResourcePath.cpp
	io/resource/ResourcePathTest.cpp
	
	../src/io/IniSection.cpp
	io/IniTest.h
	io/IniTest.cpp
	
	math/AssertionTraits.h
	math/LegacyMath.h
	math/LegacyMathTest.cpp
	util/StringTest.cpp
)

target_link_libraries(arxtest ${CppUnit_LIBRARIES})

add_custom_target(check
	COMMAND "$<TARGET_FILE:arxtest>"
	DEPENDS arxtest
	WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
	COMMENT "Running tests" VERBATIM
)
