#version 330 core
uniform sampler2D texsampler;
in vec2 fTexCoord;
void main()
{
  gl_FragColor = vec4(texture(texsampler, fTexCoord).rgb, 1.0f);
};
