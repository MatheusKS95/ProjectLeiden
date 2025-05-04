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

//END OF ARRAY RELATED STUFF

void Graphics_CreatePointlightArray(PointLightArray *array)
{
	if(array == NULL) return;

	_arrayInitPointlight(array);
}

bool Graphics_LightArrayAddPointlight(PointLightArray *array,
										Pointlight pointlight)
{
	if(array == NULL) return false;

	return _arrayPushLastPointlight(array, pointlight);
}

void Graphics_ClearLightArray(PointLightArray *array)
{
	if(array == NULL) return;

	_arrayClearPointlight(array);
}