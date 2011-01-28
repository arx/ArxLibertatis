cmake_minimum_required(VERSION 2.8)
# compile tests tool

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_FLAGS "-m64")
set(ENABLE_TESTING TRUE)
#set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Woverloaded-virtual")

include_directories(
	../include
)

add_executable(math 
	math/vectors.cpp 
	../src/renderer/EERIEMath.cpp 
)

target_link_libraries(math cppunit)
