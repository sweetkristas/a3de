#pragma once

#include "graphics.hpp"

namespace graphics
{
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
			initialised_ = true;
		}

		virtual ~SDL()
		{
			SDL_Quit();
		}
	private:
		bool initialised_;
	};

	class window_manager
	{
	public:
		window_manager();
		void create_window(const std::string& title, int x, int y, int w, int h, Uint32 flags);
		void gl_init();
		void set_icon(const std::string& icon);
		void swap();
		virtual ~window_manager();	
	private:
		SDL_Window* window_;
		SDL_GLContext glcontext_;
		int width_;
		int height_;
	};
}
