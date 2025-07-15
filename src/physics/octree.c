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

Octree *Octree_Create(OctreeObjectType type)
{
	Octree *octree = (Octree*)SDL_malloc(sizeof(Octree));
	if(octree == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Octree_Create error: failed to create octree.");
		return;
	}

	for(int i = 0; i < 8; i++)
	{
		octree->children[i] = NULL;
	}

	//FIXME Vector3 is not an array, so I don't need this
	octree->region.min = Vector3{ 0.0f };
	octree->region.max = Vector3{ 1.0f, 1.0f };

	octree->rendered = false;
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
	octree->rendered = false;
	octree->built = false;
	octree->first = false;
	octree->data_len = 0;
	octree->data_type = OBJ_TYPE_NULL;
	octree->data_uint = NULL;
	octree->data_int = NULL;
	octree->data_byte = NULL;
	octree->data_float = NULL;
	octree->data_double = NULL;
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
	Vector3 region;
	/*vec3_sub(region, o->region.max, o->region.min);

	if (region[0] <= ex_octree_min_size || region[1] <= ex_octree_min_size || region[2] <= ex_octree_min_size) {
		if (!o->first) {
		ex_octree_finalize(o);
		return;
		}
	}*/
	return;
}

void Octree_Finalize(Octree *octree)
{
	return;
}

Octree *Octree_Reset(Octree *octree)
{
	return NULL;
}

void Octree_GetCollidingCount(Octree *octree, Box *bounds, int *count)
{
	return;
}

void Octree_GetColliding(Octree *octree, Box *bounds, OctreeData *data_list, int *index)
{
	return;
}