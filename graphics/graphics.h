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

typedef SDL_GPUGraphicsPipeline* Pipeline;
typedef SDL_GPUSampler Sampler;
typedef SDL_GPUBuffer StorageBuffer;

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

typedef struct GeneralPipelines
{
	SDL_GPUGraphicsPipeline *skybox;
	SDL_GPUGraphicsPipeline *simple;
	//TODO more
} GeneralPipelines;

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

typedef struct Texture2D
{
	SDL_GPUTexture *texture;
	TextureType type;
	SDL_Surface *surface;
} Texture2D;

typedef struct DefaultTextures
{
	Texture2D default_diffuse;
	Texture2D default_normal;
	Texture2D default_spec;
	Texture2D default_emission;
} DefaultTextures;

/* SKYBOXES */
typedef struct Skybox
{
	SDL_Surface *surface[6];
	SDL_GPUTexture *gputexture;
	Sampler *sampler;
	SDL_GPUBuffer* vertex_buffer;
	SDL_GPUBuffer* index_buffer;
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

typedef struct Vertex
{
	Vector3 position;
	Vector2 uv;
	Vector3 normal;
	Vector3 tangent;
	Color color;
	Uint8 blend_indices[4];
	Uint8 blend_weights[4];
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

typedef struct Model
{
	MeshArray meshes;
	MaterialArray materials;
	Matrix4x4 transform;
} Model;

/* RENDERER */

typedef struct SimpleRenderingSetup
{
	Model *models;
	Uint8 num_models;
	Skybox *skybox;
	Sampler *sampler;
} SimpleRenderingSetup;

//TODO
typedef struct ToonRenderingSetup
{
	Model *models;
	Uint8 num_models;
	Pointlight *pointlights;
	Uint8 num_pointlights;
	//TODO need dirlight and spotlights
	Skybox *skybox;
	Sampler *sampler;
} ToonRenderingSetup;

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

bool Graphics_CreatePipelineSkybox(const char *path_vs,
										const char *path_fs);

bool Graphics_CreatePipelineSimple(const char *path_vs,
									const char *path_fs);

/* 2D TEXTURES */

Sampler* Graphics_GenerateSampler(SamplerFilter filter,
									SamplerMode mode);

void Graphics_ReleaseSampler(Sampler *sampler);

bool Graphics_LoadTextureFromMem(Texture2D *texture,
									uint8_t *buffer, size_t size,
									TextureType type);

bool Graphics_LoadTextureFromFS(Texture2D *texture,
								const char *path,
								TextureType type);

void Graphics_ReleaseTexture(Texture2D *texture);

void Graphics_UploadTexture(Texture2D *texture);

bool Graphics_SetupDefaultTextures(const char *path_d,
									const char *path_n,
									const char *path_s,
									const char *path_e);

void Graphics_ReleaseDefaultTextures();

/* SKYBOXES */

bool Graphics_LoadSkyboxFS(Skybox *skybox, const char *path_up,
							const char *path_down,
							const char *path_left,
							const char *path_right,
							const char *path_front,
							const char *path_back);

void Graphics_UploadSkybox(Skybox *skybox);
//TODO FIXME create unload/destroy skybox

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

/* RENDERER */

void Graphics_PrepareSimpleRendering();

void Graphics_FinishSimpleRendering();

void Graphics_PrepareToonRendering();

bool Graphics_CreateAndUploadStorageBuffer(StorageBuffer *buffer,
									void *data, size_t size);

void Graphics_ReleaseStorageBuffer(StorageBuffer *buffer);

void Graphics_DrawSimple(SimpleRenderingSetup *stuff,
							Color clear_color,
							Camera *camera);

/*******************************************************************
 * GLOBALS *********************************************************
 ******************************************************************/

extern GraphicsContext context;
extern GeneralPipelines pipelines;
extern DefaultTextures default_textures;

#endif
