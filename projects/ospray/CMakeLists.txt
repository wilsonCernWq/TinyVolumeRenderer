cmake_minimum_required(VERSION 2.8)
project(ospVolume)
#
# load OSPRay
FIND_PACKAGE(ospray REQUIRED)
INCLUDE_DIRECTORIES(${OSPRAY_INCLUDE_DIRS})
LIST(APPEND COMMON_LIBS ${OSPRAY_LIBRARIES})
MESSAGE(STATUS "OSPRAY_LIBRARIES ${OSPRAY_LIBRARIES}")
#
# TFN module
if (TFN_MODULE_ROOT)
  include_directories(${TFN_MODULE_ROOT}/src)
  set(TFN_MODULE_SRC
    ${TFN_MODULE_ROOT}/src/TransferFunctionModule.cpp
    ${TFN_MODULE_ROOT}/src/TransferFunctionModule.h
    ${TFN_MODULE_ROOT}/src/widgets/TransferFunctionWidget.cpp
    ${TFN_MODULE_ROOT}/src/widgets/TransferFunctionWidget.h common/volume.h)
  add_definitions(-DUSE_TFN_MODULE)
  mark_as_advanced(TFN_MODULE_SRC)
endif (TFN_MODULE_ROOT)
#
# include sources
include_directories(${CMAKE_CURRENT_LIST_DIR})
file(GLOB_RECURSE CXX "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB_RECURSE HXX "${CMAKE_CURRENT_LIST_DIR}/*.h")
add_executable(viewer_ospray
  main.cpp
  callback.cpp callback.h
  widget.cpp widget.h
  global.cpp global.h
  common/volume.cpp common/volume.h
  common/transferfunction.cpp common/transferfunction.h
  common/camera.h
  common/framebuffer.h
  common/trackball.h
  common/common.h
  ${TFN_MODULE_SRC})
target_link_libraries(viewer_ospray volume_reader ${COMMON_LIBS})
set_target_properties(viewer_ospray
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  CXX_STANDARD 11)
