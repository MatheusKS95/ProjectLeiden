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

#include <SDL3/SDL.h>
#include "linmath.h"

Vector3 Vector3_Add(Vector3 vector_a, Vector3 vector_b)
{
	Vector3 result;
	result.x = vector_a.x + vector_b.x;
	result.y = vector_a.y + vector_b.y;
	result.z = vector_a.z + vector_b.z;
	return result;
}

Vector3 Vector3_Sub(Vector3 vector_a, Vector3 vector_b)
{
	Vector3 result;
	result.x = vector_a.x - vector_b.x;
	result.y = vector_a.y - vector_b.y;
	result.z = vector_a.z - vector_b.z;
	return result;
}

Vector3 Vector3_Normalize(Vector3 vector)
{
	float magnitude = SDL_sqrtf((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
	Vector3 normalized = vector;
	normalized.x = vector.x / magnitude;
	normalized.y = vector.y / magnitude;
	normalized.z = vector.z / magnitude;
	return normalized;
}

float Vector3_Dot(Vector3 vector_a, Vector3 vector_b)
{
	float dot = (vector_a.x * vector_b.x) + (vector_a.y * vector_b.y) + (vector_a.z * vector_b.z);
	return dot;
}

Vector3 Vector3_Cross(Vector3 vector_a, Vector3 vector_b)
{
	Vector3 result;
	result.x = vector_a.y * vector_b.z - vector_b.y * vector_a.z;
	result.y = -(vector_a.x * vector_b.z - vector_b.x * vector_a.z);
	result.z = vector_a.x * vector_b.y - vector_b.x * vector_a.y;

	return result;
}

Vector3 Vector3_Scale(Vector3 vector, float scale)
{
	Vector3 result;
	result.x = vector.x * scale;
	result.y = vector.y * scale;
	result.z = vector.z * scale;

	return result;
}

Matrix4x4 Matrix4x4_Mul(Matrix4x4 matrix_a, Matrix4x4 matrix_b)
{
	Matrix4x4 result;

	result.aa = (
		(matrix_a.aa * matrix_b.aa) +
		(matrix_a.ab * matrix_b.ba) +
		(matrix_a.ac * matrix_b.ca) +
		(matrix_a.ad * matrix_b.da)
	);
	result.ab = (
		(matrix_a.aa * matrix_b.ab) +
		(matrix_a.ab * matrix_b.bb) +
		(matrix_a.ac * matrix_b.cb) +
		(matrix_a.ad * matrix_b.db)
	);
	result.ac = (
		(matrix_a.aa * matrix_b.ac) +
		(matrix_a.ab * matrix_b.bc) +
		(matrix_a.ac * matrix_b.cc) +
		(matrix_a.ad * matrix_b.dc)
	);
	result.ad = (
		(matrix_a.aa * matrix_b.ad) +
		(matrix_a.ab * matrix_b.bd) +
		(matrix_a.ac * matrix_b.cd) +
		(matrix_a.ad * matrix_b.dd)
	);
	result.ba = (
		(matrix_a.ba * matrix_b.aa) +
		(matrix_a.bb * matrix_b.ba) +
		(matrix_a.bc * matrix_b.ca) +
		(matrix_a.bd * matrix_b.da)
	);
	result.bb = (
		(matrix_a.ba * matrix_b.ab) +
		(matrix_a.bb * matrix_b.bb) +
		(matrix_a.bc * matrix_b.cb) +
		(matrix_a.bd * matrix_b.db)
	);
	result.bc = (
		(matrix_a.ba * matrix_b.ac) +
		(matrix_a.bb * matrix_b.bc) +
		(matrix_a.bc * matrix_b.cc) +
		(matrix_a.bd * matrix_b.dc)
	);
	result.bd = (
		(matrix_a.ba * matrix_b.ad) +
		(matrix_a.bb * matrix_b.bd) +
		(matrix_a.bc * matrix_b.cd) +
		(matrix_a.bd * matrix_b.dd)
	);
	result.ca = (
		(matrix_a.ca * matrix_b.aa) +
		(matrix_a.cb * matrix_b.ba) +
		(matrix_a.cc * matrix_b.ca) +
		(matrix_a.cd * matrix_b.da)
	);
	result.cb = (
		(matrix_a.ca * matrix_b.ab) +
		(matrix_a.cb * matrix_b.bb) +
		(matrix_a.cc * matrix_b.cb) +
		(matrix_a.cd * matrix_b.db)
	);
	result.cc = (
		(matrix_a.ca * matrix_b.ac) +
		(matrix_a.cb * matrix_b.bc) +
		(matrix_a.cc * matrix_b.cc) +
		(matrix_a.cd * matrix_b.dc)
	);
	result.cd = (
		(matrix_a.ca * matrix_b.ad) +
		(matrix_a.cb * matrix_b.bd) +
		(matrix_a.cc * matrix_b.cd) +
		(matrix_a.cd * matrix_b.dd)
	);
	result.da = (
		(matrix_a.da * matrix_b.aa) +
		(matrix_a.db * matrix_b.ba) +
		(matrix_a.dc * matrix_b.ca) +
		(matrix_a.dd * matrix_b.da)
	);
	result.db = (
		(matrix_a.da * matrix_b.ab) +
		(matrix_a.db * matrix_b.bb) +
		(matrix_a.dc * matrix_b.cb) +
		(matrix_a.dd * matrix_b.db)
	);
	result.dc = (
		(matrix_a.da * matrix_b.ac) +
		(matrix_a.db * matrix_b.bc) +
		(matrix_a.dc * matrix_b.cc) +
		(matrix_a.dd * matrix_b.dc)
	);
	result.dd = (
		(matrix_a.da * matrix_b.ad) +
		(matrix_a.db * matrix_b.bd) +
		(matrix_a.dc * matrix_b.cd) +
		(matrix_a.dd * matrix_b.dd)
	);

	return result;
}

Matrix4x4 Matrix4x4_Rotate(Matrix4x4 mat, Vector3 axis, float radians)
{
	axis = Vector3_Normalize(axis);
	float cos_theta = SDL_cosf(radians);
	float sin_theta = SDL_sinf(radians);
	float one_minus_cos = 1.0f - cos_theta;

	Matrix4x4 rotation_matrix =
	{
		cos_theta + axis.x * axis.x * one_minus_cos,
		axis.x * axis.y * one_minus_cos - axis.z * sin_theta,
		axis.x * axis.z * one_minus_cos + axis.y * sin_theta,
		0,

		axis.y * axis.x * one_minus_cos + axis.z * sin_theta,
		cos_theta + axis.y * axis.y * one_minus_cos,
		axis.y * axis.z * one_minus_cos - axis.x * sin_theta,
		0,

		axis.z * axis.x * one_minus_cos - axis.y * sin_theta,
		axis.z * axis.y * one_minus_cos + axis.x * sin_theta,
		cos_theta + axis.z * axis.z * one_minus_cos,
		0,

		0, 0, 0, 1
	};

	return Matrix4x4_Mul(mat, rotation_matrix);
}

Matrix4x4 Matrix4x4_RotateX(Matrix4x4 mat, float radians)
{
	Matrix4x4 aux =
	{
		1, 0, 0, 0,
		0, SDL_cosf(radians), SDL_sinf(radians), 0,
		0, -SDL_sinf(radians), SDL_cosf(radians), 0,
		0, 0, 0, 1
	};
	return Matrix4x4_Mul(mat, aux);
}

Matrix4x4 Matrix4x4_RotateY(Matrix4x4 mat, float radians)
{
	Matrix4x4 aux =
	{
		SDL_cosf(radians), 0, -SDL_sinf(radians), 0,
		SDL_sinf(radians), 1, SDL_cosf(radians), 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	return Matrix4x4_Mul(mat, aux);
}

Matrix4x4 Matrix4x4_RotateZ(Matrix4x4 mat, float radians)
{
	Matrix4x4 aux =
	{
		SDL_cosf(radians), SDL_sinf(radians), 0, 0,
		-SDL_sinf(radians), SDL_cosf(radians), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	return Matrix4x4_Mul(mat, aux);
}

Matrix4x4 Matrix4x4_Translate(Matrix4x4 mat, float x, float y, float z)
{
	Matrix4x4 transl_matrix =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1
	};
	return Matrix4x4_Mul(mat, transl_matrix);
}

Matrix4x4 Matrix4x4_Scale(Matrix4x4 mat, Vector3 scale)
{
	Matrix4x4 scale_matrix =
	{
		scale.x, 0, 0, 0,
		0, scale.y, 0, 0,
		0, 0, scale.z, 0,
		0, 0, 0, 1
	};
	return Matrix4x4_Mul(mat, scale_matrix);
}

Matrix4x4 Matrix4x4_Perspective(float fov, float aspectratio,
								float near, float far)
{
	float num = 1.0f / ((float)SDL_tanf(fov * 0.5f));
	Matrix4x4 persp =
	{
		num / aspectratio, 0, 0, 0,
		0, num, 0, 0,
		0, 0, far / (near - far), -1,
		0, 0, (near * far) / (near - far), 0
	};
	return persp;
}

Matrix4x4 Matrix4x4_LookAt(Vector3 camera_position,
							Vector3 camera_target,
							Vector3 camera_up)
{
	Vector3 target2pos =
	{
		camera_position.x - camera_target.x,
		camera_position.y - camera_target.y,
		camera_position.z - camera_target.z
	};
	Vector3 v_s1 = Vector3_Normalize(target2pos);
	Vector3 v_s2 = Vector3_Normalize(Vector3_Cross(camera_up, v_s1));
	Vector3 v_s3 = Vector3_Cross(v_s1, v_s2);

	Matrix4x4 lookat =
	{
		v_s2.x, v_s3.x, v_s1.x, 0,
		v_s2.y, v_s3.y, v_s1.y, 0,
		v_s2.z, v_s3.z, v_s1.z, 0,
		-Vector3_Dot(v_s2, camera_position), -Vector3_Dot(v_s3, camera_position), -Vector3_Dot(v_s1, camera_position), 1
	};
	return lookat;
}

float RadToDeg(float radians)
{
	return radians * (180.0f / SDL_PI_F);
}

float DegToRad(float degrees)
{
	return degrees * (SDL_PI_F / 180.0f);
}
