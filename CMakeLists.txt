##############################################################################
# CMake-based build system for vgl
# Author: Seung-Tak Noh (seungtak.noh@gmail.com)
##############################################################################
cmake_minimum_required (VERSION 3.1)
project ("vgl")


########################################
# Handling external libraries
########################################
find_package(OpenGL REQUIRED)

set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glew-cmake)


###############################################################################
# Main library (cvgl)
###############################################################################
set(VGL_HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/vgl)
set(VGL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB
  headers
  "${VGL_HEADER_DIR}/*.h"
)
file(GLOB
  sources
  "${VGL_SOURCE_DIR}/*.cpp"
)


########################################
## cvgl build
########################################
add_library(vgl STATIC ${headers} ${sources})

## add header-only libraries
target_include_directories(vgl PUBLIC
  ${CMAKE_SOURCE_DIR}/external/glm
  ${CMAKE_SOURCE_DIR}/external/glfw/include
  ${CMAKE_SOURCE_DIR}/external/glew-cmake/include
)

## link *.lib
target_link_libraries(vgl
  OpenGL::GLU
  libglew_static
  glfw
)


##############################################################################
# Sub-directories: add new project as subdirectory
##############################################################################
add_subdirectory(apps/app101)
