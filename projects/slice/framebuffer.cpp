#include "framebuffer.hpp"

void FrameBufferObject::Init(size_t W, size_t H, size_t colorBufferNum)
{
  check_error_gl("before FBO init");
  Clean();
  fboWidth  = W;
  fboHeight = H;
  fboColorBufferNum = colorBufferNum;  
  fboColorBuffer = new GLuint[fboColorBufferNum];
  
  glGenFramebuffers(1, &framebufferID);
  glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

  GLubyte *dummyData = new GLubyte[fboWidth * fboHeight * 4]();
  glGenTextures((GLuint)fboColorBufferNum, fboColorBuffer);
  for (size_t i = 0; i < fboColorBufferNum; ++i) {    
    glBindTexture(GL_TEXTURE_2D, fboColorBuffer[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fboWidth, fboHeight,
		 0, GL_RGBA, GL_UNSIGNED_BYTE, dummyData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, fboColorBuffer[i], 0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  delete [] dummyData;
  check_error_gl("FBO color buffers");
  
  glGenRenderbuffers(1, &fboDepthBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, fboDepthBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fboWidth, fboHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			    GL_RENDERBUFFER, fboDepthBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  check_error_gl("FBO depth buffers");
    
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  { fprintf(stderr, "FBO Incomplete!\n"); exit(EXIT_FAILURE); }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  check_error_gl("FBO init complete");
}

void FrameBufferObject::Bind(size_t colorBufferNum)
{
  colorBufferNum = std::min(colorBufferNum, fboColorBufferNum);
    
  glGetIntegerv(GL_VIEWPORT, previewport);
  glViewport(0, 0, fboWidth, fboHeight);

  glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
  GLenum* DrawBuffers = new GLenum[colorBufferNum];
  for (int i = 0; i <= colorBufferNum; ++i) {
    DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
  }
  glDrawBuffers(colorBufferNum, DrawBuffers);
  delete [] DrawBuffers;
}

void FrameBufferObject::Unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(previewport[0],previewport[1],previewport[2],previewport[3]);
}
