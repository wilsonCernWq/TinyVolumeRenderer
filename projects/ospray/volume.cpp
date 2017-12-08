#include <limits>
#include <mutex>
#include "common.h"
#include "global.h"
#include "../reader/volume_reader.hpp"

static int data_type = 0, data_size = 0;
static void  *vData = nullptr;
static float *gData = nullptr;
static ospcommon::vec3i data_dims(0);
static ospcommon::vec2f vRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
static ospcommon::vec2f gRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());

float GetValueRangeX() { return vRange.x; };
float GetValueRangeY() { return vRange.y; };
float GetGradientRangeX() { return gRange.x; };
float GetGradientRangeY() { return gRange.y; };

void UpdateTFN(const void *colors, const void *opacities, int colorW, int colorH, int opacityW, int opacityH)
{
  //! setup trasnfer function
  OSPData colorsData = ospNewData(colorW * colorH, OSP_FLOAT3, colors);
  ospCommit(colorsData);
  OSPData opacitiesData = ospNewData(opacityW * opacityH, OSP_FLOAT, opacities);
  ospCommit(opacitiesData);
  ospSetData(transferFcn, "colors", colorsData);
  ospSetData(transferFcn, "opacities", opacitiesData);
  ospSetVec2f(transferFcn, "valueRange", osp::vec2f{static_cast<float>(0), static_cast<float>(255)});
  ospSet1i(transferFcn, "colorWidth", colorW);
  ospSet1i(transferFcn, "colorHeight", colorH);
  ospSet1i(transferFcn, "opacityWidth", opacityW);
  ospSet1i(transferFcn, "opacityHeight", opacityH);
  ospCommit(transferFcn);
  ospRelease(colorsData);
  ospRelease(opacitiesData);
}

void UpdateHistogram(float v_lower, float v_upper, float g_lower, float g_upper)
{
  float hist_max = 0.f;
  Timer();
  hist.clear();
  hist.resize(hist_xdim * hist_ydim, 0);
  for (int x = 0; x < data_dims.x; ++x) {
    for (int y = 0; y < data_dims.y; ++y) {
      for (int z = 0; z < data_dims.z; ++z) {
        const int i = z * data_dims.y * data_dims.x + y * data_dims.x + x;
        const auto v = ReadAs<float>(vData, i, data_type);
        const auto g = gData[i];
        if (v >= v_lower && v <= v_upper && g >= g_lower && g <= g_upper) {
          const auto ix = round((hist_xdim - 1) * (v - v_lower) / (v_upper - v_lower));
          const auto iy = round((hist_ydim - 1) * (g - g_lower) / (g_upper - g_lower));
          auto &count = hist[iy * hist_xdim + ix];
          count += 1.f;
          hist_max = std::max(hist_max, count);
        }
      }
    }
  }
  tbb::parallel_for(0, hist_xdim * hist_ydim, [&](size_t k) { hist[k] /= hist_max; });
  Timer("compute histogram");
}

void CreateVolume(int argc, const char **argv)
{
  // initialization
  cleanlist.emplace_back([=]() {
    delete[] (char *) vData;
    delete[] gData;
  });

  // load volume
  Timer();
  ReadVolume(argv[1], data_type, data_size, data_dims.x, data_dims.y, data_dims.z, vData);
  Timer("load data");

  // gradient
  gData = new float[data_dims.x * data_dims.y * data_dims.z];
  Timer();
  std::mutex lock;
  tbb::parallel_for(0, data_dims.x * data_dims.y * data_dims.z, [&](size_t k) {
    size_t x = k % data_dims.x;
    size_t y = (k % (data_dims.x * data_dims.y)) / data_dims.x;
    size_t z = k / (data_dims.x * data_dims.y);
    auto i = z * data_dims.y * data_dims.x + y * data_dims.x + x;
    auto v = ReadAs<float>(vData, i, data_type);
    float idx = x == (data_dims.x - 1) ? -1 : 1;
    float idy = y == (data_dims.y - 1) ? -data_dims.x : data_dims.x;
    float idz = z == (data_dims.z - 1) ? -data_dims.x * data_dims.y : data_dims.x * data_dims.y;
    auto dx = (ReadAs<float>(vData, i + idx, data_type) - v) / idx;
    auto dy = (ReadAs<float>(vData, i + idy, data_type) - v) / idy;
    auto dz = (ReadAs<float>(vData, i + idz, data_type) - v) / idz;
    auto g = sqrtf(dx * dx + dy * dy + dz * dz);
    //lock.lock();
    //std::cout << g << std::endl;
    //lock.unlock();
    vRange.x = std::min(vRange.x, v);
    vRange.y = std::max(vRange.y, v);
    gRange.x = std::min(gRange.x, g);
    gRange.y = std::max(gRange.y, g);
  });
  Timer("compute gradient");

  // calculate histogram
  UpdateHistogram(vRange.x, vRange.y, gRange.x, gRange.y);

  // transfer function
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
  UpdateTFN(colors.data(), opacities.data(), colors.size(), 1, opacities.size(), 1);

  // create ospray volume
  Timer();
  {
    OSPVolume volume = ospNewVolume("shared_structured_volume");
    OSPData voxelData = ospNewData(data_dims.x * data_dims.y * data_dims.z * data_size, OSP_UCHAR,
                                   vData, OSP_DATA_SHARED_BUFFER);
    cleanlist.push_back([=]() {
      ospRelease(volume);
      ospRelease(voxelData);
    });
    ospSetString(volume, "voxelType", "uchar");
    ospSetVec3i(volume, "dimensions", (osp::vec3i &) data_dims);
    ospSetVec3f(volume, "gridOrigin", osp::vec3f{-1.0f, -1.0f, -1.0f});
    ospSetVec3f(volume, "gridSpacing", osp::vec3f{2.f / data_dims.x, 2.f / data_dims.y, 2.f / data_dims.z});
    ospSet1f(volume, "samplingRate", 5.0f);
    ospSet1i(volume, "adaptiveSampling", 1);
    ospSet1i(volume, "gradientShadingEnabled", 0);
    ospSet1i(volume, "useGridAccelerator", 0);
    ospSet1i(volume, "preIntegration", 0);
    ospSet1i(volume, "singleShade", 0);
    ospSetData(volume, "voxelData", voxelData);
    ospSetObject(volume, "transferFunction", transferFcn);
    ospCommit(volume);
    ospAddVolume(world, volume);
  }
  Timer("finish commit");
}
