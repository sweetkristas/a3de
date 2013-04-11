#include <iostream>
#include <sstream>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "filesystem.hpp"
#include "fonts.hpp"
#include "geometry.hpp"
#include "module.hpp"
#include "notify.hpp"
#include "obj_reader.hpp"
#include "render.hpp"
#include "shaders.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include "unit_test.hpp"
#include "wm.hpp"


// Approximate delay between frames.
#define FRAME_RATE	1000 / 60


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
    //SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    //SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    //SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    //SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
}

void file_change(const std::string& file, const boost::asio::dir_monitor_event& ev)
{
	if(ev.type == boost::asio::dir_monitor_event::modified) {
		std::cerr << "FILE MODIFIED: " << file << std::endl;
	} else if(ev.type == boost::asio::dir_monitor_event::added) {
		std::cerr << "FILE ADDED: " << file << std::endl;
	} else if(ev.type == boost::asio::dir_monitor_event::removed) {
		std::cerr << "FILE REMOVED: " << file << std::endl;
	}
}


int main(int argc, char* argv[]) 
{
	std::vector<std::string> args;
	for(int i = 0; i < argc; ++i) {
		args.push_back(argv[i]);
	}

	module::load_module("test");

	point window_size = point(1024, 768);

	try {
		if(test::run_tests() == false) {
			return -1;
		}

		graphics::SDL sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		graphics::window_manager wm;
		sdl_gl_setup();
		wm.create_window("a3de", 
			SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED, 
			window_size.x, 
			window_size.y, 
			SDL_WINDOW_OPENGL);
		wm.set_icon("images/icon.png");
		wm.gl_init();

		font::manager font_manager;

		graphics::render render_obj(wm, window_size.x, window_size.y);
		auto shader = render_obj.create_shader("simple", 
			"simple_vertex", "data/simple_color.vert", 
			"simple_fragment", "data/simple_color.frag");

		/*std::vector<std::vector<std::vector<graphics::cube_model_ptr> > > chunk;
		chunk.resize(4);
		for(size_t n = 0; n != chunk.size(); ++n) {
			chunk[n].resize(4);
			for(size_t m = 0; m != chunk[n].size(); ++m) {
				chunk[n][m].resize(4);
				for(size_t p = 0; p != chunk[n][m].size(); ++p) {
					chunk[n][m][p] = graphics::cube_model_ptr(new graphics::cube_model("images/test_image_32x32.png"));
					chunk[n][m][p]->translate(2.0f*n, -2.0f*p, 2.0f*m);
					chunk[n][m][p]->set_neighbourhood(
						n != chunk.size()-1,
						n != 0,
						m != chunk[n].size()-1,
						m != 0,
						p != chunk[n][m].size()-1,
						p != 0);
					render_obj.add_cube(shader, chunk[n][m][p]);
				}
			}
		}*/
		

		notify::manager notifications;

		SDL_Event e = {0};
		bool running = true;
		Uint32 start_time = SDL_GetTicks();
		uint64_t render_acc = 0;
		int render_cnt = 0;
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

			render_acc += delay;
			render_cnt++;
			Uint32 current_time = SDL_GetTicks();
			if(current_time - start_time >= 1000) {
				std::cerr << "Average processing time: " << double(render_acc)/double(render_cnt) << " ms" << std::endl;

				start_time = current_time;
				render_cnt = 0;
				render_acc = 0;
			}

			if(delay > FRAME_RATE) {
				std::cerr << "CYCLE TOO LONG: " << delay << std::endl;
			} else {
				SDL_Delay(FRAME_RATE - delay);
			}

			notifications.poll();
		}

		return 0;
	} catch(std::exception& e) {
		std::cerr << e.what();
	}
	return 1;
}

