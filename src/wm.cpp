#include <sstream>
#include <vector>
#include "graphics.hpp"
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

		window_ = SDL_CreateWindow(title.c_str(), x, y, w, h, flags);
		if(!window_) {
			std::stringstream ss;
			ss << "Could not create window: " << SDL_GetError() << std::endl;
			throw init_error(ss.str());
		}
		width_ = w;
		height_ = h;

		projection_ = perspective(45.0f, float(w)/float(h), 0.1f, 100.0f);
		//projection_ = mat4::identity();
		model_ = mat4::identity();
		view_ = look_at(vec3(4.0f,3.0f,-3.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		//view_ = look_at(vec3(0.0f,0.0f,5.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	}

	void window_manager::gl_init()
	{
		glcontext_ = SDL_GL_CreateContext(window_);

		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(0.1f, 0.1f, 0.0f, 0.1f);

		glViewport(0, 0, GLsizei(width_), GLsizei(height_));

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		// Enable depth test
		//glEnable(GL_DEPTH_TEST);
		
		// Accept fragment if it closer to the camera than the former one
		//glDepthFunc(GL_LESS);
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
