//
// Created by qwu on 12/9/17.
//

#ifndef OSPRAY_TRANSFERFUNCTION_H
#define OSPRAY_TRANSFERFUNCTION_H

#include "common/common.h"

class Transferfunction {
private:
  glm::vec2 vrange;
  glm::vec2 grange;
  OSPTransferFunction ospTfn = nullptr;
public:
  OSPTransferFunction OSPRayPtr() { return ospTfn; }
  void Init(const glm::vec2&, const glm::vec2&);
  void Clean() { if (ospTfn != nullptr) { ospRelease(ospTfn); ospTfn = nullptr; } }
  void Update(const void *colors, const void *opacities,
              int colorW, int colorH, int opacityW, int opacityH);

};

#endif //OSPRAY_TRANSFERFUNCTION_H
