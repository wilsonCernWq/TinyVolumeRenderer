#
# --- Libraries Setups
#
# Author: Qi WU, University of Utah, University of California, Davis
#
# Modified based on the course project from EPFL ICG course that I have taken
# This is used for configure the environment with CMAKE
#
# Build configuration file for "Intro to Graphics"
# Copyright (C) 2014 - LGG EPFL
#
# --- To understand its content:
#   http://www.cmake.org/cmake/help/syntax.html
#   http://www.cmake.org/Wiki/CMake_Useful_Variables
#
# --- Interface
#   This module wil define two global variables
# --- This is how you show a status message in the build system
message(STATUS "Loading Common Configuration")

# macro for copy shaders
macro(deployrepo SRC DEST)
  message(STATUS "-- Deploying: ${SRC} to ${DEST}")
  foreach(f ${SRC})
    file(COPY ${f} DESTINATION ${DEST})
  endforeach()
endmacro(deployrepo)

# General Settings
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_MACOSX_RPATH 1)
endif()

# Initialize library pathes and dll path
set(OPENGL_LIBS "") # those will be link for each project
set(COMMON_LIBS "") # those will be link for each project
set(COMMON_DLLS "") # those files will be copyed to the executable folder

# --- OPENGL
find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
  message(STATUS " OPENGL found!  ${OPENGL_LIBRARIES}")
  include_directories(${OPENGL_INCLUDE_DIR})
  list(APPEND COMMON_LIBS ${OPENGL_LIBRARIES})
  if(OPENGL_GLU_FOUND)
    message(STATUS " GLU found!")
  else()
    message(FATAL_ERROR " GLU not found!")
  endif()
else()
  message(FATAL_ERROR " OPENGL not found!")
endif()

# --- GLFW
option(ENABLE_GLFW "Enable GLFW Library" ON)
if(ENABLE_GLFW)
  if(EXISTS ${PROJECT_SOURCE_DIR}/external/glfw)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_INSTALL OFF)
    include_directories(${PROJECT_SOURCE_DIR}/external/glfw/include)
    add_subdirectory(${PROJECT_SOURCE_DIR}/external/glfw)
    include_directories(${PROJECT_SOURCE_DIR}/external/glad/include)
    add_library(glfw_glad
      ${PROJECT_SOURCE_DIR}/external/glad/src/glad.c)
    list(APPEND COMMON_LIBS glfw glfw_glad)
    list(APPEND OPENGL_LIBS glfw glfw_glad)
    add_definitions(-DUSE_GLFW)
  endif()

endif(ENABLE_GLFW)

# --- ImGUI
if(EXISTS ${PROJECT_SOURCE_DIR}/external/imgui)
  add_subdirectory(${PROJECT_SOURCE_DIR}/external/imgui)
  include_directories(${ImGUI_INCLUDE_DIR})
  list(APPEND COMMON_LIBS ${ImGUI_LIBRARIES})
  add_definitions(-DUSE_IMGUI)
endif()

# --- glm
if(EXISTS ${PROJECT_SOURCE_DIR}/external/glm)
  include_directories(${PROJECT_SOURCE_DIR}/external/glm)
  add_definitions(-DUSE_GLM)
endif()

# --- lodePNG
#   http://lodev.org/lodepng
if(EXISTS ${PROJECT_SOURCE_DIR}/external/lodepng)
  add_subdirectory(${PROJECT_SOURCE_DIR}/external/lodepng)
  include_directories(${LodePNG_INCLUDE_DIR})
  list(APPEND COMMON_LIBS ${LodePNG_LIBRARIES})
  add_definitions(-DUSE_LODEPNG)
endif()

# --- rapid JSON
if(EXISTS ${PROJECT_SOURCE_DIR}/external/rapidjson)
  include_directories("${PROJECT_SOURCE_DIR}/external/rapidjson/include")
  add_definitions(-DUSE_RAPIDJSON)
endif()

