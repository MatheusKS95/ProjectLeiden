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

#ifndef OCTREE_H
#define OCTREE_H

#include <SDL3/SDL.h>
#include <list.h>
#include <linmath.h>

#define OCTREE_DEFAULT_MIN_SIZE 5.0f
extern int octree_min_size;

typedef struct Box
{
	Vector3 min;
	Vector3 max;
} Box;

typedef enum
{
	OBJ_TYPE_UINT,
	OBJ_TYPE_INT,
	OBJ_TYPE_BYTE,
	OBJ_TYPE_FLOAT,
	OBJ_TYPE_DOUBLE,
	OBJ_TYPE_NULL
} OctreeObjectType;

typedef struct OctreeObject
{
	union
	{
		Uint32 data_uint;
		Sint32 data_int;
		Uint8 data_byte;
		float data_float;
		double data_double;
	};
	Box box;
} OctreeObject;

typedef struct OctreeData
{
	void *data;
	size_t len;
} OctreeData;

typedef struct Octree Octree;

struct Octree
{
	Box region;
	Octree *children[8];
	int max_life, cur_life;
	List obj_list;

	//flags etc
	bool built;
	bool first;
	OctreeObjectType data_type;

	// data
	size_t data_len;
	union {
		Uint32 *data_uint;
		Sint32 *data_int;
		Uint8 *data_byte;
		float *data_float;
		double *data_double;
	};

	int player_inside;
};

Octree *Octree_Create(OctreeObjectType type);

bool Octree_Init(Octree *octree, Box region, List objects);

void Octree_Build(Octree *octree);

void Octree_Finalize(Octree *octree);

Octree *Octree_Reset(Octree *octree);

void Octree_GetCollidingCount(Octree *octree, Box *bounds, int *count);

void Octree_GetColliding(Octree *octree, Box *bounds, OctreeData *data_list, int *index);

#endif
