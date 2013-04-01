#include <sstream>
#include <vector>
#include <GLES2/gl2.h>
#include "wm.hpp"

#include "stdafx.h"

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

		window_ = SDL_CreateWindow(title.c_str(), x, y, w, h, flags);
		if(!window_) {
			std::stringstream ss;
			ss << "Could not create window: " << SDL_GetError() << std::endl;
			throw init_error(ss.str());
		}
		width_ = w;
		height_ = h;

		projection_ = mat4(
			2.0f / w,   0.0f,       0.0f, -1.0f,
			    0.0f, 2.0f/h,       0.0f, -1.0f,
				0.0f, 0.0f, 2.0f/2000.0f, -1.0f,
				0.0f, 0.0f,         0.0f,  1.0f
			);
		model_ = mat4::identity();
		view_ = mat4::identity();
	}

	void window_manager::gl_init()
	{
		glcontext_ = SDL_GL_CreateContext(window_);

		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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
