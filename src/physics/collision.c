/*
 * Modified from original work:
 * ----------------------------
 * Copyright (c) 2018 exezin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * This file contains modified code originally licensed under the MIT License,
 * now redistributed under the terms of the GNU GPL v3.
 */

/*
 * This modified version:
 * -----------------------------------------
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

#include <collision.h>

static inline float vec3_mul_inner(Vector3 const a, Vector3 const b)
{
	float p = 0.;
	int i;
	p += b.x * a.x;
	p += b.y * a.y;
	p += b.z * a.z;
	return p;
}

static inline float vec3_len2(Vector3 const v)
{
	return vec3_mul_inner(v, v);
}

static inline float vec3_len(Vector3 const v)
{
	return SDL_sqrtf(vec3_mul_inner(v,v));
}

static inline Vector3 vec3_norm(Vector3 const v)
{
	float k = 1.0 / vec3_len(v);
	return Vector3_Scale(v, k);
}

PhysPlane Coll_PlaneNew(const Vector3 a, const Vector3 b)
{
	PhysPlane plane;
	plane.origin = a;
	plane.normal = b;
	plane.equation[0] = b.x;
	plane.equation[1] = b.y;
	plane.equation[2] = b.z;
	plane.equation[3] = -(b.x * a.x + b.y * a.y + b.z * a.z);

	return plane;
}

PhysPlane Coll_TriangleToPlane(const Vector3 a, const Vector3 b, const Vector3 c)
{
	//FIXME (probably)
	//i think i made mistakes here... need to check when testing it
	//basically math functions are kinda not the same as the one exengine uses it
	//so issues may happen

	Vector3 ba = Vector3_Sub(b, a);
	Vector3 ca = Vector3_Sub(c, a);

	Vector3 temp;
	temp = Vector3_Cross(ba, ca);
	temp = Vector3_Normalize(temp);

	PhysPlane plane;
	plane.origin = a;
	plane.normal = temp;

	plane.equation[0] = temp.x;
	plane.equation[1] = temp.y;
	plane.equation[2] = temp.z;
	plane.equation[3] = -(temp.x * a.x + temp.y * a.y + temp.z * a.z);

	return plane;
}

float Coll_SignedDistanceToPlane(const Vector3 base_point, const PhysPlane *plane)
{
	// return vec3_mul_inner(base_point, plane->normal) - vec3_mul_inner(plane->normal, plane->origin);// + plane->equation[3];
	return vec3_mul_inner(base_point, plane->normal) + plane->equation[3];
}

bool Coll_IsFrontFacing(PhysPlane *plane, const Vector3 direction)
{
	double f = vec3_mul_inner(plane->normal, direction);

	if (f <= 0.0)
		return true;

	return false;
}

bool Coll_CheckPointInTriangle(const Vector3 point, const Vector3 p1, const Vector3 p2, const Vector3 p3)
{
	Vector3 u, v, w, vw, vu, uw, uv;
	u = Vector3_Sub(p2, p1);
	v = Vector3_Sub(p3, p1);
	w = Vector3_Sub(point, p1);

	vw = Vector3_Cross(v, w);
	vu = Vector3_Cross(v, u);

	if(vec3_mul_inner(vw, vu) < 0.0f)
	{
		return false;
	}

	uw = Vector3_Cross(u, w);
	uv = Vector3_Cross(u, v);

	if(vec3_mul_inner(uw, uv) < 0.0f)
	{
		return false;
	}

	float d = vec3_len(uv);
	float r = vec3_len(vw) / d;
	float t = vec3_len(uw) / d;

	return ((r + t) <= 1.0f);
}

bool Coll_GetLowestRoot(float a, float b, float c, float max, float *root)
{
	//check if solution exists
	float determinant = b * b - 4.0f * a * c;

	//if negative there is no solution
	if(determinant < 0.0f)
		return false;

	//calculate two roots
	float sqrtD = SDL_sqrtf(determinant);
	float r1 = (-b - sqrtD) / (2.0f * a);
	float r2 = (-b + sqrtD) / (2.0f * a);

	//set x1 <= x2
	if(r1 > r2)
	{
		float temp = r2;
		r2 = r1;
		r1 = temp;
	}

	//get lowest root
	if(r1 > 0 && r1 < max)
	{
		*root = r1;
		return true;
	}

	if(r2 > 0 && r2 < max)
	{
		*root = r2;
		return true;
	}

	//no solutions
	return false;
}

//Möller–Trumbore intersection algorithm
bool Coll_RayInTriangle(Vector3 from, Vector3 to, Vector3 v0, Vector3 v1, Vector3 v2, Vector3 *intersect)
{
	Vector3 vector = vec3_norm(to);

	Vector3 edge1, edge2, h, s, q;
	edge1 = Vector3_Sub(v1, v0);
	edge2 = Vector3_Sub(v2, v0);

	h = Vector3_Cross(vector, edge2);
	float a = vec3_mul_inner(edge1, h);

	if(a > -SDL_FLT_EPSILON && a < SDL_FLT_EPSILON)
		return false;

	float f = 1.0/a;

	s = Vector3_Sub(from, v0);

	float u = f * vec3_mul_inner(s, h);
	if(u < 0.0 || u > 1.0)
		return false;

	q = Vector3_Cross(s, edge1);
	float v = f * vec3_mul_inner(vector, q);
	if (v < 0.0 || u + v > 1.0)
		return false;

	float t = f * vec3_mul_inner(edge2, q);
	if (t > SDL_FLT_EPSILON)
	{
		Vector3 tmp = Vector3_Scale(vector, t);
		*intersect = Vector3_Add(from, tmp);
		return true;
	}

	return false;
}

void Coll_CollisionCheckTriangle(CollPacket *packet, const Vector3 p1, const Vector3 p2, const Vector3 p3)
{
	if(packet == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Coll_CollisionCheckTriangle error: invalid packet.");
		return;
	}
	PhysPlane plane = Coll_TriangleToPlane(p1, p2, p3);

	// only check front facing triangles
	if(!Coll_IsFrontFacing(&plane, packet->e_norm_velocity))
		return;

	//get interval of plane intersection
	double t0, t1;
	int embedded_in_plane = 0;

	//signed distance from sphere to point on plane
	double signed_dist_to_plane = Coll_SignedDistanceToPlane(packet->e_base_point, &plane);

	//cache this as we will reuse
	float normal_dot_vel = vec3_mul_inner(plane.normal, packet->e_velocity);

	//if sphere is moving parallel to plane
	if(normal_dot_vel == 0.0f)
	{
		if(SDL_fabs(signed_dist_to_plane) >= 1.0f)
		{
			//no collision possible
			return;
		}
		else
		{
			//sphere is in plane in whole range [0..1]
			embedded_in_plane = 1;
			t0 = 0.0;
			t1 = 1.0;
		}
	}
	else
	{
		//N dot D is not 0, calc intersect interval
		//float nvi = 1.0f / normal_dot_vel;
		t0 = (-1.0 - signed_dist_to_plane) / normal_dot_vel;
		t1 = ( 1.0 - signed_dist_to_plane) / normal_dot_vel;

		//swap so t0 < t1
		if(t0 > t1)
		{
			double temp = t1;
			t1 = t0;
			t0 = temp;
		}

		//check that at least one result is within range
		if(t0 > 1.0 || t1 < 0.0)
		{
			//both values outside range [0,1] so no collision
			return;
		}

		//clamp to [0,1]
		if(t0 < 0.0) t0 = 0.0;
		if(t1 < 0.0) t1 = 0.0;
		if(t0 > 1.0) t0 = 1.0;
		if(t1 > 1.0) t1 = 1.0;
	}

	//time to check for a collision
	Vector3 collision_point;
	bool found_collision = false;
	double t = 1.0;

	//first check collision with the inside of the triangle
	if(embedded_in_plane == 0)
	{
		Vector3 plane_intersect, temp;
		plane_intersect = Vector3_Sub(packet->e_base_point, plane.normal);
		temp = Vector3_Scale(packet->e_velocity, t0);
		plane_intersect = Vector3_Add(plane_intersect, temp);

		if(Coll_CheckPointInTriangle(plane_intersect, p1, p2, p3))
		{
			found_collision = true;
			t = t0;
			collision_point = plane_intersect;
		}
	}


	//no collision yet, check against points and edges
	if (!found_collision)
	{
		Vector3 velocity, base, temp;
		velocity = packet->e_velocity;
		base = packet->e_base_point;

		float velocity_sqrt_length = vec3_len2(velocity);
		float a,b,c;
		float new_t;

		//equation is a*t^2 + b*t + c = 0
		//check against points
		a = velocity_sqrt_length;

		//p1
		temp = Vector3_Sub(base, p1);
		b = 2.0f * (vec3_mul_inner(velocity, temp));
		temp = Vector3_Sub(p1, base);
		c = vec3_len2(temp) - 1.0;
		if(Coll_GetLowestRoot(a, b, c, t, &new_t))
		{
			t = new_t;
			found_collision = true;
			collision_point = p1;
		}

		//p2
		if(!found_collision)
		{
			temp = Vector3_Sub(base, p2);
			b = 2.0f * (vec3_mul_inner(velocity, temp));
			temp = Vector3_Sub(p2, base);
			c = vec3_len2(temp) - 1.0;
			if(Coll_GetLowestRoot(a, b, c, t, &new_t))
			{
				t = new_t;
				found_collision = true;
				collision_point = p2;
			}
		}

		//p3
		if(!found_collision)
		{
			temp = Vector3_Sub(base, p3);
			b = 2.0f * (vec3_mul_inner(velocity, temp));
			temp = Vector3_Sub(p3, base);
			c = vec3_len2(temp) - 1.0;
			if(Coll_GetLowestRoot(a, b, c, t, &new_t))
			{
				t = new_t;
				found_collision = true;
				collision_point = p3;
			}
		}

		//check against edges
		//p1 -> p2
		Vector3 edge, base_to_vertex;
		edge = Vector3_Sub(p2, p1);
		base_to_vertex = Vector3_Sub(p1, base);
		float edge_sqrt_length = vec3_len2(edge);
		float edge_dot_velocity = vec3_mul_inner(edge, velocity);
		float edge_dot_base_to_vertex = vec3_mul_inner(edge, base_to_vertex);

		// calculate params for equation
		a = edge_sqrt_length * -velocity_sqrt_length + edge_dot_velocity * edge_dot_velocity;
		b = edge_sqrt_length * (2.0f * vec3_mul_inner(velocity, base_to_vertex)) -
			2.0f * edge_dot_velocity * edge_dot_base_to_vertex;
		c = edge_sqrt_length * (1.0f - vec3_len2(base_to_vertex)) +
			edge_dot_base_to_vertex * edge_dot_base_to_vertex;

		// do we collide against infinite edge
		if (Coll_GetLowestRoot(a, b, c, t, &new_t))
		{
			// check if intersect is within line segment
			float f = (edge_dot_velocity * new_t - edge_dot_base_to_vertex) / edge_sqrt_length;
			if (f >= 0.0f && f <= 1.0f)
			{
				t = new_t;
				found_collision = true;
				temp = Vector3_Scale(edge, f);
				temp = Vector3_Add(p1, temp);
				collision_point = temp;
			}
		}

		//p2 -> p3
		edge = Vector3_Sub(p3, p2);
		base_to_vertex = Vector3_Sub(p2, base);
		edge_sqrt_length = vec3_len2(edge);
		edge_dot_velocity = vec3_mul_inner(edge, velocity);
		edge_dot_base_to_vertex = vec3_mul_inner(edge, base_to_vertex);

		//calculate params for equation
		a = edge_sqrt_length * -velocity_sqrt_length + edge_dot_velocity * edge_dot_velocity;
		b = edge_sqrt_length * (2.0f * vec3_mul_inner(velocity, base_to_vertex)) -
			2.0f * edge_dot_velocity * edge_dot_base_to_vertex;
		c = edge_sqrt_length * (1.0f - vec3_len2(base_to_vertex)) +
			edge_dot_base_to_vertex * edge_dot_base_to_vertex;

		//do we collide against infinite edge
		if(Coll_GetLowestRoot(a, b, c, t, &new_t))
		{
			// check if intersect is within line segment
			float f = (edge_dot_velocity * new_t - edge_dot_base_to_vertex) / edge_sqrt_length;
			if (f >= 0.0f && f <= 1.0f)
			{
				t = new_t;
				found_collision = true;
				temp = Vector3_Scale(edge, f);
				temp = Vector3_Add(p2, temp);
				collision_point = temp;
			}
		}

		//p3 -> p1
		edge = Vector3_Sub(p1, p3);
		base_to_vertex = Vector3_Sub(p3, base);
		edge_sqrt_length = vec3_len2(edge);
		edge_dot_velocity = vec3_mul_inner(edge, velocity);
		edge_dot_base_to_vertex = vec3_mul_inner(edge, base_to_vertex);

		// calculate params for equation
		a = edge_sqrt_length * -velocity_sqrt_length + edge_dot_velocity * edge_dot_velocity;
		b = edge_sqrt_length * (2.0f * vec3_mul_inner(velocity, base_to_vertex)) -
			2.0f * edge_dot_velocity * edge_dot_base_to_vertex;
		c = edge_sqrt_length * (1.0f - vec3_len2(base_to_vertex)) +
			edge_dot_base_to_vertex * edge_dot_base_to_vertex;

		//do we collide against infinite edge
		if (Coll_GetLowestRoot(a, b, c, t, &new_t))
		{
			//check if intersect is within line segment
			float f = (edge_dot_velocity * new_t - edge_dot_base_to_vertex) / edge_sqrt_length;
			if (f >= 0.0f && f <= 1.0f)
			{
				t = new_t;
				found_collision = true;
				temp = Vector3_Scale(edge, f);
				temp = Vector3_Add(p3, temp);
				collision_point = temp;
			}
		}
	}

	// set results
	if (found_collision)
	{
		//distance to collision, t is time of collision
		double dist_to_coll = t * vec3_len(packet->e_velocity);

		// are we the closest hit?
		if(packet->found_collision == 0 || dist_to_coll < packet->nearest_distance)
		{
			packet->nearest_distance = dist_to_coll;
			packet->intersect_point = collision_point;
			packet->found_collision = 1;
			packet->t = t;
			packet->plane = plane;
			packet->a = p1;
			packet->b = p2;
			packet->c = p3;
		}
	}
}
