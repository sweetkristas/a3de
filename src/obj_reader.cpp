#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>

#include "filesystem.hpp"
#include "obj_reader.hpp"
#include "utils.hpp"

namespace obj
{
	static const boost::regex re_v("v"
		"\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"
		"\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"
		"\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"
		"(?:\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?))?");
	static const boost::regex re_vt("vt"
		"\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"
		"\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"
		"(?:\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?))?");
	static const boost::regex re_vn("vn"
		"\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"
		"\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"
		"\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)");
	static const boost::regex re_vp("vp"
		"\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"
		"(?:\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)"
		"(?:\\s+([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?))?)?");
	static const boost::regex re_f_sub("(\\d+)(?:/(\\d+)?(?:/(\\d+)?)?)?");

	void load_obj_file(const std::string& filename, obj_data& o)
	{
		std::string obj_data = sys::read_file(filename);
		std::vector<std::string> lines = utils::split(obj_data, "\n");
		BOOST_FOREACH(auto line, lines) {
			std::stringstream ss(line);
			std::string symbol;
			ss >> symbol;
			if(symbol == "v") {
				vec4 vertex(0.0f, 0.0f, 0.0f, 1.0f);
				boost::smatch what;
				if(boost::regex_match(line, what, re_v)) {
					for(int n = 1; n !=	what.size(); ++n) {
						if(what[n].str().empty() == false) {
							vertex[n-1] = boost::lexical_cast<float>(what[n]);
						}
					}
				}
				o.vertices.push_back(vertex);
			} else if(symbol == "vt") {
				vec3 uvw(0.0f, 0.0f, 0.0f);
				boost::smatch what;
				if(boost::regex_match(line, what, re_vt)) {
					for(int n = 1; n !=	what.size(); ++n) {
						if(what[n].str().empty() == false) {
							uvw[n-1] = boost::lexical_cast<float>(what[n]);
						}
					}
				}
				o.uvs.push_back(uvw);
			} else if(symbol == "vn") {
				vec3 normal(0.0f, 0.0f, 0.0f);
				boost::smatch what;
				if(boost::regex_match(line, what, re_vn)) {
					for(int n = 1; n !=	what.size(); ++n) {
						if(what[n].str().empty() == false) {
							normal[n-1] = boost::lexical_cast<float>(what[n]);
						}
					}
				}
				o.normals.push_back(normal);
			} else if(symbol == "vp") {
				vec3 psv(0.0f, 0.0f, 0.0f);
				boost::smatch what;
				if(boost::regex_match(line, what, re_vn)) {
					for(int n = 1; n !=	what.size(); ++n) {
						if(what[n].str().empty() == false) {
							psv[n-1] = boost::lexical_cast<float>(what[n]);
						}
					}
				}
				o.parameter_space_vertices.push_back(psv);
			} else if(symbol == "f") {
				std::string group;
				do {
					ss >> group;
					boost::smatch what;
					if(boost::regex_match(group, what, re_f_sub)) {
						GLushort indicies[3] = {0, 0, 0};
						for(int n = 1; n !=	what.size(); ++n) {
							if(what[n].str().empty() == false) {
								indicies[n-1] = boost::lexical_cast<GLushort>(what[n])-1;
							}
						o.face_vertex_index.push_back(indicies[0]);
						o.face_uv_index.push_back(indicies[1]);
						o.face_normal_index.push_back(indicies[2]);
						}
					}
				} while(ss.eof() == false && group.empty() == false);
			}
		}
	}
}