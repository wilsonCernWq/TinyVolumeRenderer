#version 330 core
uniform sampler2D tex2d; // previous image
uniform sampler3D tex3d; // volume data
uniform sampler2D textf; // transfer function
uniform float samplingRate;
in vec2 fTex2dCoord;
layout(location = 0) out vec4 color;
void main()
{
  color = texture(tex2d, fTex2dCoord);
};
