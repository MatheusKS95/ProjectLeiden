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

/*
 * About Assimp:
 * Assimp is huge and takes a lot of time to build.
 * It's due to usage of RTTI and other C++ stuff.
 * Assimp is kept in a demo (that will be removed later), so these are my options for official stuff:
 * > IQM: I did it before for OpenGL stuff, I just need to adapt. Don't need 3rd party tools.
 * > M3D: I have little idea how to import it, but can't be that difficult. Easy way? Official header only lib.
 * > GLTF/GLB: Too painful to do own my own, but needed. Will need cgltf. Need recursion.
 * > WAVEFRONT OBJ: C makes it difficult to parse, but it's doable. No 3rd party tools needed.
 * > OTHERS: either obsolete or niche.
 * > CUSTOM: need further study.
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
bool Graphics_SetMaterialTextures(Material *material,
									Texture2D *diffuse,
									Texture2D *normal,
									Texture2D *specular,
									Texture2D *emission,
									Texture2D *height)
{
	//this might look painful
	//and it is
	//that's why I plan to create my own model format later in the future
	if(material == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics_LoadMaterialTextures unable to proceed without a valid material.");
		return false;
	}

	//will assume everything inside material is already filled, except textures
	//not all textures are needed
	material->textures[TEXTURE_DIFFUSE] = diffuse;
	material->textures[TEXTURE_NORMAL] = normal;
	material->textures[TEXTURE_SPECULAR] = specular;
	material->textures[TEXTURE_EMISSION] = emission;
	material->textures[TEXTURE_HEIGHT] = height;
	return true;
}

bool Graphics_ImportIQMMem(Model *model, Uint8 *iqmbuffer,
							size_t iqmsize,
							const char *materialfile,
							SDL_GPUGraphicsPipeline *pipeline)
{
	if(model == NULL || iqmbuffer == NULL || iqmsize <= 0)
	{
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
		SDL_LogInfo(SDL_LOG_CATEGORY_ERROR, "Failed to load IQM model - invalid IQM file.");
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
			//TODO CLEANUP if false
			bool test = _arrayPushLastVertex(&mesh.vertices, vert);
		}
		_arrayInitIndices(&mesh.indices);
		for(unsigned int n = 0; n < icount; n++)
		{
			Uint32 indice = imported_indices[n];
			//TODO CLEANUP if false
			bool test = _arrayPushLastIndices(&mesh.indices, indice);
		}
		SDL_snprintf(mesh.meshname, 64, "%s", iqm_mesh_name);

		//IQM doesn't load any material or texture, you need to provide it later
		//TODO load material using INI (easiest right now, can expand)
		Material material = { 0 };
		SDL_snprintf(material.name, 64, "%s", iqm_material);
		if(has_material)
		{
			//TODO check if fields exist at least
			SDL_sscanf(INIGetString(material_ini, material.name, "ambient"), "%f %f %f", &material.ambient.x, &material.ambient.y, &material.ambient.z);
			SDL_sscanf(INIGetString(material_ini, material.name, "diffuse"), "%f %f %f", &material.diffuse.x, &material.diffuse.y, &material.diffuse.z);
			SDL_sscanf(INIGetString(material_ini, material.name, "specular"), "%f %f %f", &material.specular.x, &material.specular.y, &material.specular.z);
			material.emission = INIGetFloat(material_ini, material.name, "emission");
			material.shininess = INIGetFloat(material_ini, material.name, "shininess");
			//TODO textures
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
				material.textures[TEXTURE_DIFFUSE] = (Texture2D*)SDL_malloc(sizeof(Texture2D));
				if(material.textures[TEXTURE_DIFFUSE] != NULL)
					Graphics_LoadTextureFromFS(material.textures[TEXTURE_DIFFUSE], diffusepath, TEXTURE_DIFFUSE);
			}

			char normalpath[512];
			const char *norm_map = INIGetString(material_ini, iqm_material, "normal_map");
			SDL_snprintf(normalpath, sizeof(normalpath), "%s/%s", material_dir, norm_map);
			if(norm_map != NULL && SDL_strcmp(norm_map, ""))
			{
				material.textures[TEXTURE_NORMAL] = (Texture2D*)SDL_malloc(sizeof(Texture2D));
				if(material.textures[TEXTURE_NORMAL] != NULL)
					Graphics_LoadTextureFromFS(material.textures[TEXTURE_NORMAL], normalpath, TEXTURE_NORMAL);
			}

			char specularpath[512];
			const char *spec_map = INIGetString(material_ini, iqm_material, "specular_map");
			SDL_snprintf(specularpath, sizeof(specularpath), "%s/%s", material_dir, spec_map);
			if(spec_map != NULL && SDL_strcmp(spec_map, ""))
			{
				material.textures[TEXTURE_SPECULAR] = (Texture2D*)SDL_malloc(sizeof(Texture2D));
				if(material.textures[TEXTURE_SPECULAR] != NULL)
					Graphics_LoadTextureFromFS(material.textures[TEXTURE_SPECULAR], specularpath, TEXTURE_SPECULAR);
			}
		}
		else //redundant, material is already zeroed
		{
			material.ambient = (Vector3){ 0 };
			material.diffuse = (Vector3){ 0 };
			material.specular = (Vector3){ 0 };
			material.emission = material.shininess = 0.0f;
			material.textures[TEXTURE_EMISSION] = NULL;
			material.textures[TEXTURE_DIFFUSE] = NULL;
			material.textures[TEXTURE_HEIGHT] = NULL;
			material.textures[TEXTURE_NORMAL] = NULL;
			material.textures[TEXTURE_SPECULAR] = NULL;
		}
		mesh.material = material;

		//TODO CLEANUP if false
		bool testload = _arrayPushLastMeshes(&model->meshes, mesh);
	}

	model->pipeline = pipeline;

	SDL_free(vertices);
	vertices = NULL;
	SDL_free(indices);
	indices = NULL;

	return true;
}

bool Graphics_ImportIQMFS(Model *model, const char *iqmfile,
							const char *materialfile,
							SDL_GPUGraphicsPipeline *pipeline)
{
	size_t iqmfilesize;
	Uint8 *modelfile = FileIOReadBytes(iqmfile, &iqmfilesize);
	return Graphics_ImportIQMMem(model, modelfile, iqmfilesize, materialfile, pipeline);
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

	//TODO maybe separate two buffers, one for vertices and other for indices, maybe this helps
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
		if(upload_textures)
		{
			if(model->meshes.meshes[i].material.textures[TEXTURE_DIFFUSE] != NULL)
			{
				Graphics_UploadTexture(model->meshes.meshes[i].material.textures[TEXTURE_DIFFUSE]);
			}
			if(model->meshes.meshes[i].material.textures[TEXTURE_NORMAL] != NULL)
			{
				Graphics_UploadTexture(model->meshes.meshes[i].material.textures[TEXTURE_NORMAL]);
			}
			if(model->meshes.meshes[i].material.textures[TEXTURE_SPECULAR] != NULL)
			{
				Graphics_UploadTexture(model->meshes.meshes[i].material.textures[TEXTURE_SPECULAR]);
			}
			//TODO not ideal, but deal with the rest
		}
		uploadmesh(&model->meshes.meshes[i]);
	}
}
