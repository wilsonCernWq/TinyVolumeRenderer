#include "comm.hpp"

//---------------------------------------------------------------------------------------
// error check helper from EPFL ICG class
static inline const char* ErrorString(GLenum error) {
  const char* msg;
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

void _glCheckError(const char* file, int line, const char* comment) {
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "ERROR: %s (file %s, line %i: %s).\n",
	    comment, file, line, ErrorString(error));
  }
}
