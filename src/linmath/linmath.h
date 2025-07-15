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

#ifndef LINMATH_H
#define LINMATH_H

typedef struct Vector2
{
	float x, y;
} Vector2;

typedef struct Vector3
{
	float x, y, z;
} Vector3;

typedef struct Vector4
{
	float x, y, z, w;
} Vector4;

typedef struct Matrix4x4
{
	float aa, ab, ac, ad;
	float ba, bb, bc, bd;
	float ca, cb, cc, cd;
	float da, db, dc, dd;
} Matrix4x4;

Vector3 Vector3_Add(Vector3 vector_a, Vector3 vector_b);

Vector3 Vector3_Sub(Vector3 vector_a, Vector3 vector_b);

Vector3 Vector3_Normalize(Vector3 vector);

float Vector3_Dot(Vector3 vector_a, Vector3 vector_b);

Vector3 Vector3_Cross(Vector3 vector_a, Vector3 vector_b);

Vector3 Vector3_Scale(Vector3 vector, float scale);

Matrix4x4 Matrix4x4_Mul(Matrix4x4 matrix_a, Matrix4x4 matrix_b);

Matrix4x4 Matrix4x4_Rotate(Matrix4x4 mat, Vector3 axis, float radians);

Matrix4x4 Matrix4x4_RotateX(Matrix4x4 mat, float radians);

Matrix4x4 Matrix4x4_RotateY(Matrix4x4 mat, float radians);

Matrix4x4 Matrix4x4_RotateZ(Matrix4x4 mat, float radians);

Matrix4x4 Matrix4x4_Translate(Matrix4x4 mat, float x, float y, float z);

Matrix4x4 Matrix4x4_Scale(Matrix4x4 mat, Vector3 scale);

Matrix4x4 Matrix4x4_Perspective(float fov, float aspectratio,
								float near, float far);

Matrix4x4 Matrix4x4_LookAt(Vector3 camera_position,
							Vector3 camera_target,
							Vector3 camera_up);

float RadToDeg(float radians);

float DegToRad(float degrees);

#endif
