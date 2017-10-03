#version 330 core
uniform sampler3D tex3d;
uniform sampler2D textf;
in vec3 fTex3dCoord;
layout(location = 0) out vec4 color;
void main()
{
  float dataval = texture(tex3d, fTex3dCoord).r;
  vec4  datacol = texture(textf, vec2(dataval, 0.f));
  color = datacol;
};
