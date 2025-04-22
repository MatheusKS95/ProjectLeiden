#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL3/SDL.h>
#include <linmath.h>
#include <hashtable.h>

/*******************************************************************
 * GENERAL GRAPHICAL STUFF
 ******************************************************************/

typedef struct GraphicsContext
{
	unsigned int width;
	unsigned int height;
	SDL_Window *window;
	SDL_GPUDevice *device;
} GraphicsContext;

extern GraphicsContext context;

bool Graphics_Init();

void Graphics_Deinit();

void Graphics_SetContext(SDL_Window *window,
							unsigned int width,
							unsigned int height);

/*******************************************************************
 ******************************************************************/

/*******************************************************************
 * CAMERA
 ******************************************************************/

typedef struct Camera
{
	//standard camera variables
	Vector3 position;
	Vector3 front;
	Vector3 up;
	Vector3 right;
	Vector3 world_up;
	//euler angles
	float yaw;
	float pitch;
	float roll;
	//options
	float zoom;
	float aspect_ratio;
	//transforms
	Matrix4x4 view;
	Matrix4x4 projection;
} Camera;

void Graphics_InitCameraBasic(Camera *camera, Vector3 position);

void Graphics_InitCameraFull(Camera *camera, Vector3 position,
								Vector3 up, float yaw, float pitch,
								float roll, float zoom);

void Graphics_UpdateCameraPosition(Camera *camera, Vector3 position);

void Graphics_TestCameraFreecam(Camera *camera, float x_offset,
								float y_offset, bool constraint);

/*******************************************************************
 ******************************************************************/

/*******************************************************************
 * LIGHTS
 ******************************************************************/

typedef struct Pointlight
{
	Vector3 rel_position;
	Vector3 colour;
	//vec3 intensity;

	//attenuation = 1.0 / constant + (linear * distance) + (quadratic * distance^2)
	float constant;
	float linear;
	float quadratic;
} Pointlight;

typedef struct Spotlight
{
	Vector3 rel_position;
	Vector3 rel_rotation;
	Vector3 colour;
	//vec3 intensity;

	//attenuation = 1.0 / constant + (linear * distance) + (quadratic * distance^2)
	float constant;
	float linear;
	float quadratic;

	float inner_cutoff;
	float outer_cutoff;
} Spotlight;

typedef struct Dirlight
{
	Vector3 direction;
	Vector3 colour;
	//vec3 intensity;
} Dirlight;

bool Graphics_NewPointlight(Pointlight *l, Vector3 position,
							Vector3 colour, float constant,
							float linear, float quadratic);

void Graphics_DestroyPointlight(Pointlight *l);

bool Graphics_NewDirlight(Dirlight *l, Vector3 direction,
							Vector3 colour);

void Graphics_DestroyDirlight(Dirlight *l);

bool Graphics_NewSpotlight(Spotlight *l, Vector3 position,
							Vector3 rotation, Vector3 colour,
							float constant, float linear,
							float quadratic, float inctf,
							float otctf);

void Graphics_DestroySpotlight(Spotlight *l);

/*******************************************************************
 ******************************************************************/

/*******************************************************************
 * SHADERS
 ******************************************************************/

typedef enum ShaderStage
{
	SHADERSTAGE_NONE = 0,
	SHADERSTAGE_VERTEX,
	SHADERSTAGE_FRAGMENT
} ShaderStage;

typedef struct Shader
{
	SDL_GPUShader *shader;
} Shader;

typedef enum PipelineType
{
	PIPELINETYPE_3D = 0, //generic, to render a 3D object
	PIPELINETYPE_RENDERTOTEXTURE //generic, to be used for framebuffers
} PipelineType;

typedef SDL_GPUGraphicsPipeline* Pipeline;

bool Graphics_LoadShaderFromMem(Shader *shader,
								uint8_t *buffer, size_t size,
								const char *entrypoint,
								ShaderStage stage,
								Uint32 samplerCount,
								Uint32 uniformBufferCount,
								Uint32 storageBufferCount,
								Uint32 storageTextureCount);

bool Graphics_LoadShaderFromFS(Shader *shader,
								const char *path,
								const char *entrypoint,
								ShaderStage stage,
								Uint32 samplerCount,
								Uint32 uniformBufferCount,
								Uint32 storageBufferCount,
								Uint32 storageTextureCount);

Pipeline Graphics_CreatePipeline(Shader *vs, Shader *fs,
									PipelineType type,
									bool release_shader);

/*******************************************************************
 ******************************************************************/

/*******************************************************************
 * 2D TEXTURES
 ******************************************************************/
/*
 * Notice: Project Leiden doesn't support PBR at this moment
*/

typedef enum SamplerFilter
{
	SAMPLER_FILTER_LINEAR = 0,
	SAMPLER_FILTER_NEAREST
} SamplerFilter;

typedef enum SamplerMode
{
	SAMPLER_MODE_REPEAT = 0,
	SAMPLER_MODE_CLAMPTOEDGE,
	SAMPLER_MODE_MIRROREDREPEAT
} SamplerMode;

typedef enum TextureType
{
	TEXTURE_DIFFUSE = 0,
	TEXTURE_NORMAL,
	TEXTURE_SPECULAR,
	TEXTURE_EMISSION,
	TEXTURE_HEIGHT,
	TEXTURE_DEFAULT //unused, placeholder
} TextureType;

typedef SDL_GPUSampler Sampler;

typedef struct Texture2D
{
	SDL_GPUTexture *texture;
	TextureType type;
	SDL_Surface *surface;
} Texture2D;

Sampler* Graphics_GenerateSampler(SamplerFilter filter, SamplerMode mode);

void Graphics_ReleaseSampler(Sampler *sampler);

bool Graphics_LoadTextureFromMem(Texture2D *texture,
									uint8_t *buffer, size_t size,
									TextureType type);

bool Graphics_LoadTextureFromFS(Texture2D *texture,
								const char *path,
								TextureType type);

void Graphics_ReleaseTexture(Texture2D *texture);

void Graphics_UploadTexture(Texture2D *texture);

/*******************************************************************
 ******************************************************************/

/*******************************************************************
 * MODELS
 ******************************************************************/

typedef struct Color
{
	float r, g, b, a;
} Color;

typedef struct Vertex
{
	Vector3 position;
	Vector2 uv;
	Vector3 normal;
	Vector3 tangent;
	Color color;
	//TODO expand later for animations
} Vertex;

typedef struct VertexArray
{
	size_t count;
	size_t capacity;
	Vertex *vertices;
} VertexArray;

typedef struct IndexArray
{
	size_t count;
	size_t capacity;
	Uint32 *indices;
} IndexArray;

typedef struct Material
{
	Vector3 diffuse;
	Vector3 specular;
	Vector3 ambient;
	float shininess;
	float emission;
	Texture2D *textures[TEXTURE_DEFAULT];
	char name[64];
} Material;

typedef struct Mesh
{
	VertexArray vertices;
	IndexArray indices;
	SDL_GPUBuffer *vbuffer;
	SDL_GPUBuffer *ibuffer;
	Material material;
	char meshname[64];
} Mesh;

typedef struct MeshArray
{
	size_t count;
	size_t capacity;
	Mesh *meshes;
} MeshArray;

typedef struct Model
{
	MeshArray meshes;
	Matrix4x4 transform;
} Model;

bool Graphics_ImportIQM(Model *model, const char *iqmfile,
							const char *materialfile);

void Graphics_UploadModel(Model *model, bool upload_textures);

void Graphics_MoveModel(Model *model, Vector3 position);

void Graphics_ScaleModel(Model *model, float scale);

void Graphics_RotateModel(Model *model, Vector3 axis,
							float radians);

void Graphics_ReleaseModel(Model *model);

/*******************************************************************
 ******************************************************************/

/*******************************************************************
 * RENDERER - TODO
 ******************************************************************/

typedef struct Renderer
{
	SDL_GPUCommandBuffer *cmdbuf;
	SDL_GPUTexture *swapchain_texture;
	SDL_GPURenderPass *render_pass;
	Color clear_color;
	SDL_GPUTexture *texture_depth;
} Renderer;

typedef struct PointLightArray
{
	size_t count;
	size_t capacity;
	Pointlight *pointlights;
} PointLightArray;

void Graphics_CreatePointlightArray(PointLightArray *array);

bool Graphics_LightArrayAddPointlight(PointLightArray *array,
										Pointlight pointlight);

void Graphics_ClearLightArray(PointLightArray *array);

void Graphics_CreateRenderer(Renderer *renderer, Color clear_color);

void Graphics_BeginDrawing(Renderer *renderer);

void Graphics_EndDrawing(Renderer *renderer);

//test
void Graphics_DrawModelT1(Model *model, Renderer *renderer,
							Pipeline pipeline, Matrix4x4 mvp,
							Sampler *sampler);

/*******************************************************************
 ******************************************************************/


#endif
