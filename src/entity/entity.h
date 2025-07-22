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

//incomplete, file is still a placeholder

#ifndef ENTITY_H
#define ENTITY_H

#include <SDL3/SDL.h>

typedef struct EulerTransforms
{
	Vector3 position;
	Vector3 rotation;
	float scale;
} EulerTransforms;

//no physics, just existing (out of reach objects)
typedef struct NoPhysicsEntity
{
	Model *model;
	EulerTransforms transform;
} NoPhysicsEntity;

//have physics, but don't move at all (most objects in scene)
typedef struct StaticEntity
{
	Model *model;
	EulerTransforms transform;
	//OBB structure
} StaticEntity;

//TODO complete this

#endif
