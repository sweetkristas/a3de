// button_test.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <sstream>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "filesystem.hpp"
#include "geometry.hpp"
#include "obj_reader.hpp"
#include "profile_timer.hpp"
#include "shaders.hpp"
#include "utils.hpp"
#include "unit_test.hpp"
#include "wm.hpp"


// Approximate delay between frames.
#define FRAME_RATE	1000 / 60


uint8_t truncate_to_char(int value)
{
	if(value < 0) return 0;
	if(value > 255) return 255;
	return value;
}

namespace graphics
{
	class color
	{
	public:
		color( int r, int g, int b, int a=255)
		{
			c_.rgba[0] = truncate_to_char(r);
			c_.rgba[1] = truncate_to_char(g);
			c_.rgba[2] = truncate_to_char(b);
			c_.rgba[3] = truncate_to_char(a);
			set_float_values();
		}
		explicit color(uint32_t rgba = 0)
		{
			c_.value = rgba;
			set_float_values();
		}
		explicit color(const SDL_Color& col)
		{
			c_.rgba[0] = col.r;
			c_.rgba[1] = col.g;
			c_.rgba[2] = col.b;
			c_.rgba[3] = 255;
			set_float_values();
		}
		explicit color(float r, float g, float b, float a=1.0f)
		{
			rgbaf_[0] = clampf(r);
			rgbaf_[1] = clampf(g);
			rgbaf_[2] = clampf(b);
			rgbaf_[3] = clampf(a);
			c_.rgba[0] = int(rgbaf_[0]*255.0f);
			c_.rgba[1] = int(rgbaf_[1]*255.0f);
			c_.rgba[2] = int(rgbaf_[2]*255.0f);
			c_.rgba[3] = int(rgbaf_[3]*255.0f);
		}

		uint8_t r() const { return c_.rgba[0]; }
		uint8_t g() const { return c_.rgba[1]; }
		uint8_t b() const { return c_.rgba[2]; }
		uint8_t a() const { return c_.rgba[3]; }

		SDL_Color as_sdl_color() const 
		{
			SDL_Color c = { c_.rgba[0], c_.rgba[1], c_.rgba[2], 255 };
			return c;
		}

		const float* as_gl_color() const
		{
			return rgbaf_;
		}
		float* as_gl_color()
		{
			return rgbaf_;
		}

	private:
		void set_float_values()
		{
			rgbaf_[0] = c_.rgba[0] / 255.0f;
			rgbaf_[1] = c_.rgba[1] / 255.0f;
			rgbaf_[2] = c_.rgba[2] / 255.0f;
			rgbaf_[3] = c_.rgba[3] / 255.0f;
		}

		float clampf(float value)
		{
			if(value < 0.0f) return 0.0f;
			if(value > 1.0f) return 1.0f;
			return value;
		}

		union PixelUnion 
		{
			uint32_t value;
			uint8_t rgba[4];
		};
		float rgbaf_[4];

		PixelUnion c_;
	};
}

void draw_rect(const rect& r, GLint vertex_attribute_index)
{
	GLfloat varray[] = {
		r.xf(), r.yf(),
		r.xf()+r.wf(), r.yf(),
		r.xf(), r.yf()+r.hf(),
		r.xf()+r.wf(), r.yf()+r.hf()
	};
	glEnableVertexAttribArray(vertex_attribute_index);
	glVertexAttribPointer(vertex_attribute_index, 2, GL_FLOAT, 0, 0, varray);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void sdl_gl_setup()
{
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
}

namespace graphics
{
	#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	#define SURFACE_MASK 0xFF,0xFF00,0xFF0000,0xFF000000
	#define SURFACE_MASK_RGB 0xFF,0xFF00,0xFF0000,0x0
	#else
	#define SURFACE_MASK 0xFF000000,0xFF0000,0xFF00,0xFF
	#define SURFACE_MASK_RGB 0xFF0000,0xFF00,0xFF,0x0
	#endif

	static int power_of_two(int input)
	{
		int value = 1;

		while ( value < input ) {
			value <<= 1;
		}
		return value;
	}

	GLuint ConvertSDLSurfaceToTexture(SDL_Surface *source)
	{
		SDL_SetSurfaceBlendMode(source, SDL_BLENDMODE_NONE);
		int w = power_of_two(source->w);
		int h = power_of_two(source->h);
		SDL_Surface *image = SDL_CreateRGBSurface(0, w, h, 32, SURFACE_MASK);
		ASSERT_LOG(image != NULL, "Couldn't create a temporary surface.");

		SDL_Rect area;
		area.x = 0;
		area.y = 0;
		area.w = source->w;
		area.h = source->h;
		SDL_BlitSurface(source, &area, image, &area);

		GLuint t;
		glGenTextures(1, &t);
		glBindTexture(GL_TEXTURE_2D, t);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			w, h,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			image->pixels);
		glGenerateMipmap(GL_TEXTURE_2D);
		SDL_FreeSurface(image);
		return t;
	}

	class cube : public reference_counted_ptr
	{
	public:
		cube(shader::program_object_ptr shader)
			: shader_(shader)
		{
			model_ = mat4::identity();
			mm_uniform_it_ = shader->get_uniform_iterator("model_matrix");
			a_position_it_ = shader->get_attribute_iterator("a_position");
			a_tex_coord_it_ = shader->get_attribute_iterator("a_tex_coord");
			tex0_it_ = shader->get_uniform_iterator("u_tex0");

			glGenBuffers(1, &vertex_array_);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_array_);
			static const GLfloat g_vertex_buffer_data[] = {
				-1.0f,-1.0f,-1.0f, 1.0f, // triangle 1 : begin
				-1.0f,-1.0f, 1.0f, 1.0f, 
				-1.0f, 1.0f, 1.0f, 1.0f, // triangle 1 : end
				1.0f, 1.0f,-1.0f, 1.0f, // triangle 2 : begin
				-1.0f,-1.0f,-1.0f, 1.0f, 
				-1.0f, 1.0f,-1.0f, 1.0f, // triangle 2 : end
				1.0f,-1.0f, 1.0f, 1.0f, 
				-1.0f,-1.0f,-1.0f, 1.0f, 
				1.0f,-1.0f,-1.0f, 1.0f, 
				1.0f, 1.0f,-1.0f, 1.0f, 
				1.0f,-1.0f,-1.0f, 1.0f, 
				-1.0f,-1.0f,-1.0f, 1.0f, 
				-1.0f,-1.0f,-1.0f, 1.0f, 
				-1.0f, 1.0f, 1.0f, 1.0f, 
				-1.0f, 1.0f,-1.0f, 1.0f, 
				1.0f,-1.0f, 1.0f, 1.0f, 
				-1.0f,-1.0f, 1.0f, 1.0f, 
				-1.0f,-1.0f,-1.0f, 1.0f, 
				-1.0f, 1.0f, 1.0f, 1.0f, 
				-1.0f,-1.0f, 1.0f, 1.0f, 
				1.0f,-1.0f, 1.0f, 1.0f, 
				1.0f, 1.0f, 1.0f, 1.0f, 
				1.0f,-1.0f,-1.0f, 1.0f, 
				1.0f, 1.0f,-1.0f, 1.0f, 
				1.0f,-1.0f,-1.0f, 1.0f, 
				1.0f, 1.0f, 1.0f, 1.0f, 
				1.0f,-1.0f, 1.0f, 1.0f, 
				1.0f, 1.0f, 1.0f, 1.0f, 
				1.0f, 1.0f,-1.0f, 1.0f, 
				-1.0f, 1.0f,-1.0f, 1.0f, 
				1.0f, 1.0f, 1.0f, 1.0f, 
				-1.0f, 1.0f,-1.0f, 1.0f, 
				-1.0f, 1.0f, 1.0f, 1.0f, 
				1.0f, 1.0f, 1.0f, 1.0f, 
				-1.0f, 1.0f, 1.0f, 1.0f, 
				1.0f,-1.0f, 1.0f, 1.0f
			};
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);


			static const GLfloat g_uv_buffer_data[] = { 
				0.000059f, 1.0f-0.000004f, 
				0.000103f, 1.0f-0.336048f, 
				0.335973f, 1.0f-0.335903f, 
				1.000023f, 1.0f-0.000013f, 
				0.667979f, 1.0f-0.335851f, 
				0.999958f, 1.0f-0.336064f, 
				0.667979f, 1.0f-0.335851f, 
				0.336024f, 1.0f-0.671877f, 
				0.667969f, 1.0f-0.671889f, 
				1.000023f, 1.0f-0.000013f, 
				0.668104f, 1.0f-0.000013f, 
				0.667979f, 1.0f-0.335851f, 
				0.000059f, 1.0f-0.000004f, 
				0.335973f, 1.0f-0.335903f, 
				0.336098f, 1.0f-0.000071f, 
				0.667979f, 1.0f-0.335851f, 
				0.335973f, 1.0f-0.335903f, 
				0.336024f, 1.0f-0.671877f, 
				1.000004f, 1.0f-0.671847f, 
				0.999958f, 1.0f-0.336064f, 
				0.667979f, 1.0f-0.335851f, 
				0.668104f, 1.0f-0.000013f, 
				0.335973f, 1.0f-0.335903f, 
				0.667979f, 1.0f-0.335851f, 
				0.335973f, 1.0f-0.335903f, 
				0.668104f, 1.0f-0.000013f, 
				0.336098f, 1.0f-0.000071f, 
				0.000103f, 1.0f-0.336048f, 
				0.000004f, 1.0f-0.671870f, 
				0.336024f, 1.0f-0.671877f, 
				0.000103f, 1.0f-0.336048f, 
				0.336024f, 1.0f-0.671877f, 
				0.335973f, 1.0f-0.335903f, 
				0.667969f, 1.0f-0.671889f, 
				1.000004f, 1.0f-0.671847f, 
				0.667979f, 1.0f-0.335851f
			};

			glGenBuffers(1, &uvbuffer_);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_);
			glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

			SDL_Surface* surf = IMG_Load("images/uvtemplate.png");
			ASSERT_LOG(surf != NULL, "Failed to load image: images/uvtemplate.png " << IMG_GetError());
			tex_id_ = ConvertSDLSurfaceToTexture(surf);
			SDL_FreeSurface(surf);
		}

		virtual ~cube()
		{
			glDeleteBuffers(1, &vertex_array_);
			glDeleteBuffers(1, &uvbuffer_);
			glDeleteTextures(1, &tex_id_);
		}

		void draw(GLuint vbos[2], int n) const
		{
			shader_->set_uniform(mm_uniform_it_, model_.get());

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_id_);
			glUniform1i(tex0_it_->second.location, 0);

			glEnableVertexAttribArray(a_position_it_->second.location);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_array_);
			glVertexAttribPointer(
				a_position_it_->second.location, // The attribute we want to configure
				4,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);
			glDrawArrays(GL_TRIANGLES, 0, 12*3);
			glDisableVertexAttribArray(0);

			//glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
			//glEnableVertexAttribArray(a_position_it_->second.location);
			//glVertexAttribPointer(a_position_it_->second.location,
			//	4,
			//	GL_FLOAT,
			//	GL_FALSE,
			//	0,
			//	0);
			//glDrawElements(GL_TRIANGLES, n, GL_UNSIGNED_SHORT, 0);			
			//glDisableVertexAttribArray(a_position_it_->second.location);
		}
	protected:
	private:
		mat4 model_;
		shader::program_object_ptr shader_;
		shader::const_actives_map_iterator mm_uniform_it_;
		shader::const_actives_map_iterator a_position_it_;
		shader::const_actives_map_iterator a_tex_coord_it_;
		shader::const_actives_map_iterator tex0_it_;
		//shader::const_actives_map_iterator a_color_it_;
		//shader::const_actives_map_iterator u_color_it_;

		GLuint vertex_array_;
		GLuint uvbuffer_;
		GLuint tex_id_;
	};

	typedef boost::intrusive_ptr<cube> cube_ptr;
	typedef boost::intrusive_ptr<const cube> const_cube_ptr;

	class render
	{
	public:
		render(graphics::window_manager& wm, int w, int h) 
			: wm_(wm), width_(w), height_(h)
		{
			obj::load_obj_file("data/test/cube.obj", cube_);

			eye_ = vec3(4.0f,3.0f,10.0f);

			//projection_ = perspective(45.0f, float(w)/float(h), 0.1f, 100.0f);
			projection_ = mat4::identity();
			view_ = look_at(eye_, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

			// Create vertex buffer object for vertex array.
			glGenBuffers(2, cube_vbos_);
			glBindBuffer(GL_ARRAY_BUFFER, cube_vbos_[0]);
			glBufferData(GL_ARRAY_BUFFER, 
				cube_.vertices.size()*sizeof(vec4), 
				&cube_.vertices[0], 
				GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_vbos_[1]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
				cube_.face_vertex_index.size()*sizeof(GLushort), 
				&cube_.face_vertex_index[0], 
				GL_STATIC_DRAW);
		}

		virtual ~render()
		{
			glDeleteBuffers(2, cube_vbos_);
		}

		shader::program_object_ptr create_shader(const std::string& name, 
			const std::string& vname, 
			const std::string& vfname, 
			const std::string& fname,
			const std::string& ffname)
		{
			auto it = shaders_.find(name);
			if(it == shaders_.end()) {
				shader::program_object_ptr new_shader(new shader::program_object(name,
					shader::shader(GL_VERTEX_SHADER, vname, sys::read_file(vfname)),
					shader::shader(GL_FRAGMENT_SHADER, fname, sys::read_file(ffname))));
				shader_object so;
				so.shader = new_shader;
				so.vm_uniform_it = new_shader->get_uniform_iterator("view_matrix");
				so.pm_uniform_it = new_shader->get_uniform_iterator("projection_matrix");
				shaders_[name] = so;
				new_shader->make_active();
				new_shader->set_uniform(so.vm_uniform_it, view());
				new_shader->set_uniform(so.pm_uniform_it, projection());
				return new_shader;
			}
			return it->second.shader;
		}

		void add_object(shader::program_object_ptr shader, cube_ptr obj)
		{
			auto it = cube_draw_list_.find(shader);
			if(it == cube_draw_list_.end()) {
				std::vector<cube_ptr> v;
				v.push_back(obj);
				cube_draw_list_[shader] = v;
			} else {
				it->second.push_back(obj);
			}
		}

		void draw()
		{
			profile::manager manager("render::draw()");

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			for(auto it = cube_draw_list_.begin(); it != cube_draw_list_.end(); ++it) {
				if(it->second.size() != 0) {
					it->first->make_active();
					for(auto obj = it->second.begin(); obj != it->second.end(); ++obj) {
						(*obj)->draw(cube_vbos_, cube_.face_vertex_index.size());
					}
				}
			}
		}

		void view_change(float dx, float dy, float dz)
		{
			eye_ += vec3(dx, dy, dz);
			if(eye_.z > 10) {
				eye_.z = 10;
			}
			if(eye_.z < -10) {
				eye_.z = -10;
			}
			view_ = look_at(eye_, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

			for(auto it = shaders_.begin(); it != shaders_.end(); ++it) {
				it->second.shader->set_uniform(it->second.vm_uniform_it, view());
			}
		}

		const float* view() { return view_.get(); }
		const float* projection() { return projection_.get(); }
	protected:
	private:
		obj::obj_data cube_;
		GLuint cube_vbos_[2];

		int width_;
		int height_;

		mat4 view_;
		mat4 projection_;

		vec3 eye_;

		graphics::window_manager& wm_;

		struct shader_object
		{
			shader::program_object_ptr shader;
			shader::const_actives_map_iterator vm_uniform_it;
			shader::const_actives_map_iterator pm_uniform_it;
		};

		std::map<std::string, shader_object> shaders_;

		std::map<shader::program_object_ptr, std::vector<cube_ptr> > cube_draw_list_;
	};
}


/*void render(graphics::window_manager& wm, int width, int height)
{
	profile::manager manager("render");

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//draw_rect(rect(100,100,100,100), graphics::color(255,0,0));

	//int scale = 1;
	rect button_left = rect(0, 0, 79, 111);

	rect button_right = rect(0, 0, 79, 111);
	rect button_attack_toggle = rect(0, 0, 100, 100);
	rect button_attack_square = rect(0, 0, 95, 95);
	rect button_attack_arrow_up = rect(0, 0, 95, 63);
	rect button_attack_arrow_down = rect(0, 0, 95, 63);
	rect button_jump_circle = rect(0, 0, 95, 95);
	rect button_action_star = rect(0, 0, 95, 95);
	rect button_jumpdown_semicicle = rect(0, 0, 95, 63);

	button_left += point(48, height - 230);
	button_right += point(208, height - 230);

	button_attack_square += point(width - 188,  height - 276);
	button_attack_arrow_up += point(width - 190, height - 348);
	button_attack_arrow_down += point(width - 190, height - 142);

	button_jump_circle += point(width - 374, height - 210);
	button_action_star += point(width - 568, height - 210);
	button_jumpdown_semicicle += point(width - 374, height - 326);

	static shader::program_object_ptr simple_shader;
	static shader::const_actives_map_iterator color_attribute;
	static shader::const_actives_map_iterator a_position_it;
	static GLuint cube_array_vbo;
	if(simple_shader == NULL) {
		simple_shader.reset(new shader::program_object("simple", 
			shader::shader(GL_VERTEX_SHADER, "simple_vertex", sys::read_file("data/simple_color.vert")), 
			shader::shader(GL_FRAGMENT_SHADER, "simple_fragment", sys::read_file("data/simple_color.frag"))));
		color_attribute = simple_shader->get_attribute_iterator("a_color");
		a_position_it = simple_shader->get_attribute_iterator("a_position");

		simple_shader->make_active();

		shader::const_actives_map_iterator mm_uniform_it = simple_shader->get_uniform_iterator("model_matrix");
		shader::const_actives_map_iterator vm_uniform_it = simple_shader->get_uniform_iterator("view_matrix");
		shader::const_actives_map_iterator pm_uniform_it = simple_shader->get_uniform_iterator("projection_matrix");

		simple_shader->set_uniform(mm_uniform_it, wm.model());
		simple_shader->set_uniform(vm_uniform_it, wm.view());
		simple_shader->set_uniform(pm_uniform_it, wm.projection());

	}
	simple_shader->make_active();

	//float color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
	//simple_shader->set_uniform(color_uniform, color);

	const GLfloat g_color_buffer_data[] = { 
		0.583f,  0.771f,  0.014f,
		0.609f,  0.115f,  0.436f,
		0.327f,  0.483f,  0.844f,
		0.822f,  0.569f,  0.201f,
		0.435f,  0.602f,  0.223f,
		0.310f,  0.747f,  0.185f,
		0.597f,  0.770f,  0.761f,
		0.559f,  0.436f,  0.730f,
		0.359f,  0.583f,  0.152f,
		0.483f,  0.596f,  0.789f,
		0.559f,  0.861f,  0.639f,
		0.195f,  0.548f,  0.859f,
		0.014f,  0.184f,  0.576f,
		0.771f,  0.328f,  0.970f,
		0.406f,  0.615f,  0.116f,
		0.676f,  0.977f,  0.133f,
		0.971f,  0.572f,  0.833f,
		0.140f,  0.616f,  0.489f,
		0.997f,  0.513f,  0.064f,
		0.945f,  0.719f,  0.592f,
		0.543f,  0.021f,  0.978f,
		0.279f,  0.317f,  0.505f,
		0.167f,  0.620f,  0.077f,
		0.347f,  0.857f,  0.137f,
		0.055f,  0.953f,  0.042f,
		0.714f,  0.505f,  0.345f,
		0.783f,  0.290f,  0.734f,
		0.722f,  0.645f,  0.174f,
		0.302f,  0.455f,  0.848f,
		0.225f,  0.587f,  0.040f,
		0.517f,  0.713f,  0.338f,
		0.053f,  0.959f,  0.120f,
		0.393f,  0.621f,  0.362f,
		0.673f,  0.211f,  0.457f,
		0.820f,  0.883f,  0.371f,
		0.982f,  0.099f,  0.879f
	};
	//GLuint VertexArrayID;
	//glGenVertexArrays(1, &VertexArrayID);
	//glBindVertexArray(VertexArrayID);

	//GLuint vertexbuffer;
	//glGenBuffers(1, &vertexbuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(varray), varray, GL_STATIC_DRAW);

	glEnableVertexAttribArray(a_position_it->second.location);
	glEnableVertexAttribArray(color_attribute->second.location);

	//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		a_position_it->second.location,
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		varray				// array buffer offset
	);
	glVertexAttribPointer(
		color_attribute->second.location,
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		g_color_buffer_data				// array buffer offset
	);
	glDrawArrays(GL_TRIANGLES, 0, 12*3);
	glDisableVertexAttribArray(a_position_it->second.location);
	glDisableVertexAttribArray(color_attribute->second.location);
*/
	//glDeleteBuffers(1, &vertexbuffer);
	//glEnableVertexAttribArray(a_position_it->second.location);
	//glVertexAttribPointer(a_position_it->second.location, 2, GL_FLOAT, 0, 0, varray);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	/*simple_shader->set_uniform(color_uniform, graphics::color(255,0,0).as_gl_color());
	draw_rect(button_left, a_position_it->second.location);

	simple_shader->set_uniform(color_uniform, graphics::color(255,0,0).as_gl_color());
	draw_rect(button_right, a_position_it->second.location);

	simple_shader->set_uniform(color_uniform, graphics::color(128,0,0).as_gl_color());
	draw_rect(button_attack_toggle, a_position_it->second.location);

	simple_shader->set_uniform(color_uniform, graphics::color(0,255,0).as_gl_color());
	draw_rect(button_attack_square, a_position_it->second.location);

	simple_shader->set_uniform(color_uniform, graphics::color(0,128,0).as_gl_color());
	draw_rect(button_attack_arrow_up, a_position_it->second.location);

	simple_shader->set_uniform(color_uniform, graphics::color(0,128,0).as_gl_color());
	draw_rect(button_attack_arrow_down, a_position_it->second.location);

	simple_shader->set_uniform(color_uniform, graphics::color(0,0,255).as_gl_color());
	draw_rect(button_jump_circle, a_position_it->second.location);

	simple_shader->set_uniform(color_uniform, graphics::color(0,255,255).as_gl_color());
	draw_rect(button_action_star, a_position_it->second.location);

	simple_shader->set_uniform(color_uniform, graphics::color(255,255,0).as_gl_color());
	draw_rect(button_jumpdown_semicicle, a_position_it->second.location);
	*/

//}

int main(int argc, char* argv[]) 
{
	std::vector<std::string> args;
	for(int i = 0; i < argc; ++i) {
		args.push_back(argv[i]);
	}

	point window_size = point(800, 480);

	try {
		if(test::run_tests() == false) {
			return -1;
		}

		graphics::SDL sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		graphics::window_manager wm;
		sdl_gl_setup();
		wm.create_window("Button position test", 
			SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, 
			window_size.x, 
			window_size.y, 
			SDL_WINDOW_OPENGL);
		wm.gl_init();

		graphics::render render_obj(wm, window_size.x, window_size.y);
		auto shader = render_obj.create_shader("simple", 
			"simple_vertex", "data/simple_color.vert", 
			"simple_fragment", "data/simple_color.frag");
		render_obj.add_object(shader, graphics::cube_ptr(new graphics::cube(shader)));

		/*std::vector<std::vector<std::vector<uint8_t> > > chunk;
		chunk.resize(256);
		for(size_t n = 0; n != chunk.size(); ++n) {
			chunk[n].resize(256);
			for(size_t m = 0; m != chunk[n].size(); ++m) {
				chunk[n][m].resize(256);
			}
		}*/

		SDL_Event e = {0};
		bool running = true;
		while(running) {
			Uint32 cycle_start_tick = SDL_GetTicks();
			SDL_PollEvent(&e);
			switch(e.type) {
			case SDL_MOUSEWHEEL:
				render_obj.view_change(0.0f, 0.0f, float(e.wheel.y));
				break;
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				if(e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					running = false;
				} else if(e.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					render_obj.view_change(-0.5f, 0.0f, 0.0f);
				} else if(e.key.keysym.scancode == SDL_SCANCODE_LEFT) {
					render_obj.view_change(0.5f, 0.0f, 0.0f);
				} else if(e.key.keysym.scancode == SDL_SCANCODE_DOWN) {
					render_obj.view_change(0.0f, -0.5f, 0.0f);
				} else if(e.key.keysym.scancode == SDL_SCANCODE_UP) {
					render_obj.view_change(0.0f, 0.5f, 0.0f);
				}
				break;
			}

			render_obj.draw();
			wm.swap();
			Uint32 delay = SDL_GetTicks() - cycle_start_tick;
			if(delay > FRAME_RATE) {
				std::cerr << "CYCLE TOO LONG: " << delay << std::endl;
			} else {
				SDL_Delay(FRAME_RATE - delay);
			}
		}

		return 0;
	} catch(std::exception& e) {
		std::cerr << e.what();
	}
	return 1;
}

