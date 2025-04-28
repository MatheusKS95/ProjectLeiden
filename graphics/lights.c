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
 * @file lights.c
 * @brief Implementation file for lighing stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <graphics.h>

bool Graphics_NewPointlight(Pointlight *l, Vector3 position,
							Vector3 colour, float constant,
							float linear, float quadratic)
{
	if(l == NULL)
	{
		return false;
	}
	l->position = position;
	l->colour = colour;

	l->constant = constant;
	l->linear = linear;
	l->quadratic = quadratic;
	return true;
}

void Graphics_DestroyPointlight(Pointlight *l)
{
	if(l == NULL)
		return;

	SDL_free(l);
}

bool Graphics_NewDirlight(Dirlight *l, Vector3 direction,
							Vector3 colour)
{
	if(l == NULL)
	{
		return false;
	}
	l->direction = direction;
	l->colour = colour;
	return true;
}

void Graphics_DestroyDirlight(Dirlight *l)
{
	if(l == NULL)
		return;

	SDL_free(l);
}

bool Graphics_NewSpotlight(Spotlight *l, Vector3 position,
							Vector3 rotation, Vector3 colour,
							float constant, float linear,
							float quadratic, float inctf,
							float otctf)
{
	if(l == NULL)
	{
		return false;
	}
	l->rel_position = position;
	l->rel_rotation = rotation;
	l->colour = colour;

	l->constant = constant;
	l->linear = linear;
	l->quadratic = quadratic;

	l->inner_cutoff = inctf;
	l->outer_cutoff = otctf;
	return true;
}

void Graphics_DestroySpotlight(Spotlight *l)
{
	if(l == NULL)
		return;

	SDL_free(l);
}
