/**
 * @file models.c
 * @brief Implementation file for model stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <assimp/cimport.h>        // Plain-C interface
#include <assimp/scene.h>          // Output data structure
#include <assimp/postprocess.h>    // Post processing flags

#include <graphics.h>
#include <fileio.h>

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

static bool _arrayRealocTextures(TextureArray *arr, size_t new_size)
{
	if(arr->textures != NULL)
	{
		Texture2D *aux = (Texture2D*)SDL_realloc(arr->textures, sizeof(Texture2D) * new_size);
		if(aux != NULL)
		{
			arr->textures = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static void _arrayInitTextures(TextureArray *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->textures = (Texture2D*)SDL_calloc(arr->capacity, sizeof(Texture2D));
}

static bool _arrayPushLastTextures(TextureArray *arr, Texture2D value)
{
	if(arr->capacity == arr->count)
	{
		if(!_arrayRealocTextures(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->textures[arr->count - 1] = value;
	return true;
}

static bool _arrayPopAtTextures(TextureArray *arr, unsigned int index, Texture2D *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->textures[index];
	SDL_memmove(&arr->textures[index], &arr->textures[index + 1], sizeof(Texture2D) * ((arr->count - 1) - index));
	arr->count--;
	_arrayRealocTextures(arr, arr->capacity - 1);
	return true;
}

static void _arrayDestroyTextures(TextureArray *arr)
{
	SDL_free(arr->textures);
	arr->textures = NULL;
}

static void _arrayClearTextures(TextureArray *arr)
{
	_arrayDestroyTextures(arr);
	_arrayInitTextures(arr);
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

static bool _arrayRealocMaterials(MaterialArray *arr, size_t new_size)
{
	if(arr->materials != NULL)
	{
		Material *aux = (Material*)SDL_realloc(arr->materials, sizeof(Material) * new_size);
		if(aux != NULL)
		{
			arr->materials = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static void _arrayInitMaterials(MaterialArray *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->materials = (Material*)SDL_calloc(arr->capacity, sizeof(Material));
}

static bool _arrayPushLastMaterials(MaterialArray *arr, Material value)
{
	if(arr->capacity == arr->count)
	{
		if(!_arrayRealocMaterials(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->materials[arr->count - 1] = value;
	return true;
}

static bool _arrayPopAtMaterials(MaterialArray *arr, unsigned int index, Material *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->materials[index];
	SDL_memmove(&arr->materials[index], &arr->materials[index + 1], sizeof(Material) * ((arr->count - 1) - index));
	arr->count--;
	_arrayRealocMaterials(arr, arr->capacity - 1);
	return true;
}

static void _arrayDestroyMaterials(MaterialArray *arr)
{
	SDL_free(arr->materials);
	arr->materials = NULL;
}

static void _arrayClearMaterials(MaterialArray *arr)
{
	_arrayDestroyMaterials(arr);
	_arrayInitMaterials(arr);
}

/**************************************************************************************
 * ASSIMP RELATED
***************************************************************************************/
static void assimp_loadtextures(Model *model, Mesh *mesh, struct aiMaterial *material, enum aiTextureType type, const char *typename)
{
	for(unsigned int i = 0; i < aiGetMaterialTextureCount(material, type); i++)
	{
		struct aiString str;
		aiGetMaterialTexture(material, type, 0, &str, NULL, NULL, NULL, NULL, NULL, NULL);
		if(HashtableFind(model->textures, str.data) != NULL)
		{
			continue;
		}
		Texture2D *texture = (Texture2D*)SDL_malloc(sizeof(Texture2D));
		if(texture == NULL) continue;
		TextureType texttype;
		switch(type)
		{
			case aiTextureType_DIFFUSE: texttype = TEXTURE_DIFFUSE; break;
			case aiTextureType_NORMALS: texttype = TEXTURE_NORMAL; break;
			case aiTextureType_SPECULAR: texttype = TEXTURE_SPECULAR; break;
			case aiTextureType_EMISSIVE: texttype = TEXTURE_EMISSION; break;
			case aiTextureType_HEIGHT: texttype = TEXTURE_HEIGHT; break;
			default: break;
		}
		if(!Graphics_LoadTextureFromFS(texture, str.data, texttype))
		{
			continue;
		}
		if(!HashtableInsert(model->textures, str.data, texture))
		{
			SDL_free(texture);
			continue;
		}
	}
	//CONTINUE
}

static void assimp_processmesh(Model *model, Mesh *mesh, struct aiMesh *aimesh, const struct aiScene *scene)
{
	_arrayInitVertex(&mesh->vertices);
	_arrayInitIndices(&mesh->indices);

	for(unsigned int i = 0; i < aimesh->mNumVertices; ++i)
	{
		Vertex vertex = { 0 };
		vertex.position.x = aimesh->mVertices[i].x;
		vertex.position.y = aimesh->mVertices[i].y;
		vertex.position.z = aimesh->mVertices[i].z;

		vertex.normal.x = aimesh->mNormals[i].x;
		vertex.normal.y = aimesh->mNormals[i].y;
		vertex.normal.z = aimesh->mNormals[i].z;

		vertex.uv.x = aimesh->mTextureCoords[0][i].x;
		vertex.uv.y = aimesh->mTextureCoords[0][i].y;

		vertex.tangent.x = aimesh->mTangents[i].x;
		vertex.tangent.y = aimesh->mTangents[i].y;
		vertex.tangent.z = aimesh->mTangents[i].z;

		if(aimesh->mColors[0] != NULL)
		{
			vertex.color.r = aimesh->mColors[0][i].r;
			vertex.color.g = aimesh->mColors[0][i].g;
			vertex.color.b = aimesh->mColors[0][i].b;
			vertex.color.a = aimesh->mColors[0][i].a;
		}
		else
		{
			vertex.color.r = 0.0f;
			vertex.color.g = 0.0f;
			vertex.color.b = 0.0f;
			vertex.color.a = 0.0f;
		}

		_arrayPushLastVertex(&mesh->vertices, vertex);
	}

	//debug, safety purposes
	if(aimesh->mNumVertices != mesh->vertices.count)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Assimp vertex count and mesh vertex count are not equal");
	}

	for(unsigned int i = 0; i < aimesh->mNumFaces; ++i)
	{
		struct aiFace face = aimesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			//printf("%d ", face.mIndices[j]);
			_arrayPushLastIndices(&mesh->indices, face.mIndices[j]);
		}
	}
	//SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Model loading: Vertices processed: %d of %d. Indices processed: %d of %d", (int)mesh->vertices.count, aimesh->mNumVertices, (int)mesh->indices.count, aimesh->mNumFaces * 3);
	SDL_snprintf(mesh->meshname, 64, "%s", aimesh->mName.data);

	//TODO get this stuff done
	struct aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];
	struct aiString name;
	struct aiString path;
	//aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &path, NULL, NULL,	NULL, NULL, NULL, NULL);
	SDL_snprintf(mesh->matname, 64, "%s", aimesh->mName.data);
}

static void assimp_processnode(Model *model, struct aiNode *node, const struct aiScene *scene)
{
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		struct aiMesh *aimesh = scene->mMeshes[node->mMeshes[i]];
		Mesh mesh = { 0 };
		assimp_processmesh(model, &mesh, aimesh, scene);
		_arrayPushLastMeshes(&model->meshes, mesh);
	}
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		assimp_processnode(model, node->mChildren[i], scene);
	}
}

bool Graphics_ImportModelMem(Model *model, Uint8 *buffer,
								size_t size)
{
	/*
	 * About Assimp:
	 * Assimp is huge and takes a lot of time to build.
	 * It's due to usage of RTTI and other C++ stuff.
	 * Despite this, I'll keep using it for now, but I plan to make other loaders on my own.
	 * > IQM: I did it before for OpenGL stuff, I just need to adapt. Don't need 3rd party tools.
	 * > M3D: I have little idea how to import it, but can't be that difficult. Easy way? 3rd party tools.
	 * > GLTF/GLB: Too painful to do own my own, but needed. Will need cgltf. Need recursion.
	 * > WAVEFRONT OBJ: C makes it difficult to parse, but it's doable. No 3rd party tools needed.
	 * > OTHERS: either obsolete or niche.
	 * > CUSTOM: need further study.
	 */
	if(model == NULL || buffer == NULL)
	{
		return false;
	}
	const struct aiScene *scene = aiImportFileFromMemory((char*)buffer, size,
															aiProcess_CalcTangentSpace |
															aiProcess_Triangulate |
															aiProcess_JoinIdenticalVertices |
															aiProcess_GenSmoothNormals |
															aiProcess_FlipUVs,
															NULL);

	if(scene == NULL || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == NULL)
	{
		return false;
	}
	_arrayInitMeshes(&model->meshes);
	_arrayInitMaterials(&model->materials);
	model->textures = HashtableInit();

	//TODO process node

	aiReleaseImport(scene);
	return false;
}

bool Graphics_ImportModelFS(Model *model, const char *path)
{
	size_t filesize;
	Uint8 *file = FileIOReadBytes(path, &filesize);
	return Graphics_ImportModelMem(model, file, filesize);
}

