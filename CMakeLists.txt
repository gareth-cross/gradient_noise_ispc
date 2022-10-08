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
include(AddISPCLibrary)

add_subdirectory(third_party)
add_subdirectory(src/gradient_noise)

# Main project library
set(EXE_TARGET "noise")
add_executable(${EXE_TARGET} "src/main.cc")
add_dependencies(${EXE_TARGET} gradient_noise stb fmt)
target_link_libraries(${EXE_TARGET} gradient_noise stb fmt)

target_compile_features(${EXE_TARGET} PUBLIC cxx_std_17)
if (MSVC)
  target_compile_options(${EXE_TARGET} PRIVATE /W4 /WX /D_USE_MATH_DEFINES /wd4244)
else ()
  target_compile_options(${EXE_TARGET} PRIVATE
      -Wall -Wextra -pedantic -Werror -Wno-sign-compare)
endif ()
