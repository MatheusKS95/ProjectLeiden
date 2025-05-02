#version 450
layout (location = 0) in vec3 FragPos;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec4 Tangent;
layout (location = 4) in vec4 VColor;
layout (location = 5) in vec3 ModNorm;

layout (location = 0) out vec4 FragColor;

//Texture (set = 2: sampled textures)
layout(set = 2, binding = 0) uniform sampler2D texture0;

//Storage buffer for lights (set = 2: after sampled textures)
//might change this later, it's based on a test i made with raylib
struct Light {
	int  enabled;
	int  type;
	vec3 position;
	vec3 target;
	vec4 color;
};
layout(std430, set = 2, binding = 1) buffer Lights {
	Light lights[];
};

//Uniform buffer (set = 3: uniform buffers)
layout(std140, set = 3, binding = 0) uniform UBO {
	vec4 colDiffuse;
	vec4 ambient;
	vec3 viewPos;
	// std140 needs 16 bytes alignment, but kept empty
	float pad0;
} ubo;

//Constants
#define MAX_LIGHTS       256
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT       1

void main()
{
	vec4 texelColor = texture(texture0, TexCoord);
	vec3 normal    = normalize(Normal);
	vec3 viewD     = normalize(ubo.viewPos - FragPos);
    
	float NdotL = 0.0;

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights[i].enabled == 1)
		{
			vec3 L;
			if (lights[i].type == LIGHT_DIRECTIONAL) {
				L = -normalize(lights[i].target - lights[i].position);
			} else { // LIGHT_POINT
				L = normalize(lights[i].position - FragPos);
			}
			float d = max(dot(normal, L), 0.0);
			NdotL = d;
		}
	}

	if      (NdotL > 0.95) NdotL = 1.0;
	else if (NdotL > 0.60) NdotL = 0.90;
	else if (NdotL > 0.40) NdotL = 0.80;
	else                   NdotL = 0.70;

	FragColor = texelColor * ubo.colDiffuse * NdotL;
	FragColor.a = 1.0;
}
