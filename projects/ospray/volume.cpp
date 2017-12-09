#include <limits>
#include <mutex>
#include "common/common.h"
#include "global.h"
#include "../reader/volume_reader.hpp"

static int data_type = 0, data_size = 0;
static void *data_ptr = nullptr;
static std::vector<float> vData;
static std::vector<float> gData;
static std::vector<float> aData;
static ospcommon::vec3i data_dims(0);
static ospcommon::vec2f vRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
static ospcommon::vec2f gRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
static ospcommon::vec2f aRange(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());

float GetValueRangeX() { return vRange.x; };
float GetValueRangeY() { return vRange.y; };
float Get1stGradientRangeX() { return gRange.x; };
float Get1stGradientRangeY() { return gRange.y; };
float Get2ndGradientRangeX() { return aRange.x; };
float Get2ndGradientRangeY() { return aRange.y; };

void UpdateTFN(const void *colors, const void *opacities, int colorW, int colorH, int opacityW, int opacityH) {
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

void UpdateHistogram() {
  // timer
  Timer();
  // setup histogram volume
  if (histVolume != nullptr) delete[] histVolume;
  histVolume = new std::atomic<size_t>[histCount]();
  // compute histogram
  const float vScale = (float)(histXDim - 1) / (vRange.y - vRange.x);
  const float gScale = (float)(histYDim - 1) / (gRange.y - gRange.x);
  const float aScale = (float)(histZDim - 1) / (aRange.y - aRange.x);
  tbb::parallel_for(0, (data_dims.x - 2) * (data_dims.y - 2) * (data_dims.z - 2), [&](size_t k) {
    const float v = vData[k];
    const float g = gData[k];
    const float a = aData[k];
    const auto ix = (size_t) round((v - vRange.x) * vScale);
    const auto iy = (size_t) round((g - gRange.x) * gScale);
    const auto iz = (size_t) round((a - aRange.x) * aScale);
    ++histVolume[histIdx(ix, iy, iz)];
  });
  // timing
  Timer("compute histogram");
}

void CreateVolume(int argc, const char **argv) {
  //-----------------------------------------------------------------------------------------------------------------//
  // initialization
  //-----------------------------------------------------------------------------------------------------------------//
  cleanlist.emplace_back([=]() {
    delete[] (char *) data_ptr;
  });

  //-----------------------------------------------------------------------------------------------------------------//
  // load volume
  //-----------------------------------------------------------------------------------------------------------------//
  Timer();
  ReadVolume(argv[1], data_type, data_size, data_dims.x, data_dims.y, data_dims.z, data_ptr);
  Timer("load data");

  //-----------------------------------------------------------------------------------------------------------------//
  // gradient
  //-----------------------------------------------------------------------------------------------------------------//
  vData.resize((data_dims.x - 2) * (data_dims.y - 2) * (data_dims.z - 2));
  gData.resize((data_dims.x - 2) * (data_dims.y - 2) * (data_dims.z - 2));
  aData.resize((data_dims.x - 2) * (data_dims.y - 2) * (data_dims.z - 2));
  Timer();
  tbb::parallel_for(0, data_dims.x * data_dims.y * data_dims.z, [&](size_t k) {
    size_t x = k % data_dims.x;
    size_t y = (k % (data_dims.x * data_dims.y)) / data_dims.x;
    size_t z = k / (data_dims.x * data_dims.y);
    bool valid = true;
    if (x == data_dims.x - 1) { valid = false; }
    if (y == data_dims.y - 1) { valid = false; }
    if (z == data_dims.z - 1) { valid = false; }
    if (x == 0) { valid = false; }
    if (y == 0) { valid = false; }
    if (z == 0) { valid = false; }
    if (valid) {
      const size_t i = z * data_dims.y * data_dims.x + y * data_dims.x + x;
      const size_t idx = 1;
      const size_t idy = data_dims.x;
      const size_t idz = data_dims.x * data_dims.y;
      const auto v = ReadAs<float>(data_ptr, i, data_type);
      const auto fpx = ReadAs<float>(data_ptr, i + idx, data_type);
      const auto fnx = ReadAs<float>(data_ptr, i - idx, data_type);
      const auto fpy = ReadAs<float>(data_ptr, i + idy, data_type);
      const auto fny = ReadAs<float>(data_ptr, i - idy, data_type);
      const auto fpz = ReadAs<float>(data_ptr, i + idz, data_type);
      const auto fnz = ReadAs<float>(data_ptr, i - idz, data_type);
      float g = ospcommon::length(ospcommon::vec3f((fpx - fnx) / 2.f,
                                                   (fpy - fny) / 2.f,
                                                   (fpz - fnz) / 2.f));
      float a =
        ospcommon::length(ospcommon::vec3f(fpx - v,
                                           fpy - v,
                                           fpz - v)) -
        ospcommon::length(ospcommon::vec3f(v - fnx,
                                           v - fny,
                                           v - fnz));
      const size_t new_id = (x - 1) + (y - 1) * (data_dims.x - 2) + (z - 1) * (data_dims.x - 2) * (data_dims.y - 2);
      vData[new_id] = v;
      gData[new_id] = g;
      aData[new_id] = a;
      vRange.x = std::min(vRange.x, v);
      vRange.y = std::max(vRange.y, v);
      gRange.x = std::min(gRange.x, g);
      gRange.y = std::max(gRange.y, g);
      aRange.x = std::min(aRange.x, a);
      aRange.y = std::max(aRange.y, a);
    }
  });
  Timer("compute gradient");

  //-----------------------------------------------------------------------------------------------------------------//
  // calculate histogram
  //-----------------------------------------------------------------------------------------------------------------//
  UpdateHistogram();

  //-----------------------------------------------------------------------------------------------------------------//
  // transfer function
  //-----------------------------------------------------------------------------------------------------------------//
  const std::vector<float> colors =
    {
      0, 0, 0.563,
      0, 0, 1,
      0, 1, 1,
      0.5, 1, 0.5,
      1, 1, 0,
      1, 0, 0,
      0.5, 0, 0,
    };
  const std::vector<float> opacities = {1.0f, 0.8f, 0.6f, 0.4f, 0.2f, 0.0f};
  UpdateTFN(colors.data(), opacities.data(), colors.size() / 3, 1, opacities.size(), 1);

  //-----------------------------------------------------------------------------------------------------------------//
  // create ospray volume
  //-----------------------------------------------------------------------------------------------------------------//
  Timer();
  {
    OSPVolume volume = ospNewVolume("shared_structured_volume");
    OSPData voxelData = ospNewData(data_dims.x * data_dims.y * data_dims.z * data_size, OSP_UCHAR,
                                   data_ptr, OSP_DATA_SHARED_BUFFER);
    cleanlist.push_back([=]() {
      ospRelease(volume);
      ospRelease(voxelData);
    });
    ospSetString(volume, "voxelType", "uchar");
    ospSetVec3i(volume, "dimensions", (osp::vec3i &) data_dims);
    ospSetVec3f(volume, "gridOrigin", osp::vec3f{-1.0f, -1.0f, -1.0f});
    ospSetVec3f(volume, "gridSpacing", osp::vec3f{2.f / data_dims.x, 2.f / data_dims.y, 2.f / data_dims.z});
    ospSet1f(volume, "samplingRate", 0.125f);
    ospSet1f(volume, "adaptiveMaxSamplingRate", 1.f);
    ospSet1i(volume, "adaptiveSampling", true);
    ospSet1i(volume, "gradientShadingEnabled", true);
    ospSet1i(volume, "preIntegration", false);
    ospSet1i(volume, "singleShade", false);
    ospSetData(volume, "voxelData", voxelData);
    ospSetObject(volume, "transferFunction", transferFcn);
    ospCommit(volume);
    ospAddVolume(world, volume);
  }
  Timer("finish commit");
}
