#version 450
layout (location = 0) in vec3 FragPos;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec4 Tangent;
layout (location = 4) in vec4 VColor;

layout (location = 0) out vec4 FragColor;

layout (set = 2, binding = 0) uniform sampler2D diffuse;

layout (set = 3, binding = 0) uniform UniformBlock
{
	vec2 resolution;
};

vec3 get_edge()
{
	float x = 1.0 / resolution.x;
	float y = 1.0 / resolution.y;

	vec4 horizEdge = vec4(0.0);
	horizEdge -= texture(diffuse, vec2(TexCoord.x - x, TexCoord.y - y)) * 1.0;
	horizEdge -= texture(diffuse, vec2(TexCoord.x - x, TexCoord.y    )) * 2.0;
	horizEdge -= texture(diffuse, vec2(TexCoord.x - x, TexCoord.y + y)) * 1.0;
	horizEdge += texture(diffuse, vec2(TexCoord.x + x, TexCoord.y - y)) * 1.0;
	horizEdge += texture(diffuse, vec2(TexCoord.x + x, TexCoord.y    )) * 2.0;
	horizEdge += texture(diffuse, vec2(TexCoord.x + x, TexCoord.y + y)) * 1.0;

	vec4 vertEdge = vec4(0.0);
	vertEdge -= texture(diffuse, vec2(TexCoord.x - x, TexCoord.y - y)) * 1.0;
	vertEdge -= texture(diffuse, vec2(TexCoord.x    , TexCoord.y - y)) * 2.0;
	vertEdge -= texture(diffuse, vec2(TexCoord.x + x, TexCoord.y - y)) * 1.0;
	vertEdge += texture(diffuse, vec2(TexCoord.x - x, TexCoord.y + y)) * 1.0;
	vertEdge += texture(diffuse, vec2(TexCoord.x    , TexCoord.y + y)) * 2.0;
	vertEdge += texture(diffuse, vec2(TexCoord.x + x, TexCoord.y + y)) * 1.0;

	vec3 edge = sqrt((horizEdge.rgb * horizEdge.rgb) + (vertEdge.rgb * vertEdge.rgb));
	return edge;
}

void main()
{
	vec3 edge = 1.0 - get_edge();

	//finalColor = vec4(edge, 1.0);
	edge = 1.0 - edge;
	vec3 color = vec3( 1.0 - max(max(edge.x, edge.y), edge.z));
	FragColor = vec4(color , texture(diffuse, TexCoord).a);
}
