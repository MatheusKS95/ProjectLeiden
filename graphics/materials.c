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
#include <ini.h>
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

/*bool Graphics_LoadMaterialsFromINI(MaterialArray *matarray,
									const char *path)
{
	//TODO - NOT IMPLEMENTED
	//this if for allowing to load materials separately from model
	return false;
}*/

bool Graphics_LoadModelMaterials(Model *model,
									const char *material_path)
{
	if(model == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Unable to load materials, invalid model.");
		return false;
	}
	INIstruct *material_ini = ININew();
	if(material_path != NULL)
	{
		if(!INILoad(material_ini, material_path))
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Unable to load materials, invalid INI.");
			return false;
		}
	}
	else
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Unable to load materials, invalid INI.");
		return false;
	}

	//FIXME should test if it's already empty or not
	_arrayInitMaterial(&model->materials);

	for(int i = 0; i < model->meshes.count; i++)
	{
		Material *aux = Graphics_GetMaterialByName(&model->materials, model->meshes.meshes[i].material_name);
		if(aux != NULL && SDL_strncmp(model->meshes.meshes[i].material_name, aux->material_name, 64) == 0)
		{
			//already loaded this material, continue...
			continue;
		}

		char material_dir[256];
		SDL_strlcpy(material_dir, material_path, sizeof(material_dir));
		char* lastslash = strrchr(material_dir, '/');
		if (lastslash != NULL)
		{
			*lastslash = '\0';
		}
		else
		{
			material_dir[0] = '\0';
		}

		Material material = { 0 };

		char diffusepath[512];
		char *iqm_material = model->meshes.meshes[i].material_name;
		SDL_snprintf(material.material_name, 64, "%s", iqm_material);
		const char *diff_map = INIGetString(material_ini, iqm_material, "diffuse_map");
		SDL_snprintf(diffusepath, sizeof(diffusepath), "%s/%s", material_dir, diff_map);
		if(diff_map != NULL && SDL_strcmp(diff_map, "") == 0)
		{
			material.diffuse_map = (Texture2D*)SDL_malloc(sizeof(Texture2D));
			if(material.diffuse_map != NULL)
				Graphics_LoadTextureFromFS(material.diffuse_map, diffusepath, TEXTURE_DIFFUSE);
		}

		char normalpath[512];
		const char *norm_map = INIGetString(material_ini, iqm_material, "normal_map");
		SDL_snprintf(normalpath, sizeof(normalpath), "%s/%s", material_dir, norm_map);
		if(norm_map != NULL && SDL_strcmp(norm_map, "") == 0)
		{
			material.normal_map = (Texture2D*)SDL_malloc(sizeof(Texture2D));
			if(material.normal_map != NULL)
				Graphics_LoadTextureFromFS(material.normal_map, normalpath, TEXTURE_NORMAL);
		}

		char specularpath[512];
		const char *spec_map = INIGetString(material_ini, iqm_material, "specular_map");
		SDL_snprintf(specularpath, sizeof(specularpath), "%s/%s", material_dir, spec_map);
		if(spec_map != NULL && SDL_strcmp(spec_map, "") == 0)
		{
			material.specular_map = (Texture2D*)SDL_malloc(sizeof(Texture2D));
			if(material.specular_map != NULL)
				Graphics_LoadTextureFromFS(material.specular_map, specularpath, TEXTURE_SPECULAR);
		}

		char emissionpath[512];
		const char *emission_map = INIGetString(material_ini, iqm_material, "emission_map");
		SDL_snprintf(emissionpath, sizeof(emissionpath), "%s/%s", material_dir, emission_map);
		if(emission_map != NULL && SDL_strcmp(emission_map, "") == 0)
		{
			material.emission_map = (Texture2D*)SDL_malloc(sizeof(Texture2D));
			if(material.emission_map != NULL)
				Graphics_LoadTextureFromFS(material.emission_map, emissionpath, TEXTURE_EMISSION);
		}

		if(!_arrayPushLastMaterial(&model->materials, material))
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Unable to load material %s into model.", iqm_material);
			continue;
		}
	}

	return true;
}

Material* Graphics_GetMaterialByName(MaterialArray *matarray,
										const char *name)
{
	if(matarray == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Unable to get material, invalid array.");
		return NULL;
	}
	Material *result = NULL;
	//this is not recommended but...
	for(size_t i = 0; i < matarray->count; i++)
	{
		if(SDL_strncmp(name, matarray->materials[i].material_name, 64) == 0)
			result = &matarray->materials[i];
	}
	return result;
}

void Graphics_UploadMaterial(Material *material)
{
	if(material == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Unable to upload material, invalid material.");
		return;
	}

	if(material->diffuse_map != NULL)
		Graphics_UploadTexture(material->diffuse_map);
	if(material->normal_map != NULL)
		Graphics_UploadTexture(material->normal_map);
	if(material->specular_map != NULL)
		Graphics_UploadTexture(material->specular_map);
	if(material->emission_map != NULL)
		Graphics_UploadTexture(material->emission_map);
	return;
}

void Graphics_UploadMaterials(MaterialArray *materials)
{
	if(materials == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Unable to upload materials, invalid array.");
		return;
	}

	for(size_t i = 0; i < materials->count; i++)
	{
		Graphics_UploadMaterial(&materials->materials[i]);
	}

	return;
}

void Graphics_ReleaseMaterial(Material *material)
{
	if(material == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Unable to release material, invalid material.");
		return;
	}

	if(material->diffuse_map != NULL)
		Graphics_ReleaseTexture(material->diffuse_map);
	if(material->normal_map != NULL)
		Graphics_ReleaseTexture(material->normal_map);
	if(material->specular_map != NULL)
		Graphics_ReleaseTexture(material->specular_map);
	if(material->emission_map != NULL)
		Graphics_ReleaseTexture(material->emission_map);
	return;
}

void Graphics_ReleaseMaterials(MaterialArray *materials)
{
	if(materials == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Graphics: Error: Unable to release materials, invalid array.");
		return;
	}

	for(size_t i = 0; i < materials->count; i++)
	{
		Graphics_ReleaseMaterial(&materials->materials[i]);
	}
	_arrayClearMaterial(materials);
	return;
}