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

#ifndef COLLISION_H
#define COLLISION_H

#include <SDL3/SDL.h>
#include <linmath.h>
#include <octree.h>

typedef struct PhysPlane
{
	Vector3 origin;
	Vector3 normal;
	float equation[4];
} PhysPlane;

typedef struct CollPacket
{
	//r3 space
	Vector3 r3_velocity;
	Vector3 r3_position;

	//ellipsoid space
	Vector3 e_radius;
	Vector3 e_velocity;
	Vector3 e_norm_velocity;
	Vector3 e_base_point;

	//original tri points
	Vector3 a, b, c;

	//hit information
	int found_collision;
	float nearest_distance;
	double t;
	Vector3 intersect_point;
	PhysPlane plane;

	// iteration depth
	int depth;
} CollPacket;

#endif
