#version 330 core
uniform sampler2D tex2d;
uniform sampler3D tex3d;
in vec2 fTex2dCoord;
in vec3 fTex3dCoord;
layout(location = 0) out vec4 color;
void main()
{
  color = vec4(texture(tex2d, fTex2dCoord).rgb, 1.0f);
};
