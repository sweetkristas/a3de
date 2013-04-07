#include <boost/foreach.hpp>
#include <sstream>

#include "filesystem.hpp"
#include "obj_reader.hpp"
#include "utils.hpp"

namespace obj
{
	void load_obj_file(const std::string& filename, obj_data& o)
	{
		std::string obj_data = sys::read_file(filename);
		std::vector<std::string> lines = utils::split(obj_data, "\n");
		BOOST_FOREACH(auto line, lines) {
			std::stringstream ss;
			std::string symbol;
			ss >> symbol;
			if(symbol == "v") {
				vec3 vertex;
				ss >> vertex.x >> vertex.y >> vertex.z;
				o.vertices.push_back(vertex);
			} else if(symbol == "vt") {
				vec3 uv;
				ss >> uv.x >> uv.y;
				o.uvs.push_back(uv);
			} else if(symbol == "vn") {
				vec3 normal;
				ss >> normal.x >> normal.y >> normal.z;
				o.normals.push_back(normal);
			} else if(symbol == "f") {
			}
		}
	}
}