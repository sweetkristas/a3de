#include <boost/shared_array.hpp>

#include "asserts.hpp"
#include "cubes.hpp"
#include "render.hpp"

namespace cube
{
	namespace 
	{
		const int num_array_buffers = 2;

		boost::shared_array<GLuint>& cube_array_buffer()
		{
			static boost::shared_array<GLuint> res;
			if(res == NULL) {
				res = boost::shared_array<GLuint>(new GLuint[num_array_buffers], graphics::vbo_deleter(num_array_buffers));
				glGenBuffers(num_array_buffers, &res[0]);
			}
			return res;
		}
	}

	world::world(shader::program_object_ptr shader, const std::string& fname)
		: shader_(shader), size_x_(0), size_y_(0), size_z_(0)
	{
		model_ = glm::mat4(1.0f);

		// grab actives iterators from shader so we can use them later to draw.
		mm_uniform_it_ = shader->get_uniform_iterator("model_matrix");
		a_position_it_ = shader->get_attribute_iterator("a_position");
		a_tex_coord_it_ = shader->get_attribute_iterator("a_tex_coord");
		tex0_it_ = shader->get_uniform_iterator("u_tex0");

		// Load surface and iterate over the red channel to get height details.
		graphics::surface_ptr surf = new graphics::surface(fname);
		SDL_Surface* s = surf->get();
		SDL_PixelFormat *fmt = s->format;
		ASSERT_LOG(fmt->BitsPerPixel != 8, fname << " needs to be 8 bpp format, found: " << fmt->BitsPerPixel);
		uint32_t* pixels = (uint32_t*)s->pixels;
		for(int x = 0; x != s->w; ++x) {
			for(int y = 0; y != s->h; ++y) {
				int z = pixels[y * s->pitch + x] & SURFACE_MASK_R;
				if(z != 0) {
					blocks_[boost::make_tuple(x,y,z)] = 1; // hard coded block type.
				}
			}
		}
		size_x_ = s->w;
		size_y_ = s->h;
		size_z_ = 256;

		arrays_ = cube_array_buffer();
	}

	world::~world()
	{
	}

	bool world::is_solid(int x, int y, int z) const
	{
		auto it = blocks_.find(boost::make_tuple(x,y,z));
		if(it == blocks_.end()) {
			return false;
		}
		return it->second != 0;
	}

	void world::build_world()
	{
		for(auto b = blocks_.begin(); b != blocks_.end(); ++b) {
			int x, y, z;
			boost::tie(x, y, z) = b->first;
			if(x > 0) {
				if(is_solid(x-1, y, z) == false) {
					add_face(x,y,z,LEFT);
				}
			} else {
				add_face(x,y,z,LEFT);
			}
			if(x < size_x_ - 1) {
				if(is_solid(x+1, y, z)) {
					add_face(x,y,z,RIGHT);
				}
			} else {
				add_face(x,y,z,RIGHT);
			}
			if(y > 0) {
				if(is_solid(x, y-1, z) == false) {
					add_face(x,y,z,BOTTOM);
				}
			} else {
				add_face(x,y,z,BOTTOM);
			}
			if(y < size_y_ - 1) {
				if(is_solid(x, y+1, z)) {
					add_face(x,y,z,TOP);
				}
			} else {
				add_face(x,y,z,TOP);
			}
			if(z > 0) {
				if(is_solid(x, y, z-1) == false) {
					add_face(x,y,z,FRONT);
				}
			} else {
				add_face(x,y,z,FRONT);
			}
			if(z < size_z_ - 1) {
				if(is_solid(x, y, z+1) == false) {
					add_face(x,y,z,BACK);
				}
			} else {
				add_face(x,y,z,BACK);
			}
		}

		
		//glBindBuffer(GL_ARRAY_BUFFER, arrays_[0]);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(cube_face_varray), cube_face_varray, GL_STATIC_DRAW);
		//glBindBuffer(GL_ARRAY_BUFFER, arrays_[0]);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(cube_face_tarray), cube_face_tarray, GL_STATIC_DRAW);
	}

	void world::add_face(int x, int y, int z, int direction)
	{
		switch(direction) {
		case FRONT:
			vertices_[FRONT].push_back(x+1); vertices_[FRONT].push_back(y+1); vertices_[FRONT].push_back(z+1);
			vertices_[FRONT].push_back(x+1); vertices_[FRONT].push_back(y+1); vertices_[FRONT].push_back(z);
			vertices_[FRONT].push_back(x); vertices_[FRONT].push_back(y+1); vertices_[FRONT].push_back(z+1);

			vertices_[FRONT].push_back(x); vertices_[FRONT].push_back(y+1); vertices_[FRONT].push_back(z+1);
			vertices_[FRONT].push_back(x+1); vertices_[FRONT].push_back(y+1); vertices_[FRONT].push_back(z);
			vertices_[FRONT].push_back(x); vertices_[FRONT].push_back(y+1); vertices_[FRONT].push_back(z);
			break;
		case RIGHT:
		case TOP:
		case BACK:
		case LEFT:
		case BOTTOM:
		}
	}

	void world::draw() const
	{
		shader_->make_active();

		shader_->set_uniform(mm_uniform_it_, model());

	}
}
