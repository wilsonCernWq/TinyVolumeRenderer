//
// Created by qwu on 12/7/17.
//
#include "widget.h"
#include "common.h"
#include "global.h"
#include "volume.h"

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

static GLuint hist_tex0 = 0; // f'  vs f
static GLuint hist_tex1 = 0; // f'' vs f
static bool hist_render = true;
static float hist_range[6] = {0.f};

namespace tfn {
namespace tfn_widget {
//------------------------------------------------------------------------------------------------//
// Local
//------------------------------------------------------------------------------------------------//
void RenderTFNTexture(GLuint &tex, size_t width, size_t height) {
  GLint prevBinding = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevBinding);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLuint) width, (GLuint) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  if (prevBinding) { glBindTexture(GL_TEXTURE_2D, (GLuint) prevBinding); }
}
void RenderTFN() {
  if (hist_tex0 == 0) { RenderTFNTexture(hist_tex0, histXDim, histYDim); }
  if (hist_tex1 == 0) { RenderTFNTexture(hist_tex1, histXDim, histZDim); }
  if (hist_render) {
    std::vector <size_t> hist_tex0_count(histXDim * histYDim, 0);
    size_t hist_tex0_max(0);
    for (int k = 0; k < histXDim * histYDim; ++k) {
      const size_t ix = k % histXDim;
      const size_t iy = k / histXDim;
      const float v = ix * (GetValueRangeY() - GetValueRangeX()) / histXDim + GetValueRangeX();
      const float g = iy * (Get1stGradientRangeY() - Get1stGradientRangeX()) / histYDim + Get1stGradientRangeX();
      size_t count = 0;
      if (v >= hist_range[0] && v <= hist_range[1] &&
          g >= hist_range[2] && g <= hist_range[3])
      {
        for (size_t iz = 0; iz < histZDim; ++iz) {
          count += histVolumeAccess(ix, iy, iz);
        }
        hist_tex0_max = std::max(hist_tex0_max, count);
      }
      hist_tex0_count[k] = count;
    }
    std::vector <uint8_t> hist_tex0_data(histXDim * histYDim * 4);
    for (int k = 0; k < histXDim * histYDim; ++k) {
      uint8_t value = static_cast<uint8_t>(255 * static_cast<float>(hist_tex0_count[k]) / hist_tex0_max);
      hist_tex0_data[4 * k + 0] = value;
      hist_tex0_data[4 * k + 1] = value;
      hist_tex0_data[4 * k + 2] = value;
      hist_tex0_data[4 * k + 3] = 255;
    }
    glBindTexture(GL_TEXTURE_2D, hist_tex0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLuint) histXDim, (GLuint) histYDim, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 static_cast<const void *>(hist_tex0_data.data()));
    hist_render = false;
  }
}
//------------------------------------------------------------------------------------------------//
// Global
//------------------------------------------------------------------------------------------------//
void InitUI() {
  hist_range[0] = GetValueRangeX();
  hist_range[1] = GetValueRangeY();
  hist_range[2] = Get1stGradientRangeX();
  hist_range[3] = Get1stGradientRangeY();
  hist_range[4] = Get2ndGradientRangeX();
  hist_range[5] = Get2ndGradientRangeY();
}
void DrawUI() {
  RenderTFN();
  if (!ImGui::Begin("2D Transfer Function Widget")) {
    ImGui::End();
    return;
  }
  ImGui::Text("2D Transfer Function");
  if (ImGui::DragFloatRange2("value range", &hist_range[0], &hist_range[1],
                             0.25f, GetValueRangeX(), GetValueRangeY(), "Min: %.1f", "Max: %.1f"))
  {
    hist_render = true;
  }
  if (ImGui::DragFloatRange2("gradient range", &hist_range[2], &hist_range[3],
                             0.25f, Get1stGradientRangeX(), Get1stGradientRangeY(), "Min: %.1f", "Max: %.1f"))
  {
    hist_render = true;
  }
  glBindTexture(GL_TEXTURE_2D, hist_tex0);
  ImGui::Image(reinterpret_cast<void *>(hist_tex0), ImVec2(300, 300));
  ImGui::End();
}
}
}
