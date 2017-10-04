#pragma once
#include "comm.hpp"

class FrameBufferObject {
private:
  size_t fboWidth = 640, fboHeight = 480;
  GLuint  framebufferID  = 0;
  GLuint* fboColorBuffer = NULL;
  GLuint  fboDepthBuffer = 0;
  GLint   previewport[4];
  size_t  fboColorBufferNum = 0;
public:
  FrameBufferObject() = default;
  ~FrameBufferObject() { if (fboColorBuffer) delete [] fboColorBuffer; }  
  void Init(size_t, size_t, size_t);
  void Bind(size_t);
  void BindSingle(size_t);
  void Unbind();
  GLuint GetID() { return framebufferID; }
  GLuint GetColor(size_t i) { return fboColorBuffer[i]; }
  GLuint GetDepth()         { return fboDepthBuffer;    }
  void Clean() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
    glDeleteFramebuffers(1, &framebufferID);
    glDeleteRenderbuffers(1, &fboDepthBuffer);
    if (fboColorBuffer) {
      glDeleteTextures(fboColorBufferNum, fboColorBuffer);
      delete [] fboColorBuffer;
      fboColorBuffer = NULL;
    }
  }
};
