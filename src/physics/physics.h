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

#define PHYSICS_FRACTIONS_PER_UNIT 512			///< one fixed point unit, don't change
#define PHYSICS_F PHYSICS_FRACTIONS_PER_UNIT	///< short for TPE_FRACTIONS_PER_UNIT
#define PHYSICS_JOINT_SIZE_MULTIPLIER 32		///< joint size is scaled (size saving)

/** Function used for defining static environment, working similarly to an SDF
  (signed distance function). The parameters are: 3D point P, max distance D.
  The function should behave like this: if P is inside the solid environment
  volume, P will be returned; otherwise closest point (by Euclidean distance) to
  the solid environment volume from P will be returned, except for a case when
  this closest point would be further away than D, in which case any arbitrary
  point further away than D may be returned (this allows for optimizations). */
typedef Vector3 (*PhysClosestPointFunction)(Vector3, Sint32);

/** Function that can be used as a joint-joint or joint-environment collision
  callback, parameters are following: body1 index, joint1 index, body2 index,
  joint2 index, collision world position. If body1 index is the same as body1
  index, then collision type is body-environment, otherwise it is body-body
  type. The function has to return either 1 if the collision is to be allowed
  or 0 if it is to be discarded. This can besides others be used to disable
  collisions between some bodies. */
typedef Uint8 (*PhysCollisionCallback)(Uint16, Uint16, Uint16, Uint16, Vector3);

typedef struct PhysJoint
{
	Vector3 position;
	Vector3 velocity;
	Uint8 size_divided;
} PhysJoint;

typedef struct PhysConnection
{
	Uint8 joint1;
	Uint8 joint2;
	Uint16 length;
} PhysConnection;

typedef struct PhysBody
{
	PhysJoint *joints;
	size_t num_joints;
	PhysConnection *connections;
	size_t num_connections;
	Uint16 joint_mass; //mass of a single joint
	Uint16 friction; //friction of each joint
	Uint16 elasticity; //elasticity of each joint
	Uint8 flags;
	Uint8 deactivate_count;
} PhysBody;

typedef struct PhysWorld
{
	PhysBody *bodies;
	size_t num_bodies;
	PhysClosestPointFunction environment_function;
	PhysCollisionCallback collision_callback;
} PhysWorld;

Vector3 Phys_EnvGround(Vector3 point, Sint32 height);

PhysJoint Phys_NewJoint(Vector3 position, Uint32 size);

void Phys_BodyInit(PhysBody *body, PhysJoint *joints, Uint8 joint_count,
					PhysConnection *connections, Uint8 connection_count,
					Sint32 mass);

void Phys_WorldInit(PhysWorld *world, PhysBody *bodies, Uint16 body_count,
						PhysClosestPointFunction environmentFunction);

#endif
