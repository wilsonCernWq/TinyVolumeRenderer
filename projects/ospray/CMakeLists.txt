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
if(TFN_MODULE_ROOT)
  include_directories(${TFN_MODULE_ROOT}/src)
  set(TFN_MODULE_SRC
    ${TFN_MODULE_ROOT}/src/TransferFunctionModule.cpp
    ${TFN_MODULE_ROOT}/src/widgets/TransferFunctionWidget.cpp)
  add_definitions(-DUSE_TFN_MODULE)
  mark_as_advanced(TFN_MODULE_SRC)
endif(TFN_MODULE_ROOT)
#
# include sources
file(GLOB_RECURSE CXX "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB_RECURSE HXX "${CMAKE_CURRENT_LIST_DIR}/*.h")
add_executable(ospVolume ${CXX} ${HXX} ${TFN_MODULE_SRC})
target_link_libraries(ospVolume ${COMMON_LIBS})
set_target_properties(ospVolume
  PROPERTIES
  CXX_STANDARD 11)
