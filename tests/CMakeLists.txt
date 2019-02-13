cmake_minimum_required(VERSION 2.8)
# compile tests tool

suboption(CppUnit_USE_STATIC_LIBS "Statically link CppUnit" BOOL ${USE_STATIC_LIBS})
set(RUN_TARGET CACHE STRING "Wrapper to run built targets")
mark_as_advanced(RUN_TARGET)
set(default_RUN_TESTS OFF)
if(DEVELOPER AND (NOT CMAKE_CROSSCOMPILING OR NOT RUN_TARGET STREQUAL ""))
	set(default_RUN_TESTS ON)
endif()
suboption(RUN_TESTS "Run tests as part of the default build target" BOOL ${default_RUN_TESTS})

find_package(CppUnit REQUIRED)

# CppUnit requires RTTI
string(REGEX REPLACE "-fno-rtti" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

include_directories(SYSTEM ${CppUnit_INCLUDE_DIR})

# TODO this is required because the debug log has too many dependencies
add_definitions(-UARX_DEBUG)

include_directories(
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}
)

add_executable(arxtest
	TestMain.cpp
	
	${PROJECT_SOURCE_DIR}/src/graphics/Math.cpp
	${PROJECT_SOURCE_DIR}/src/graphics/Color.h
	${PROJECT_SOURCE_DIR}/src/graphics/Renderer.cpp
	${PROJECT_SOURCE_DIR}/src/game/Camera.cpp
	${PROJECT_SOURCE_DIR}/src/util/String.cpp
	
	graphics/ColorTest.cpp
	
	${PROJECT_SOURCE_DIR}/src/io/fs/FilePath.cpp
	io/fs/FilePathTest.cpp
	
	${PROJECT_SOURCE_DIR}/src/io/resource/ResourcePath.cpp
	io/resource/ResourcePathTest.cpp
	
	${PROJECT_SOURCE_DIR}/src/io/IniSection.cpp
	io/IniTest.h
	io/IniTest.cpp
	
	math/AssertionTraits.h
	math/LegacyMath.h
	math/LegacyMathTest.cpp
	util/StringTest.cpp
)

target_link_libraries(arxtest ${CppUnit_LIBRARIES})

set(run_tests)
if(RUN_TESTS)
	set(run_tests ALL)
endif()

set(arxtest_binary "$<TARGET_FILE:arxtest>")
if(CMAKE_VERSION VERSION_LESS 2.8.12)
	get_property(arxtest_binary TARGET arxtest PROPERTY LOCATION_${CMAKE_BUILD_TYPE})
endif()

add_custom_target(check ${run_tests}
	COMMAND ${RUN_TARGET} "${arxtest_binary}"
	DEPENDS arxtest
	WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
	COMMENT "Running tests" VERBATIM
)
