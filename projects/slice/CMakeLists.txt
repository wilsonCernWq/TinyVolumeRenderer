#
# initialize project
get_filename_component(PID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(TOLOWER ${PID} PID)
#
# install dlls or shader
file(GLOB SHADERS "shaders/*.glsl")
set(COPYLIST ${SHADERS})
if(EXECUTABLE_OUTPUT_PATH)
  DeployRepo("${COPYLIST}" "${EXECUTABLE_OUTPUT_PATH}/${PID}")
else()
  DeployRepo("${COPYLIST}" "${CMAKE_BINARY_DIR}/${PID}")
endif()
#
# include sources
file(GLOB MAIN "main.cpp")
file(GLOB CXX  "*.cpp")
file(GLOB HXX  "*.hpp")
foreach(f ${MAIN})
  list(REMOVE_ITEM CXX ${f})
endforeach()
include_directories(${VolumeReader_INCLUDE_DIRS})
#
# add library
add_library(common_${PID} ${CXX} ${HXX})
target_link_libraries(common_${PID} ${VolumeReader_LIBRARIES} ${COMMON_LIBS})
set_target_properties(common_${PID}
  PROPERTIES
  CXX_STANDARD 11)
#
# add executable
add_executable(viewer_${PID} ${MAIN})
target_link_libraries(viewer_${PID} common_${PID} ${VolumeReader_LIBRARIES} ${COMMON_LIBS})
set_target_properties(viewer_${PID}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  CXX_STANDARD 11)
