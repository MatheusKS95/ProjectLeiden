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

/**
 * @file models.c
 * @brief Implementation file for model stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <graphics.h>
#include <fileio.h>
#include <ini.h>
#include <iqm.h>

//FIXME a lot of stuff was disabled due to incoming changes to material and rendering

/*
 * About 3D model support:
 * > IQM: Official support, no dependency. External INI as material data.
 * > M3D: Not supported. Not planned.
 * > GLTF/GLB: Not supported. Planned (with cgltf).
 * > WAVEFRONT OBJ: Not supported. Not planned.
 * > FBX: Not supported. Not planned.
 * > COLLADA: Not supported. Not planned (legacy).
 * > CUSTOM: need further study.
 * > OTHER: either niche or legacy. Not supported. Not planned.
 *
 * IQM:
 * > Pros:
 * >> Easiest, quickest to load.
 * >> Support skeletal animations.
 * > Cons:
 * >> Old design, from Quake era.
 * >> No material besides a string.
 * >> No blendshapes and other stuff to animate faces.
 * I chose this because I already know how to load it. The material issue was solved
 * (more like MacGyverism kind of solved) using the existing INI parser due to
 * flexibility. Don't support blendshapes (used often to animate clothes and faces).
 * It's likely I'll replace IQM in favor of another format in a future version.
 *
 * M3D:
 * > Pros:
 * >> Quick to load.
 * >> Lightweight file.
 * >> Support skeletal animations.
 * >> Material included.
 * > Cons:
 * >> Complicated to load (even with the official header-only lib).
 * >> Not sure about the rest.
 * Support for this format was intended but, based on some design choices, I ended up
 * dropping the idea.
 *
 * GLTF/GLB:
 * > Pros:
 * >> Industry standard.
 * >> All the information needed, including embedded textures.
 * >> Support blendshapes and more advanced animations.
 * > Cons:
 * >> Very complicated to load (will require cgltf and recursion to help).
 * >> Scene graph is not compatible with Project Leiden's architecture.
 * >> Big, with lots of unnecessary information.
 * >> GLTF is text-based (JSON), thus inefficient to load, but GLB is binary.
 * Despite cons, support for this format is planned. Scene graph info will be
 * discarded upon loading and only mesh data and materials will be kept (pretty much
 * how raylib deals with gltf). A scene graph might be developed as default in a
 * future version, so GLB might become the official model/scene format.
 *
 * WAVEFRONT OBJ:
 * > Pros:
 * >> Despite age, still industry standard somehow.
 * >> Straightforward in design.
 * >> Flexible.
 * > Cons:
 * >> No material, requires separate file (mtl).
 * >> No animations.
 * >> Text based format is inefficient to load, string parsing can be dangerous.
 * Supporting this format was intended, but the amount of work required to parse it
 * and check all the possible variations of a given property made us drop it.
 * Official IQM cli converter tool is able to convert OBJ to IQM easily, so another
 * reason to drop OBJ.
 *
 * FBX
 * > Pros:
 * >> ?
 * > Cons:
 * >> Proprietary (yet there are open-source loaders).
 * I've seen a lot of implementations but seems like this format is not great for use
 * here. Also, official IQM cli converter tool is able to convert FBX to IQM.
 *
 * COLLADA (DAE)
 * > Pros:
 * >> Same as GLTF, except embedded textures (not big of a deal however)
 * > Cons:
 * >> No binary file.
 * >> Is text format (dae is xml), painful and inefficient to load without right tools.
 * >> Complicated format, with scene graph design, and lots of unneeded data.
 * >> Legacy, replaced by GLTF.
 * Painful to work with. Not even considered.
 *
 * CUSTOM FORMAT
 * This offers greatest freedom. We can include anything that's needed and avoid bloat.
 * However, this require further study to establish a specification and blender
 * plugins. This might be the format that will replace IQM, if we keep the engine
 * smaller and simpler (like raylib).
 *
 * OTHER FORMATS:
 * We don't plan to support any other format. Assimp could, but there are no benefit
 * into support niche formats or formats that could be easily imported into blender
 * and exported as either something supported or something that the IQM cli tools
 * support.
*/

/**************************************************************************************
 * ARRAY HELPERS (vertices, indices, textures, meshes and materials
***************************************************************************************/
static bool _arrayRealocVertex(VertexArray *arr, size_t new_size)
{
	if(arr->vertices != NULL)
	{
		Vertex *aux = (Vertex*)SDL_realloc(arr->vertices, sizeof(Vertex) * new_size);
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
	arr->vertices = (Vertex*)SDL_calloc(arr->capacity, sizeof(Vertex));
}

static bool _arrayPushLastVertex(VertexArray *arr, Vertex value)
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

static bool _arrayPopAtVertex(VertexArray *arr, unsigned int index, Vertex *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->vertices[index];
	SDL_memmove(&arr->vertices[index], &arr->vertices[index + 1], sizeof(Vertex) * ((arr->count - 1) - index));
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

/**************************************************************************************
 * From the header
***************************************************************************************/
static bool _import_iqm_buffer(Model *model, Uint8 *iqmbuffer,
									size_t iqmsize,
									const char *materialfile)
{
	if(model == NULL || iqmbuffer == NULL || iqmsize <= 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Invalid model structure or invalid file.");
		return false;
	}

	bool has_material = false;
	INIstruct *material_ini = ININew();
	if(materialfile != NULL)
	{
		if(INILoad(material_ini, materialfile))
			has_material = true;
	}

	struct iqmheader header;
	SDL_memcpy(header.magic, iqmbuffer, 16);
	unsigned int *head = (unsigned int *)&iqmbuffer[16];
	header.version = head[0];
	if(SDL_strcmp(header.magic, IQM_MAGIC) != 0 || header.version != IQM_VERSION)
	{
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to load IQM model - invalid IQM file.");
		return NULL;
	}

	SDL_memcpy(&header, iqmbuffer, sizeof(struct iqmheader));

	struct iqmmesh *meshes = (struct iqmmesh *)&iqmbuffer[header.ofs_meshes];
	char *file_texts = header.ofs_text ? (char *)&iqmbuffer[header.ofs_text] : "";

	Vertex *vertices = (Vertex*)SDL_malloc(sizeof(Vertex) * (header.num_vertexes));
	float *position, *uv, *normal, *tangent;
	uint8_t *blend_indexes, *blend_weights, *color;
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
				blend_indexes = (uint8_t *)&iqmbuffer[vertarr.offset];
				/*for(int x = 0; x < header.num_vertexes; x++)
				{
					SDL_memcpy(&vertices[x].blend_indexes, &blend_indexes[x * vertarr.size], vertarr.size * sizeof(uint8_t));
				}*/
				break;
			}
			case IQM_BLENDWEIGHTS:
			{
				blend_weights = (uint8_t *)&iqmbuffer[vertarr.offset];
				/*for(int x = 0; x < header.num_vertexes; x++)
				{
					SDL_memcpy(&vertices[x].blend_weights, &blend_weights[x * vertarr.size], vertarr.size * sizeof(uint8_t));
				}*/
				break;
			}
			case IQM_COLOR:
			{
				color = (uint8_t *)&iqmbuffer[vertarr.offset];
				for(int x = 0; x < header.num_vertexes; x++)
				{
					SDL_memcpy(&vertices[x].color, &color[x * vertarr.size], vertarr.size * sizeof(uint8_t));
				}
			}
		}
	}
	//bones and joints (not used yet) TODO

	//indices
	Uint32 *indices = (Uint32*)SDL_malloc(sizeof(Uint32) * (header.num_triangles * 3));
	SDL_memcpy(indices, &iqmbuffer[header.ofs_triangles], (header.num_triangles * 3) * sizeof(Uint32));

	_arrayInitMeshes(&model->meshes);

	//add meshes to the model
	uint32_t index_offset = 0;
	for(int i = 0; i < header.num_meshes; i++)
	{
		Vertex *imported_vertices = &vertices[meshes[i].first_vertex];
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
		_arrayInitVertex(&mesh.vertices);
		for(unsigned int m = 0; m < vcount; m++)
		{
			Vertex vert = imported_vertices[m];
			if(!_arrayPushLastVertex(&mesh.vertices, vert))
			{
				SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to load IQM model - unable to copy vertices.");
				SDL_free(vertices);
				vertices = NULL;
				SDL_free(indices);
				indices = NULL;
				return false;
			}
		}
		_arrayInitIndices(&mesh.indices);
		for(unsigned int n = 0; n < icount; n++)
		{
			Uint32 indice = imported_indices[n];
			if(!_arrayPushLastIndices(&mesh.indices, indice))
			{
				SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Failed to load IQM model - unable to copy indices.");
				SDL_free(vertices);
				vertices = NULL;
				SDL_free(indices);
				indices = NULL;
				return false;
			}
		}
		SDL_snprintf(mesh.meshname, 64, "%s", iqm_mesh_name);

		//IQM doesn't load any material or texture, you need to provide it
		/*if(has_material)
		{
			char material_dir[256];
			SDL_strlcpy(material_dir, materialfile, sizeof(material_dir));
			char* lastslash = strrchr(material_dir, '/');
			if (lastslash != NULL) {
				*lastslash = '\0';
			} else {
				// Não foi encontrado um caractere de barra (possivelmente um caminho relativo)
				material_dir[0] = '\0';
			}

			char diffusepath[512];
			const char *diff_map = INIGetString(material_ini, iqm_material, "diffuse_map");
			SDL_snprintf(diffusepath, sizeof(diffusepath), "%s/%s", material_dir, diff_map);
			if(diff_map != NULL && SDL_strcmp(diff_map, ""))
			{
				mesh.diffuse_map = (Texture2D*)SDL_malloc(sizeof(Texture2D));
				if(mesh.diffuse_map != NULL)
					Graphics_LoadTextureFromFS(mesh.diffuse_map, diffusepath, TEXTURE_DIFFUSE);
			}

			char normalpath[512];
			const char *norm_map = INIGetString(material_ini, iqm_material, "normal_map");
			SDL_snprintf(normalpath, sizeof(normalpath), "%s/%s", material_dir, norm_map);
			if(norm_map != NULL && SDL_strcmp(norm_map, ""))
			{
				mesh.normal_map = (Texture2D*)SDL_malloc(sizeof(Texture2D));
				if(mesh.normal_map != NULL)
					Graphics_LoadTextureFromFS(mesh.normal_map, normalpath, TEXTURE_NORMAL);
			}

			char specularpath[512];
			const char *spec_map = INIGetString(material_ini, iqm_material, "specular_map");
			SDL_snprintf(specularpath, sizeof(specularpath), "%s/%s", material_dir, spec_map);
			if(spec_map != NULL && SDL_strcmp(spec_map, ""))
			{
				mesh.specular_map = (Texture2D*)SDL_malloc(sizeof(Texture2D));
				if(mesh.specular_map != NULL)
					Graphics_LoadTextureFromFS(mesh.specular_map, specularpath, TEXTURE_SPECULAR);
			}

			char emissionpath[512];
			const char *emission_map = INIGetString(material_ini, iqm_material, "emission_map");
			SDL_snprintf(emissionpath, sizeof(emissionpath), "%s/%s", material_dir, emission_map);
			if(emission_map != NULL && SDL_strcmp(emission_map, ""))
			{
				mesh.emission_map = (Texture2D*)SDL_malloc(sizeof(Texture2D));
				if(mesh.emission_map != NULL)
					Graphics_LoadTextureFromFS(mesh.emission_map, emissionpath, TEXTURE_EMISSION);
			}

			char heightpath[512];
			const char *height_map = INIGetString(material_ini, iqm_material, "height_map");
			SDL_snprintf(heightpath, sizeof(heightpath), "%s/%s", material_dir, height_map);
			if(height_map != NULL && SDL_strcmp(height_map, ""))
			{
				mesh.height_map = (Texture2D*)SDL_malloc(sizeof(Texture2D));
				if(mesh.height_map != NULL)
					Graphics_LoadTextureFromFS(mesh.height_map, heightpath, TEXTURE_HEIGHT);
			}
		}
		else
		{
			mesh.diffuse_map = NULL;
			mesh.normal_map = NULL;
			mesh.specular_map = NULL;
			mesh.emission_map = NULL;
			mesh.height_map = NULL;
		}*/

		//TODO CLEANUP if false
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

	model->transform = (Matrix4x4){ 0 };
	model->transform.aa = model->transform.bb = model->transform.cc = model->transform.dd = 1.0f;

	SDL_free(vertices);
	vertices = NULL;
	SDL_free(indices);
	indices = NULL;

	return true;
}

bool Graphics_ImportIQM(Model *model, const char *iqmfile,
							const char *materialfile)
{
	size_t iqmfilesize;
	Uint8 *modelfile = FileIOReadBytes(iqmfile, &iqmfilesize);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Graphics: Info: Starting to load %s", iqmfile);
	return _import_iqm_buffer(model, modelfile, iqmfilesize, materialfile);
}

static void uploadmesh(Mesh *mesh)
{
	if(mesh == NULL) return;

	mesh->vbuffer = SDL_CreateGPUBuffer(
		context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = sizeof(Mesh) * mesh->vertices.count
		}
	);

	mesh->ibuffer = SDL_CreateGPUBuffer(
		context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_INDEX,
			.size = sizeof(Uint32) * mesh->indices.count
		}
	);

	SDL_GPUTransferBuffer* vbufferTransferBuffer = SDL_CreateGPUTransferBuffer(
		context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(Mesh) * mesh->vertices.count)
		}
	);
	SDL_GPUTransferBuffer* ibufferTransferBuffer = SDL_CreateGPUTransferBuffer(
		context.device,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(Uint32) * mesh->indices.count)
		}
	);

	Vertex* bufferTransferData = SDL_MapGPUTransferBuffer(context.device, vbufferTransferBuffer, false);
	SDL_memcpy(bufferTransferData, mesh->vertices.vertices, sizeof(Vertex) * mesh->vertices.count);
	SDL_UnmapGPUTransferBuffer(context.device, vbufferTransferBuffer);
	Uint32* indexData = SDL_MapGPUTransferBuffer(context.device, ibufferTransferBuffer, false);
	SDL_memcpy(indexData, mesh->indices.indices, sizeof(Uint32) * mesh->indices.count);
	SDL_UnmapGPUTransferBuffer(context.device, ibufferTransferBuffer);

	// Upload the transfer data to the GPU buffers
	SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context.device);
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
			.size = sizeof(Vertex) * mesh->vertices.count
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
			.size = sizeof(Uint32) * mesh->indices.count
		},
		false
	);
	SDL_EndGPUCopyPass(copyPass);

	SDL_ReleaseGPUTransferBuffer(context.device, vbufferTransferBuffer);
	SDL_ReleaseGPUTransferBuffer(context.device, ibufferTransferBuffer);

	SDL_SubmitGPUCommandBuffer(cmdbuf);

	//DynarrayClearVertex(&mesh->arrv);
	//DynarrayClearIndices(&mesh->arri);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Mesh %s uploaded.", mesh->meshname);
}

void Graphics_UploadModel(Model *model, bool upload_textures)
{
	if(model == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "No model to upload.");
		return;
	}

	for(Uint32 i = 0; i < model->meshes.count; i++)
	{
		/*if(upload_textures)
		{
			if(model->meshes.meshes[i].diffuse_map != NULL)
			{
				Graphics_UploadTexture(model->meshes.meshes[i].diffuse_map);
			}
			if(model->meshes.meshes[i].normal_map != NULL)
			{
				Graphics_UploadTexture(model->meshes.meshes[i].normal_map);
			}
			if(model->meshes.meshes[i].specular_map != NULL)
			{
				Graphics_UploadTexture(model->meshes.meshes[i].specular_map);
			}
			if(model->meshes.meshes[i].emission_map != NULL)
			{
				Graphics_UploadTexture(model->meshes.meshes[i].emission_map);
			}
			if(model->meshes.meshes[i].height_map != NULL)
			{
				Graphics_UploadTexture(model->meshes.meshes[i].height_map);
			}
		}*/
		uploadmesh(&model->meshes.meshes[i]);
	}
}

void Graphics_MoveModel(Model *model, Vector3 position)
{
	Matrix4x4 result = Matrix4x4_Translate(model->transform, position.x, position.y, position.z);
	model->transform = result;
}

void Graphics_ScaleModel(Model *model, float scale)
{
	Matrix4x4 result = Matrix4x4_Scale(model->transform, (Vector3){scale, scale, scale});
	model->transform = result;
}

void Graphics_RotateModel(Model *model, Vector3 axis,
							float radians)
{
	Matrix4x4 result = Matrix4x4_Rotate(model->transform, axis, radians);
	model->transform = result;
}

void Graphics_ReleaseModel(Model *model)
{
	for(Uint32 i = 0; i < model->meshes.count; i++)
	{
		//destroy buffers
		SDL_ReleaseGPUBuffer(context.device, model->meshes.meshes[i].vbuffer);
		SDL_ReleaseGPUBuffer(context.device, model->meshes.meshes[i].ibuffer);

		//destroy textures
		/*if(model->meshes.meshes[i].diffuse_map != NULL)
			Graphics_ReleaseTexture(model->meshes.meshes[i].diffuse_map);
		if(model->meshes.meshes[i].normal_map != NULL)
			Graphics_ReleaseTexture(model->meshes.meshes[i].normal_map);
		if(model->meshes.meshes[i].specular_map != NULL)
			Graphics_ReleaseTexture(model->meshes.meshes[i].specular_map);
		if(model->meshes.meshes[i].emission_map != NULL)
			Graphics_ReleaseTexture(model->meshes.meshes[i].emission_map);
		if(model->meshes.meshes[i].height_map != NULL)
			Graphics_ReleaseTexture(model->meshes.meshes[i].height_map);*/

		//destroy arrays
		_arrayDestroyIndices(&model->meshes.meshes[i].indices);
		_arrayDestroyVertex(&model->meshes.meshes[i].vertices);
	}
	//finally, destroy meshes
	_arrayDestroyMeshes(&model->meshes);
}
