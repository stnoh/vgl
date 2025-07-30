##############################################################################
# CMake-based build system for vgl
# Author: Seung-Tak Noh (seungtak.noh@gmail.com)
##############################################################################
cmake_minimum_required (VERSION 3.1)
project ("vgl" LANGUAGES CXX)


########################################
# Handling external libraries
########################################
find_package(OpenGL REQUIRED)

## GLFW
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)

## GLEW
add_subdirectory(${CMAKE_SOURCE_DIR}/external/glew-cmake)

## AntTweakBar
set(ATB_GUIRENDERER_USE_D3D11 OFF CACHE BOOL "" FORCE)
set(ATB_GUIRENDERER_USE_D3D10 OFF CACHE BOOL "" FORCE)
set(ATB_GUIRENDERER_USE_D3D9 OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/AntTweakBar)


###############################################################################
# Main library (vgl)
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
## vgl build
########################################
add_library(vgl STATIC ${headers} ${sources})

## add header-only libraries
target_include_directories(vgl PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/external/glm
  ${CMAKE_SOURCE_DIR}/external/glfw/include
  ${CMAKE_SOURCE_DIR}/external/glew-cmake/include
  ${CMAKE_SOURCE_DIR}/external/AntTweakBar/TweakBar/include
)

## link *.lib
target_link_libraries(vgl
  OpenGL::GLU
  libglew_static
  glfw
  AntTweakBar
)


## suppress some known warnings
target_compile_options(AntTweakBar PRIVATE "/wd4068") ## AntTweakBar-related warnings
target_compile_options(AntTweakBar PRIVATE "/wd4302") ## AntTweakBar-related warnings

##############################################################################
# Sub-directories: add new project as subdirectory
##############################################################################
add_subdirectory(apps/app101)
add_subdirectory(apps/app102)
