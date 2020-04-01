#
#--- Libraries Setups
#
# Author: Qi WU, University of Utah
#
# Modified based on the course project from EPFL ICG course that I have taken
# This is used for configure the environment with CMAKE
#
# Build configuration file for "Intro to Graphics"
# Copyright (C) 2014 - LGG EPFL
#
#--- To understand its content:
#   http://www.cmake.org/cmake/help/syntax.html
#   http://www.cmake.org/Wiki/CMake_Useful_Variables
#
#--- Interface
#   This module wil define two global variables
#--- This is how you show a status message in the build system
MESSAGE(STATUS "Interactive Computer Graphics - Loading Common Configuration")
#
# define macro
#
MACRO(DeployRepo SRC DEST)
  MESSAGE(STATUS "-- Deploying: ${SRC} to ${DEST}")
  FOREACH(f ${SRC})
    FILE(COPY ${f} DESTINATION ${DEST})
  ENDFOREACH()
ENDMACRO(DeployRepo)
#
# General Settings
#
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native")
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_MACOSX_RPATH 1)
ENDIF()
#
# Initialize library pathes and dll path
#
SET(OPENGL_LIBS "") # those will be link for each project
SET(COMMON_LIBS "") # those will be link for each project
SET(COMMON_DLLS "") # those files will be copyed to the executable folder
#
#--- OPENGL
#
FIND_PACKAGE(OpenGL REQUIRED)
IF(OPENGL_FOUND)
  MESSAGE(STATUS " OPENGL found!  ${OPENGL_LIBRARIES}")
  INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
  LIST(APPEND COMMON_LIBS ${OPENGL_LIBRARIES})
  IF(OPENGL_GLU_FOUND)
    MESSAGE(STATUS " GLU found!")
  ELSE()
    MESSAGE(FATAL_ERROR " GLU not found!")
  ENDIF()
ELSE()
  MESSAGE(FATAL_ERROR " OPENGL not found!")
ENDIF()
#
#--- GLEW
#
OPTION(ENABLE_GLEW "Enable GLEW Library" OFF)
IF(ENABLE_GLEW)
  FIND_PACKAGE(GLEW REQUIRED)
  IF(GLEW_FOUND)
    MESSAGE(STATUS " GLEW found!  ${GLEW_LIBRARIES}")
    INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIR})
    LIST(APPEND COMMON_LIBS ${GLEW_LIBRARIES})
    LIST(APPEND OPENGL_LIBS ${GLEW_LIBRARIES})
  ELSE()
    MESSAGE(FATAL_ERROR " GLEW not found!")
  ENDIF()
ENDIF()
#
#--- CMake extension to load GLUT
#
OPTION(ENABLE_GLUT "Enable GLUT Library" OFF)
IF(ENABLE_GLUT)
  FIND_PACKAGE(GLUT REQUIRED)
  IF(GLUT_FOUND)
    MESSAGE(STATUS " GLUT found!  ${GLUT_LIBRARIES}")
    INCLUDE_DIRECTORIES(${GLUT_INCLUDE_DIR})
    LIST(APPEND COMMON_LIBS ${GLUT_LIBRARIES})
    LIST(APPEND OPENGL_LIBS ${GLUT_LIBRARIES})
    IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      MARK_AS_ADVANCED(GLUT_cocoa_LIBRARY)
      ADD_DEFINITIONS(-DUSE_GLUT)
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR " GLUT not found!")
  ENDIF()
ENDIF()
#
#--- GLFW
#
OPTION(ENABLE_GLFW "Enable GLFW Library" ON)
IF(ENABLE_GLFW)
  IF(EXISTS ${PROJECT_SOURCE_DIR}/external/glfw)
    SET(GLFW_BUILD_DOCS     OFF)
    SET(GLFW_BUILD_EXAMPLES OFF)
    SET(GLFW_BUILD_TESTS    OFF)
    SET(GLFW_BUILD_INSTALL  OFF)
    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/external/glfw/include)
    ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/external/glfw)
    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/external/glad/include)
    ADD_LIBRARY(glfw_glad
      ${PROJECT_SOURCE_DIR}/external/glad/src/glad.c)
    LIST(APPEND COMMON_LIBS glfw glfw_glad)
    LIST(APPEND OPENGL_LIBS glfw glfw_glad)
    ADD_DEFINITIONS(-DUSE_GLFW)
  ENDIF()

ENDIF(ENABLE_GLFW)
#
#----------------------------------------------------------------------------
#
#--- ImGUI
#
IF(EXISTS ${PROJECT_SOURCE_DIR}/external/imgui)
  ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/external/imgui)
  INCLUDE_DIRECTORIES(${ImGUI_INCLUDE_DIR})
  LIST(APPEND COMMON_LIBS ${ImGUI_LIBRARIES})
  ADD_DEFINITIONS(-DUSE_IMGUI)
ENDIF()
#
#--- glm
#
IF(EXISTS ${PROJECT_SOURCE_DIR}/external/glm)
  INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/external/glm)
  ADD_DEFINITIONS(-DUSE_GLM)
ENDIF()
#
#--- lodePNG
#   http://lodev.org/lodepng
IF(EXISTS ${PROJECT_SOURCE_DIR}/external/lodepng)
  ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/external/lodepng)
  INCLUDE_DIRECTORIES(${LodePNG_INCLUDE_DIR})
  LIST(APPEND COMMON_LIBS ${LodePNG_LIBRARIES})
  ADD_DEFINITIONS(-DUSE_LODEPNG)
ENDIF()
#
#--- rapid JSON
#
IF(EXISTS ${PROJECT_SOURCE_DIR}/external/rapidjson)
  INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/external/rapidjson/include")
  ADD_DEFINITIONS(-DUSE_RAPIDJSON)
ENDIF()  
#
#----------------------------------------------------------------------------
#
#--- TBB
#
#FIND_PACKAGE(TBB)
#IF (TBB_FOUND)
#  INCLUDE_DIRECTORIES(${TBB_INCLUDE_DIR})
#  LIST(APPEND COMMON_LIBS ${TBB_LIBRARY})
#  IF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
#  ENDIF ()
#  ADD_DEFINITIONS(-DUSE_TBB)
#ENDIF ()
#
#--- OpenMP
#
FIND_PACKAGE(OpenMP)
IF (OPENMP_FOUND)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  ADD_DEFINITIONS(-DUSE_OMP)
ENDIF()
