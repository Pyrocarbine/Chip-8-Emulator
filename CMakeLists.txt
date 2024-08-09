cmake_minimum_required(VERSION 3.14)
project(chip8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR/3rdParty})

#set(SDL2_PATH "/Users/aaronhan/Projects/Chip-8_Emulator/3rdParty/SDL2.framework")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})



add_executable(
        chip8
        Source/Chip8.cpp
        Source/Main.cpp
        Source/Platform.cpp)

target_compile_options(chip8 PRIVATE -Wall)

target_link_libraries(chip8 ${SDL2_LIBRARIES})

