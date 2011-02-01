cmake_minimum_required(VERSION 2.8)
# compile unpak tool

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_FLAGS "-Wall -Werror -Wextra -Woverloaded-virtual")

include_directories(
	../include
)

add_executable(unpaker 
unpak/unpak.cpp 
../src/io/PakEntry.cpp 
../src/io/PakReader.cpp 
../src/io/HashMap.cpp 
../src/io/Logger.cpp 
../src/io/Blast.cpp)

# target_link_libraries(unpak z)
