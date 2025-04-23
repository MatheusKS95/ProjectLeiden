#version 450
layout (location = 0) in vec3 FragPos;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec4 Tangent;
layout (location = 4) in vec4 VColor;

layout (location = 0) out vec4 FragColor;

#define MAX_POINT_LIGHTS 256

layout (set = 2, binding = 0) uniform sampler2D diffuse_map;
#layout (set = 2, binding = 1) uniform sampler2D normal_map;
#layout (set = 2, binding = 2) uniform sampler2D specular_map;
#layout (set = 2, binding = 3) uniform sampler2D emission_map;
#layout (set = 2, binding = 4) uniform sampler2D height_map;

layout (set = 3, binding = 0) uniform MaterialBlock
{
	bool has_diffusemap;
	bool has_normalmap;
	bool has_specularmap;
	bool has_emissionmap;
	bool has_heightmap;
	vec3 diffuse;
	vec3 specular;
	vec3 ambient;
	float shininess;
	float emission;
};

void main()
{
	if(has_diffusemap)
	{
		FragColor = texture(diffuse_map, TexCoord);
	}
	else
	{
		FragColor = vec4(diffuse, 1.0);
	}
}
