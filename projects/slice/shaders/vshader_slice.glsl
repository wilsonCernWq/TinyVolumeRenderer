#version 330 core
in  vec3 vPosition;
out vec2 fTex2dCoord;
out vec3 fTex3dCoord;
void main()
{
  gl_Position = vec4(vPosition, 1.0);
  fTex2dCoord = vPosition.xy * 0.5 + 0.5;
  fTex3dCoord = vPosition * 0.5f + 0.5f;
};
