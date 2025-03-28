/**
 * @file dynarray.c
 * @brief Dynamic array implementation file
 *
 * This file TODO
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <dynarray.h>

static bool DynarrayRealocFloat(DynArrayF *arr, size_t new_size)
{
	if(arr->items != NULL)
	{
		float *aux = (float*)SDL_realloc(arr->items, sizeof(float) * new_size);
		if(aux != NULL)
		{
			arr->items = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

static bool DynarrayRealocInt(DynArrayI *arr, size_t new_size)
{
	if(arr->items != NULL)
	{
		int *aux = (int*)SDL_realloc(arr->items, sizeof(int) * new_size);
		if(aux != NULL)
		{
			arr->items = aux;
			arr->capacity = new_size;
			return true;
		}
	}
	return false;
}

void DynarrayInitFloat(DynArrayF *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->items = (float*)SDL_calloc(arr->capacity, sizeof(float));
}

void DynarrayInitInt(DynArrayI *arr)
{
	arr->capacity = 1;
	arr->count = 0;
	arr->items = (int*)SDL_calloc(arr->capacity, sizeof(int));
}

bool DynarrayPushLastFloat(DynArrayF *arr, float value)
{
	if(arr->capacity == arr->count)
	{
		if(!DynarrayRealocFloat(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->items[arr->count - 1] = value;
	return true;
}

bool DynarrayPushLastInt(DynArrayI *arr, int value)
{
	if(arr->capacity == arr->count)
	{
		if(!DynarrayRealocInt(arr, arr->capacity + 1))
		{
			return false;
		}
	}
	arr->count++;
	arr->items[arr->count - 1] = value;
	return true;
}

bool DynarrayPopAtFloat(DynArrayF *arr, unsigned int index, float *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->items[index];
	SDL_memmove(&arr->items[index], &arr->items[index + 1], sizeof(float) * ((arr->count - 1) - index));
	arr->count--;
	DynarrayRealocFloat(arr, arr->capacity - 1);
	return true;
}

bool DynarrayPopAtInt(DynArrayI *arr, unsigned int index, int *value)
{
	if(arr->count == 0)
	{
		return false;
	}
	*value = arr->items[index];
	SDL_memmove(&arr->items[index], &arr->items[index + 1], sizeof(int) * ((arr->count - 1) - index));
	arr->count--;
	DynarrayRealocInt(arr, arr->capacity - 1);
	return true;
}

void DynarrayDestroyFloat(DynArrayF *arr)
{
	SDL_free(arr->items);
	arr->items = NULL;
	SDL_free(arr);
}

void DynarrayDestroyInt(DynArrayI *arr)
{
	SDL_free(arr->items);
	arr->items = NULL;
	SDL_free(arr);
}

void DynarrayClearFloat(DynArrayF *arr)
{
	DynarrayDestroyFloat(arr);
	DynarrayInitFloat(arr);
}

void DynarrayClearInt(DynArrayI *arr)
{
	DynarrayDestroyInt(arr);
	DynarrayInitInt(arr);
}
