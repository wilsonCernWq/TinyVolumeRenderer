#version 330 core
in  vec3 vPosition;
layout(location = 1) in  vec2 vTexCoord;
out vec2 fTexCoord;
uniform mat4 MVP;
void main()
{
  gl_Position = MVP * vec4(vPosition, 1.0);
  fTexCoord = vTexCoord;
};
