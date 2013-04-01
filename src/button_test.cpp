// button_test.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <sstream>
#include <vector>

#include "geometry.hpp"
#include "utils.hpp"
#include "unit_test.hpp"
#include "wm.hpp"

#include "stdafx.h"

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
		}
		explicit color(uint32_t rgba = 0)
		{
			c_.value = rgba;
		}
		explicit color(const SDL_Color& col)
		{
			c_.rgba[0] = col.r;
			c_.rgba[1] = col.g;
			c_.rgba[2] = col.b;
			c_.rgba[3] = 255;
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

	private:
		union PixelUnion 
		{
			uint32_t value;
			uint8_t rgba[4];
		};

		PixelUnion c_;
	};
}

void draw_rect(const rect& r, const graphics::color& color)
{
	GLfloat varray[] = {
		r.xf(), r.yf(),
		r.xf()+r.wf(), r.yf(),
		r.xf(), r.yf()+r.hf(),
		r.xf()+r.wf(), r.yf()+r.hf()
	};
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glColor4ub(color.r(),color.g(),color.b(),color.a());
	glVertexPointer(2, GL_FLOAT, 0, varray);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glColor4ub(255, 255, 255, 255);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
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

void render(int width, int height)
{
	glClearColor(0, 0, 0, 1);
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

	draw_rect(button_left, graphics::color(255,0,0));
	draw_rect(button_right, graphics::color(255,0,0));
	draw_rect(button_attack_toggle, graphics::color(128,0,0));
	draw_rect(button_attack_square, graphics::color(0,255,0));
	draw_rect(button_attack_arrow_up, graphics::color(0,128,0));
	draw_rect(button_attack_arrow_down, graphics::color(0,128,0));
	draw_rect(button_jump_circle, graphics::color(0,0,255));
	draw_rect(button_action_star, graphics::color(0,255,255));
	draw_rect(button_jumpdown_semicicle, graphics::color(255,255,0));
}

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

		SDL_Event e = {0};
		while(e.type != SDL_KEYDOWN && e.type != SDL_QUIT) {
			Uint32 cycle_start_tick = SDL_GetTicks();
			SDL_PollEvent(&e);
			render(window_size.x, window_size.y);
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

