# Copyright 2022 Gareth Cross
cmake_minimum_required(VERSION 3.17)
project(noise_ispc VERSION 0.1
    DESCRIPTION "Gradient noise with ISPC"
    LANGUAGES CXX)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}/cmake
    )

# Detect windows, and set the architecture.
option(OVERRIDE_ARCH_ON_WIN32 "Automatically set x64 on windows platform." ON)
if (MSVC AND ${OVERRIDE_ARCH_ON_WIN32})
    message(STATUS "Changing architecture because we are on windows.")
    set(CMAKE_GENERATOR_PLATFORM "x64" CACHE STRING "" FORCE)
endif ()

include(AddISPCLibrary)

add_subdirectory(third_party)
add_subdirectory(src/gradient_noise)

# Main project library
set(EXE_TARGET "noise")
add_executable(${EXE_TARGET} "src/main.cc")
add_dependencies(${EXE_TARGET} gradient_noise stb fmt glm)
target_link_libraries(${EXE_TARGET} gradient_noise stb fmt glm)
target_compile_features(${EXE_TARGET} PUBLIC cxx_std_17)

# Option to relax floating point constraints
option(USE_FP_FAST "Use /fp:fast when building on windows." OFF)

if (MSVC)
  target_compile_options(${EXE_TARGET} PRIVATE /W4 /WX /D_USE_MATH_DEFINES /wd4244)
  if (USE_FP_FAST)
    target_compile_options(${EXE_TARGET} PRIVATE /fp:fast)
  endif()
else ()
  target_compile_options(${EXE_TARGET} PRIVATE
      -Wall -Wextra -pedantic -Werror -Wno-sign-compare)
endif ()
