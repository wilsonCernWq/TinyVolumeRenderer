# welcome to ECS 277
cmake_minimum_required(VERSION 3.0)
project(ecs277-template)

# load required libraries
include(cmake/common.cmake)

# install shader
set(SHADERS 
  renderer/raycast_vshader.glsl
  renderer/raycast_fshader.glsl
  quad_vshader.glsl 
  quad_fshader.glsl)
if(EXECUTABLE_OUTPUT_PATH)
  deployrepo("${SHADERS}" "${EXECUTABLE_OUTPUT_PATH}")
else()
  deployrepo("${SHADERS}" "${CMAKE_CURRENT_BINARY_DIR}")
endif()

# add program
add_executable(viewer main.cpp
  ${SHADERS}
  renderer/volume_object.cpp
  renderer/volume_object.hpp
  camera.cpp
  camera.hpp
  framebuffer.cpp
  framebuffer.hpp
  opengl.cpp
  opengl.hpp
  quad.cpp
  quad.hpp
  reader.cpp
  reader.hpp
  texture.cpp
  texture.hpp
  trackball.cpp
  trackball.hpp
  viewer.cpp
  viewer.hpp)
target_include_directories(viewer PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(viewer PUBLIC ${COMMON_LIBS})
set_target_properties(viewer PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  CXX_STANDARD 11)

# install dlls
if(EXECUTABLE_OUTPUT_PATH)
  deployrepo("${COMMON_DLLS}" "${EXECUTABLE_OUTPUT_PATH}")
else()
  deployrepo("${COMMON_DLLS}" "${CMAKE_CURRENT_BINARY_DIR}")
endif()
