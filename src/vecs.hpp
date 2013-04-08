#pragma once

#include "asserts.hpp"
#include "sdl_stdinc.h"

struct vec3
{
	float x, y, z;
	vec3() : x(0.0f), y(0.0f), z(0.0f)
	{}

	inline vec3(float cx, float cy, float cz) : x(cx), y(cy), z(cz)
	{}

	inline float& operator[](size_t n)
	{
		ASSERT_LOG(n < 3, "vec4: Error index outside bounds");
		switch(n) {
		case 0: return x;
		case 1: return y;
		case 2: return z;
		default: break;
		}
		return x;
	}

	inline vec3 operator+=(vec3& n)
	{
		x += n.x;
		y += n.y;
		z += n.z;
		return *this;
	}
};

inline vec3 operator-(const vec3& lhs, const vec3& rhs)
{
	return vec3(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z);
}

inline vec3 normalise(const vec3& n)
{
	float length = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
	return vec3(n.x/length, n.y/length, n.z/length);
}

inline vec3 cross(const vec3& a, const vec3& b)
{
	return vec3(
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x
	);
}

inline float dot(const vec3& a, const vec3& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

struct vec4
{
	float x, y, z, w;

	vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
	{}

	inline vec4(float cx, float cy, float cz, float cw) : x(cx), y(cy), z(cz), w(cw)
	{}

	const float* get() const
	{
		return &x;
	}

	inline float& operator[](size_t n)
	{
		ASSERT_LOG(n < 4, "vec4: Error index outside bounds");
		switch(n) {
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
		default: break;
		}
		return x;
	}
};

struct mat4
{
	vec4 x;
	vec4 y;
	vec4 z;
	vec4 w;

	mat4()
	{}

	const float* get() const
	{
		return &x.x;
	}

	inline static mat4 identity()
	{
		return mat4(
			1.0f, 0.0f, 0.0f, 0.0f, 
			0.0f, 1.0f, 0.0f, 0.0f, 
			0.0f, 0.0f, 1.0f, 0.0f, 
			0.0f, 0.0f, 0.0f, 1.0f);
	}

	inline mat4(
		float cxx, float cxy, float cxz, float cxw,
		float cyx, float cyy, float cyz, float cyw,
		float czx, float czy, float czz, float czw,
		float cwx, float cwy, float cwz, float cww) :
		x(cxx, cxy, cxz, cxw),
		y(cyx, cyy, cyz, cyw),
		z(czx, czy, czz, czw),
		w(cwx, cwy, cwz, cww)
	{}
};

// Calculate a perspective projection transformation matrix
inline mat4 perspective(float field_of_view, float aspect_ratio, float near_clip, float far_clip)
{
	// Convert to radians
	float rad = field_of_view / 180.0f * float(M_PI);
	float range = tan(rad/2.0f) * near_clip;
	float left = -range * aspect_ratio;
	float right = range * aspect_ratio;
	float bottom = -range;
	float top = range;
	return mat4(
		2.0f*near_clip/(right-left), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f*near_clip/(top-bottom), 0.0f, 0.0f,
		0.0f, 0.0f, -(far_clip+near_clip)/(far_clip-near_clip), -2.0f*far_clip*near_clip/(far_clip-near_clip), 
		0.0f, 0.0f, -1.0f, 0.0f
	);
}

// Calculate orthographic project transformation matrix.
inline mat4 ortho(float field_of_view, float aspect_ratio, float near_clip, float far_clip)
{
	float rad = field_of_view / 180.0f * float(M_PI);
	float range = tan(rad/2.0f) * near_clip;
	float left = -range * aspect_ratio;
	float right = range * aspect_ratio;
	float bottom = -range;
	float top = range;
	return mat4(
		2.0f/(right-left), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f/(top-bottom), 0.0f, 0.0f,
		0.0f, 0.0f, -2.0f/(far_clip-near_clip), (far_clip+near_clip)/(far_clip-near_clip),
		0.0f, 0.0f, 0.0f, 1.0f 
	);
}

// Orthographic projection using specified co-ordinates.
inline mat4 ortho(float left, float right, float top, float bottom, float near_clip, float far_clip)
{
	return mat4(
		2.0f/(right-left), 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f/(top-bottom), 0.0f, 0.0f,
		0.0f, 0.0f, -2.0f/(far_clip-near_clip), (far_clip+near_clip)/(far_clip-near_clip),
		0.0f, 0.0f, 0.0f, 1.0f 
	);
}

inline mat4 look_at(const vec3& eye, const vec3& centre, const vec3& up)
{
	vec3 f = normalise(centre - eye);
	vec3 u = normalise(up);
	vec3 s = normalise(cross(f, u));
	u = cross(s, f);

	return mat4(
		 s.x,  s.y,  s.z, -dot(s,eye),
		 u.x,  u.y,  u.z, -dot(u,eye),
		-f.x, -f.y, -f.z,  dot(f,eye),
		0.0f, 0.0f, 0.0f,        1.0f
	);
}
