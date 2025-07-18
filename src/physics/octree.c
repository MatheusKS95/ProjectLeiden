/*
 * Modified from original work:
 * ----------------------------
 * Copyright (c) 2018 exezin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * This file contains modified code originally licensed under the MIT License,
 * now redistributed under the terms of the GNU GPL v3.
 */

/*
 * This modified version:
 * -----------------------------------------
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

#include <octree.h>

int octree_min_size = OCTREE_DEFAULT_MIN_SIZE;

static inline Box _newbox(Vector3 min, Vector3 max)
{
	Box b;
	b.min = min;
	b.max = max;
	return b;
};

static inline bool _aabb_aabb(Box a, Box b)
{
	return (a.min.x <= b.max.x &&
			a.max.x >= b.min.x &&
			a.min.y <= b.max.y &&
			a.max.y >= b.min.y &&
			a.min.z <= b.max.z &&
			a.max.z >= b.min.z);
};

static inline bool _aabb_inside(Box outer, Box inner)
{
	return (outer.min.x <= inner.min.x &&
			outer.max.x >= inner.max.x &&
			outer.min.y <= inner.min.y &&
			outer.max.y >= inner.max.y &&
			outer.min.z <= inner.min.z &&
			outer.max.z >= inner.max.z);
};

static void* _octree_data_ptr(Octree *octree)
{
	switch(octree->data_type)
	{
		case OBJ_TYPE_UINT:
			if(octree->data_uint != NULL)
				return octree->data_uint;
			break;
		case OBJ_TYPE_INT:
			if(octree->data_int != NULL)
				return octree->data_int;
			break;
		case OBJ_TYPE_BYTE:
			if(octree->data_byte != NULL)
				return octree->data_byte;
			break;
		case OBJ_TYPE_FLOAT:
			if(octree->data_float != NULL)
				return octree->data_float;
			break;
		case OBJ_TYPE_DOUBLE:
			if(octree->data_double != NULL)
				return octree->data_double;
			break;
		default:
			return NULL;
			break;
	}

	return NULL;
}

Octree *Octree_Create(OctreeObjectType type)
{
	Octree *octree = (Octree*)SDL_malloc(sizeof(Octree));
	if(octree == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Octree_Create error: failed to create octree.");
		return NULL;
	}

	for(int i = 0; i < 8; i++)
	{
		octree->children[i] = NULL;
	}

	//FIXME Vector3 is not an array, so I don't need this
	octree->region.min = (Vector3){ 0.0f, 0.0f };
	octree->region.max = (Vector3){ 1.0f, 1.0f };

	octree->built = false;
	octree->first = true;
	List_Init(&octree->obj_list);

	octree_min_size = octree_min_size;

	octree->data_len = 0;
	octree->data_type = type;
	octree->data_uint = NULL;
	octree->data_int = NULL;
	octree->data_byte = NULL;
	octree->data_float = NULL;
	octree->data_double = NULL;

	return octree;
}

bool Octree_Init(Octree *octree, Box region, List objects)
{
	if(octree == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Octree_Init error: invalid octree.");
		return false;
	}

	SDL_memcpy(&octree->region, &region, sizeof(Box));
	for(int i = 0; i < 8; i++)
	{
		octree->children[i] = NULL;
	}
	octree->obj_list = objects;
	octree->built = false;
	octree->first = false;
	octree->data_len = 0;
	octree->data_type = OBJ_TYPE_NULL;
	octree->data_uint = NULL;
	octree->data_int = NULL;
	octree->data_byte = NULL;
	octree->data_float = NULL;
	octree->data_double = NULL;
	return true;
}

void Octree_Build(Octree *octree)
{
	if(octree == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Octree_Build error: invalid octree.");
		return;
	}
	if(octree->obj_list.first == NULL)
	{
		return;
	}
	if(octree->obj_list.first->next == NULL)
	{
		Octree_Finalize(octree);
		return;
	}

	//our size
	Vector3 region = Vector3_Sub(octree->region.max, octree->region.min);

	if(region.x <= octree_min_size || region.y <= octree_min_size || region.z <= octree_min_size)
	{
		if (!octree->first)
		{
			Octree_Finalize(octree);
			return;
		}
	}

	Vector3 half = Vector3_Scale(region, 0.5f);
	Vector3 centre = Vector3_Add(octree->region.min, half);

	//octant regions
	Box octants[8];
	octants[0] = _newbox(octree->region.min, centre);
	octants[1] = _newbox((Vector3){centre.x, octree->region.min.y, octree->region.min.z}, (Vector3){octree->region.max.x, centre.y, centre.z});
	octants[2] = _newbox((Vector3){centre.x, octree->region.min.y, centre.z}, (Vector3){octree->region.max.z, centre.y, octree->region.max.z});
	octants[3] = _newbox((Vector3){octree->region.min.x, octree->region.min.y, centre.z}, (Vector3){centre.x, centre.y, octree->region.max.z});
	octants[4] = _newbox((Vector3){octree->region.min.x, centre.y, octree->region.min.z}, (Vector3){centre.x, octree->region.max.y, centre.z});
	octants[5] = _newbox((Vector3){centre.x, centre.y, octree->region.min.z}, (Vector3){octree->region.max.x, octree->region.max.y, centre.z});
	octants[6] = _newbox(centre, octree->region.max);
	octants[7] = _newbox((Vector3){octree->region.min.x, centre.y, centre.z}, (Vector3){centre.x, octree->region.max.y, octree->region.max.z});

	// object lists
	List obj_lists[8];
	size_t obj_lenghts[8];
	for (int i = 0; i < 8; i++)
	{
		List_Init(&obj_lists[i]);
		obj_lenghts[i] = 0;
	}

	// add objects to appropriate octant
	size_t obj_count = 0;
	ListItem *n = octree->obj_list.first;
	while(n->value != NULL)
	{
		int found = 0;

		for(int j = 0; j < 8; j++)
		{
			OctreeObject *obj = (OctreeObject*)n->value;
			if(_aabb_inside(octants[j], obj->box))
			{
				List_AddLast(&obj_lists[j], (void*)n->value); //FIXME deal with bool
				obj_lenghts[j]++;
				found = 1;
				break;
			}
		}

		//remove obj from this list
		if (found)
		{
			ListItem *next = n->next;
			List_Remove(&octree->obj_list, (void*)n->value);
			if(next != NULL)
			{
				n = next;
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{
			obj_count++;
		}

		if (n->next != NULL)
			n = n->next;
		else
			break;
	}

	//create children
	for(int i = 0; i < 8; i++)
	{
		if(obj_lists[i].first != NULL)
		{
			octree->children[i] = (Octree*)SDL_malloc(sizeof(Octree)); //FIXME deal with null (original code don't deal with it either)
			Octree_Init(octree->children[i], octants[i], obj_lists[i]);
			octree->children[i]->data_len  = obj_lenghts[i];
			octree->children[i]->data_type = octree->data_type;
			Octree_Build(octree->children[i]);
		}
		else
		{
			octree->children[i] = NULL;
		}
	}

	octree->data_len = obj_count;
	Octree_Finalize(octree);
	return;
}

void Octree_Finalize(Octree *octree)
{
	if(octree == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Octree_Finalize error: invalid octree.");
		return;
	}

	//move object data into a flat array
	int i = 0;
	ListItem *n = octree->obj_list.first;
	while(n->value != NULL)
	{
		OctreeObject *data = n->value;

		switch(octree->data_type)
		{
			case OBJ_TYPE_UINT:
			{
				if (i == 0)
				{
					octree->data_uint = (Uint32*)SDL_malloc(octree->data_len * sizeof(Uint32));
				}
				memcpy(&octree->data_uint[i], &data->data_uint, sizeof(Uint32));
				break;
			}
			case OBJ_TYPE_INT:
			{
				if (i == 0)
				{
					octree->data_int = (Sint32*)SDL_malloc(octree->data_len * sizeof(Sint32));
				}
				memcpy(&octree->data_int[i], &data->data_int, sizeof(Sint32));
				break;
			}
			case OBJ_TYPE_BYTE:
			{
				if (i == 0)
				{
					octree->data_byte = (Uint8*)SDL_malloc(octree->data_len * sizeof(Uint8));
				}
				memcpy(&octree->data_byte[i], &data->data_byte, sizeof(Uint8));
				break;
			}
			case OBJ_TYPE_FLOAT:
			{
				if (i == 0)
				{
					octree->data_float  = (float*)SDL_malloc(octree->data_len * sizeof(float));
				}
				memcpy(&octree->data_float[i], &data->data_float, sizeof(float));
				break;
			}
			case OBJ_TYPE_DOUBLE:
			{
				if (i == 0)
				{
					octree->data_double = (double*)SDL_malloc(octree->data_len * sizeof(double));
				}
				memcpy(&octree->data_double[i], &data->data_double, sizeof(double));
				break;
			}
			case OBJ_TYPE_NULL: break;
		}

		SDL_free(n->value);
		n->value = NULL;
		i++;
		if(n->next != NULL)
			n = n->next;
		else
			break;
	}

	//destroy our temp list
	if (octree->obj_list.first != NULL)
	{
		List_Destroy(&octree->obj_list);
	}

	octree->built = true;

	return;
}

Octree *Octree_Reset(Octree *octree)
{
	if(octree == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Octree_Reset error: invalid octree.");
		return NULL;
	}

	for (int i=0; i<8; i++)
	{
		if (octree->children[i] != NULL)
		{
			Octree_Reset(octree->children[i]);
		}
	}

	if (octree->obj_list.first != NULL)
	{
		List_Destroy(&octree->obj_list);
	}

	if (octree->data_len > 0 && octree->data_type != OBJ_TYPE_NULL)
	{
		switch(octree->data_type)
		{
			case OBJ_TYPE_UINT:
				if(octree->data_uint != NULL)
					SDL_free(octree->data_uint);
				break;
			case OBJ_TYPE_INT:
				if (octree->data_int != NULL)
					SDL_free(octree->data_int);
				break;
			case OBJ_TYPE_BYTE:
				if(octree->data_byte != NULL)
					SDL_free(octree->data_byte);
				break;
			case OBJ_TYPE_FLOAT:
				if (octree->data_float != NULL)
					SDL_free(octree->data_float);
				break;
			case OBJ_TYPE_DOUBLE:
				if (octree->data_double != NULL)
					SDL_free(octree->data_double);
				break;
			default: break; //just because without this QtCreator keeps complaining
		}
	}

	int data_type = octree->data_type;
	if (!octree->first)
	{
		SDL_free(octree);
	}
	else
	{
		SDL_free(octree);
		return Octree_Create(data_type);
	}

	return NULL;
}

void Octree_GetCollidingCount(Octree *octree, Box *bounds, int *count)
{
	if(octree == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Octree_GetCollidingCount error: invalid octree.");
		return;
	}

	//add our data to the list
	void *oct_data = _octree_data_ptr(octree);
	if(oct_data != NULL)
	{
		if (!_aabb_aabb(octree->region, *bounds))
			return;

		(*count)++;
	}

	//recurse adding data to the list
	for(int i = 0; i < 8; i++)
		if(octree->children[i] != NULL)
			Octree_GetCollidingCount(octree->children[i], bounds, count);
	return;
}

void Octree_GetColliding(Octree *octree, Box *bounds, OctreeData *data_list, int *index)
{
	if(octree == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Octree_GetColliding error: invalid octree.");
		return;
	}

	//add our data to the list
	void *oct_data = _octree_data_ptr(octree);
	if(oct_data != NULL)
	{
		if (!_aabb_aabb(octree->region, *bounds))
			return;

		data_list[*index].len = octree->data_len;
		data_list[*index].data = oct_data;
		(*index)++;
	}

	//recurse adding data to the list
	for(int i = 0; i < 8; i++)
		if(octree->children[i] != NULL)
			Octree_GetColliding(octree->children[i], bounds, data_list, index);
	return;
}
