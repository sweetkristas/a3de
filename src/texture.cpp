#include <map>

#include "asserts.hpp"
#include "profile_timer.hpp"
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

	void texture::load_file_into_texture(const std::string& fname, texture* tex)
	{
		SDL_Surface* source = IMG_Load(fname.c_str());
		ASSERT_LOG(source != NULL, "Failed to load image: " << fname << " : " << IMG_GetError());
		texture_from_surface(source, tex);
	}

	void texture::texture_from_surface(SDL_Surface* source, texture* tex)
	{
		SDL_SetSurfaceBlendMode(source, SDL_BLENDMODE_NONE);

		tex->width_ = GLfloat(power_of_two(source->w));
		tex->height_ = GLfloat(power_of_two(source->h));

		SDL_Surface *image = SDL_CreateRGBSurface(0, int(tex->width_), int(tex->height_), 32, SURFACE_MASK);
		ASSERT_LOG(image != NULL, "Couldn't create a temporary surface.");

		SDL_Rect area;
		area.x = 0;
		area.y = 0;
		area.w = source->w;
		area.h = source->h;
		if(tex->flags_ & SCALE_IMAGE_TO_TEXTURE) {
			SDL_Rect image_area;
			image_area.x = 0;
			image_area.y = 0;
			image_area.w = int(tex->width_);
			image_area.h = int(tex->height_);
			SDL_BlitSurface(source, &area, image, &image_area);
			tex->ratio_w_ = 1.0;
			tex->ratio_h_ = 1.0;
		} else {
			SDL_BlitSurface(source, &area, image, &area);
			tex->ratio_w_ = source->w / tex->width_;
			tex->ratio_h_ = source->h / tex->height_;
		}

		glGenTextures(1, &tex->tex_id_);
		glBindTexture(GL_TEXTURE_2D, tex->tex_id_);
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			int(tex->width_), int(tex->height_),
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			image->pixels);
		if(tex->flags_ & GENERATE_MIPMAP) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		SDL_FreeSurface(image);
	}

	texture::texture(const std::string& fname, unsigned tf)
		: tex_id_(0), name_(fname), flags_(tf)
	{
		load_file_into_texture(fname, this);
	}

	texture::texture(surface_ptr s, unsigned tf)
		: flags_(tf)
	{
		texture_from_surface(s.get(), this);
	}

	const_texture_ptr texture::get(const std::string& fname, unsigned tf)
	{
		if(tf & NO_CACHE) {
			return new texture(fname, tf);
		}
		auto it = texture_cache().find(fname);
		if(it == texture_cache().end()) {
			texture_ptr t = texture_ptr(new texture(fname, tf));
			texture_cache()[fname] = t;
			return t;
		}
		return it->second;
	}

	GLfloat texture::tc_x(GLfloat x) const
	{
		return ratio_w_ * x;
	}

	GLfloat texture::tc_y(GLfloat y) const
	{
		return ratio_h_ * y;
	}

	const_texture_ptr texture::get(surface_ptr s, unsigned tf)
	{
		return new texture(s, tf);
	}

	void texture::rebuild_cache()
	{
		for(auto it = texture_cache().begin(); it != texture_cache().end(); ++it) {
			load_file_into_texture(it->first, it->second.get());
		}
	}
}
