//
// Created by qwu on 12/9/17.
//

#include "transferfunction.h"

void Transferfunction::Init(const glm::vec2& r) {
  range = r;
  ospTfn = ospNewTransferFunction("piecewise_linear_2d");
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
  const std::vector<float> opacities = {1.0f, 0.8f, 0.6f, 0.4f, 0.2f, 0.0f};
  Update(colors.data(), opacities.data(), colors.size() / 3, 1, opacities.size(), 1);
}
void Transferfunction::Update(const void *colors,
                              const void *opacities,
                              int colorW, int colorH,
                              int opacityW, int opacityH)
{
  OSPData colorsData = ospNewData(colorW * colorH, OSP_FLOAT3, colors);
  ospCommit(colorsData);
  OSPData opacitiesData = ospNewData(opacityW * opacityH, OSP_FLOAT, opacities);
  ospCommit(opacitiesData);
  ospSetData(ospTfn, "colors", colorsData);
  ospSetData(ospTfn, "opacities", opacitiesData);
  ospSetVec2f(ospTfn, "valueRange", osp::vec2f{range.x, range.y});
  ospSet1i(ospTfn, "colorWidth", colorW);
  ospSet1i(ospTfn, "colorHeight", colorH);
  ospSet1i(ospTfn, "opacityWidth", opacityW);
  ospSet1i(ospTfn, "opacityHeight", opacityH);
  ospCommit(ospTfn);
  ospRelease(colorsData);
  ospRelease(opacitiesData);
}