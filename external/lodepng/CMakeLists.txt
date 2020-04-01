# LodePNG
add_library(lodepng
  lodepng.cpp
  lodepng.h)
add_library(lodepng_util
  lodepng_util.cpp
  lodepng_util.h)
target_link_libraries(lodepng_util lodepng)
set(LodePNG_LIBRARIES lodepng lodepng_util PARENT_SCOPE)
set(LodePNG_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} PARENT_SCOPE)

