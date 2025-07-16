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

#include <collision.h>

PhysPlane Coll_PlaneNew(const Vector3 a, const Vector3 b)
{
	PhysPlane plane;
	plane.origin = a;
	plane.normal = b;
	plane.equation[0] = b.x;
	plane.equation[1] = b.y;
	plane.equation[2] = b.z;
	plane.equation[3] = -(b.x * a.x + b.y * a.y + b.z * a.z);

	return plane;
}

PhysPlane Coll_TriangleToPlane(const Vector3 a, const Vector3 b, const Vector3 c)
{
	//FIXME (probably)
	//i think i made mistakes here... need to check when testing it
	//basically math functions are kinda not the same as the one exengine uses it
	//so issues may happen

	Vector3 ba = Vector3_Sub(b, a);
	Vector3 ca = Vector3_Sub(c, a);

	Vector3 temp;
	temp = Vector3_Cross(ba, ca);
	temp = Vector3_Normalize(temp);

	PhysPlane plane;
	plane.origin = a;
	plane.normal = temp;

	plane.equation[0] = temp.x;
	plane.equation[1] = temp.y;
	plane.equation[2] = temp.z;
	plane.equation[3] = -(temp.x * a.x + temp.y * a.y + temp.z * a.z);

	return plane;
}

static inline float vec3_mul_inner(Vector3 const a, Vector3 const b)
{
	float p = 0.;
	int i;
	p += b.x * a.x;
	p += b.y * a.y;
	p += b.z * a.z;
	return p;
}

float Coll_SignedDistanceToPlane(const Vector3 base_point, const PhysPlane *plane)
{
	// return vec3_mul_inner(base_point, plane->normal) - vec3_mul_inner(plane->normal, plane->origin);// + plane->equation[3];
	return vec3_mul_inner(base_point, plane->normal) + plane->equation[3];
}

bool Coll_IsFrontFacing(PhysPlane *plane, const Vector3 direction)
{
	double f = vec3_mul_inner(plane->normal, direction);

	if (f <= 0.0)
		return true;

	return false;
}

bool Coll_CheckPointInTriangle(const Vector3 point, const Vector3 p1, const Vector3 p2, const Vector3 p3)
{
	return false;
}

bool Coll_GetLowestRoot(float a, float b, float c, float max, float *root)
{
	return false;
}

bool Coll_RayInTriangle(Vector3 from, Vector3 to, Vector3 v0, Vector3 v1, Vector3 v2, Vector3 intersect)
{
	return false;
}

void Coll_CollisionCheckTriangle(CollPacket *packet, const Vector3 p1, const Vector3 p2, const Vector3 p3)
{
	return;
}
