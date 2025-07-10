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

static PhysCollisionCallback _Phys_CollisionCallback;

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

static Sint32 physics_jointsize(PhysJoint *joint)
{
	return joint->size_divided * PHYSICS_JOINT_SIZE_MULTIPLIER;
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

bool Phys_IsBodyActive(const PhysBody *body)
{
	if(body == NULL) return false;
	return !(body->flags & PHYSICS_BODY_FLAG_DEACTIVATED);
}

Vector3 Phys_GetBodyCenterOfMass(const PhysBody *body)
{
	//note that joint sizes don't play a role as all weight the same

	Vector3 result = { 0 };

	if(body == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Physics error: Phys_GetBodyCenterOfMass - invalid body.");
		return result;
	}

	const PhysJoint *j = body->joints;

	for(size_t i = 0; i < body->num_joints; ++i)
	{
		result = Vector3_Add(result, j->position);
		j++;
	}

	result.x /= body->num_joints;
	result.y /= body->num_joints;
	result.z /= body->num_joints;

	return result;
}

void Phys_ApplyGravityToBody(PhysBody *body, float downwards_accel)
{
	if(body == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Physics error: Phys_GetBodyCenterOfMass - invalid body.");
		return;
	}

	if((body->flags & PHYSICS_BODY_FLAG_DEACTIVATED) || (body->flags & PHYSICS_BODY_FLAG_DISABLED))
	{
		return;
	}

	for (size_t i = 0; i < body->num_joints; ++i)
	{
		body->joints[i].velocity.y -= downwards_accel;
	}
}

void Phys_GetBodyAABB(const PhysBody *body, Vector3 *v_min, Vector3 *v_max)
{
	//TODO FIXME check if body and vectors are NULL or not
	*v_min = body->joints[0].position;
	*v_max = *v_min;

	Sint32 js = PHYSICS_JOINT_SIZE(body->joints[0]);

	v_min->x -= js;
	v_min->y -= js;
	v_min->z -= js;

	v_max->x += js;
	v_max->y += js;
	v_max->z += js;

	for (uint16_t i = 1; i < body->num_joints; ++i)
	{
		Sint32 v;

		js = PHYSICS_JOINT_SIZE(body->joints[i]);

		#define test(c) \
			v = body->joints[i].position.c - js; \
			if (v < v_min->c) \
			v_min->c = v; \
			v += 2 * js; \
			if (v > v_max->c) \
			v_max->c = v;

			test(x)
			test(y)
			test(z)

		#undef test
	}
}

//TODO finish this
void Phys_WorldStep(PhysWorld *world)
{
	_Phys_CollisionCallback = world->collision_callback;

	for(size_t i = 0; i < world->num_bodies; ++i)
	{
		PhysBody *body = world->bodies + i;

		if (body->flags & (PHYSICS_BODY_FLAG_DEACTIVATED | PHYSICS_BODY_FLAG_DISABLED))
			continue;

		PhysJoint *joint = body->joints;
		PhysJoint *joint2;

		Vector3 origin_point = body->joints[0].position;

		for(size_t j = 0; j < body->num_joints; ++j) //apply velocities
		{
			//non-rotating bodies will copy the 1st joint's velocity
			if (body->flags & PHYSICS_BODY_FLAG_NONROTATING)
			{
				joint->velocity.x = body->joints[0].velocity.x;
				joint->velocity.y = body->joints[0].velocity.y;
				joint->velocity.z = body->joints[0].velocity.z;
			}

			joint->position.x += joint->velocity.x;
			joint->position.y += joint->velocity.y;
			joint->position.z += joint->velocity.z;

			joint++;
		}

		PhysConnection *connection = body->connections;

		Vector3 aabb_min, aabb_max;
		Phys_GetBodyAABB(body, &aabb_min, &aabb_max);

		//continue
	}
}