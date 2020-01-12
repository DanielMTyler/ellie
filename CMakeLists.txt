#==================================
#Copyright (C) 2020 Daniel M Tyler.
#  This file is part of Ellie.
#==================================

cmake_minimum_required(VERSION 3.15.6)
# Must include C or .c files will be silently ignored.
project(ellie C CXX)

# Set a default build type if none was specified.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)



if(WIN32 AND "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    # Remove "lib" prefix on DLL names.
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_MODULE_PREFIX "")
    add_executable(ellie-bin src/platform_win64.cpp deps/glad/glad.c deps/imgui/imgui.cpp deps/imgui/imgui_demo.cpp deps/imgui/imgui_draw.cpp deps/imgui/imgui_widgets.cpp deps/imgui/examples/imgui_impl_opengl3.cpp deps/imgui/examples/imgui_impl_sdl.cpp deps/imgui/misc/cpp/imgui_stdlib.cpp)
else()
    message(FATAL_ERROR "Unknown OS.")
endif()

# MODULE means no .a file for linking.
add_library(ellie-lib MODULE src/game.cpp)

set_target_properties(ellie-bin PROPERTIES OUTPUT_NAME ellie)
set_target_properties(ellie-lib PROPERTIES OUTPUT_NAME ellie)
target_compile_definitions(ellie-bin PRIVATE ORGANIZATION="DanielMTyler" PROJECT_NAME="ellie")
target_compile_definitions(ellie-lib PRIVATE ORGANIZATION="DanielMTyler" PROJECT_NAME="ellie")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND NOT "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
    target_compile_options(ellie-bin PRIVATE -Werror -Wall -Wextra -Wpedantic)
    target_compile_options(ellie-lib PRIVATE -Werror -Wall -Wextra -Wpedantic)
else()
    message(FATAL_ERROR "Unknown Compiler.")
endif()



# Uncomment this and dependencies' includes will use the SYSTEM flag and warnings will be ignored by certain compilers.
#set(DEPS_INCLUDE_SYSTEM_SPEC SYSTEM)

target_include_directories(ellie-bin ${DEPS_INCLUDE_SYSTEM_SPEC} PRIVATE deps/imgui deps/imgui/examples deps/imgui/misc/cpp)
target_include_directories(ellie-lib ${DEPS_INCLUDE_SYSTEM_SPEC} PRIVATE deps/imgui deps/imgui/examples deps/imgui/misc/cpp)

target_include_directories(ellie-bin ${DEPS_INCLUDE_SYSTEM_SPEC} PRIVATE deps/glad)
target_include_directories(ellie-lib ${DEPS_INCLUDE_SYSTEM_SPEC} PRIVATE deps/glad)

target_include_directories(ellie-bin ${DEPS_INCLUDE_SYSTEM_SPEC} PRIVATE deps/glm)
target_include_directories(ellie-lib ${DEPS_INCLUDE_SYSTEM_SPEC} PRIVATE deps/glm)

find_package(fmt 6.1.2 REQUIRED)
target_link_libraries(ellie-bin fmt::fmt)
target_link_libraries(ellie-lib fmt::fmt)

find_package(SDL2 REQUIRED)
target_include_directories(ellie-bin ${DEPS_INCLUDE_SYSTEM_SPEC} PRIVATE ${SDL2_INCLUDE_DIRS})
# Required because SDL.h is included in global.hpp.
target_include_directories(ellie-lib ${DEPS_INCLUDE_SYSTEM_SPEC} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(ellie-bin ${SDL2_LIBRARIES})
message(WARNING "SDL2 should be v2.0.10, but it can't be verified automatically.")

find_package(spdlog 1.5.0 REQUIRED)
target_link_libraries(ellie-bin spdlog::spdlog)
target_link_libraries(ellie-lib spdlog::spdlog)

target_include_directories(ellie-bin ${DEPS_INCLUDE_SYSTEM_SPEC} PRIVATE deps/stb)
target_include_directories(ellie-lib ${DEPS_INCLUDE_SYSTEM_SPEC} PRIVATE deps/stb)
