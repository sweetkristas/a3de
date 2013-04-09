#include <map>

#include "asserts.hpp"
#include "texture.hpp"

namespace graphics
{
	namespace
	{
		static int power_of_two(int input)
		{
			int value = 1;

			while(value < input) {
				value <<= 1;
			}
			return value;
		}

		GLuint load_file_into_texture(const std::string& fname)
		{
			SDL_Surface* source = IMG_Load(fname.c_str());
			ASSERT_LOG(source != NULL, "Failed to load image: " << fname << " : " << IMG_GetError());

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
			SDL_FreeSurface(source);
			return t;
		}

		std::map<std::string, texture_ptr>& texture_cache()
		{
			static std::map<std::string, texture_ptr> res;
			return res;
		}
	}

	texture::texture()
		: tex_id_(0)
	{
	}

	texture::texture(const std::string& fname)
		: tex_id_(0), name_(fname)
	{
		tex_id_ = load_file_into_texture(fname);
	}

	const_texture_ptr texture::get(const std::string& fname)
	{
		auto it = texture_cache().find(fname);
		if(it == texture_cache().end()) {
			texture_ptr t = texture_ptr(new texture(fname));
			texture_cache()[fname] = t;
			return t;
		}
		return it->second;
	}

	void texture::rebuild_cache()
	{
		for(auto it = texture_cache().begin(); it != texture_cache().end(); ++it) {
			it->second->tex_id_ = load_file_into_texture(it->first);
		}
	}
}
