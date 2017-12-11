//
// Created by qwu on 12/7/17.
//
#include "widget.h"
#include "common/common.h"
#include "global.h"
#include <imgui.h>

using vec2f = glm::vec2;
using vec3f = glm::vec3;

//------------------------------------------------------------------------------------------------//
//
//------------------------------------------------------------------------------------------------//
static bool hist_render = true;
static GLuint hist_tex[2] = {0,0}; // openGL texture storages for f'  vs f, f'' vs f 
static float  hist_clamp[2] = {100.f, 100.f}; // histogram maximum count / histogram real maximum
static float  hist_gamma[2] = {1.f, 1.f};     // gamma correction
static float  hist_range[6] = {0.f}; // histogram value range

//------------------------------------------------------------------------------------------------//
//
//------------------------------------------------------------------------------------------------//
static bool array_render = true;
static vec2f array1d_range;
static vec2f array2d_range;
static std::vector<float> array1d_p;
static std::vector<float> array2d_p;

static float g_threshold = 0.f;
static float sigma = 1.f;

//------------------------------------------------------------------------------------------------//
template<typename T> T clamp(T v, T l, T u) { return std::min(u, std::max(l, v)); }
//------------------------------------------------------------------------------------------------//
namespace tfn {
namespace tfn_widget {
//------------------------------------------------------------------------------------------------//
// Auto-Classification
//------------------------------------------------------------------------------------------------//
float OpacityFunction(float x)
{
  if (x < 0) return std::max(x + sigma, 0.f);
  else return  std::max(-x + sigma, 0.f);
}
void GenerateClassification()
{
  array1d_p.resize(volume.GetHistogram().HistXDim());
  {
    tbb::parallel_for(size_t(0), volume.GetHistogram().HistXDim(), [&](size_t ix) {
      float ag = 0.f, aa = 0.f;
      size_t n = 0;
      for (size_t k = 0; k < volume.GetHistogram().HistCountYZ(); ++k) {
        const size_t iy = k % volume.GetHistogram().HistYDim();
        const size_t iz = k / volume.GetHistogram().HistYDim();
        const float g = volume.SampleHistY(iy);
        const float a = volume.SampleHistZ(iz);
        const size_t m = volume.GetHistogram().At(ix, iy, iz);
        ag += g * m;
        aa += a * m;
        n += m;
      }
      ag /= n;
      aa /= n;
      array1d_p[ix] = OpacityFunction(sigma * sigma * aa / (std::max(ag - g_threshold, 0.f)));
      array1d_range.x = std::min(array1d_p[ix], array1d_range.x);
      array1d_range.y = std::max(array1d_p[ix], array1d_range.y);
    });
  }
  array2d_p.resize(volume.GetHistogram().HistCountXY());
  {
    for (size_t k = 0; k < volume.GetHistogram().HistCountXY(); ++k) {
      const size_t ix = k % volume.GetHistogram().HistXDim();
      const size_t iy = k / volume.GetHistogram().HistXDim();
      const float g = volume.SampleHistY(iy);
      float aa = 0.f;
      size_t n = 0;
      for (size_t iz = 0; iz < volume.GetHistogram().HistZDim(); ++iz) {
        const float a = volume.SampleHistZ(iz);
        const size_t m = volume.GetHistogram().At(ix, iy, iz);
        aa += a * m;
        n += m;
      }
      aa /= n;
      array2d_p[k] = OpacityFunction(sigma * sigma * aa / (std::max(g - g_threshold, 0.f)));
      array2d_range.x = std::min(array2d_p[k], array2d_range.x);
      array2d_range.y = std::max(array2d_p[k], array2d_range.y);
    }
  }
  const std::vector<float> colors =
    {
      0.00f, 0.00f, 1.00f,
      0.00f, 0.25f, 0.75f,
      0.00f, 0.50f, 0.50f,
      0.00f, 0.75f, 0.25f,
      0.00f, 1.00f, 0.00f,
      0.25f, 0.75f, 0.00f,
      0.50f, 0.50f, 0.00f,
      0.75f, 0.25f, 0.00f,
      1.00f, 0.00f, 0.00f,
    };
  volume.GetTransferFunction().Update(colors.data(), array1d_p.data(), colors.size() / 3, 1, array1d_p.size(), 1);
  ClearOSPRay();
}
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
  if (array_render)
  {
    GenerateClassification();
    array_render = false;
  }
}
void DrawHistogramInfo()
{
  if (!ImGui::Begin("Histogram Volume Information")) {
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
  if (ImGui::SliderFloat("##clamp-1", &hist_clamp[1], 0.1f, 100.f, "f'' Max Clamp: %.1f%%")) {
    hist_render = true;
  }
  //----------------------------------------------------------------------------------------------//
  // Preparation
  //----------------------------------------------------------------------------------------------//
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  float canvas_x = ImGui::GetCursorScreenPos().x;
  float canvas_y = ImGui::GetCursorScreenPos().y;
  float canvas_avail_x = ImGui::GetContentRegionAvail().x;
  float canvas_avail_y = ImGui::GetContentRegionAvail().y;
  const float mouse_x = ImGui::GetMousePos().x;
  const float mouse_y = ImGui::GetMousePos().y;
  const float scroll_x = ImGui::GetScrollX();
  const float scroll_y = ImGui::GetScrollY();
  const float margin = 5.f;
  //----------------------------------------------------------------------------------------------//
  // Draw Images
  //----------------------------------------------------------------------------------------------//
  {
    canvas_y += margin;
    const float image_size  = (canvas_avail_x - margin) / 2;
    ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
    ImGui::Image(reinterpret_cast<void *>(hist_tex[0]), ImVec2(image_size, image_size));
    ImGui::SameLine();
    ImGui::SetCursorScreenPos(ImVec2(canvas_x + margin + image_size, canvas_y));
    ImGui::Image(reinterpret_cast<void *>(hist_tex[1]), ImVec2(image_size, image_size));
    canvas_y += image_size;
  }
  ImGui::End();
}
void DrawTransferFunction()
{
  if (!ImGui::Begin("Semi-Automatic TFN Widget")) {
    ImGui::End();
    return;
  }
  if (ImGui::SliderFloat("sigma", &sigma, 0.f, 100.f, "%.4f"))
  {
    array_render = true;
  }
  if (ImGui::SliderFloat("g_threshold", &g_threshold,
                         volume.Get1stGradientRangeX(),
                         volume.Get1stGradientRangeY(), "%.4f"))
  {
    array_render = true;
  }
  // Preparation
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  float canvas_x = ImGui::GetCursorScreenPos().x;
  float canvas_y = ImGui::GetCursorScreenPos().y;
  float canvas_avail_x = ImGui::GetContentRegionAvail().x;
  float canvas_avail_y = ImGui::GetContentRegionAvail().y;
  const float mouse_x = ImGui::GetMousePos().x;
  const float mouse_y = ImGui::GetMousePos().y;
  const float scroll_x = ImGui::GetScrollX();
  const float scroll_y = ImGui::GetScrollY();
  const float margin = 5.f;
  // Draw 1D array
  {
    canvas_y += margin;
    const float scale = 1.f / std::max(std::abs(array1d_range.y), std::abs(array1d_range.x));
    const float width  = (canvas_avail_x - margin) / 2.f - 1;
    const float height = 40.f;
    const float offset_x = canvas_x;
    const float offset_y = canvas_y + height;
    ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
    draw_list->AddLine(ImVec2(offset_x, offset_y), ImVec2(offset_x + width + 1, offset_y), 0xFF0000FF, 1);
    draw_list->AddLine(ImVec2(offset_x, offset_y - height), ImVec2(offset_x, offset_y + height), 0xFF0000FF, 1);
    draw_list->AddTriangleFilled(ImVec2(offset_x, offset_y - height),
                                 ImVec2(offset_x-3.f, offset_y - height+3.f),
                                 ImVec2(offset_x+3.f, offset_y - height+3.f),
                                 0xFF0000FF);
    for (int i = 0; i < array1d_p.size()-1; ++i)
    {
      std::vector<ImVec2> polyline;
      const float x0 = (float)i     / (array1d_p.size() - 1);
      const float y0 = array1d_p[i  ] * scale;
      const float x1 = (float)(i+1) / (array1d_p.size() - 1);
      const float y1 = array1d_p[i+1] * scale;
      polyline.emplace_back(offset_x + x0 * width,     offset_y);
      polyline.emplace_back(offset_x + x0 * width,     offset_y - y0 * height);
      polyline.emplace_back(offset_x + x1 * width + 1, offset_y - y1 * height);
      polyline.emplace_back(offset_x + x1 * width + 1, offset_y);
      draw_list->AddConvexPolyFilled(polyline.data(), (int)polyline.size(), 0x1FD8D8D8, true);
    }
    canvas_y += height * 2.f;
  }
  ImGui::End();
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
  array1d_range = vec2f(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
  array2d_range = vec2f(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
}
void DrawUI() {
  RenderTFN();
  DrawHistogramInfo();
  DrawTransferFunction();
}
}
}
