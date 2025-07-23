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

#ifndef PHYSICS_H
#define PHYSICS_H

#include <SDL3/SDL.h>
#include <linmath.h>

/*typedef struct Engine
{
	float displacement;
} Engine;

//Basic ass car
typedef struct Car
{
	Vector3 position; //central position (x, y, z)
	Vector3 velocity; //linear velocity
	float angle; //yaw
	float pitch, roll; //for suspension
	float speed;

	//float max_speed; //lemme do some basic engine calculations
	//float acceleration;

	float turn_rate; //how much car turns
	float suspension_softness;
	float wheel_base; //distance between axles (affects pitch)
	float track_width; //distance between wheels from the same axle (affect roll)
} Car;*/

//box for collision
typedef struct AABB
{
	Vector3 center;
	Vector3 half_size;
} AABB;

//triangle used for mesh collision
typedef struct Triangle
{
	Vector3 v0, v1, v2;
} Triangle;

//ground collision
typedef struct GroundContact
{
	float ground_height;
	Vector3 normal;
} GroundContact;

bool Physics_AABBvsAABB(AABB a, AABB b);

bool Physics_AABBvsTriangle(AABB box, Triangle tri);

#endif
