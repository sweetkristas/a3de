#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/shared_array.hpp>

#include "asserts.hpp"
#include "filesystem.hpp"
#include "graphics.hpp"
#include "profile_timer.hpp"
#include "render.hpp"


namespace graphics
{
	namespace
	{
		static const GLfloat cube_vertex_buffer_data[] = {
			-1.0f,-1.0f,-1.0f, // triangle 1 : begin
			-1.0f,-1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f, // triangle 1 : end
			1.0f, 1.0f,-1.0f, // triangle 2 : begin
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f,-1.0f, // triangle 2 : end
			1.0f,-1.0f, 1.0f, 
			-1.0f,-1.0f,-1.0f,
			1.0f,-1.0f,-1.0f, 
			1.0f, 1.0f,-1.0f, 
			1.0f,-1.0f,-1.0f, 
			-1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f,-1.0f,
			1.0f,-1.0f, 1.0f, 
			-1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f,-1.0f, 1.0f,
			1.0f,-1.0f, 1.0f, 
			1.0f, 1.0f, 1.0f, 
			1.0f,-1.0f,-1.0f, 
			1.0f, 1.0f,-1.0f, 
			1.0f,-1.0f,-1.0f, 
			1.0f, 1.0f, 1.0f, 
			1.0f,-1.0f, 1.0f, 
			1.0f, 1.0f, 1.0f, 
			1.0f, 1.0f,-1.0f, 
			-1.0f, 1.0f,-1.0f,
			1.0f, 1.0f, 1.0f, 
			-1.0f, 1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 
			-1.0f, 1.0f, 1.0f,
			1.0f,-1.0f, 1.0f,
		};

		static const GLfloat cube_uv_buffer_data[] = { 
			0.000000f, 1.000000f, 
			0.000000f, 0.666666f, 
			0.333333f, 0.666666f, 
			1.000000f, 1.000000f, 
			0.666666f, 0.666666f, 
			1.000000f, 0.666666f, 
			0.666666f, 0.666666f, 
			0.333333f, 0.333333f, 
			0.666666f, 0.333333f, 
			1.000000f, 1.000000f, 
			0.666666f, 1.000000f, 
			0.666666f, 0.666666f, 
			0.000000f, 1.000000f, 
			0.333333f, 0.666666f, 
			0.333333f, 1.000000f, 
			0.666666f, 0.666666f, 
			0.333333f, 0.666666f, 
			0.333333f, 0.333333f, 
			1.000000f, 0.333333f, 
			1.000000f, 0.666666f, 
			0.666666f, 0.666666f, 
			0.666666f, 1.000000f, 
			0.333333f, 0.666666f, 
			0.666666f, 0.666666f, 
			0.333333f, 0.666666f, 
			0.666666f, 1.000000f, 
			0.333333f, 1.000000f, 
			0.000000f, 0.666666f, 
			0.000000f, 0.333333f, 
			0.333333f, 0.333333f, 
			0.000000f, 0.666666f, 
			0.333333f, 0.333333f, 
			0.333333f, 0.666666f, 
			0.666666f, 0.333333f, 
			1.000000f, 0.333333f, 
			0.666666f, 0.666666f,
		};

		/*static const GLfloat cube_vertex_buffer_data[] = {
			// Front face
			-1,-1,1, 1,-1,1, -1,1,1, 1,1,1,
			// Right face
			1,1,1, 1,-1,1, 1,1,-1, 1,-1,-1,
			// Back face
			1,-1,-1, -1,-1,-1, 1,1,-1, -1,1,-1,
			// Left face
			-1,1,-1, -1,-1,-1, -1,1,1, -1,-1,1,
			// Bottom face
			-1,-1,1, -1,-1,-1, 1,-1,1, 1,-1,-1,
       
			// move to top
			1,-1,-1, -1,1,1,

			// Top Face
			-1,1,1, 1,1,1, -1,1,-1, 1,1,-1
		};

		static const GLfloat cube_uv_buffer_data[] = {
			// Front face
			0,0, 1,0, 0,1, 1,1,
			// Right face
			0,1, 0,0, 1,1, 1,0,
			// Back face
			0,0, 1,0, 0,1, 1,1,
			// Left face
			0,1, 0,0, 1,1, 1,0,
			// Bottom face
			0,1, 0,0, 1,1, 1,0,
       
			1,0, 0,0,
       
			// Top face
			0,0, 1,0, 0,1, 1,1
		};*/

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
				glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_buffer_data), cube_vertex_buffer_data, GL_STATIC_DRAW);
				glBindBuffer(GL_ARRAY_BUFFER, res[1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(cube_uv_buffer_data), cube_uv_buffer_data, GL_STATIC_DRAW);
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
			glBindBuffer(GL_ARRAY_BUFFER, array_buffers_[0]);
			glVertexAttribPointer(
				a_position_it_->second.location, // The attribute we want to configure
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			glEnableVertexAttribArray(a_tex_coord_it_->second.location);
			glBindBuffer(GL_ARRAY_BUFFER, array_buffers_[1]);
			glVertexAttribPointer(
				a_tex_coord_it_->second.location, // The attribute we want to configure
				2,                            // size : U+V => 2
				GL_FLOAT,                     // type
				GL_FALSE,                     // normalized?
				0,                            // stride
				(void*)0                      // array buffer offset
			);

			glDrawArrays(GL_TRIANGLES, 0, 12*3);
			//glDrawArrays(GL_TRIANGLES, 0, 26);
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

	bool cube_model::is_fully_occluded()
	{
		return pos_x_neighbour_ && neg_x_neighbour_ 
			&& pos_y_neighbour_ && neg_y_neighbour_ 
			&& pos_z_neighbour_ && neg_z_neighbour_;
	}

	render::render(graphics::window_manager& wm, int w, int h) 
			: wm_(wm), width_(w), height_(h)
	{
		eye_ = glm::vec3(10.0f,-10.0f,10.0f);
		view_ = glm::lookAt(eye_, 
			glm::vec3(0.0f, 0.0f, 0.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f));
		projection_ = glm::perspective(45.0f, float(w)/float(h), 0.1f, 100.0f);
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
		profile::manager manager("render::draw()");

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		for(auto it = cube_shader_map_.begin(); it != cube_shader_map_.end(); ++it) {
			if(it->second.cube_draw_list_.size() != 0) {
				it->first->make_active();
				for(auto obj = it->second.cube_draw_list_.begin(); obj != it->second.cube_draw_list_.end(); ++obj) {
					it->second.cube_->draw(*obj);
				}
			}
		}
	}

	void render::view_change(float dx, float dy, float dz)
	{
		eye_ += glm::vec3(dx, dy, dz);
		if(eye_.z > 10) {
			eye_.z = 10;
		}
		if(eye_.z < -10) {
			eye_.z = -10;
		}
		view_ = glm::lookAt(eye_, 
			glm::vec3(0.0f, 0.0f, 0.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f));

		for(auto it = cube_shader_map_.begin(); it != cube_shader_map_.end(); ++it) {
			it->first->set_uniform(it->second.vm_uniform_it, view());
		}
	}
}
