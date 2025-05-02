#version 450
layout (location = 0) in vec3 ModNorm;

layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(ModNorm, 1.0);
}
