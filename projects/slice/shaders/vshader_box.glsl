#version 330 core
in  vec3 vPosition;
in  vec3 vColor;
out vec3 fColor;
uniform mat4 MVP;
void main()
{
  gl_Position = MVP * vec4(vPosition, 1.0);
  fColor = vColor;
};
