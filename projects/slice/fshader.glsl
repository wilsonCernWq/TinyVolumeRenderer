#version 330 core

out vec4 color;

void main() {
	float c = clamp(1 - gl_FragCoord.z, 0.6, 1.0);
	color = vec4(c, c, c, 1.0f);
}