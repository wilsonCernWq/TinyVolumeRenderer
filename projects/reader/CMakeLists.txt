#
# initialize project
get_filename_component(PID ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(TOLOWER ${PID} PID)
#
# include sources
file(GLOB CXX  "*.cpp")
file(GLOB HXX  "*.hpp")
#
# add library
add_library(volume_reader ${CXX} ${HXX})
target_link_libraries(volume_reader ${COMMON_LIBS})
set_target_properties(volume_reader
  PROPERTIES
  CXX_STANDARD 11)
SET(VolumeReader_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR} PARENT_SCOPE)
SET(VolumeReader_LIBRARIES volume_reader PARENT_SCOPE)
