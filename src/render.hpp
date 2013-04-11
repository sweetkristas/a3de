#pragma once

#include <boost/shared_ptr.hpp>
#include <glm/glm.hpp>

#include "color.hpp"
#include "geometry.hpp"
#include "ref_counted_ptr.hpp"
#include "shaders.hpp"
#include "texture.hpp"
#include "wm.hpp"

namespace graphics
{
	class cube;

	class cube_model : public reference_counted_ptr
	{
	public:
		cube_model();
		explicit cube_model(const std::string& texname);
		virtual ~cube_model();
		void translate(float dx, float dy, float dz);
		void rotate(float angle, const glm::vec3& axis);
		glm::mat4& model() const;
		GLuint tex_id() const;
		void set_neighbourhood(int px, int nx, int py, int ny, int pz, int nz);
		bool is_fully_occluded();
	private:
		mutable glm::mat4 model_;
		const_texture_ptr tex_;
		uint8_t pos_x_neighbour_ : 1;
		uint8_t neg_x_neighbour_ : 1;
		uint8_t pos_y_neighbour_ : 1;
		uint8_t neg_y_neighbour_ : 1;
		uint8_t pos_z_neighbour_ : 1;
		uint8_t neg_z_neighbour_ : 1;
	};
	typedef boost::intrusive_ptr<cube_model> cube_model_ptr;
	typedef boost::intrusive_ptr<const cube_model> const_cube_model_ptr;

	class render
	{
	public:
		render(graphics::window_manager& wm, int w, int h);
		virtual ~render();

		shader::program_object_ptr create_shader(const std::string& name, 
			const std::string& vname, 
			const std::string& vfname, 
			const std::string& fname,
			const std::string& ffname);

		void add_cube(shader::program_object_ptr shader, cube_model_ptr obj);
		void draw();
		void view_change(float dx, float dy, float dz);
		const float* view() { return &view_[0][0]; }
		const float* projection() { return &projection_[0][0]; }

		static void blit_2d_texture(const_texture_ptr tex, const rect& r, const color& c);
		//static void blit_2d_texture(texture_ptr tex);
	protected:
	private:
		int width_;
		int height_;

		glm::mat4 view_;
		glm::mat4 projection_;

		glm::vec3 eye_;

		graphics::window_manager& wm_;

		struct cube_shader_object
		{
			boost::shared_ptr<cube> cube_;
			shader::const_actives_map_iterator vm_uniform_it;
			shader::const_actives_map_iterator pm_uniform_it;
			std::vector<cube_model_ptr> cube_draw_list_;
		};
		std::map<shader::program_object_ptr, cube_shader_object> cube_shader_map_;
	};
}
