#version 330 core
uniform sampler2D tex2d; // previous image
uniform sampler3D tex3d; // volume data
uniform sampler2D textf; // transfer function
uniform float samplingRate;
in vec2 fTex2dCoord;
in vec3 fTex3dCoord;
layout(location = 0) out vec4 color;
void main()
{
  // read the data value (normalized)
  float dataval = texture(tex3d, fTex3dCoord).r;
  // sample from transfer function
  vec4  datacol = texture(textf, vec2(dataval, 0.f));
  // background color
  vec4  bgcol   = texture(tex2d, fTex2dCoord);
  // compose -- back to front composition
  float alpha = datacol.a / samplingRate;
  color.rgb = bgcol.rgb + (1.f - bgcol.a) * datacol.rgb * alpha;
  color.a   = bgcol.a   + (1.f - bgcol.a) * alpha;
};
