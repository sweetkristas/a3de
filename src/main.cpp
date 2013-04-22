#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

#include "btinterface.hpp"
#include "filesystem.hpp"
#include "fonts.hpp"
#include "geometry.hpp"
#include "json.hpp"
#include "module.hpp"
#include "notify.hpp"
#include "obj_reader.hpp"
#include "profile_timer.hpp"
#include "render.hpp"
#include "render_text.hpp"
#include "shaders.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include "unit_test.hpp"
#include "wm.hpp"


// Approximate delay between frames.
#define FRAME_RATE	1000 / 60

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

namespace 
{
	glm::vec3 position = glm::vec3(4.0f,3.0f, 20.0f); 
	// Initial horizontal angle : toward -Z
	float horizontal_angle = float(M_PI);
	// Initial vertical angle : none
	float vertical_angle = 0.0f;
	// Initial Field of View
	float initial_fov = 45.0f;
	float speed = 0.1f; // 3 units / second
	float mouse_speed = 0.005f;
}

bool process_events(graphics::render& render_obj)
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_t = float(SDL_GetTicks() - last_time);

	int rmx, rmy;
	SDL_GetRelativeMouseState(&rmx, &rmy);

	horizontal_angle += mouse_speed * rmx;
	vertical_angle   += mouse_speed * rmy;

	glm::vec3 direction(
		cos(vertical_angle) * sin(horizontal_angle), 
		sin(vertical_angle),
		cos(vertical_angle) * cos(horizontal_angle)
	);
	glm::vec3 right = glm::vec3(
		sin(horizontal_angle - float(M_PI)/2.0f), 
		0,
		cos(horizontal_angle - float(M_PI)/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross(right, direction);

	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		switch(e.type) {
		case SDL_MOUSEWHEEL:
			initial_fov = std::max<float>(15.0, std::min<float>(90.0, initial_fov + e.wheel.y * 3.0f));
			break;
		case SDL_QUIT:
			return false;
		case SDL_KEYDOWN:
			if(e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				return false;
			} else if(e.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
				position += right * delta_t * speed;
			} else if(e.key.keysym.scancode == SDL_SCANCODE_LEFT) {
				position -= right * delta_t * speed;
			} else if(e.key.keysym.scancode == SDL_SCANCODE_DOWN) {
				position -= direction * delta_t * speed;
			} else if(e.key.keysym.scancode == SDL_SCANCODE_UP) {
				position += direction * delta_t * speed;
			}
			break;
		}
	}

	render_obj.set_view(initial_fov, position, direction, up);
	last_time =  SDL_GetTicks();
	return true;
}

int phys_sim(const node::node& world)
{
	bullet::manager bman(world);
 
	for (int i=0 ; i<300 ; i++) {
		//profile::manager pman("simstep");
		bman.step();
 
		btTransform trans;
		bman.get_rigid_body(1)->getMotionState()->getWorldTransform(trans);
 
		std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
	}

	return 0;
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
		node::node world = json::parse_from_file(module::map_file("data/world.cfg"));
		//phys_sim(world);
	} catch(json::parse_error& e) {
		std::cerr << "Parse Error: " << e.what() << std::endl;
	}

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

		SDL_SetRelativeMouseMode(SDL_TRUE);

		font::manager font_manager;

		graphics::render render_obj(wm, window_size.x, window_size.y);
		auto shader = render_obj.create_shader("simple", 
			"simple_vertex", "data/simple_color.vert", 
			"simple_fragment", "data/simple_color.frag");

		const int num_cubes = 32;
		std::vector<std::vector<std::vector<graphics::cube_model_ptr> > > chunk;
		chunk.resize(num_cubes);
		for(size_t n = 0; n != chunk.size(); ++n) {
			chunk[n].resize(num_cubes);
			for(size_t m = 0; m != chunk[n].size(); ++m) {
				chunk[n][m].resize(num_cubes);
				for(size_t p = 0; p != chunk[n][m].size(); ++p) {
					chunk[n][m][p] = graphics::cube_model_ptr(new graphics::cube_model("images/uvtemplate.png"));
					chunk[n][m][p]->translate(1.0f*n, 1.0f*m, 1.0f*p);
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
		}
		
		notify::manager notifications;

		bool running = true;
		Uint32 start_time = SDL_GetTicks();
		uint64_t render_acc = 0;
		int render_cnt = 0;

		while(running) {
			profile::timer ptimer;

			Uint32 cycle_start_tick = SDL_GetTicks();

			notifications.poll();

			running = process_events(render_obj);

			double frame_processing_time = ptimer.elapsed_time_microseconds();
			render_obj.draw();
			double frame_render_time = ptimer.elapsed_time_microseconds() - frame_processing_time;

			std::stringstream ss1, ss2;
			ss1 << "Frame draw time (uS): " << std::fixed << frame_render_time;
			graphics::renderer::text::quick_draw(render_obj, -1.0f, -1.0f, ss1.str(), "Tauri-Regular.ttf", 14, graphics::color(1.0f, 1.0f, 0.5f));
			ss2 << "Frame process time (uS): " << std::fixed << (frame_processing_time+frame_render_time);
			graphics::renderer::text::quick_draw(render_obj, 0.0f, -1.0f, ss2.str(), "Tauri-Regular.ttf", 14, graphics::color(1.0f, 1.0f, 0.5f));
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
		}

		return 0;
	} catch(std::exception& e) {
		std::cerr << e.what();
	}
	return 1;
}

