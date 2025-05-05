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
 * @file scene.c
 * @brief Scene controls (before rendering)
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <graphics.h>
#include <SDL3/SDL.h>

//ARRAY RELATED STUFF
static bool _arrayRealocPointlight(PointLightArray *arr, size_t new_size)
{
	if(arr->pointlights != NULL)
	{
		Pointlight *aux = (Pointlight*)SDL_realloc(arr->pointlights, sizeof(Pointlight) * new_size);
		if(aux != NULL)
		{
			arr->pointlights = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static void _arrayInitPointlight(PointLightArray *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->pointlights = (Pointlight*)SDL_calloc(arr->capacity, sizeof(Pointlight));
}

static bool _arrayPushLastPointlight(PointLightArray *arr, Pointlight value)
{
	if(arr->capacity == arr->count)
	{
		if(!_arrayRealocPointlight(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->pointlights[arr->count - 1] = value;
	return true;
}

static bool _arrayPopAtPointlight(PointLightArray *arr, unsigned int index, Pointlight *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->pointlights[index];
	SDL_memmove(&arr->pointlights[index], &arr->pointlights[index + 1], sizeof(Pointlight) * ((arr->count - 1) - index));
	arr->count--;
	_arrayRealocPointlight(arr, arr->capacity - 1);
	return true;
}

static void _arrayDestroyPointlight(PointLightArray *arr)
{
	SDL_free(arr->pointlights);
	arr->pointlights = NULL;
}

static void _arrayClearPointlight(PointLightArray *arr)
{
	_arrayDestroyPointlight(arr);
	_arrayInitPointlight(arr);
}

static bool _arrayRealocModel(ModelArray *arr, size_t new_size)
{
	if(arr->models != NULL)
	{
		Model *aux = (Model*)SDL_realloc(arr->models, sizeof(Model) * new_size);
		if(aux != NULL)
		{
			arr->models = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static void _arrayInitModel(ModelArray *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->models = (Model*)SDL_calloc(arr->capacity, sizeof(Model));
}

static bool _arrayPushLastModel(ModelArray *arr, Model value)
{
	if(arr->capacity == arr->count)
	{
		if(!_arrayRealocModel(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->models[arr->count - 1] = value;
	return true;
}

static bool _arrayPopAtModel(ModelArray *arr, unsigned int index, Model *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->models[index];
	SDL_memmove(&arr->models[index], &arr->models[index + 1], sizeof(Model) * ((arr->count - 1) - index));
	arr->count--;
	_arrayRealocModel(arr, arr->capacity - 1);
	//TODO or FIXME, but I need somewhere to destroy the model when done with the scene
	return true;
}

static void _arrayDestroyModel(ModelArray *arr)
{
	//TODO delete models individually using the appropriate function!
	SDL_free(arr->models);
	arr->models = NULL;
}

static void _arrayClearModel(ModelArray *arr)
{
	//TODO delete models individually using the appropriate function!
	_arrayDestroyModel(arr);
	_arrayInitModel(arr);
}

//END OF ARRAY RELATED STUFF

bool Graphics_CreateScene(GraphicsScene *scene,
							PipelineRenderingType type)
{
	if(scene == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: scene structure required to initialize it.");
		return false;
	}
	_arrayInitModel(&scene->modelarray);
	_arrayInitPointlight(&scene->plightarray);
	scene->uploaded = false;
	scene->type = type;

	//TODO create pipelines according to type
	return true;
}

bool Graphics_AddModelToScene(GraphicsScene *scene, Model *model)
{
	if(scene == NULL || model == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't add model to scene.");
		return false;
	}
	//TODO if scene was already uploaded, upload new model too
	bool result = false;
	result = _arrayPushLastModel(&scene->modelarray, *model);

	if(scene->uploaded)
		Graphics_UploadModel(model, true); //todo make a way to check if you want to upload textures
		//however i need to re-think textures first

	return result;
}

bool Graphics_RemoveModelFromScene(GraphicsScene *scene,
									size_t index,
									Model *model)
{
	if(scene == NULL || model == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't remove model from scene.");
		return false;
	}

	bool result = false;
	result = _arrayPopAtModel(&scene->modelarray, index, model);
	Graphics_ReleaseModel(model);
	return result;
}

bool Graphics_ClearModelsFromScene(GraphicsScene *scene)
{
	if(scene == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't clear models from scene - invalid scene.");
		return false;
	}

	bool result = true;
	for(int i = 0; i < scene->modelarray.count; i++)
	{
		Model *model = NULL;
		if(!Graphics_RemoveModelFromScene(scene, i, model))
			result = false;
	}
	_arrayClearModel(&scene->modelarray);
	return result;
}

bool Graphics_AddPointlightToScene(GraphicsScene *scene,
								   Pointlight *pointlight)
{
	if(scene == NULL || pointlight == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't add point light to scene.");
		return false;
	}
	if(scene->type == PIPELINE_5THGEN)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_ERROR, "Graphics: Warning: 5th generation graphics didn't have lighting. Any light added won't be used.");
		//just a warning, function goes as normal
	}
	//TODO if scene was already uploaded, need to re-create the buffer
	return _arrayPushLastPointlight(&scene->plightarray, *pointlight);
}

bool Graphics_RemovePointlightFromScene(GraphicsScene *scene,
										size_t index,
										Pointlight *pointlight)
{
	if(scene == NULL || pointlight == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't remove pointlight from scene.");
		return false;
	}
	//TODO if scene was already uploaded, need to re-create the buffer
	return _arrayPopAtPointlight(&scene->plightarray, index, pointlight);
}

bool Graphics_ClearPointlightsFromScene(GraphicsScene *scene)
{
	if(scene == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't clear point lights from scene - invalid scene.");
		return false;
	}

	bool result = true;
	for(int i = 0; i < scene->plightarray.count; i++)
	{
		Pointlight *light = NULL;
		if(!Graphics_RemovePointlightFromScene(scene, i, light))
			result = false;
	}
	_arrayClearPointlight(&scene->plightarray);
	return result;
}

void _upload_pointlights(GraphicsScene *scene)
{
	//TODO
	if(scene == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't upload lights - invalid scene.");
		return;
	}
	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't upload pointlights because it's not fully implemented yet.");

	//TODO need to check appropriate buffer for lighting
	/*scene->plightbuffer = SDL_CreateGPUBuffer(
		context.device,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_INDIRECT,
			.size = sizeof(Mesh) * mesh->vertices.count
		}
	);*/
	return;
}

void Graphics_UploadScene(GraphicsScene *scene)
{
	if(scene == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Can't upload scene - invalid scene.");
		return;
	}

	//first, let's upload all the models (and their textures)
	for(size_t i = 0; i < scene->modelarray.count; i++)
	{
		Graphics_UploadModel(&scene->modelarray.models[i], true);
	}

	//now, let's upload all the lights... TODO
	//TODO create static function to deal with pointlight uploading

	scene->uploaded = true;
}
