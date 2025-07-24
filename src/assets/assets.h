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

#ifndef ASSETS_H
#define ASSETS_H

#include <SDL3/SDL.h>
#include <linmath.h>

/*******************************************************************
 * STRUCTURES AND ENUMS ********************************************
 ******************************************************************/

typedef struct Color
{
	float r, g, b, a;
} Color;

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

/* TEXTURES */
typedef struct Texture2D
{
	SDL_GPUTexture *texture;
	SDL_Surface *surface;
} Texture2D;

/* SKYBOXES */
/*typedef struct Skybox
{
	SDL_GPUTexture *gputexture;
	SDL_GPUBuffer *vertex_buffer;
	SDL_GPUBuffer *index_buffer;
} Skybox;*/

/* MODELS */

typedef struct Vertex3D
{
	Vector3 position;
	Vector2 uv;
	//not really used
	//Vector3 normal;
	//Vector3 tangent;
	//Color color;
	//Uint8 blend_indices[4];
	//Uint8 blend_weights[4];
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
	//RAM buffers
	VertexArray varray;
	IndexArray iarray;
	//GPU buffers
	SDL_GPUBuffer *vbuffer;
	SDL_GPUBuffer *ibuffer;
	//texture
	Texture2D diffuse;
	//mesh name
	char meshname[64];
} Mesh;

//auxilliary
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
} Model;


/*******************************************************************
 * FUNCTIONS *******************************************************
 ******************************************************************/

/* CAMERA */

void InitCameraBasic(Camera *camera, Vector3 position,
						float aspect_ratio);

void InitCameraFull(Camera *camera, Vector3 position, Vector3 up,
						float yaw, float pitch, float roll,
						float zoom, float aspect_ratio);

void UpdateCameraPosition(Camera *camera, Vector3 position);

void TestCameraFreecam(Camera *camera, float x_offset,
						float y_offset, bool constraint);

/* TEXTURES */

//also uploads to gpu, be careful
bool LoadTextureFile(SDL_GPUDevice *device, Texture2D *texture,
						const char *path);

void ReleaseTexture2D(SDL_GPUDevice *device, Texture2D *texture);

/* SKYBOXES */
//TODO

/* MODELS */

bool ImportIQM(SDL_GPUDevice *device, Model *model,
				const char *iqmfile);

void ReleaseModel(SDL_GPUDevice *device, Model *model);

#endif
