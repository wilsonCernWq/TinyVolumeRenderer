#version 330 core
uniform sampler2D tex2d; // previous image
uniform sampler3D tex3d; // volume data
uniform sampler2D textf; // transfer function
in vec2 fTex2dCoord;
in vec3 fTex3dCoord;
layout(location = 0) out vec4 finalcolor;
void main()
{
  // read the data value (normalized)
  float dataval = texture(tex3d, fTex3dCoord).r;
  // sample from transfer function
  vec4  datacol = texture(textf, vec2(dataval, 0.f));
  // background color
  vec4  bgcol   = texture(tex2d, fTex2dCoord);
  // compose
  // -- front to back
  float alpha = bgcol.a   + (1.f - bgcol.a) * datacol.a;
  vec3  color = bgcol.rgb + (1.f - bgcol.a) * datacol.rgb * datacol.a;
  // -- back to front
  // vec3  color = datacol.rgb * datacol.a + (1.f - datacol.a) * bgcol.rgb;
  finalcolor = vec4(color, 1.f);
};
