#version 330 core
in  vec3 vPosition;
out vec3 fRayDir;
uniform mat4 MVP;
uniform vec3 camera; // camera in world coordinate
void main()
{
  gl_Position = MVP * vec4(vPosition, 1.0);
  fRayDir = vPosition - camera;
}
