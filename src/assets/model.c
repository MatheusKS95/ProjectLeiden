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

#include <iqm.h>
#include <assets.h>
#include <fileio.h>

/**************************************************************************************
 * ARRAY HELPERS (vertices, indices, textures, meshes and materials
***************************************************************************************/
static bool _arrayRealocVertex(VertexArray *arr, size_t new_size)
{
	if(arr->vertices != NULL)
	{
		Vertex3D *aux = (Vertex3D*)SDL_realloc(arr->vertices, sizeof(Vertex3D) * new_size);
		if(aux != NULL)
		{
			arr->vertices = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static void _arrayInitVertex(VertexArray *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->vertices = (Vertex3D*)SDL_calloc(arr->capacity, sizeof(Vertex3D));
}

static bool _arrayPushLastVertex(VertexArray *arr, Vertex3D value)
{
	if(arr->capacity == arr->count)
	{
		if(!_arrayRealocVertex(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->vertices[arr->count - 1] = value;
	return true;
}

static bool _arrayPopAtVertex(VertexArray *arr, unsigned int index, Vertex3D *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->vertices[index];
	SDL_memmove(&arr->vertices[index], &arr->vertices[index + 1], sizeof(Vertex3D) * ((arr->count - 1) - index));
	arr->count--;
	_arrayRealocVertex(arr, arr->capacity - 1);
	return true;
}

static void _arrayDestroyVertex(VertexArray *arr)
{
	SDL_free(arr->vertices);
	arr->vertices = NULL;
}

static void _arrayClearVertex(VertexArray *arr)
{
	_arrayDestroyVertex(arr);
	_arrayInitVertex(arr);
}

static bool _arrayRealocIndices(IndexArray *arr, size_t new_size)
{
	if(arr->indices != NULL)
	{
		Uint32 *aux = (Uint32*)SDL_realloc(arr->indices, sizeof(Uint32) * new_size);
		if(aux != NULL)
		{
			arr->indices = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static void _arrayInitIndices(IndexArray *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->indices = (Uint32*)SDL_calloc(arr->capacity, sizeof(Uint32));
}

static bool _arrayPushLastIndices(IndexArray *arr, Uint32 value)
{
	if(arr->capacity == arr->count)
	{
		if(!_arrayRealocIndices(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->indices[arr->count - 1] = value;
	return true;
}

static bool _arrayPopAtIndices(IndexArray *arr, unsigned int index, Uint32 *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->indices[index];
	SDL_memmove(&arr->indices[index], &arr->indices[index + 1], sizeof(Uint32) * ((arr->count - 1) - index));
	arr->count--;
	_arrayRealocIndices(arr, arr->capacity - 1);
	return true;
}

static void _arrayDestroyIndices(IndexArray *arr)
{
	SDL_free(arr->indices);
	arr->indices = NULL;
}

static void _arrayClearIndices(IndexArray *arr)
{
	_arrayDestroyIndices(arr);
	_arrayInitIndices(arr);
}

static bool _arrayRealocMeshes(MeshArray *arr, size_t new_size)
{
	if(arr->meshes != NULL)
	{
		Mesh *aux = (Mesh*)SDL_realloc(arr->meshes, sizeof(Mesh) * new_size);
		if(aux != NULL)
		{
			arr->meshes = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static void _arrayInitMeshes(MeshArray *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->meshes = (Mesh*)SDL_calloc(arr->capacity, sizeof(Mesh));
}

static bool _arrayPushLastMeshes(MeshArray *arr, Mesh value)
{
	if(arr->capacity == arr->count)
	{
		if(!_arrayRealocMeshes(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->meshes[arr->count - 1] = value;
	return true;
}

static bool _arrayPopAtMeshes(MeshArray *arr, unsigned int index, Mesh *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->meshes[index];
	SDL_memmove(&arr->meshes[index], &arr->meshes[index + 1], sizeof(Mesh) * ((arr->count - 1) - index));
	arr->count--;
	_arrayRealocMeshes(arr, arr->capacity - 1);
	return true;
}

static void _arrayDestroyMeshes(MeshArray *arr)
{
	SDL_free(arr->meshes);
	arr->meshes = NULL;
}

static void _arrayClearMeshes(MeshArray *arr)
{
	_arrayDestroyMeshes(arr);
	_arrayInitMeshes(arr);
}

static void uploadmesh(SDL_GPUDevice *device, Mesh *mesh)
{
	if(mesh == NULL) return;

	mesh->vbuffer = SDL_CreateGPUBuffer(
		device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = sizeof(Vertex3D) * mesh->varray.count
		}
	);

	mesh->ibuffer = SDL_CreateGPUBuffer(
		device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_INDEX,
			.size = sizeof(Uint32) * mesh->iarray.count
		}
	);

	SDL_GPUTransferBuffer* vbufferTransferBuffer = SDL_CreateGPUTransferBuffer(
		device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(Vertex3D) * mesh->varray.count)
		}
	);
	SDL_GPUTransferBuffer* ibufferTransferBuffer = SDL_CreateGPUTransferBuffer(
		device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(Uint32) * mesh->iarray.count)
		}
	);

	Vertex3D* bufferTransferData = SDL_MapGPUTransferBuffer(device, vbufferTransferBuffer, false);
	SDL_memcpy(bufferTransferData, mesh->varray.vertices, sizeof(Vertex3D) * mesh->varray.count);
	SDL_UnmapGPUTransferBuffer(device, vbufferTransferBuffer);
	Uint32* indexData = SDL_MapGPUTransferBuffer(device, ibufferTransferBuffer, false);
	SDL_memcpy(indexData, mesh->iarray.indices, sizeof(Uint32) * mesh->iarray.count);
	SDL_UnmapGPUTransferBuffer(device, ibufferTransferBuffer);

	// Upload the transfer data to the GPU buffers
	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdbuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = vbufferTransferBuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = mesh->vbuffer,
			.offset = 0,
			.size = sizeof(Vertex3D) * mesh->varray.count
		},
		false
	);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = ibufferTransferBuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = mesh->ibuffer,
			.offset = 0,
			.size = sizeof(Uint32) * mesh->iarray.count
		},
		false
	);
	SDL_EndGPUCopyPass(copyPass);

	SDL_ReleaseGPUTransferBuffer(device, vbufferTransferBuffer);
	SDL_ReleaseGPUTransferBuffer(device, ibufferTransferBuffer);

	SDL_SubmitGPUCommandBuffer(cmdbuf);

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Mesh %s uploaded.", mesh->meshname);
}

//FIXME incomplete untested
bool ImportIQM(SDL_GPUDevice *device, Model *model,
				const char *iqmfile)
{
	size_t iqmfilesize;
	Uint8 *iqmbuffer = FileIOReadBytes(iqmfile, &iqmfilesize);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Graphics: Info: Starting to load %s", iqmfile);
	if(model == NULL || iqmbuffer == NULL || iqmfilesize <= 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Invalid model structure or invalid file.");
		return false;
	}

	struct iqmheader header;
	SDL_memcpy(header.magic, iqmbuffer, 16);
	unsigned int *head = (unsigned int *)&iqmbuffer[16];
	header.version = head[0];
	if(SDL_strcmp(header.magic, IQM_MAGIC) != 0 || header.version != IQM_VERSION)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to load IQM model - invalid IQM file.");
		return false;
	}

	SDL_memcpy(&header, iqmbuffer, sizeof(struct iqmheader));

	struct iqmmesh *meshes = (struct iqmmesh *)&iqmbuffer[header.ofs_meshes];
	char *file_texts = header.ofs_text ? (char *)&iqmbuffer[header.ofs_text] : "";

	Vertex3D *vertices = (Vertex3D*)SDL_malloc(sizeof(Vertex3D) * (header.num_vertexes));
	float *position, *uv, *normal, *tangent;
	Uint8 *blend_indexes, *blend_weights, *color;
	struct iqmvertexarray *vertarrs = (struct iqmvertexarray *)&iqmbuffer[header.ofs_vertexarrays];

	for(int i = 0; i < header.num_vertexarrays; i++)
	{
		struct iqmvertexarray vertarr = vertarrs[i];

		switch(vertarr.type)
		{
			case IQM_POSITION:
			{
				position = (float *)&iqmbuffer[vertarr.offset];
				for(int x = 0; x < header.num_vertexes; x++)
				{
					SDL_memcpy(&vertices[x].position, &position[x * vertarr.size], vertarr.size * sizeof(float));
				}
				break;
			}
			case IQM_TEXCOORD:
			{
				uv = (float *)&iqmbuffer[vertarr.offset];
				for(int x = 0; x < header.num_vertexes; x++)
				{
					SDL_memcpy(&vertices[x].uv, &uv[x * vertarr.size], vertarr.size * sizeof(float));
				}
				break;
			}
			case IQM_NORMAL:
			{
				normal = (float *)&iqmbuffer[vertarr.offset];
				for(int x = 0; x < header.num_vertexes; x++)
				{
					SDL_memcpy(&vertices[x].normal, &normal[x * vertarr.size], vertarr.size * sizeof(float));
				}
				break;
			}
			case IQM_TANGENT:
			{
				tangent = (float *)&iqmbuffer[vertarr.offset];
				for(int x = 0; x < header.num_vertexes; x++)
				{
					SDL_memcpy(&vertices[x].tangent, &tangent[x * vertarr.size], vertarr.size * sizeof(float));
				}
				break;
			}
			case IQM_BLENDINDEXES:
			{
				blend_indexes = (Uint8 *)&iqmbuffer[vertarr.offset];
				for(int x = 0; x < header.num_vertexes; x++)
				{
					SDL_memcpy(&vertices[x].blend_indices, &blend_indexes[x * vertarr.size], vertarr.size * sizeof(Uint8));
				}
				break;
			}
			case IQM_BLENDWEIGHTS:
			{
				blend_weights = (uint8_t *)&iqmbuffer[vertarr.offset];
				for(int x = 0; x < header.num_vertexes; x++)
				{
					SDL_memcpy(&vertices[x].blend_weights, &blend_weights[x * vertarr.size], vertarr.size * sizeof(Uint8));
				}
				break;
			}
			case IQM_COLOR:
			{
				color = (uint8_t *)&iqmbuffer[vertarr.offset];
				for(int x = 0; x < header.num_vertexes; x++)
				{
					SDL_memcpy(&vertices[x].color, &color[x * vertarr.size], vertarr.size * sizeof(Uint8));
				}
			}
		}
	}
	//TODO process bones and joints (need to create structures to handle them)
	//TODO process animations (also need structures to handle them)

	//indices
	Uint32 *indices = (Uint32*)SDL_malloc(sizeof(Uint32) * (header.num_triangles * 3));
	SDL_memcpy(indices, &iqmbuffer[header.ofs_triangles], (header.num_triangles * 3) * sizeof(Uint32));

	_arrayInitMeshes(&model->meshes);

	//add meshes to the model
	uint32_t index_offset = 0;
	for(int i = 0; i < header.num_meshes; i++)
	{
		Vertex3D *imported_vertices = &vertices[meshes[i].first_vertex];
		uint32_t *imported_indices = &indices[meshes[i].first_triangle * 3];

		uint32_t offset = 0;
		for(int k = 0; k < meshes[i].num_triangles * 3; k++)
		{
			imported_indices[k] -= index_offset;
			if(imported_indices[k] > offset)
			{
				offset = imported_indices[k];
			}
		}
		index_offset += ++offset;

		uint16_t vcount = meshes[i].num_vertexes;
		uint16_t icount = meshes[i].num_triangles * 3;

		// get material and texture names
		char *iqm_material = &file_texts[meshes[i].material];
		char *iqm_mesh_name = &file_texts[meshes[i].name];

		Mesh mesh = { 0 };
		_arrayInitVertex(&mesh.varray);
		for(unsigned int m = 0; m < vcount; m++)
		{
			Vertex3D vert = imported_vertices[m];
			if(!_arrayPushLastVertex(&mesh.varray, vert))
			{
				SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to load IQM model - unable to copy vertices.");
				SDL_free(vertices);
				vertices = NULL;
				SDL_free(indices);
				indices = NULL;
				_arrayDestroyVertex(&mesh.varray);
				return false;
			}
		}
		_arrayInitIndices(&mesh.iarray);
		for(unsigned int n = 0; n < icount; n++)
		{
			Uint32 indice = imported_indices[n];
			if(!_arrayPushLastIndices(&mesh.iarray, indice))
			{
				SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to load IQM model - unable to copy indices.");
				SDL_free(vertices);
				vertices = NULL;
				SDL_free(indices);
				indices = NULL;
				_arrayDestroyIndices(&mesh.iarray);
				return false;
			}
		}
		SDL_snprintf(mesh.meshname, 64, "%s", iqm_mesh_name);

		char path_copy[512];
		SDL_strlcpy(path_copy, iqmfile, sizeof(path_copy));
		path_copy[sizeof(path_copy) - 1] = '\0';
		char *dirpath = FileIOGetDirName(path_copy);
		char fullpath[1024];
		SDL_snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, iqm_material);
		if(!LoadTextureFile(device, &mesh.diffuse, fullpath))
		{
			SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to load model's texture.");
		}

		//everything might be ok here, so i can finally upload the mesh
		uploadmesh(device, &mesh);

		//DynarrayClearVertex(&mesh->arrv);
		//DynarrayClearIndices(&mesh->arri);

		//TODO CLEANUP if false, also cleanup arrays
		if(!_arrayPushLastMeshes(&model->meshes, mesh))
		{
			SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to load IQM model - unable to copy meshes.");
			SDL_free(vertices);
			vertices = NULL;
			SDL_free(indices);
			indices = NULL;
			return false;
		}
	}

	SDL_free(vertices);
	vertices = NULL;
	SDL_free(indices);
	indices = NULL;

	return true;
}

void ReleaseModel(SDL_GPUDevice *device, Model *model)
{
	for(Uint32 i = 0; i < model->meshes.count; i++)
	{
		//destroy buffers
		SDL_ReleaseGPUBuffer(device, model->meshes.meshes[i].vbuffer);
		SDL_ReleaseGPUBuffer(device, model->meshes.meshes[i].ibuffer);

		ReleaseTexture2D(device, &model->meshes.meshes[i].diffuse);

		//destroy arrays
		_arrayDestroyIndices(&model->meshes.meshes[i].iarray);
		_arrayDestroyVertex(&model->meshes.meshes[i].varray);
	}
	//finally, destroy meshes
	_arrayDestroyMeshes(&model->meshes);
}
