#pragma once

#include <vector>
#include "vecs.hpp"
 
namespace obj
{
	struct obj_data
	{
		std::vector<vec3> vertices;
		std::vector<vec3> uvs;
		std::vector<vec3> normals;
	};

	void load_obj_file(const std::string& filename, obj_data& o);
}
