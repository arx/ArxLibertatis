cmake_minimum_required(VERSION 2.8)
# compile tests tool

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_FLAGS "-m64")
set(ENABLE_TESTING TRUE)
#set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Woverloaded-virtual")

include_directories(
	../src
)

add_executable(arxtest
        testMain.cpp
        ../src/graphics/GraphicsUtility.cpp
        graphics/GraphicsUtilityTest.cpp
        math/vectors.cpp
        ../src/graphics/Math.cpp
		../src/graphics/Color.h
		graphics/ColorTest.cpp
)

target_link_libraries(arxtest cppunit)
