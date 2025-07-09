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

static Uint32 nonzero(Uint32 x)
{
	return x != 0 ? x : 1;
}

//temporary, should be moved to linmath
static float vectordistance(Vector3 a, Vector3 b)
{
	float dx = b.x - a.x;
	float dy = b.y - a.y;
	float dz = b.z - a.z;
	return SDL_sqrt(dx * dx + dy * dy + dz * dz);
}

Vector3 Phys_EnvGround(Vector3 point, Sint32 height)
{
	if(point.y > height)
	{
		point.y = height;
	}
	return point;
}

PhysJoint Phys_NewJoint(Vector3 position, Uint32 size)
{
	PhysJoint result;
	result.velocity = (Vector3){0.0f, 0.0f, 0.0f};
	result.position = position;

	size /= PHYSICS_JOINT_SIZE_MULTIPLIER;

	if(size > 255)
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Physics warning: Phys_NewJoint - joint size too big on PhysJoint.");
	}

	result.size_divided = size;

	return result;
}

void Phys_BodyInit(PhysBody *body, PhysJoint *joints, Uint8 joint_count,
					PhysConnection *connections, Uint8 connection_count,
					Sint32 mass)
{
	if(body == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Physics error: Phys_BodyInit - invalid body.");
		return;
	}
	body->joints = joints;
	body->num_joints = joint_count;
	body->connections = connections;
	body->num_connections = connection_count;
	body->deactivate_count = 0;
	body->friction = PHYSICS_F / 2.0f;
	body->elasticity = PHYSICS_F / 2.0f;
	body->flags = 0;
	body->joint_mass = nonzero(mass / joint_count);

	for(Uint32 i = 0; i < connection_count; ++i)
	{
		float distance = vectordistance(joints[connections[i].joint1].position, joints[connections[i].joint2].position);

		if(distance > 60000.0f)
		{
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Physics warning: Phys_BodyInit - distance way too large.");
		}

		connections[i].length = distance != 0.0f ? distance : 1.0f; //prevents division-by-zero errors
	}
	return;
}

void Phys_WorldInit(PhysWorld *world, PhysBody *bodies, Uint16 body_count,
						PhysClosestPointFunction environmentFunction)
{
	if(world == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Physics error: Phys_WorldInit - invalid world.");
		return;
	}
	world->bodies = bodies;
	world->num_bodies = body_count;
	world->environment_function = environmentFunction;
	world->collision_callback = NULL;
	return;
}