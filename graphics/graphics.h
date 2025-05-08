/*
 * Copyright (C) 2025 Matheus Klein Schaefer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>. 
 */

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
	Vector3 position;
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

/*
 * Ultimate goal: drop all of this and either:
 * 1) Compile shader at runtime and keep SPIR-V in memory (doesn't
 *    need to keep track of generated SPIR-V and always up to date,
 *    but it's slower).
 * 2) First run, build SPIR-V and store in the userfolder.
 *    If it's in the userfolder, use them. Otherwise, re-generate
 *    them (might need to keep track of latest shader version and
 *    cleanup older builds).
*/

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

typedef struct Mesh
{
	VertexArray vertices;
	IndexArray indices;
	SDL_GPUBuffer *vbuffer;
	SDL_GPUBuffer *ibuffer;

	//material
	Texture2D *diffuse_map;
	Texture2D *normal_map;
	Texture2D *specular_map;
	Texture2D *emission_map;
	Texture2D *height_map; //aka bump map
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
 * SCENE - DROP THIS
 ******************************************************************/

typedef struct PointLightArray
{
	size_t count;
	size_t capacity;
	Pointlight *pointlights;
} PointLightArray;

typedef struct ModelArray
{
	size_t count;
	size_t capacity;
	Model *models;
} ModelArray;

//For cel shading (aka toon shader, anime shader)
//more advanced, three (or more) passes needed
typedef struct AnimePipeline
{
	Pipeline norm;
	Pipeline outline;
	Pipeline toon;
	Sampler *sampler;
} AnimePipeline;

//for retro 3D (like PS1, N64, Saturn)
//super basic, one pass shader
typedef struct FifthGenPipeline
{
	Pipeline fifthgen;
	Sampler *sampler;
} FifthGenPipeline;

typedef enum PipelineRenderingType
{
	PIPELINE_ANIME = 0,
	PIPELINE_5THGEN
} PipelineRenderingType;

//NOTICE: GraphicsScene is the Graphics part of a bigger scene
//structure. There will be an AudioScene and a PhysicsScene later,
//also part of a later all-encopassing Scene.
typedef struct GraphicsScene
{
	//status regarding if it's on GPU (true = uploaded to GPU)
	bool models_uploaded;
	bool plights_uploaded;

	//light arrays (before upload) and light buffers (after upload)
	//none used by the fifth-gen pipeline
	PointLightArray plightarray; //memory array
	SDL_GPUBuffer *plightbuffer; //buffer for uploading

	//model array (own uploading)
	ModelArray modelarray;

	//rendering style TODO rename
	PipelineRenderingType type;
	union
	{
		AnimePipeline anime;
		FifthGenPipeline fifthgen;
	};
	//TODO add other things like other light sources
	//buffers for lights, skyboxes...
} GraphicsScene;

bool Graphics_CreateScene(GraphicsScene *scene,
							PipelineRenderingType type);

bool Graphics_AddModelToScene(GraphicsScene *scene, Model *model);

bool Graphics_RemoveModelFromScene(GraphicsScene *scene,
									size_t index,
									Model *model);

bool Graphics_ClearModelsFromScene(GraphicsScene *scene);

void Graphics_UploadModelsFromScene(GraphicsScene *scene);

bool Graphics_AddPointlightToScene(GraphicsScene *scene,
									Pointlight *pointlight);

bool Graphics_RemovePointlightFromScene(GraphicsScene *scene,
										size_t index,
										Pointlight *pointlight);

bool Graphics_ClearPointlightsFromScene(GraphicsScene *scene);

void Graphics_UploadPointlightsFromScene(GraphicsScene *scene);

/*******************************************************************
 ******************************************************************/

/*******************************************************************
 * RENDERER - REWORK
 ******************************************************************/

typedef struct Renderer
{
	SDL_GPUCommandBuffer *cmdbuf;
	SDL_GPUTexture *swapchain_texture;
	SDL_GPURenderPass *render_pass;
	Color clear_color;
	SDL_GPUTexture *texture_depth;
} Renderer;

typedef SDL_GPUBuffer StorageBuffer;

//it's freaking huge
typedef struct RenderingStageDesc
{
	Pipeline pipeline;
	Sampler *sampler;

	//vertex
	void *vertex_ubo;
	size_t vertex_ubo_size;
	StorageBuffer **vert_storage_buffers;
	size_t vert_storage_buffers_count;
	//you need to extract the texture from inside Texture2D, TODO
	SDL_GPUTexture **vert_storage_textures;
	size_t vert_storage_textures_count;

	//fragment
	void *fragment_ubo;
	size_t fragment_ubo_size;
	StorageBuffer **frag_storage_buffers;
	size_t frag_storage_buffers_count;
	//you need to extract the texture from inside Texture2D, TODO
	SDL_GPUTexture **frag_storage_textures;
	size_t frag_storage_textures_count;

	//texture overrides (will be rendered instead of the ones
	//in the mesh) - keep null if unneeded
	//fragment shader only
	Texture2D *diffuse_map_or;
	Texture2D *normal_map_or;
	Texture2D *specular_map_or;
	Texture2D *emission_map_or;
	Texture2D *height_map_or;
} RenderingStageDesc;

bool Graphics_CreateAndUploadStorageBuffer(StorageBuffer *buffer,
									void *data, size_t size);

void Graphics_ReleaseStorageBuffer(StorageBuffer *buffer);

void Graphics_CreateRenderer(Renderer *renderer, Color clear_color);

void Graphics_BeginDrawing(Renderer *renderer);

void Graphics_EndDrawing(Renderer *renderer);

//test, will be removed when scene renderer is done
void Graphics_DrawModelT1(Model *model, Renderer *renderer,
							Pipeline pipeline, Matrix4x4 mvp,
							Sampler *sampler);

void Graphics_DrawMesh(Mesh *mesh, Renderer *renderer,
						RenderingStageDesc *desc);

void Graphics_DrawModel(Model *model, Renderer *renderer,
						RenderingStageDesc *desc);

/*******************************************************************
 ******************************************************************/

#endif
