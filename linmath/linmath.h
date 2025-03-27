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

Vector3 Vector3_Normalize(Vector3 vector);

float Vector3_Dot(Vector3 vector_a, Vector3 vector_b);

Vector3 Vector3_Cross(Vector3 vector_a, Vector3 vector_b);

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
