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
../src/hermes/PakEntry.cpp 
../src/hermes/PakReader.cpp 
../src/hermes/HashMap.cpp 
../src/hermes/Logger.cpp 
../src/hermes/blast.cpp)

# target_link_libraries(unpak z)
