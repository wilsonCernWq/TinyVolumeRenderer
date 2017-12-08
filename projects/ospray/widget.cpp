//
// Created by qwu on 12/7/17.
//
#include "widget.h"

#include "common.h"
#include "global.h"

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

static GLuint hist_tex = 0;
static bool hist_changed = true;

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
	glBindTexture(GL_TEXTURE_2D, prevBinding);
      }
    }
    //------------------------------------------------------------------------------------------------//
    // Global
    //------------------------------------------------------------------------------------------------//
    void InitUI() {}
    void DrawUI()
    {
      if (!ImGui::Begin("2D Transfer Function Widget")) {
	ImGui::End();
	return;
      }
      ImGui::Text("2D Transfer Function");
      if (hist_tex == 0)
      {
	RenderTFNTexture(hist_tex, hist_xdim, hist_ydim);
      }
      if (hist_changed)
      {
	std::vector<char> hist_data(hist.size() * 4);
	for (int i = 0; i < hist.size(); ++i)
	{
	  hist_data[i * 4 + 0] = 255 * hist[i];
	  hist_data[i * 4 + 1] = 255 * hist[i];
	  hist_data[i * 4 + 2] = 255 * hist[i];
	  hist_data[i * 4 + 3] = 255;
	}
	glBindTexture(GL_TEXTURE_2D, hist_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, hist_xdim, hist_ydim, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		     static_cast<const void *>(hist_data.data()));
	hist_changed = false;
      }
      ImGui::Image(reinterpret_cast<void *>(hist_tex), ImVec2(500, 100));
      ImGui::End();
    }
  }
}
