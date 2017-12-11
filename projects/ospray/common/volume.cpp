#include "volume.h"
#include "../../reader/volume_reader.hpp"
#include <mutex>

Volume::HistVolume::HistVolume(size_t x, size_t y, size_t z)
  : histXDim(x), histYDim(y), histZDim(z), histCount(histXDim * histYDim * histZDim) {};

void Volume::HistVolume::UpdateHistogram(const vec3s &data_dims,
                                         const vec2f &vRange,
                                         const vec2f &gRange,
                                         const vec2f &aRange,
                                         const std::vector<float> &vData,
                                         const std::vector<float> &gData,
                                         const std::vector<float> &aData)

{
  // timer
  Timer();
  // setup histogram volume
  if (histVolume != nullptr) { delete[] histVolume; }
  histVolume = new std::atomic<size_t>[histCount];
  // compute histogram
  const float vScale = (float)(histXDim - 1) / (vRange.y - vRange.x);
  const float gScale = (float)(histYDim - 1) / (gRange.y - gRange.x);
  const float aScale = (float)(histZDim - 1) / (aRange.y - aRange.x);
  tbb::parallel_for(size_t(0), (data_dims.x - 2) * (data_dims.y - 2) * (data_dims.z - 2), [&](size_t k) {
    const float v = vData[k];
    const float g = gData[k];
    const float a = aData[k];
    const auto ix = (size_t) round((v - vRange.x) * vScale);
    const auto iy = (size_t) round((g - gRange.x) * gScale);
    const auto iz = (size_t) round((a - aRange.x) * aScale);
    ++histVolume[Id(ix, iy, iz)];
  });
  // timing
  Timer("compute histogram");
}

void Volume::ComputeGradients() {
  vData.resize((data_dims.x - 2) * (data_dims.y - 2) * (data_dims.z - 2));
  gData.resize((data_dims.x - 2) * (data_dims.y - 2) * (data_dims.z - 2));
  aData.resize((data_dims.x - 2) * (data_dims.y - 2) * (data_dims.z - 2));
  Timer();
  std::mutex guard;
  tbb::parallel_for(size_t(0), data_dims.x * data_dims.y * data_dims.z, [&](size_t k) {
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
      const auto v = ReadAs<float>(data_ptr, (int)i, data_type);
      const auto fpx = ReadAs<float>(data_ptr, (int)(i + idx), data_type);
      const auto fnx = ReadAs<float>(data_ptr, (int)(i - idx), data_type);
      const auto fpy = ReadAs<float>(data_ptr, (int)(i + idy), data_type);
      const auto fny = ReadAs<float>(data_ptr, (int)(i - idy), data_type);
      const auto fpz = ReadAs<float>(data_ptr, (int)(i + idz), data_type);
      const auto fnz = ReadAs<float>(data_ptr, (int)(i - idz), data_type);
      float g = ospcommon::length(ospcommon::vec3f((fpx - fnx) / 2.f / data_spacing,
                                                   (fpy - fny) / 2.f / data_spacing,
                                                   (fpz - fnz) / 2.f / data_spacing));
      float a =
          (fpx + fnx - 2.f * v) / (data_spacing * data_spacing) +
          (fpy + fny - 2.f * v) / (data_spacing * data_spacing) +
          (fpz + fnz - 2.f * v) / (data_spacing * data_spacing);
      const size_t new_id = (x - 1) + (y - 1) * (data_dims.x - 2) + (z - 1) * (data_dims.x - 2) * (data_dims.y - 2);
      vData[new_id] = v;
      gData[new_id] = g;
      aData[new_id] = a;
    }
  });
  const auto block_start = size_t(0);
  const auto block_end = (data_dims.x - 2) * (data_dims.y - 2) * (data_dims.z - 2);
  tbb::parallel_reduce(tbb::blocked_range<size_t>(block_start, block_end,
    [&](size_t k)
    {
    const float v = vData[k];
    const float g = gData[k];
    const float a = aData[k];
    vRange.x = std::min(vRange.x, v);
    vRange.y = std::max(vRange.y, v);
    gRange.x = std::min(gRange.x, g);
    gRange.y = std::max(gRange.y, g);
    aRange.x = std::min(aRange.x, a);
    aRange.y = std::max(aRange.y, a);
    if (a < aRange.x) {
      std::cout << "xx" << std::endl;
    }
  },
  [&]());

  Timer("compute gradient");
}

void Volume::Init(int argc, const char **argv) {
  //-----------------------------------------------------------------------------------------------------------------//
  // load volume
  //-----------------------------------------------------------------------------------------------------------------//
  Timer();
  int _size, _dimx, _dimy, _dimz;
  ReadVolume(argv[1], data_type, _size, _dimx, _dimy, _dimz, data_ptr);
  data_size = size_t(_size); data_dims = vec3s(_dimx, _dimy, _dimz);
  //data_spacing = std::min(2.f / data_dims.z, std::min(2.f / data_dims.x, 2.f / data_dims.y));
  data_spacing = 1.f;
  Timer("load data");

  //-----------------------------------------------------------------------------------------------------------------//
  // gradient
  //-----------------------------------------------------------------------------------------------------------------//
  ComputeGradients();

  //-----------------------------------------------------------------------------------------------------------------//
  // calculate histogram
  //-----------------------------------------------------------------------------------------------------------------//
  histogram.UpdateHistogram(data_dims, vRange, gRange, aRange, vData, gData, aData);

  //-----------------------------------------------------------------------------------------------------------------//
  // transfer function
  //-----------------------------------------------------------------------------------------------------------------//
  tfn.Init(vRange, gRange);

  //-----------------------------------------------------------------------------------------------------------------//
  // create ospray volume
  //-----------------------------------------------------------------------------------------------------------------//
  Timer();
  {
    ospVolume = ospNewVolume("shared_structured_volume");
    ospVoxelData = ospNewData(data_size, OSP_UCHAR, data_ptr, OSP_DATA_SHARED_BUFFER);
    ospSetVec3i(ospVolume, "dimensions", osp::vec3i{(int)data_dims.x, (int)data_dims.y, (int)data_dims.z});
    ospSetVec3f(ospVolume, "gridOrigin", osp::vec3f{-0.5f * data_spacing * data_dims.x,
                                                    -0.5f * data_spacing * data_dims.y,
                                                    -0.5f * data_spacing * data_dims.z});
    ospSetVec3f(ospVolume, "gridSpacing", osp::vec3f{data_spacing, data_spacing, data_spacing});
    ospSet1i(ospVolume, "adaptiveSampling", true);
    ospSet1i(ospVolume, "gradientShadingEnabled", true);
    ospSet1i(ospVolume, "preIntegration", false);
    ospSet1i(ospVolume, "singleShade", false);
    ospSet1f(ospVolume, "samplingRate", 0.125f);
    ospSet1f(ospVolume, "adaptiveMaxSamplingRate", 1.f);
    ospSetData(ospVolume, "voxelData", ospVoxelData);
    ospSetString(ospVolume, "voxelType", "uchar");
    ospSetObject(ospVolume, "transferFunction", tfn.OSPRayPtr());
    ospCommit(ospVolume);
  }
  Timer("finish commit");
}

void Volume::Clean() {
  if (data_ptr != nullptr) { delete[] (char *) data_ptr; data_ptr = nullptr; }
  if (ospVolume != nullptr) { ospRelease(ospVolume); ospVolume = nullptr; }
  if (ospVoxelData != nullptr) { ospRelease(ospVoxelData); ospVoxelData = nullptr; }
  tfn.Clean();
}