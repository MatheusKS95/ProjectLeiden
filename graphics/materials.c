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
 * @file materials.c
 * @brief Implementation file for material stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <SDL3/SDL.h>
#include <graphics.h>

/**************************************************************************************
 * ARRAY HELPERS
***************************************************************************************/
static bool _arrayRealocMaterial(MaterialArray *arr, size_t new_size)
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

static void _arrayInitMaterial(MaterialArray *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->materials = (Material*)SDL_calloc(arr->capacity, sizeof(Material));
}

static bool _arrayPushLastMaterial(MaterialArray *arr, Material value)
{
	if(arr->capacity == arr->count)
	{
		if(!_arrayRealocMaterial(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->materials[arr->count - 1] = value;
	return true;
}

static bool _arrayPopAtMaterial(MaterialArray *arr, unsigned int index, Material *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->materials[index];
	SDL_memmove(&arr->materials[index], &arr->materials[index + 1], sizeof(Material) * ((arr->count - 1) - index));
	arr->count--;
	_arrayRealocMaterial(arr, arr->capacity - 1);
	return true;
}

static void _arrayDestroyMaterial(MaterialArray *arr)
{
	SDL_free(arr->materials);
	arr->materials = NULL;
}

static void _arrayClearMaterial(MaterialArray *arr)
{
	_arrayDestroyMaterial(arr);
	_arrayInitMaterial(arr);
}

bool Graphics_LoadMaterialsFromINI(MaterialArray *matarray,
									const char *path)
{
	return false;
}

Material* Graphics_GetMaterialByName(MaterialArray *matarray,
										const char *name)
{
	return NULL;
}

void Graphics_UploadMaterial(Material *material)
{
	return;
}

void Graphics_UploadMaterials(MaterialArray *materials)
{
	return;
}

void Graphics_ReleaseMaterial(Material *material)
{
	return;
}

void Graphics_ReleaseMaterials(MaterialArray *materials)
{
	return;
}