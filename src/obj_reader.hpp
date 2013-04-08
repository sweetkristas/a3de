#pragma once

#include <vector>
#include <cstdint>
#include "graphics.hpp"
#include "vecs.hpp"
 
namespace obj
{
	struct obj_data
	{
		std::vector<vec4> vertices;
		std::vector<vec3> uvs;
		std::vector<vec3> normals;

		// vertex, uv, normal
		std::vector<GLushort> face_vertex_index;
		std::vector<GLushort> face_uv_index;
		std::vector<GLushort> face_normal_index;

		std::vector<vec3> parameter_space_vertices;
	};

	void load_obj_file(const std::string& filename, obj_data& o);
}
