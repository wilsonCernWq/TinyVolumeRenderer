//
// Created by qwu on 12/7/17.
//
#include "widget.h"
#include "common/common.h"
#include "global.h"
#include <imgui.h>

static bool hist_render = true;
static GLuint hist_tex[2] = {0,0}; // f'  vs f, f'' vs f
static float  hist_clamp[2] = {100.f, 100.f};
static float  hist_gamma[2] = {1.f, 1.f};
static float  hist_range[6] = {0.f};

template<typename T> T clamp(T v, T l, T u) { return std::min(u, std::max(l, v)); }

namespace tfn {
namespace tfn_widget {
//------------------------------------------------------------------------------------------------//
// Local
//------------------------------------------------------------------------------------------------//
void CreateTFNTexture(GLuint &tex, size_t width, size_t height) {
  GLint prevBinding = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevBinding);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, (GLuint) width, (GLuint) height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  if (prevBinding) { glBindTexture(GL_TEXTURE_2D, (GLuint) prevBinding); }
}
void RenderTFNTexture(GLuint &tex, const std::vector <size_t>& counts,
                      size_t width, size_t height, float upper, float gamma)
{
  std::vector <uint8_t> tex_data(width * height * 3);
  for (int k = 0; k < width * height; ++k) {
    const float ratio = std::pow(clamp(static_cast<float>(counts[k]) / upper, 0.f, 1.f), gamma);
    const auto value = static_cast<uint8_t>(255 * ratio);
    tex_data[3 * k + 0] = value;
    tex_data[3 * k + 1] = value;
    tex_data[3 * k + 2] = value;
  }
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, (GLuint) width, (GLuint) height, 0, GL_RGB, GL_UNSIGNED_BYTE,
               static_cast<const void *>(tex_data.data()));
}
void RenderTFN() {
  if (hist_tex[0] == 0) {
    CreateTFNTexture(hist_tex[0],
                     volume.GetHistogram().HistXDim(),
                     volume.GetHistogram().HistYDim());
  }
  if (hist_tex[1] == 0) {
    CreateTFNTexture(hist_tex[1],
                     volume.GetHistogram().HistXDim(),
                     volume.GetHistogram().HistZDim());
  }
  if (hist_render) {
    //---------------------------------------------------------------------------------------------------------------//
    {
      std::vector <size_t> hist_count(volume.GetHistogram().HistCountXY(),0);
      size_t hist_max(0);
      for (int k = 0; k < volume.GetHistogram().HistCountXY(); ++k) {
        const size_t ix = k % volume.GetHistogram().HistXDim();
        const size_t iy = k / volume.GetHistogram().HistXDim();
        const float v = volume.SampleHistX(ix);
        const float g = volume.SampleHistY(iy);
        size_t count = 0;
        if (v >= hist_range[0] && v <= hist_range[1] && g >= hist_range[2] && g <= hist_range[3])
        {
          for (size_t iz = 0; iz < volume.GetHistogram().HistZDim(); ++iz) {
            count += volume.GetHistogram().At(ix, iy, iz);
          }
          hist_max = std::max(hist_max, count);
        }
        hist_count[k] = count;
      }
      RenderTFNTexture(hist_tex[0], hist_count,
                       volume.GetHistogram().HistXDim(),
                       volume.GetHistogram().HistYDim(),
                       0.01f * hist_clamp[0] * hist_max, hist_gamma[0]);
    }
    //---------------------------------------------------------------------------------------------------------------//
    {
      std::vector <size_t> hist_count(volume.GetHistogram().HistCountXZ(), 0);
      size_t hist_max(0);
      for (int k = 0; k < volume.GetHistogram().HistCountXZ(); ++k) {
        const size_t ix = k % volume.GetHistogram().HistXDim();
        const size_t iz = k / volume.GetHistogram().HistXDim();
        const float v = volume.SampleHistX(ix);
        const float a = volume.SampleHistZ(iz);
        size_t count = 0;
        if (v >= hist_range[0] && v <= hist_range[1] && a >= hist_range[4] && a <= hist_range[5])
        {
          for (size_t iy = 0; iy < volume.GetHistogram().HistYDim(); ++iy) {
            count += volume.GetHistogram().At(ix, iy, iz);
          }
          hist_max = std::max(hist_max, count);
        }
        hist_count[k] = count;
      }
      RenderTFNTexture(hist_tex[1], hist_count,
                       volume.GetHistogram().HistXDim(),
                       volume.GetHistogram().HistZDim(),
                       0.01f * hist_clamp[1] * hist_max, hist_gamma[1]);
    }
    //---------------------------------------------------------------------------------------------------------------//
    hist_render = false;
  }
}
//------------------------------------------------------------------------------------------------//
// Global
//------------------------------------------------------------------------------------------------//
void InitUI() {
  hist_range[0] = volume.GetValueRangeX();
  hist_range[1] = volume.GetValueRangeY();
  hist_range[2] = volume.Get1stGradientRangeX();
  hist_range[3] = volume.Get1stGradientRangeY();
  hist_range[4] = volume.Get2ndGradientRangeX();
  hist_range[5] = volume.Get2ndGradientRangeY();
}
void DrawUI() {
  RenderTFN();
  if (!ImGui::Begin("2D Transfer Function Widget")) {
    ImGui::End();
    return;
  }
  ImGui::Text("2D Transfer Function");
  if (ImGui::DragFloatRange2("f    range", &hist_range[0], &hist_range[1], 0.25f,
                             volume.GetValueRangeX(),
                             volume.GetValueRangeY(),
                             "Min: %.1f", "Max: %.1f"))
  {
    hist_render = true;
  }
  if (ImGui::DragFloatRange2("f'  range", &hist_range[2], &hist_range[3], 0.25f,
                             volume.Get1stGradientRangeX(),
                             volume.Get1stGradientRangeY(),
                             "Min: %.1f", "Max: %.1f"))
  {
    hist_render = true;
  }
  if (ImGui::DragFloatRange2("f'' range", &hist_range[4], &hist_range[5], 0.25f,
                             volume.Get2ndGradientRangeX(),
                             volume.Get2ndGradientRangeY(),
                             "Min: %.1f", "Max: %.1f"))
  {
    hist_render = true;
  }
  if (ImGui::SliderFloat("##gamma-0", &hist_gamma[0], 0.001f, 1.f, "f'  Gamma: %.3f"))
  {
    hist_render = true;
  }
  if (ImGui::SliderFloat("##gamma-1", &hist_gamma[1], 0.001f, 1.f, "f'' Gamma: %.3f"))
  {
    hist_render = true;
  }
  if (ImGui::SliderFloat("##clamp-0", &hist_clamp[0], 0.1f, 100.f, "f'  Max Clamp: %.1f%%"))
  {
    hist_render = true;
  }
  if (ImGui::SliderFloat("##clamp-1", &hist_clamp[1], 0.1f, 100.f, "f'' Max Clamp: %.1f%%"))
  {
    hist_render = true;
  }
  ImGui::Image(reinterpret_cast<void *>(hist_tex[0]), ImVec2(200, 200));
  ImGui::SameLine();
  ImGui::Image(reinterpret_cast<void *>(hist_tex[1]), ImVec2(200, 200));
  ImGui::End();
}
}
}
