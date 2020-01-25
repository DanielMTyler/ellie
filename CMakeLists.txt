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



set(ORGANIZATION "Ellie" CACHE STRING "Organization name.")
set(PROJECT_NAME "Ellie" CACHE STRING "Project name.")

set(THIRDPARTY_SOURCES thirdparty/glad/glad.c thirdparty/imgui/imgui.cpp thirdparty/imgui/imgui_demo.cpp thirdparty/imgui/imgui_draw.cpp thirdparty/imgui/imgui_widgets.cpp thirdparty/imgui/examples/imgui_impl_opengl3.cpp thirdparty/imgui/examples/imgui_impl_sdl.cpp thirdparty/imgui/misc/cpp/imgui_stdlib.cpp)



if(WIN32 AND "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
# Win64.
    # Remove "lib" prefix on DLL names.
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_MODULE_PREFIX "")
    add_executable(ellie-bin src/app_win64.cpp ${THIRDPARTY_SOURCES})
else()
    message(FATAL_ERROR "Unknown OS.")
endif()

set_target_properties(ellie-bin PROPERTIES OUTPUT_NAME ellie)
target_compile_definitions(ellie-bin PRIVATE ORGANIZATION="${ORGANIZATION}" PROJECT_NAME="${PROJECT_NAME}")

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" AND NOT "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC")
# clang++.
    target_compile_options(ellie-bin PRIVATE -Werror -Wall -Wextra -Wpedantic)
else()
    message(FATAL_ERROR "Unknown Compiler.")
endif()



target_include_directories(ellie-bin PRIVATE thirdparty/imgui thirdparty/imgui/examples thirdparty/imgui/misc/cpp)

target_include_directories(ellie-bin PRIVATE thirdparty/glad)

target_include_directories(ellie-bin PRIVATE thirdparty/glm)

find_package(fmt 6.1.2 REQUIRED)
target_link_libraries(ellie-bin fmt::fmt)

find_package(SDL2 REQUIRED)
target_include_directories(ellie-bin PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(ellie-bin ${SDL2_LIBRARIES})
message(WARNING "SDL2 should be v2.0.10, but it can't be verified automatically.")

find_package(spdlog 1.5.0 REQUIRED)
target_link_libraries(ellie-bin spdlog::spdlog)

target_include_directories(ellie-bin PRIVATE thirdparty/stb)
