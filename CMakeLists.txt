cmake_minimum_required(VERSION 3.0)
project(RayTracer)
#
# options
OPTION(EMABLE_SHARED "Enable shared Library" OFF)
if(EMABLE_SHARED) 
  set(BUILD_SHARED_LIBS ON)
else(EMABLE_SHARED) 
  set(BUILD_SHARED_LIBS OFF)
endif(EMABLE_SHARED)
#
# include host specification file
cmake_host_system_information(RESULT SYSNAME QUERY HOSTNAME)
message(STATUS "Trying to include ${SYSNAME}.cmake configuration file")
include(cmake/config-site/${SYSNAME}.cmake OPTIONAL RESULT_VARIABLE CONFIGFOUND)
if (NOT CONFIGFOUND)
  message(STATUS "Failed to load the configuration file")
  message(STATUS "Helper message:\n
    To build the program sucessfully, you might need to provide following paths to your cmake
    *) path to the root directory of your standalone TBB library as '-DTBB_ROOT=<..>'       
  ")
endif()
#
# load required libraries
include(cmake/common.cmake)
#
# load projects
set(projdir ${CMAKE_CURRENT_LIST_DIR}/projects)
file(GLOB children RELATIVE ${projdir} ${projdir}/*)
foreach(child ${children})
  if(IS_DIRECTORY ${projdir}/${child})
      add_subdirectory(${projdir}/${child})
  endif()
endforeach()
#
# install data
set(COPYLIST ${COMMON_DLLS})
if(EXECUTABLE_OUTPUT_PATH)
  DeployRepo("${COPYLIST}" "${EXECUTABLE_OUTPUT_PATH}")
else()
  DeployRepo("${COPYLIST}" "${CMAKE_BINARY_DIR}")
endif()
