//
// Created by qwu on 12/7/17.
//
#pragma once
#ifndef OSPRAY_VOLUME_H
#define OSPRAY_VOLUME_H

void UpdateTFN(const void *colors, const void *opacities, int colorW, int colorH, int opacityW, int opacityH);

void CreateVolume(int argc, const char **argv);

#endif //OSPRAY_VOLUME_H
