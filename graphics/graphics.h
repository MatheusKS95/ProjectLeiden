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
 * TYPEDEFS ********************************************************
 ******************************************************************/

typedef SDL_GPUGraphicsPipeline Pipeline;
typedef SDL_GPUShader Shader;
typedef SDL_GPUSampler Sampler;
typedef SDL_GPUBuffer StorageBuffer; //TODO remove this
typedef SDL_GPUBuffer GPUBuffer;
typedef SDL_GPUCommandBuffer CommandBuffer;
typedef SDL_GPUTexture GPUTexture;
typedef SDL_GPURenderPass RenderPass;

/*******************************************************************
 * STRUCTURES AND ENUMS ********************************************
 ******************************************************************/

/* GENERAL USE */

typedef struct GraphicsContext
{
	//WINDOW
	unsigned int width;
	unsigned int height;
	SDL_Window *window;

	//GPU DEVICE
	SDL_GPUDevice *device;
} GraphicsContext;

/* CAMERA */

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

/* LIGHTS */

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

/* 2D TEXTURES */

typedef enum SamplerFilter
{
	SAMPLER_FILTER_LINEAR = 0,
	SAMPLER_FILTER_NEAREST
} SamplerFilter;

typedef enum SamplerMipmapMode
{
	SAMPLER_MIPMAPMODE_LINEAR = 0,
	SAMPLER_MIPMAPMODE_NEAREST
} SamplerMipmapMode;

typedef enum SamplerAddressMode
{
	SAMPLER_MODE_REPEAT = 0,
	SAMPLER_MODE_CLAMPTOEDGE,
	SAMPLER_MODE_MIRROREDREPEAT
} SamplerAddressMode;

typedef struct Texture2D
{
	GPUTexture *texture;
	SDL_Surface *surface;
} Texture2D;

typedef struct DefaultTextures
{
	Texture2D default_diffuse;
	//Texture2D default_normal; //not needed here
	//Texture2D default_spec; //not needed here
} DefaultTextures;

/* SKYBOXES */
typedef struct Skybox
{
	SDL_Surface *surface[6];
	GPUTexture *gputexture;
	Sampler *sampler;
	SDL_GPUBuffer* vertex_buffer;
	SDL_GPUBuffer* index_buffer;
	Pipeline *pipeline;
} Skybox;

/* MATERIAL */

typedef struct Material
{
	Texture2D *diffuse_map;
	Texture2D *normal_map;
	Texture2D *specular_map;
	Texture2D *emission_map;
	char material_name[64];
} Material;

typedef struct MaterialArray
{
	size_t count;
	size_t capacity;
	Material *materials;
} MaterialArray;

/* MODELS */

typedef struct Color
{
	float r, g, b, a;
} Color;

typedef struct Vertex3D
{
	Vector3 position;
	Vector2 uv;
	Vector3 normal;
	Vector3 tangent;
	Color color;
	Uint8 blend_indices[4];
	Uint8 blend_weights[4];
} Vertex3D;

typedef struct VertexArray
{
	size_t count;
	size_t capacity;
	Vertex3D *vertices;
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
	GPUBuffer *vbuffer;
	GPUBuffer *ibuffer;
	char material_name[64];

	char meshname[64];
} Mesh;

/*in preparation for future work
typedef struct Bone
{
	char bone_name[64];
	int parent;
	Vector3 position, scale;
	Vector4 rotation;
	Matrix4x4 transform;
} Bone;

typedef struct Anim
{
	char anim_name[64];
	Uint32 first, last;
	float rate;
	bool loop;
} Anim;

typedef struct Pose
{
	Vector3 translation, scale;
	Vector4 rotation;
} Pose;*/

typedef struct MeshArray
{
	size_t count;
	size_t capacity;
	Mesh *meshes;
} MeshArray;

//might be broken up into several specialized types
typedef struct Model
{
	MeshArray meshes;
	MaterialArray materials;
	Matrix4x4 transform;
} Model;

/* EFFECTS (aka POST-PROCESSING) */

typedef struct EffectBuffers
{
	GPUBuffer *effect_vbuffer;
	GPUBuffer *effect_ibuffer;
} EffectBuffers;

/*******************************************************************
 * FUNCTIONS *******************************************************
 ******************************************************************/

/* GENERAL USE */

bool Graphics_Init();

void Graphics_Deinit();

void Graphics_SetContext(SDL_Window *window,
							unsigned int width,
							unsigned int height);

/* CAMERA */

void Graphics_InitCameraBasic(Camera *camera, Vector3 position);

void Graphics_InitCameraFull(Camera *camera, Vector3 position,
								Vector3 up, float yaw, float pitch,
								float roll, float zoom);

void Graphics_UpdateCameraPosition(Camera *camera, Vector3 position);

void Graphics_TestCameraFreecam(Camera *camera, float x_offset,
								float y_offset, bool constraint);

/* LIGHTS */

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

/* SHADERS AND PIPELINES */

Shader* Graphics_LoadShader(const char *path,
							SDL_GPUShaderStage stage,
							Uint32 samplerCount,
							Uint32 uniformBufferCount,
							Uint32 storageBufferCount,
							Uint32 storageTextureCount);

Pipeline *Graphics_Generate3DPipelineFull(Shader *vs, Shader *fs,
											bool release_shaders);

/* 2D TEXTURES */

Sampler* Graphics_GenerateSampler(SamplerFilter filter,
									SamplerMipmapMode mpmode,
									SamplerAddressMode addmode);

void Graphics_ReleaseSampler(Sampler *sampler);

bool Graphics_LoadTextureFromMem(Texture2D *texture,
									uint8_t *buffer, size_t size);

bool Graphics_LoadTextureFromFS(Texture2D *texture,
								const char *path);

void Graphics_ReleaseTexture(Texture2D *texture);

void Graphics_UploadTexture(Texture2D *texture);

GPUTexture *Graphics_GenerateDepthTexture(int width, int height);

GPUTexture *Graphics_GenerateRenderTexture(int width, int height);

/* SKYBOXES */

bool Graphics_CreatePipelineSkybox(Skybox *skybox,
									const char *path_vs,
									const char *path_fs);

bool Graphics_LoadSkyboxFS(Skybox *skybox, const char *path_up,
							const char *path_down,
							const char *path_left,
							const char *path_right,
							const char *path_front,
							const char *path_back);

void Graphics_UploadSkybox(Skybox *skybox);

void Graphics_ReleaseSkybox(Skybox *skybox);

/* MATERIALS */

//TODO
/*bool Graphics_LoadMaterialsFromINI(MaterialArray *matarray,
									const char *path);*/

bool Graphics_LoadModelMaterials(Model *model,
									const char *material_path);

Material* Graphics_GetMaterialByName(MaterialArray *matarray,
										const char *name);

void Graphics_UploadMaterial(Material *material);

void Graphics_UploadMaterials(MaterialArray *materials);

void Graphics_ReleaseMaterial(Material *material);

void Graphics_ReleaseMaterials(MaterialArray *materials);

/* MODELS */

bool Graphics_ImportIQM(Model *model, const char *iqmfile);

void Graphics_UploadModel(Model *model, bool upload_textures);

void Graphics_MoveModel(Model *model, Vector3 position);

void Graphics_ScaleModel(Model *model, float scale);

void Graphics_RotateModel(Model *model, Vector3 axis,
							float radians);

void Graphics_ReleaseModel(Model *model);

/* RENDERING */
//things here are lower level due to the fact they are easier to
//work with that way - specially if i want to play around with
//more than one rendering style

bool Graphics_CreateAndUploadStorageBuffer(StorageBuffer *buffer,
									void *data, size_t size);

void Graphics_ReleaseStorageBuffer(StorageBuffer *buffer);

CommandBuffer *Graphics_SetupCommandBuffer();

void Graphics_CommitCommandBuffer(CommandBuffer *cmdbuf);

GPUTexture *Graphics_AcquireSwapchainTexture(CommandBuffer *cmdbuf);

RenderPass *Graphics_BeginRenderPass(CommandBuffer *cmdbuf,
										GPUTexture *render_texture,
										GPUTexture *depth_texture,
										Color clear_color);

void Graphics_EndRenderPass(RenderPass *renderpass);

void Graphics_BindPipeline(RenderPass *renderpass,
							Pipeline *pipeline);

void Graphics_BindVertexBuffers(RenderPass *renderpass,
								GPUBuffer *buffer,
								Uint32 buffer_offset,
								Uint32 first_slot,
								Uint32 num_bindings);

//NOTE: indices must be 32 bit
void Graphics_BindIndexBuffers(RenderPass *renderpass,
								GPUBuffer *buffer,
								Uint32 buffer_offset);

void Graphics_BindMeshBuffers(RenderPass *renderpass, Mesh *mesh);

void Graphics_BindFragmentSampledTexture(RenderPass *renderpass,
											Texture2D *texture,
											Sampler *sampler,
											Uint32 first_slot,
											Uint32 num_bindings);

void Graphics_BindFragmentSampledGPUTexture(RenderPass *renderpass,
											GPUTexture *texture,
											Sampler *sampler,
											Uint32 first_slot,
											Uint32 num_bindings);

void Graphics_PushVertexUniforms(CommandBuffer *cmdbuf,
									Uint32 slot, void *data,
									size_t length);

void Graphics_PushFragmentUniforms(CommandBuffer *cmdbuf,
									Uint32 slot, void *data,
									size_t length);

void Graphics_DrawPrimitives(RenderPass *renderpass,
								size_t num_indices,
								Uint32 num_instances,
								int first_index, int vertex_offset,
								Uint32 first_instance);

/* EFFECTS (aka POST-PROCESSING) */

Pipeline *Graphics_GenerateEffectsPipeline(Shader *vs, Shader *fs,
											bool release_shaders);

EffectBuffers *Graphics_GenerateEffectBuffers();

/*******************************************************************
 * GLOBALS *********************************************************
 ******************************************************************/

extern GraphicsContext context;

#endif
