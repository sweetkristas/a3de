#pragma once

#include <cstdint>
#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders.hpp"
#include "texture.hpp"

namespace cube
{
	typedef boost::unordered_map<boost::tuple<int,int,int>, uint16_t> block_type;
	class world
	{
	public:
		enum {
			FRONT,
			RIGHT,
			TOP,
			BACK,
			LEFT,
			BOTTOM,
		};
		world(shader::program_object_ptr shader, const std::string& fname);
		virtual ~world();

		const GLfloat* model() const { return glm::value_ptr(model_); }
		void draw() const;

		void build_world();
		void add_face(int x, int y, int z, int direction);

	protected:
		bool is_solid(int x, int y, int z) const;
	private:
		shader::program_object_ptr shader_;
		block_type blocks_;
		glm::mat4 model_;

		int size_x_;
		int size_y_;
		int size_z_;

		shader::program_object_ptr shader_;
		shader::const_actives_map_iterator mm_uniform_it_;
		shader::const_actives_map_iterator a_position_it_;
		shader::const_actives_map_iterator a_tex_coord_it_;
		shader::const_actives_map_iterator tex0_it_;

		boost::shared_array<GLuint> arrays_;

		std::vector<GLfloat> vertices_[6];

		world() {}
		world(const world&) {}
	};

}