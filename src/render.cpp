#include <sstream>
#include <iomanip>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/shared_array.hpp>

#include "asserts.hpp"
#include "filesystem.hpp"
#include "graphics.hpp"
#include "profile_timer.hpp"
#include "render.hpp"
#include "render_text.hpp"


namespace graphics
{
	namespace
	{
		static const GLfloat cube_face_varray[6][12] = {
			{
				// front
				// CW winding
				//0.0f, 0.0f, 0.0f,
				//0.0f, 1.0f, 0.0f,
				//1.0f, 0.0f, 0.0f,
				//1.0f, 1.0f, 0.0f,
				// CCW winding
				0.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 0.0f,
			}, {
				// right
				1.0f, 0.0f, 0.0f,
				1.0f, 0.0f, -1.0f,
				1.0f, 1.0f, 0.0f,
				1.0f, 1.0f, -1.0f,
			}, {
				// top
				1.0f, 0.0f, 0.0f,
				1.0f, 1.0f, 0.0f,
				0.0f, 1.0f, -1.0f,
				1.0f, 1.0f, -1.0f,
			}, {
				// back
				1,0,-1,
				0,0,-1,
				0,1,-1,
				1,1,-1,
			}, {
				// left
				0,0,-1,
				0,1,-1,
				0,1,0,
				0,0,0,
			}, {
				// bottom
				0,0,-1,
				0,0,0,
				1,0,0,
				1,0,-1,
			}
		};

		static const GLfloat cube_face_tarray[6][8] = {
			{
				// front
				0.0f, 1.0f/3.0f,
				1.0f/3.0f, 1.0f/3.0f,
				0.0f, 0.0f,
				1.0f/3.0f, 0.0f,
			}, {
				// right
				1.0f/3.0f, 1.0f/3.0f, 
				2.0f/3.0f, 1.0f/3.0f, 
				1.0f/3.0f, 0.000000f, 
				2.0f/3.0f, 0.000000f, 
			}, {
				// top
				2.0f/3.0f, 1.0f/3.0f, 
				1.000000f, 1.0f/3.0f, 
				2.0f/3.0f, 0.000000f, 
				1.000000f, 0.000000f, 
			}, {
				// back
				0.000000f, 1.000000f, 
				0.333333f, 1.000000f, 
				0.000000f, 0.666666f, 
				0.333333f, 0.666666f, 
			}, {
				// left
				0.000000f, 1.000000f, 
				0.333333f, 1.000000f, 
				0.000000f, 0.666666f, 
				0.333333f, 0.666666f, 
			}, {
				// bottom
				0.000000f, 1.000000f, 
				0.333333f, 1.000000f, 
				0.000000f, 0.666666f, 
				0.333333f, 0.666666f, 
			}
		};

		static const int num_array_buffers = 2;

		struct cube_array_buffer_deleter
		{
			void operator()(GLuint* d) 
			{
				glDeleteBuffers(num_array_buffers, d);
				delete [num_array_buffers] d;
			}
		};

		boost::shared_array<GLuint>& cube_array_buffer()
		{
			static boost::shared_array<GLuint> res;
			if(res == NULL) {
				res = boost::shared_array<GLuint>(new GLuint[num_array_buffers], cube_array_buffer_deleter());
				glGenBuffers(num_array_buffers, &res[0]);
				glBindBuffer(GL_ARRAY_BUFFER, res[0]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(cube_face_varray), cube_face_varray, GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, res[1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(cube_face_tarray), cube_face_tarray, GL_DYNAMIC_DRAW);
			}
			return res;
		}
	}

	class cube
	{
	public:
		cube(shader::program_object_ptr shader)
			: shader_(shader)
		{
			mm_uniform_it_ = shader->get_uniform_iterator("model_matrix");
			a_position_it_ = shader->get_attribute_iterator("a_position");
			a_tex_coord_it_ = shader->get_attribute_iterator("a_tex_coord");
			tex0_it_ = shader->get_uniform_iterator("u_tex0");

			array_buffers_ = cube_array_buffer();
		}

		virtual ~cube()
		{}

		void draw(cube_model_ptr cm) const
		{
			if(cm->is_fully_occluded()) {
				return;
			}

			shader_->set_uniform(mm_uniform_it_, &cm->model()[0][0]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, cm->tex_id());
			glUniform1i(tex0_it_->second.location, 0);

			glEnableVertexAttribArray(a_position_it_->second.location);
			glEnableVertexAttribArray(a_tex_coord_it_->second.location);

			for(int i = cube_model::FRONT; i <= cube_model::TOP; ++i) {
				if(!cm->should_draw_face(i)) {
					continue;
				}

				glBindBuffer(GL_ARRAY_BUFFER, array_buffers_[0]);
				glVertexAttribPointer(
					a_position_it_->second.location, // The attribute we want to configure
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					sizeof(GLfloat)*i*12,  // stride
					(void*)0            // array buffer offset
				);

				glBindBuffer(GL_ARRAY_BUFFER, array_buffers_[1]);
				glVertexAttribPointer(
					a_tex_coord_it_->second.location, // The attribute we want to configure
					2,                            // size : U+V => 2
					GL_FLOAT,                     // type
					GL_FALSE,                     // normalized?
					sizeof(GLfloat)*i*12,			// stride
					(void*)0                      // array buffer offset
				);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}

			glDisableVertexAttribArray(a_position_it_->second.location);
			glDisableVertexAttribArray(a_tex_coord_it_->second.location);
		}
	protected:
	private:
		shader::program_object_ptr shader_;
		shader::const_actives_map_iterator mm_uniform_it_;
		shader::const_actives_map_iterator a_position_it_;
		shader::const_actives_map_iterator a_tex_coord_it_;
		shader::const_actives_map_iterator tex0_it_;

		boost::shared_array<GLuint> array_buffers_;
	};

	cube_model::cube_model()
	{
		model_ = glm::mat4(1.0f);
	}

	cube_model::cube_model(const std::string& texname)
	{
		tex_ = texture::get(texname);
		model_ = glm::mat4(1.0f);
	}

	cube_model::~cube_model()
	{
	}

	void cube_model::translate(float dx, float dy, float dz)
	{
		glm::vec3 v(dx, dy, dz);
		model_ = glm::translate(model_, v);
	}

	void cube_model::rotate(float angle, const glm::vec3& axis)
	{
		model_ = glm::rotate(model_, angle, axis);
	}

	glm::mat4& cube_model::model() const 
	{ 
		return model_; 
	}

	GLuint cube_model::tex_id() const 
	{ 
		ASSERT_LOG(tex_ != NULL, "Call of cube_model::tex_id() when texture is null.");
		return tex_->id();
	}

	void cube_model::set_neighbourhood(int px, int nx, int py, int ny, int pz, int nz)
	{
		pos_x_neighbour_ = px;
		neg_x_neighbour_ = nx;
		pos_y_neighbour_ = py;
		neg_y_neighbour_ = ny;
		pos_z_neighbour_ = pz;
		neg_z_neighbour_ = nz;
	}

	bool cube_model::should_draw_face(int f)
	{
		switch(f) {
			case FRONT: return pos_z_neighbour_ == false;
			case RIGHT:	return pos_x_neighbour_ == false;
			case TOP:	return pos_y_neighbour_ == false;
			case BACK:	return pos_z_neighbour_ == true;
			case LEFT:	return pos_x_neighbour_ == true;
			case BOTTOM:return pos_y_neighbour_ == true;
		}
		return false;
	}

	bool cube_model::is_fully_occluded()
	{
		return pos_x_neighbour_ && neg_x_neighbour_ 
			&& pos_y_neighbour_ && neg_y_neighbour_ 
			&& pos_z_neighbour_ && neg_z_neighbour_;
	}

	namespace
	{
		shader::program_object_ptr tex2d_shader;
		shader::const_actives_map_iterator tex2d_u_texmap_it;
		shader::const_actives_map_iterator tex2d_a_position_it;
		shader::const_actives_map_iterator tex2d_a_texcoord_it;

		shader::program_object_ptr poly_shader;
		shader::const_actives_map_iterator poly_u_color_it;
		shader::const_actives_map_iterator poly_a_position_it;
		
		
		boost::shared_array<GLuint> generic_vbo;
		const int num_generic_vbo = 2;
		struct vbo_deleter
		{
			void operator()(GLuint* vbo)
			{
				glDeleteBuffers(num_generic_vbo, vbo);
			}
		};
	}

	render::render(graphics::window_manager& wm, int w, int h) 
			: wm_(wm), width_(w), height_(h)
	{
		//view_ = glm::lookAt(glm::vec3(4.0f,3.0f,10.0f), 
		//	glm::vec3(0.0f, 0.0f, 0.0f), 
		//	glm::vec3(0.0f, 1.0f, 0.0f));
		//projection_ = glm::perspective(45.0f, float(w)/float(h), 0.1f, 10.0f);
		view_ = glm::mat4();
		projection_ = glm::mat4();

		tex2d_shader.reset(new shader::program_object("texture_shader_2d",
			shader::shader(GL_VERTEX_SHADER, "texture_2d_vert", sys::read_file("data/texture_2d.vert")),
			shader::shader(GL_FRAGMENT_SHADER, "texture_2d_frag", sys::read_file("data/texture_2d.frag"))));
		tex2d_u_texmap_it = tex2d_shader->get_uniform_iterator("u_tex_map");
		tex2d_a_position_it = tex2d_shader->get_attribute_iterator("a_position");
		tex2d_a_texcoord_it = tex2d_shader->get_attribute_iterator("a_texcoord");
		//tex2d_shader->make_active();

		poly_shader.reset(new shader::program_object("poly_shader_2d",
			shader::shader(GL_VERTEX_SHADER, "poly_2d_vert", sys::read_file("data/simple_poly.vert")),
			shader::shader(GL_FRAGMENT_SHADER, "poly_2d_frag", sys::read_file("data/simple_poly.frag"))));
		poly_u_color_it = poly_shader->get_uniform_iterator("u_color");
		poly_a_position_it = poly_shader->get_attribute_iterator("a_position");
		
		generic_vbo.reset(new GLuint[num_generic_vbo], vbo_deleter());
		glGenBuffers(num_generic_vbo, generic_vbo.get());
	}

	render::~render()
	{
	}

	namespace
	{
		typedef std::map<std::string, shader::program_object_ptr> shader_map;
		shader_map& get_shader_map()
		{
			static shader_map res;
			return res;
		}
	}

	shader::program_object_ptr render::create_shader(const std::string& name, 
		const std::string& vname, 
		const std::string& vfname, 
		const std::string& fname,
		const std::string& ffname)
	{
		auto it = get_shader_map().find(name);
		if(it == get_shader_map().end()) {
			shader::program_object_ptr new_shader(new shader::program_object(name,
				shader::shader(GL_VERTEX_SHADER, vname, sys::read_file(vfname)),
				shader::shader(GL_FRAGMENT_SHADER, fname, sys::read_file(ffname))));
			get_shader_map()[name] = new_shader;

			cube_shader_object cso;
			cso.cube_.reset(new cube(new_shader));
			cso.vm_uniform_it = new_shader->get_uniform_iterator("view_matrix");
			cso.pm_uniform_it = new_shader->get_uniform_iterator("projection_matrix");

			new_shader->make_active();
			new_shader->set_uniform(cso.vm_uniform_it, view());
			new_shader->set_uniform(cso.pm_uniform_it, projection());

			cube_shader_map_[new_shader] = cso;
			return new_shader;
		}
		return it->second;
	}

	void render::add_cube(shader::program_object_ptr shader, cube_model_ptr obj)
	{
		auto it = cube_shader_map_.find(shader);
		ASSERT_LOG(it != cube_shader_map_.end(), "render::add_cube() was passed a shader object not created by us.");
		it->second.cube_draw_list_.push_back(obj);
	}

	void render::draw()
	{
		//profile::manager manager("render::draw()");
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		for(auto it = cube_shader_map_.begin(); it != cube_shader_map_.end(); ++it) {
			if(it->second.cube_draw_list_.size() != 0) {
				it->first->make_active();
				it->first->set_uniform(it->second.vm_uniform_it, view());
				it->first->set_uniform(it->second.pm_uniform_it, projection());
				for(auto obj = it->second.cube_draw_list_.begin(); obj != it->second.cube_draw_list_.end(); ++obj) {
					it->second.cube_->draw(*obj);
				}
			}
		}
		
		//draw_rect(rect(-0.5f, -0.5f, 1.0f, 1.0f), color(0, 255, 0));
	}

	void render::set_view(float fov, const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up)
	{
		view_ = glm::lookAt(position, position+direction, up);
		projection_ = glm::perspective(fov, float(width_)/float(height_), 0.1f, 100.0f);
	}

	void render::draw_rect(const rect& r, const color& c)
	{
		GLfloat vc_array[] = {
			r.xf(), r.yf(),
			r.xf()+r.wf(), r.yf(),
			r.xf(), r.yf()+r.hf(),
			r.xf()+r.wf(), r.yf()+r.hf(),
		};
		poly_shader->make_active();
		poly_shader->set_uniform(poly_u_color_it, c.as_gl_color());

		glEnableVertexAttribArray(poly_a_position_it->second.location);
		glBindBuffer(GL_ARRAY_BUFFER, generic_vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vc_array), vc_array, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(poly_a_position_it->second.location, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(poly_a_position_it->second.location);
	}

	void render::blit_2d_texture(const_texture_ptr tex, GLfloat x, GLfloat y)
	{
		GLfloat w = 2.0f*tex->width()/width_;
		GLfloat h = 2.0f*tex->height()/height_;
		static const GLfloat tc_array[] = {
			0.0f, 1.0f,
			1.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f,
		};
		GLfloat vc_array[] = {
			x, y,
			x+w, y,
			x, y+h,
			x+w, y+h,
		};
		tex2d_shader->make_active();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex->id());
		glUniform1i(tex2d_u_texmap_it->second.location, 0);

		glEnableVertexAttribArray(tex2d_a_position_it->second.location);
		glBindBuffer(GL_ARRAY_BUFFER, generic_vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vc_array), vc_array, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			tex2d_a_position_it->second.location, // The attribute we want to configure
			2,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			0					// array buffer offset
		);

		glEnableVertexAttribArray(tex2d_a_texcoord_it->second.location);
		glBindBuffer(GL_ARRAY_BUFFER, generic_vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tc_array), tc_array, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(
			tex2d_a_texcoord_it->second.location, // The attribute we want to configure
			2,                            // size : U+V => 2
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			0							  // array buffer offset
		);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(tex2d_a_position_it->second.location);
		glDisableVertexAttribArray(tex2d_a_texcoord_it->second.location);
	}
}
