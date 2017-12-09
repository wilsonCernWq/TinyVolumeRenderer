#pragma once
#ifndef OSPRAY_FRAMEBUFFER_H
#define OSPRAY_FRAMEBUFFER_H

#include "common/common.h"

class Framebuffer {
private:
  OSPFrameBuffer ospFB = nullptr;
  uint32_t *mapped_ptr;
  uint8_t frame_idx = 0;
  std::vector<uint32_t> frame_data[2];
  GLuint texID;
  GLuint fboID;
  size_t W, H;
public:

  OSPFrameBuffer OSPRayPtr() { return ospFB; }

  void Init(size_t width, size_t height) {
    check_error_gl("Initializing OpenGL texture in 'framebuffer.h'");
    glGenFramebuffers(1, &fboID);
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    Resize(width, height);
    glBindTexture(GL_TEXTURE_2D, texID);
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    check_error_gl("Successfully initialized OpenGL texture");
  }

  void Clean() {
    if (ospFB != nullptr) {
      ospUnmapFrameBuffer(mapped_ptr, ospFB);
      ospFreeFrameBuffer(ospFB);
      ospFB = nullptr;
    }
  }

  void Upload() {
    if (ospFB != nullptr) {
      glBindTexture(GL_TEXTURE_2D, texID);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, frame_data[frame_idx].data());
      glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID);
      glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_COLOR_BUFFER_BIT, GL_NEAREST);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }

  void Swap() {
    uint8_t process_idx = (frame_idx == 0) ? uint8_t(1) : uint8_t(0);
    std::copy(mapped_ptr, mapped_ptr + W * H, frame_data[process_idx].data());
    frame_idx = process_idx;
  }

  void CleanBuffer() {
    ospFrameBufferClear(ospFB, OSP_FB_COLOR | OSP_FB_ACCUM);
  }

  void Resize(size_t width, size_t height) {
    W = width;
    H = height;
    // Resize OpenGL FB
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Resize OSPRay FB
    Clean();
    ospFB = ospNewFrameBuffer(osp::vec2i{(int) W, (int) H}, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
    ospFrameBufferClear(ospFB, OSP_FB_COLOR | OSP_FB_ACCUM);
    mapped_ptr = (uint32_t *) ospMapFrameBuffer(ospFB, OSP_FB_COLOR);
    frame_data[0].resize(W * H, uint32_t(0));
    frame_data[1].resize(W * H, uint32_t(0));
    frame_idx = 0;
  }
};

#endif //OSPRAY_FRAMEBUFFER_H