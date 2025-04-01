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

bool Graphics_ImportModelMem(Model *model, Uint8 *buffer,
								size_t size)
{
	return false;
}

bool Graphics_ImportModelFS(Model *model, const char *path)
{
	size_t filesize;
	Uint8 *file = FileIOReadBytes(path, &filesize);
	return Graphics_ImportModelMem(model, file, filesize);
}

