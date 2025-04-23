#version 450
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec4 in_tangent;
layout (location = 4) in vec4 in_vcolor;

layout (location = 0) out vec3 FragPos;
layout (location = 1) out vec2 TexCoord;
layout (location = 2) out vec3 Normal;
layout (location = 3) out vec4 Tangent;
layout (location = 4) out vec4 VColor;

layout (set = 1, binding = 0) uniform UniformBlock
{
	mat4 mvp;
};

void main()
{
	FragPos = in_pos;
	TexCoord = in_uv;
	Normal = in_normal;
	Tangent = in_tangent;
	VColor = in_vcolor;
	gl_Position = mvp * vec4(in_pos, 1.0);
}

