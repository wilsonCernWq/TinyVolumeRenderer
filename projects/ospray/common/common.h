//!
//! This file contains headers of all external libraries
//!
#pragma once
#ifndef OSPRAY_COMMON_H
#define OSPRAY_COMMON_H

#define NOMINMAX

//
// include ospray
//
#include "ospray/ospray.h"
#include "ospray/ospcommon/vec.h"

//
// include cpp standard library
//
#include <cmath>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <atomic>
#include <algorithm>
#include <functional>

//
// threading
//
#include <omp.h>
#include <tbb/tbb.h>
//
// GLFW
//
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//
// GLM
//
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

//! @name error check helper from EPFL ICG class
static inline const char *ErrorString(GLenum error) {
  const char *msg;
  switch (error) {
#define Case(Token)  case Token: msg = #Token; break;
    Case(GL_INVALID_ENUM);
    Case(GL_INVALID_VALUE);
    Case(GL_INVALID_OPERATION);
    Case(GL_INVALID_FRAMEBUFFER_OPERATION);
    Case(GL_NO_ERROR);
    Case(GL_OUT_OF_MEMORY);
#undef Case
  }
  return msg;
}

//! @name check error
static inline void _glCheckError
  (const char *file, int line, const char *comment) {
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "ERROR: %s (file %s, line %i: %s).\n", comment, file, line, ErrorString(error));
  }
}

#ifndef NDEBUG
# define check_error_gl(x) _glCheckError(__FILE__, __LINE__, x)
#else
# define check_error_gl(x) ((void)0)
#endif

//! @name writePPM Helper function to write the rendered image as PPM file
inline void writePPM
  (const char *fileName, const glm::ivec2 &size, const uint32_t *pixel) {
  using namespace ospcommon;
  FILE *file = fopen(fileName, "wb");
  fprintf(file, "P6\n%i %i\n255\n", size.x, size.y);
  unsigned char *out = (unsigned char *) alloca(3 * size.x);
  for (int y = 0; y < size.y; y++) {
    const unsigned char *in =
      (const unsigned char *) &pixel[(size.y - 1 - y) * size.x];
    for (int x = 0; x < size.x; x++) {
      out[3 * x + 0] = in[4 * x + 0];
      out[3 * x + 1] = in[4 * x + 1];
      out[3 * x + 2] = in[4 * x + 2];
    }
    fwrite(out, 3 * size.x, sizeof(char), file);
  }
  fprintf(file, "\n");
  fclose(file);
}

// Timer
inline void Timer(std::string str = "") {
  static bool timing = false;
  static std::chrono::system_clock::time_point t1, t2;
  if (!timing) {
    timing = true;
    t1 = std::chrono::system_clock::now();
  } else {
    t2 = std::chrono::system_clock::now();
    std::chrono::duration<double> dur = t2 - t1;
    std::cout << "[timer] " << str << " " << dur.count() << " seconds" << std::endl;
    timing = false;
  }
}
#endif //OSPRAY_COMMON_H
