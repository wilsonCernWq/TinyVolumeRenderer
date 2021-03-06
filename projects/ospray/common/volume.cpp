#include "volume.h"
#include "../../reader/volume_reader.hpp"
#include <mutex>

float ParallelMin(const std::vector<float>& array, float& v)
{
  using iter_t = std::vector<float>::const_iterator;
  return tbb::parallel_reduce
    (tbb::blocked_range<iter_t>(array.begin(), array.end()),
     v,
     [](const tbb::blocked_range<iter_t>& r, float init)->float {
       for (auto a = r.begin(); a!=r.end(); ++a)
       {
         init = std::min(init, *a);
       }
       return init;
     },
     [](float x, float y)->float { return std::min(x,y); });
}

float ParallelMax(const std::vector<float>& array, float& v)
{
  using iter_t = std::vector<float>::const_iterator;
  return tbb::parallel_reduce
    (tbb::blocked_range<iter_t>(array.begin(), array.end()),
     v,
     [](const tbb::blocked_range<iter_t>& r, float init)->float {
       for (auto a = r.begin(); a!=r.end(); ++a)
       {
         init = std::max(init, *a);
       }
       return init;
     },
     [](float x, float y)->float { return std::max(x,y); });
}

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
      const float g
        = ospcommon::length(ospcommon::vec3f((fpx - fnx) / 2.f / data_spacing,
                                             (fpy - fny) / 2.f / data_spacing,
                                             (fpz - fnz) / 2.f / data_spacing));
      const float a
        = (fpx + fnx - 2.f * v) / (data_spacing * data_spacing) +
          (fpy + fny - 2.f * v) / (data_spacing * data_spacing) +
          (fpz + fnz - 2.f * v) / (data_spacing * data_spacing);
      const size_t new_id = (x - 1) + (y - 1) * (data_dims.x - 2) + (z - 1) * (data_dims.x - 2) * (data_dims.y - 2);
      vData[new_id] = v;
      gData[new_id] = g;
      aData[new_id] = a;
    }
  });
  vRange.x = ParallelMin(vData, vRange.x);
  gRange.x = ParallelMin(gData, gRange.x);
  aRange.x = ParallelMin(aData, aRange.x);
  vRange.y = ParallelMax(vData, vRange.y);
  gRange.y = ParallelMax(gData, gRange.y);
  aRange.y = ParallelMax(aData, aRange.y);
  Timer("compute gradient");
  std::cout << "[ospray] volume max: " << vRange.x << " min: " << vRange.y << std::endl;
}

void Volume::Init(int argc, const char **argv) {
  //-----------------------------------------------------------------------------------------------------------------//
  // load volume
  //-----------------------------------------------------------------------------------------------------------------//
  Timer();
  int _size, _dimx, _dimy, _dimz;
  ReadVolume(argv[1], data_type, _size, _dimx, _dimy, _dimz, data_ptr);
  data_size = size_t(_size); data_dims = vec3s(_dimx, _dimy, _dimz);
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
    ospSetObject(ospVolume, "transferFunction", tfn.OSPRayPtr());
    switch (data_type) {
    case (UCHAR):
      // --> working
      ospSetString(ospVolume, "voxelType", "uchar");
      ospVoxelData = ospNewData(data_dims.x * data_dims.y * data_dims.z,
				OSP_UCHAR, data_ptr, OSP_DATA_SHARED_BUFFER);
      break;
    case (CHAR):
      // --> not tested
      fprintf(stderr, "Error: Unsupported type CHAR %i", data_type);
      exit(EXIT_FAILURE);
    case (UINT8):
      // --> working
      ospSetString(ospVolume, "voxelType", "uchar");
      ospVoxelData = ospNewData(data_dims.x * data_dims.y * data_dims.z,
				OSP_UCHAR, data_ptr, OSP_DATA_SHARED_BUFFER);
      break;
    case (UINT16):
      // --> working
      ospSetString(ospVolume, "voxelType", "ushort");
      ospVoxelData = ospNewData(data_dims.x * data_dims.y * data_dims.z,
      			   OSP_USHORT, data_ptr, OSP_DATA_SHARED_BUFFER);
      break;
    case (UINT32):
      fprintf(stderr, "Error: Unsupported type UINT32 %i", data_type);
      exit(EXIT_FAILURE);
    case (UINT64):
      fprintf(stderr, "Error: Unsupported type UINT64 %i", data_type);
      exit(EXIT_FAILURE);
    case (INT8):
      fprintf(stderr, "Error: Unsupported type INT8 %i", data_type);
      exit(EXIT_FAILURE);
    case (INT16):
      // --> working
      ospSetString(ospVolume, "voxelType", "short");
      ospVoxelData = ospNewData(data_dims.x * data_dims.y * data_dims.z,
				OSP_SHORT, data_ptr, OSP_DATA_SHARED_BUFFER);
      break;
    case (INT32):
      fprintf(stderr, "Error: Unsupported type INT32 %i", data_type);
      exit(EXIT_FAILURE);
    case (FLOAT32):
      // --> working
      ospSetString(ospVolume, "voxelType", "float");
      ospVoxelData = ospNewData(data_dims.x * data_dims.y * data_dims.z,
				OSP_FLOAT, data_ptr, OSP_DATA_SHARED_BUFFER);
      break;
    case (DOUBLE64):
      // --> not tested
      ospSetString(ospVolume, "voxelType", "double");
      ospVoxelData = ospNewData(data_dims.x * data_dims.y * data_dims.z,
				OSP_DOUBLE, data_ptr, OSP_DATA_SHARED_BUFFER);
      break;
    default:
      fprintf(stderr, "Error: Unrecognized type %i", data_type);
      exit(EXIT_FAILURE);
    }
    ospSetData(ospVolume, "voxelData", ospVoxelData);
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
