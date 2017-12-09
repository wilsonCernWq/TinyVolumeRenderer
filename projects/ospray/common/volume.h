//
// Created by qwu on 12/7/17.
//
#pragma once
#ifndef OSPRAY_VOLUME_H
#define OSPRAY_VOLUME_H

#include "common/common.h"
#include "common/transferfunction.h"
#include <limits>
#include <atomic>

class Volume {
private:
  using vec2f = glm::vec2;
  using vec3f = glm::vec3;
  using vec2s = glm::tvec2<size_t>;
  using vec3s = glm::tvec3<size_t>;
  using vec2i = glm::ivec2;
  using vec3i = glm::ivec3;
private:
  class HistVolume {
  private:
    const size_t histXDim, histYDim, histZDim;
    const size_t histCount;
    std::atomic<size_t>* histVolume = nullptr;
  public:
    HistVolume(size_t x, size_t y, size_t z);
    size_t Id(size_t x, size_t y, size_t z) const { return z * histYDim * histXDim + y * histXDim + x; }
    std::atomic<size_t>&       At(size_t x, size_t y, size_t z)       { return histVolume[Id(x, y, z)]; }
    const std::atomic<size_t>& At(size_t x, size_t y, size_t z) const { return histVolume[Id(x, y, z)]; }
    const size_t HistXDim() const { return histXDim; };
    const size_t HistYDim() const { return histYDim; };
    const size_t HistZDim() const { return histZDim; };
    const size_t HistCount() const { return histCount; };
    const size_t HistCountXY() const { return histXDim * histYDim; };
    const size_t HistCountYZ() const { return histYDim * histZDim; };
    const size_t HistCountXZ() const { return histXDim * histZDim; };
    void UpdateHistogram(const vec3s& data_dims, const vec2f& vRange, const vec2f& gRange, const vec2f& aRange,
                         const std::vector<float>& vData,
                         const std::vector<float>& gData,
                         const std::vector<float>& aData);
  };
private:
  HistVolume histogram;
  Transferfunction tfn;

  void *data_ptr = nullptr;
  int data_type = 0;
  size_t data_size = 0;
  vec3s data_dims = vec3s(0);

  std::vector<float> vData;
  std::vector<float> gData;
  std::vector<float> aData;
  vec2f vRange;
  vec2f gRange;
  vec2f aRange;

  OSPVolume ospVolume = nullptr;
  OSPData ospVoxelData = nullptr;
public:
  OSPVolume OSPRayPtr() { return ospVolume; }
  Transferfunction& GetTransferFunction() { return tfn; }
  const HistVolume& GetHistogram() { return histogram; }
  Volume() : histogram(64, 64, 64)
  {
    vRange = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
    gRange = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
    aRange = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
  }
  ~Volume() { Clean(); }
  void Clean();
  void Init(int argc, const char **argv);
  float GetValueRangeX() { return vRange.x; };
  float GetValueRangeY() { return vRange.y; };
  float Get1stGradientRangeX() { return gRange.x; };
  float Get1stGradientRangeY() { return gRange.y; };
  float Get2ndGradientRangeX() { return aRange.x; };
  float Get2ndGradientRangeY() { return aRange.y; };
  float SampleHistX(size_t i) {
    return i * (GetValueRangeY() - GetValueRangeX()) / histogram.HistXDim() + GetValueRangeX();
  }
  float SampleHistY(size_t i) {
    return i * (Get1stGradientRangeY() - Get1stGradientRangeX()) / histogram.HistYDim() + Get1stGradientRangeX();
  }
  float SampleHistZ(size_t i) {
    return i * (Get2ndGradientRangeY() - Get2ndGradientRangeX()) / histogram.HistZDim() + Get2ndGradientRangeX();
  }
  void ComputeGradients();
};

#endif //OSPRAY_VOLUME_H
