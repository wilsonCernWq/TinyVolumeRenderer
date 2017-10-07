#version 330 core
in vec3 vPosition;
out vec2 fTexCoord;
void main()
{
  gl_Position = vec4(vPosition, 1.0);
  fTexCoord = vPosition.xy * 0.5f + 0.5f;
};
