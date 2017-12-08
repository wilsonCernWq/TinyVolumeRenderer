//
// Created by qwu on 12/7/17.
//
#include "widget.h"
#include "common.h"
#include "global.h"
#include "volume.h"

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

static GLuint hist_tex = 0;
static float hist_range[4] = {0};
static float hist_percentage[4] = {0};
static bool hist_render = true;
static bool hist_reload = false;

namespace tfn {
namespace tfn_widget {
//------------------------------------------------------------------------------------------------//
// Local
//------------------------------------------------------------------------------------------------//
void RenderTFNTexture(GLuint &tex, int width, int height)
{
  GLint prevBinding = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevBinding);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if (prevBinding) {
    glBindTexture(GL_TEXTURE_2D, (GLuint)prevBinding);
  }
}
void RenderTFN() {
  if (hist_reload)
  {
    UpdateHistogram((hist_range[1] - hist_range[0]) * hist_percentage[0] * 0.01f + hist_range[0],
                    (hist_range[1] - hist_range[0]) * hist_percentage[1] * 0.01f + hist_range[0],
                    (hist_range[3] - hist_range[2]) * hist_percentage[2] * 0.01f + hist_range[2],
                    (hist_range[3] - hist_range[2]) * hist_percentage[3] * 0.01f + hist_range[2]);
    hist_reload = false;
    hist_render = true;
  }
  if (hist_tex == 0) { RenderTFNTexture(hist_tex, hist_xdim, hist_ydim); }
  if (hist_render) {
    std::vector<char> hist_data(hist.size() * 4);
    for (int i = 0; i < hist.size(); ++i) {
      hist_data[i * 4 + 0] = 255 * hist[i];
      hist_data[i * 4 + 1] = 255 * hist[i];
      hist_data[i * 4 + 2] = 255 * hist[i];
      hist_data[i * 4 + 3] = 255;
    }
    glBindTexture(GL_TEXTURE_2D, hist_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, hist_xdim, hist_ydim, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 static_cast<const void *>(hist_data.data()));
    hist_render = false;
  }
}
//------------------------------------------------------------------------------------------------//
// Global
//------------------------------------------------------------------------------------------------//
void InitUI()
{
  hist_range[0] = GetValueRangeX();
  hist_range[1] = GetValueRangeY();
  hist_range[2] = GetGradientRangeX();
  hist_range[3] = GetGradientRangeY();
  hist_percentage[0] = 0.f;
  hist_percentage[1] = 100.f;
  hist_percentage[2] = 0.f;
  hist_percentage[3] = 100.f;
}
void DrawUI() {
  RenderTFN();
  if (!ImGui::Begin("2D Transfer Function Widget")) { ImGui::End(); return; }
  ImGui::Text("2D Transfer Function");
  if (ImGui::DragFloatRange2("value range", &hist_percentage[0], &hist_percentage[1],
                             0.25f, 0.f, 100.f, "Min: %.1f %%", "Max: %.1f %%"))
  {
    hist_reload = true;
  }
  if (ImGui::DragFloatRange2("gradient range", &hist_percentage[2], &hist_percentage[3],
                             0.25f, 0.f, 100.f, "Min: %.1f %%", "Max: %.1f %%"))
  {
    hist_reload = true;
  }
  ImGui::Image(reinterpret_cast<void*>(hist_tex), ImVec2(500, 100));
  ImGui::End();
}
}
}
