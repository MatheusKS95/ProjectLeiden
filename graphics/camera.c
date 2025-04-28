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

/**
 * @file camera.c
 * @brief Implementation file for camera stuff
 *
 * @author
 * - Matheus Klein Schaefer (email here)
*/

#include <graphics.h>

///DEFAULT CAMERA VALUES
//will be removed and replaced by settings-based variables in the
//conf file
static const float YAW =			-90.0f;
static const float PITCH =			0.0f;
static const float ROLL =			0.0f;
static const float SPEED =			0.1f;
static const float SENSITIVITY =	0.2f;
static const float ZOOM =			45.0f;

//TODO FIXME
//move this to camera_update
static void camera_get_view_matrix(Camera *input)
{
	Vector3 sum = Vector3_Add(input->position, input->front);
	Matrix4x4 result = Matrix4x4_LookAt(input->position, sum, input->up);
	input->view = result;
}

static void camera_update(Camera *input)
{
	Vector3 front;

	front.x = SDL_cos(DegToRad(input->yaw)) * SDL_cos(DegToRad(input->pitch));
	front.y = SDL_sin(DegToRad(input->pitch));
	front.z = SDL_sin(DegToRad(input->yaw)) * SDL_cos(DegToRad(input->pitch));
	input->front = Vector3_Normalize(front);

	Vector3 crossprod = Vector3_Cross(input->front, input->world_up);
	input->right = Vector3_Normalize(crossprod);

	crossprod = Vector3_Cross(input->right, input->front);
	input->up = Vector3_Normalize(crossprod);

	//use config to read draw distance, maybe...? FIXME TODO
	input->projection = Matrix4x4_Perspective(DegToRad(input->zoom), input->aspect_ratio, 0.1f, 5000.0f);
	//FIXME since it's here and no longer on rendering stuff
	camera_get_view_matrix(input);
}

void Graphics_InitCameraBasic(Camera *camera, Vector3 position)
{
	camera->position = position;
	camera->up = (Vector3){0.0f, 1.0f, 0.0f};
	camera->world_up = camera->up;
	camera->yaw = YAW;
	camera->pitch = PITCH;
	camera->roll = ROLL;
	camera->zoom = ZOOM;
	camera->aspect_ratio = (float)context.width / (float)context.height;

	camera_update(camera);
}

void Graphics_InitCameraFull(Camera *camera, Vector3 position,
								Vector3 up, float yaw, float pitch,
								float roll, float zoom)
{
	camera->position = position;

	camera->up = up;
	camera->world_up = camera->up;

	camera->yaw = yaw;
	camera->pitch = pitch;
	camera->roll = roll;
	camera->zoom = zoom;
	camera->aspect_ratio = (float)context.width / (float)context.height;

	camera_update(camera);
	return;
}

void Graphics_UpdateCameraPosition(Camera *camera, Vector3 position)
{
	camera->position = position;
	camera_update(camera);
}

void Graphics_TestCameraFreecam(Camera *camera, float x_offset,
								float y_offset, bool constraint)
{
	x_offset *= SENSITIVITY;
	y_offset *= SENSITIVITY;

	camera->yaw += x_offset;
	camera->pitch += y_offset;

	if(constraint)
	{
		if(camera->pitch > 89.0f)
			camera->pitch = 89.0f;
		if(camera->pitch < -89.0f)
			camera->pitch = -89.0f;
	}

	camera_update(camera);
	return;
}
