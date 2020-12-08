#==================================
#Copyright (C) 2020 Daniel M Tyler.
#  This file is part of Ellie.
#==================================

cmake_minimum_required(VERSION 3.18.0)
# Must include C or .c files will be silently ignored.
project(ellie C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ellie-bin thirdparty/glad/src/glad.c src/app.cpp src/app_interface.cpp src/main.cpp src/view.cpp)
set_target_properties(ellie-bin PROPERTIES OUTPUT_NAME ellie)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND NOT "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
# clang++.
target_compile_options(ellie-bin PRIVATE -Werror -Wall -Wextra -Wpedantic)
else()
    message(FATAL_ERROR "Compiler not yet supported.")
endif()

find_package(SDL2 2.0.12 REQUIRED)
target_link_libraries(ellie-bin SDL2::SDL2 SDL2::SDL2main)

# @note Indicated as SYSTEM to ignore thirdparty warnings/errors.
target_include_directories(ellie-bin SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/glad/include ${CMAKE_SOURCE_DIR}/thirdparty/glad/src)
