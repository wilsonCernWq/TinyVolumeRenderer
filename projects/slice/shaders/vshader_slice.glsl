#version 330 core
in  vec3 vPosition;
out vec3 fTex3dCoord;
uniform mat4 MVP;
void main()
{
  gl_Position = /*MVP */ vec4(vPosition, 1.0);
  fTex3dCoord = vPosition * 0.5f + 0.5f;
};
