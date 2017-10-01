#
# build LodePNG
#
ADD_LIBRARY(lodepng
   lodepng.cpp
   lodepng.h)
ADD_LIBRARY(lodepng_util
  lodepng_util.cpp
  lodepng_util.h)
TARGET_LINK_LIBRARIES(lodepng_util lodepng)
SET(LodePNG_LIBRARIES lodepng lodepng_util PARENT_SCOPE)
SET(LodePNG_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR} PARENT_SCOPE)

