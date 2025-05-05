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
	scene->type = type;

	//TODO create pipelines according to type
	return true;
}