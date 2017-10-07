#pragma once
#include "comm.hpp"

class FrameBufferObject {
private:
  size_t  fboWidth = 640, fboHeight = 480;
  size_t  fboColorBufferNum = 0;
  GLuint  framebufferID  = 0;
  GLuint* fboColorBuffer = NULL;
  GLuint  fboDepthBuffer = 0;
  GLint   previewport[4];
public:
  FrameBufferObject() = default;
  ~FrameBufferObject() { if (fboColorBuffer) delete [] fboColorBuffer; }  
  void Init(size_t, size_t, size_t);
  void BindMultiple(size_t, size_t);
  void BindSingle(size_t);
  void UnBindAll();
  void Clean();
  void Reset();
  GLuint GetID() { return framebufferID; }
  GLuint GetColor(size_t i) { return fboColorBuffer[i]; }
  GLuint GetDepth()         { return fboDepthBuffer;    }
};
