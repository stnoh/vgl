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
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)

## GLEW
set(USE_GLU ON CACHE BOOL "" FORCE) ## use GLU for compatibility
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glew-cmake)

## AntTweakBar
set(ATB_GUIRENDERER_USE_LEGACY_GL ON CACHE BOOL "" FORCE)
#set(ATB_GUIRENDERER_USE_CORE_GL OFF CACHE BOOL "" FORCE)
#set(ATB_GUIRENDERER_USE_D3D11 OFF CACHE BOOL "" FORCE)
#set(ATB_GUIRENDERER_USE_D3D10 OFF CACHE BOOL "" FORCE)
#set(ATB_GUIRENDERER_USE_D3D9 OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

#set(ATB_EVENT_HANDLER_GLFW OFF CACHE BOOL "" FORCE)
set(ATB_EVENT_HANDLER_GLFW3 ON CACHE BOOL "" FORCE)
#set(ATB_EVENT_HANDLER_GLUT OFF CACHE BOOL "" FORCE)
#set(ATB_EVENT_HANDLER_SDL12 OFF CACHE BOOL "" FORCE)
#set(ATB_EVENT_HANDLER_SDL13 OFF CACHE BOOL "" FORCE)
#set(ATB_EVENT_HANDLER_SDL20 OFF CACHE BOOL "" FORCE)
#set(ATB_EVENT_HANDLER_SFML OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/AntTweakBar)


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

add_library(vgl STATIC ${headers} ${sources})


###############################################################################
# external library (tinyfiledialog)
###############################################################################
project ("tinyfd" LANGUAGES C)

file(GLOB
  header_tinyfd
  "${CMAKE_CURRENT_SOURCE_DIR}/external/tinyfd/tinyfiledialogs.h"
)
file(GLOB
  source_tinyfd
  "${CMAKE_CURRENT_SOURCE_DIR}/external/tinyfd/tinyfiledialogs.c"
)

add_library(tinyfd STATIC ${header_tinyfd} ${source_tinyfd})


## add headers
target_include_directories(vgl PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
  ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
  ${CMAKE_CURRENT_SOURCE_DIR}/external/glew-cmake/include
  ${CMAKE_CURRENT_SOURCE_DIR}/external/AntTweakBar/TweakBar/include
  ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyfd
  ${CMAKE_CURRENT_SOURCE_DIR}/external/dirent/include
)

## link *.lib
target_link_libraries(vgl
  OpenGL::GLU
  libglew_static
  glfw
  AntTweakBar
  tinyfd
  $<$<PLATFORM_ID:Linux>:X11> ## add X11 subsystem for Linux
)


## suppress some known warnings (for Windows)
if(WIN32)
target_compile_options(AntTweakBar PRIVATE "/wd4068") ## AntTweakBar-related warnings
target_compile_options(AntTweakBar PRIVATE "/wd4302") ## AntTweakBar-related warnings
endif()


##############################################################################
# Sub-directories: add new project as subdirectory
##############################################################################
add_subdirectory(apps/app101)
add_subdirectory(apps/app102)
add_subdirectory(apps/app103)
add_subdirectory(apps/app104)
add_subdirectory(apps/app105)
add_subdirectory(apps/app106)
add_subdirectory(apps/app107)

add_subdirectory(apps/app201)
add_subdirectory(apps/app202)

add_subdirectory(apps/app301)
add_subdirectory(apps/app302)

add_subdirectory(apps/app401)
