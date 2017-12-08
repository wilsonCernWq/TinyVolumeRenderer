#include <limits>
#include <tbb/tbb.h>
#include "common.h"
#include "global.h"
#include "../reader/volume_reader.hpp"

void Timer(std::string str = "")
{
  static bool timing = false;
  static std::chrono::system_clock::time_point t1, t2;
  if (!timing) {
    timing = true;
    t1 = std::chrono::system_clock::now();
  }
  else {
    t2 = std::chrono::system_clock::now();
    std::chrono::duration<double> dur = t2 - t1;
    std::cout << str << " " << dur.count() << " seconds" << std::endl;
    timing = false;
  }
}

void SetupTF(const void *colors, const void *opacities, int colorW, int colorH, int opacityW, int opacityH)
{
  //! setup trasnfer function
  OSPData colorsData = ospNewData(colorW * colorH, OSP_FLOAT3, colors);
  ospCommit(colorsData);
  OSPData opacitiesData = ospNewData(opacityW * opacityH, OSP_FLOAT, opacities);
  ospCommit(opacitiesData);
  ospSetData(transferFcn, "colors", colorsData);
  ospSetData(transferFcn, "opacities", opacitiesData);
  ospSetVec2f(transferFcn, "valueRange",
              osp::vec2f{static_cast<float>(0),
                         static_cast<float>(255)});
  ospSet1i(transferFcn, "colorWidth", colorW);
  ospSet1i(transferFcn, "colorHeight", colorH);
  ospSet1i(transferFcn, "opacityWidth", opacityW);
  ospSet1i(transferFcn, "opacityHeight", opacityH);
  ospCommit(transferFcn);
  ospRelease(colorsData);
  ospRelease(opacitiesData);
}

void volume(int argc, const char **argv)
{
  int data_type = 0, data_size = 0;
  ospcommon::vec3i dims;
  void*  volumeData = nullptr;
  float* gradientData = nullptr;

  Timer("");
  ReadVolume(argv[1], data_type, data_size, dims.x, dims.y, dims.z, volumeData);
  Timer("load data");

  gradientData = new float [dims.x * dims.y * dims.z];
  ospcommon::vec2f vRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
  ospcommon::vec2f gRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());

  cleanlist.push_back([=]() { delete[] (char*)volumeData; delete[] gradientData; });

  // gradient
  Timer("");
  tbb::parallel_for(0, dims.x * dims.y * dims.z, [&](size_t k) {
    size_t x = k % dims.x;
    size_t y = (k % (dims.x * dims.y)) / dims.x;
    size_t z = k / (dims.x * dims.y);
    auto i = z * dims.y * dims.x + y * dims.x + x;
    auto v = ReadAs<float>(volumeData, i, data_type);
    float idx = x == (dims.x - 1) ? -1 : 1;
    float idy = y == (dims.y - 1) ? -dims.x : dims.x;
    float idz = z == (dims.z - 1) ? -dims.x * dims.y : dims.x * dims.y;
    auto dx = (ReadAs<float>(volumeData, i+idx, data_type) - v) / idx;
    auto dy = (ReadAs<float>(volumeData, i+idy, data_type) - v) / idy;
    auto dz = (ReadAs<float>(volumeData, i+idz, data_type) - v) / idz;
    auto g = sqrtf(dx * dx + dy * dy + dz * dz);
    vRange.x = std::min(vRange.x, v);
    vRange.y = std::max(vRange.y, v);
    gRange.x = std::min(gRange.x, g);
    gRange.y = std::max(gRange.y, g);
  });
  Timer("compute gradient");

  //! calculate hist
  Timer("");
  float hist_max = 0.f;
  hist.resize(hist_xdim * hist_ydim, 0);
  for (int x = 0; x < dims.x; ++x) {
    for (int y = 0; y < dims.y; ++y) {
      for (int z = 0; z < dims.z; ++z) {
        const int i = z * dims.y * dims.x + y * dims.x + x;
        const float v = ReadAs<float>(volumeData, i, data_type);
        const float g = gradientData[i];
        const int ix = round((hist_xdim - 1) * (v - vRange.x) / (vRange.y - vRange.x));
        const int iy = round((hist_ydim - 1) * (g - gRange.x) / (gRange.y - gRange.x));
        auto& hv = hist[iy * hist_xdim + ix];
        hv += 1.f;
        hist_max = std::max(hist_max, hv);
      }
    }
  }
  tbb::parallel_for(0, hist_xdim * hist_ydim, [&](size_t k) { hist[k] /= hist_max; });
  Timer("compute histogram");

  //! transfer function
  const std::vector<ospcommon::vec3f> colors =
  {
  ospcommon::vec3f(0, 0, 0.563),
  ospcommon::vec3f(0, 0, 1),
  ospcommon::vec3f(0, 1, 1),
  ospcommon::vec3f(0.5, 1, 0.5),
  ospcommon::vec3f(1, 1, 0),
  ospcommon::vec3f(1, 0, 0),
  ospcommon::vec3f(0.5, 0, 0),
  };
  const std::vector<float> opacities = {0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f};
  SetupTF(colors.data(), opacities.data(), colors.size(), 1, opacities.size(), 1);

  //! create ospray volume
  Timer();
  {
    OSPVolume volume = ospNewVolume("shared_structured_volume");
    OSPData voxelData = ospNewData(dims.x * dims.y * dims.z * data_size, OSP_UCHAR, volumeData, OSP_DATA_SHARED_BUFFER);
    cleanlist.push_back([=]() {
      ospRelease(volume);
      ospRelease(voxelData);
    });
    ospSetString(volume, "voxelType", "uchar");
    ospSetVec3i(volume, "dimensions", (osp::vec3i &) dims);
    ospSetVec3f(volume, "gridOrigin", osp::vec3f{-1.0f, -1.0f, -1.0f});
    ospSetVec3f(volume, "gridSpacing", osp::vec3f{2.f/dims.x, 2.f/dims.y, 2.f/dims.z});
    ospSet1f(volume, "samplingRate", 9.0f);
    ospSet1i(volume, "gradientShadingEnabled", 0);
    ospSet1i(volume, "useGridAccelerator", 0);
    ospSet1i(volume, "adaptiveSampling", 0);
    ospSet1i(volume, "preIntegration", 0);
    ospSet1i(volume, "singleShade", 0);
    ospSetData(volume, "voxelData", voxelData);
    ospSetObject(volume, "transferFunction", transferFcn);
    ospCommit(volume);
    ospAddVolume(world, volume);
  }
  Timer("finish commit");
}