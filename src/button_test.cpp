// button_test.cpp : Defines the entry point for the application.
//

#include "SDL.h"
#define NO_SDL_GLEXT
#include "SDL_opengl.h"
//#include <GL/glext.h>
#include <iostream>
#include <sstream>
#include <vector>

// Approximate delay between frames.
#define FRAME_RATE	1000 / 60

template<class T> std::vector<T> split(const T& str, const T& delimiters) 
{
    std::vector<T> v;
    typename T::size_type start = 0;
    auto pos = str.find_first_of(delimiters, start);
    while(pos != T::npos) {
        if(pos != start) // ignore empty tokens
            v.emplace_back(str, start, pos - start);
        start = pos + 1;
        pos = str.find_first_of(delimiters, start);
    }
    if(start < str.length()) // ignore trailing delimiter
        v.emplace_back(str, start, str.length() - start); // add what's left of the string
    return v;
}

class init_error : public std::exception
{
public:
	init_error() : exception(), msg_(SDL_GetError())
	{}
	init_error(const std::string& msg) : exception(), msg_(msg)
	{}
	virtual ~init_error() throw()
	{}
	virtual const char* what() const throw() { return msg_.c_str(); }
private:
	std::string msg_;
};

class SDL
{
public:
	SDL(Uint32 flags = 0)
	{
		if (SDL_Init(flags) < 0) {
			std::stringstream ss;
			ss << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
			throw init_error(ss.str());
		}
	}
	void create_window(const std::string& title, int x, int y, int w, int h, Uint32 flags)
	{
		window_ = SDL_CreateWindow(title.c_str(), x, y, w, h, flags);
		if(!window_) {
			std::stringstream ss;
			ss << "Could not create window: " << SDL_GetError() << std::endl;
			throw init_error(ss.str());
		}
		width_ = w;
		height_ = h;
	}
	void gl_init()
	{
		glcontext_ = SDL_GL_CreateContext(window_);
		glMatrixMode(GL_PROJECTION | GL_MODELVIEW);
		glLoadIdentity();
		glOrtho(0, width_, height_, 0, 0, 1);

		glShadeModel(GL_SMOOTH);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	void swap() 
	{
		SDL_GL_SwapWindow(window_);
	}
	virtual ~SDL()
	{
		SDL_GL_DeleteContext(glcontext_);
		SDL_DestroyWindow(window_);
		SDL_Quit();
	}
private:
	SDL_Window* window_;
	SDL_GLContext glcontext_;
	int width_;
	int height_;
};

struct point 
{
	explicit point(const std::string& str)
	{
		std::vector<std::string> buf = split<std::string>(str, ",");
		if(buf.size() != 2) {
			x = y = 0;
		} else {
			x = strtol(buf[0].c_str(), NULL, 10);
			y = strtol(buf[1].c_str(), NULL, 10);
		}
	}
	explicit point(int x=0, int y=0) : x(x), y(y)
	{}
	explicit point(const std::vector<int>& v)
	{
		if(v.empty()) {
			x = y = 0;
		} else if(v.size() == 1) {
			x = v[0];
			y = 0;
		} else {
			x = v[0];
			y = v[1];
		}
	}

	std::string to_string() const
	{
		std::stringstream ss;
		ss << x << "," << y;
		return ss.str();
	}

	union 
	{
		struct { int x, y; };
		int buf[2];
	};
};

class rect 
{
public:
	static rect from_coordinates(int x1, int y1, int x2, int y2);
	explicit rect(const std::string& str);
	explicit rect(int x=0, int y=0, int w=0, int h=0)
		: top_left_(std::min(x, x+w), std::min(y, y+h)),
		bottom_right_(std::max(x, x+w), std::max(y, y+h))
	{}
	explicit rect(const std::vector<int>& v)
	{
		switch(v.size()) {
		case 2:
			*this = rect::from_coordinates(v[0], v[1], v[0], v[1]);
			break;
		case 3:
			*this = rect::from_coordinates(v[0], v[1], v[2], v[1]);
			break;
		case 4:
			*this = rect::from_coordinates(v[0], v[1], v[2], v[3]);
			break;
		default:
			*this = rect();
			break;
		}
	}
	int x() const { return top_left_.x; }
	int y() const { return top_left_.y; }
	int x2() const { return bottom_right_.x; }
	int y2() const { return bottom_right_.y; }
	int w() const { return bottom_right_.x - top_left_.x; }
	int h() const { return bottom_right_.y - top_left_.y; }

	GLfloat xf() const { return GLfloat(top_left_.x); }
	GLfloat yf() const { return GLfloat(top_left_.y); }
	GLfloat x2f() const { return GLfloat(bottom_right_.x); }
	GLfloat y2f() const { return GLfloat(bottom_right_.y); }
	GLfloat wf() const { return GLfloat(bottom_right_.x - top_left_.x); }
	GLfloat hf() const { return GLfloat(bottom_right_.y - top_left_.y); }

	int mid_x() const { return (x() + x2())/2; }
	int mid_y() const { return (y() + y2())/2; }

	point& top_left() { return top_left_; }
	point& bottom_right() { return bottom_right_; }
	const point& top_left() const { return top_left_; }
	const point& bottom_right() const { return bottom_right_; }

	rect& operator+= (const point& p)
	{
		top_left_.x += p.x;
		top_left_.y += p.y;
		bottom_right_.x += p.x;
		bottom_right_.y += p.y;
		return *this;
	}

	std::string to_string() const
	{
		std::stringstream ss;
		ss << x() << "," << y() << "," << (x2()-1) << "," << (y2()-1);
		return ss.str();
	}

	SDL_Rect sdl_rect() const { SDL_Rect r = {x(), y(), w(), h()}; return r; }

	bool empty() const { return w() == 0 || h() == 0; }
private:
	point top_left_, bottom_right_;
};

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
		SDL sdl(SDL_INIT_VIDEO /*| SDL_INIT_AUDIO*/);
		sdl_gl_setup();
		sdl.create_window("Button position test", 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED, 
			window_size.x, 
			window_size.y, 
			SDL_WINDOW_OPENGL);
		sdl.gl_init();

		SDL_Event e = {0};
		while(e.type != SDL_KEYDOWN && e.type != SDL_QUIT) {
			Uint32 cycle_start_tick = SDL_GetTicks();
			SDL_PollEvent(&e);
			render(window_size.x, window_size.y);
			sdl.swap();
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

