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
	l->rel_position = position;
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
