#version 330 core
in  vec3 vPosition;
in  vec3 vTexCoord;
out vec2 fTex2dCoord;
out vec3 fTex3dCoord;
void main()
{
  gl_Position = vec4(vPosition, 1.0);
  fTex2dCoord = vPosition.xy * 0.5 + 0.5; // to sample from background
  fTex3dCoord = vTexCoord;                // to sample from volume
};
