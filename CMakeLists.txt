cmake_minimum_required(VERSION 3.10)
project(DrumMachine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(DrumMachine main.cpp)

if(WIN32)
	target_link_libraries(DrumMachine PRIVATE winmm)
endif()