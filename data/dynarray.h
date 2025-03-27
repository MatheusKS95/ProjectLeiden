/**
 * @file dynarray.h
 * @brief Dynamic array definitions and management
 *
 * Dynamic array management stuff. Can be used almost everywhere.
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stdbool.h>
#include <SDL3/SDL_stdinc.h>

typedef struct DynArrayF
{
	size_t count;
	size_t capacity;
	float *items;
} DynArrayF;

typedef struct DynArrayI
{
	size_t count;
	size_t capacity;
	int *items;
} DynArrayI;

void DynarrayInitFloat(DynArrayF *arr);
void DynarrayInitInt(DynArrayI *arr);

bool DynarrayPushLastFloat(DynArrayF *arr, float value);
bool DynarrayPushLastInt(DynArrayI *arr, int value);

bool DynarrayPopAtFloat(DynArrayF *arr, unsigned int index, float *value);
bool DynarrayPopAtInt(DynArrayI *arr, unsigned int index, int *value);

void DynarrayDestroyFloat(DynArrayF *arr);
void DynarrayDestroyInt(DynArrayI *arr);

void DynarrayClearFloat(DynArrayF *arr);
void DynarrayClearInt(DynArrayI *arr);

#endif // DYNARRAY_H
