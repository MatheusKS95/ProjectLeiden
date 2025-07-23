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

#include <physics.h>

bool Physics_AABBvsAABB(AABB a, AABB b)
{
	return (SDL_fabs(a.center.x - b.center.x) < (a.half_size.x + b.half_size.x)) &&
			(SDL_fabs(a.center.y - b.center.y) < (a.half_size.y + b.half_size.y)) &&
			(SDL_fabs(a.center.z - b.center.z) < (a.half_size.z + b.half_size.z));
}

//uses SAT
bool Physics_AABBvsTriangle(AABB box, Triangle tri)
{
	//move triangle to AABB local space (centre at origin)
	Vector3 v0 = Vector3_Sub(tri.v0, box.center);
	Vector3 v1 = Vector3_Sub(tri.v1, box.center);
	Vector3 v2 = Vector3_Sub(tri.v2, box.center);

	//triangle edges
	Vector3 f0 = Vector3_Sub(v1, v0);
	Vector3 f1 = Vector3_Sub(v2, v1);
	Vector3 f2 = Vector3_Sub(v0, v2);

	//AABB axes
	Vector3 u0 = {1.0f, 0.0f, 0.0f};
	Vector3 u1 = {0.0f, 1.0f, 0.0f};
	Vector3 u2 = {0.0f, 0.0f, 1.0f};

	//test 9 axes cross products
	Vector3 axes[13];
	int i = 0;
	axes[i++] = u0; axes[i++] = u1; axes[i++] = u2; //AABB axes
	axes[i++] = Vector3_Cross(f0, u0);
	axes[i++] = Vector3_Cross(f0, u1);
	axes[i++] = Vector3_Cross(f0, u2);
	axes[i++] = Vector3_Cross(f1, u0);
	axes[i++] = Vector3_Cross(f1, u1);
	axes[i++] = Vector3_Cross(f1, u2);
	axes[i++] = Vector3_Cross(f2, u0);
	axes[i++] = Vector3_Cross(f2, u1);
	axes[i++] = Vector3_Cross(f2, u2);
	//triangle normal (f0 x f1)
	axes[i++] = Vector3_Cross(f0, f1);

	//SAT test for each axis
	for(int j = 0; j < 13; ++j)
	{
		Vector3 axis = axes[j];

		//skip axis if it's near zero (degenerate)
		float len2 = Vector3_Dot(axis, axis);
		if (len2 < 1e-6f)
			continue;

		//project triangle
		float p0 = Vector3_Dot(v0, axis);
		float p1 = Vector3_Dot(v1, axis);
		float p2 = Vector3_Dot(v2, axis);

		float minTri = SDL_min(p0, SDL_min(p1, p2));
		float maxTri = SDL_max(p0, SDL_max(p1, p2));

		//project AABB
		float r =
			box.half_size.x * SDL_fabs(axis.x) +
			box.half_size.y * SDL_fabs(axis.y) +
			box.half_size.z * SDL_fabs(axis.z);

		//SAT check
		if (minTri > r || maxTri < -r)
			return false; //separating axis found
	}

	//no separating axis found: collision
	return true;
}

//TODO