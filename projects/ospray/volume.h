//
// Created by qwu on 12/7/17.
//
#pragma once
#ifndef OSPRAY_VOLUME_H
#define OSPRAY_VOLUME_H

float GetValueRangeX();
float GetValueRangeY();
float GetGradientRangeX();
float GetGradientRangeY();
void UpdateTFN(const void *colors, const void *opacities, int colorW, int colorH, int opacityW, int opacityH);
void UpdateHistogram(float v_lower, float v_upper, float g_lower, float g_upper);
void CreateVolume(int argc, const char **argv);

#endif //OSPRAY_VOLUME_H
