#include <iostream>
#include <sstream>
#include <vector>
#include "graphics.hpp"
#include "profile_timer.hpp"
#include "wm.hpp"

namespace graphics
{
	namespace 
	{
		std::vector<window_manager>& get_windows()
		{
			static std::vector<window_manager> res;
			return res;
		}
	}

	window_manager::window_manager()
	{
	}
		
	void window_manager::create_window(const std::string& title, int x, int y, int w, int h, Uint32 flags)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		profile::manager prof("SDL_CreateWindow");
		window_ = SDL_CreateWindow(title.c_str(), x, y, w, h, flags);
		if(!window_) {
			std::stringstream ss;
			ss << "Could not create window: " << SDL_GetError() << std::endl;
			throw init_error(ss.str());
		}
		width_ = w;
		height_ = h;
	}

	void window_manager::set_icon(const std::string& icon)
	{
		SDL_Surface* img = IMG_Load(icon.c_str());
		if(img) {
			SDL_SetWindowIcon(window_, img);
			SDL_FreeSurface(img);
		} else {
			std::cerr << "Unable to load icon: " << icon << std::endl;
		}
	}

	void window_manager::gl_init()
	{
		profile::manager prof("SDL_GL_CreateContext");
		glcontext_ = SDL_GL_CreateContext(window_);

		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		glViewport(0, 0, GLsizei(width_), GLsizei(height_));

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		// Enable depth test
		glEnable(GL_DEPTH_TEST);
		// Accept fragment if it closer to the camera than the former one
		glDepthFunc(GL_LESS); 
		// Cull triangles which normal is not towards the camera
		glEnable(GL_CULL_FACE);
	}

	void window_manager::swap() 
	{
		SDL_GL_SwapWindow(window_);
	}

	window_manager::~window_manager()
	{
		SDL_GL_DeleteContext(glcontext_);
		SDL_DestroyWindow(window_);
	}
}
