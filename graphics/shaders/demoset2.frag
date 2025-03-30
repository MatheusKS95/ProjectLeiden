#version 450
layout (location = 0) in vec3 FragPos;
layout (location = 1) in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;

layout (set = 2, binding = 0) uniform sampler2D diffuse;

void main()
{
	FragColor = texture(diffuse, TexCoord);
}
