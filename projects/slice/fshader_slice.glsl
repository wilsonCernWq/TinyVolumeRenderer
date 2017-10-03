#version 330 core
uniform sampler3D tex3d;
in vec3 fTex3dCoord;
layout(location = 0) out vec4 color;
void main()
{
  if (texture(tex3d, fTex3dCoord).r > 0) {
    color = vec4(0.f, 1.f, 0.f, 1.f);
  }
  else {
    color = vec4(1.f, 0.f, 0.f, 1.f);
  }
};
