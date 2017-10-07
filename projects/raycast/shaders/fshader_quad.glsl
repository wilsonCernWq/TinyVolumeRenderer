#version 330 core
uniform sampler2D tex2d;
in vec2 fTexCoord;
layout(location = 0) out vec4 color;
void main()
{
  color = vec4(texture(tex2d, fTexCoord).rgb, 1.0f);
};
