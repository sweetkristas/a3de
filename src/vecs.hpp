#pragma once

struct vec4
{
	float x, y, z, w;

	vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
	{}

	inline vec4(float cx, float cy, float cz, float cw) : x(cx), y(cy), z(cz), w(cw)
	{}
};

struct mat4
{
	vec4 x;
	vec4 y;
	vec4 z;
	vec4 w;

	mat4()
	{}

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
