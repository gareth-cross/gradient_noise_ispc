add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/stb>
)

set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
add_subdirectory(fmt)

# add glm
add_subdirectory(glm)
