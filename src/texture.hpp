#pragma once

#include <boost/shared_ptr.hpp>

#include "graphics.hpp"
#include "ref_counted_ptr.hpp"

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#	define SURFACE_MASK 0xFF,0xFF00,0xFF0000,0xFF000000
#	define SURFACE_MASK_RGB 0xFF,0xFF00,0xFF0000,0x0
#else
#	define SURFACE_MASK 0xFF000000,0xFF0000,0xFF00,0xFF
	#define SURFACE_MASK_RGB 0xFF0000,0xFF00,0xFF,0x0
#endif

namespace graphics
{
	class texture;
	typedef boost::intrusive_ptr<texture> texture_ptr;
	typedef boost::intrusive_ptr<const texture> const_texture_ptr;

	typedef boost::shared_ptr<SDL_Surface> surface_ptr;

	class texture : public reference_counted_ptr
	{
	public:
		enum
		{
			SCALE_IMAGE_TO_TEXTURE	= 1,
			GENERATE_MIPMAP			= 2,
			NO_CACHE				= 4,
		};
		GLuint id() const { return tex_id_; }

		GLfloat tc_x(GLfloat x) const;
		GLfloat tc_y(GLfloat y) const;

		GLfloat width() const { return width_; }
		GLfloat height() const { return height_; }

		static const_texture_ptr get(const std::string& fname, unsigned tf=SCALE_IMAGE_TO_TEXTURE|GENERATE_MIPMAP);
		static const_texture_ptr get(surface_ptr, unsigned tf=SCALE_IMAGE_TO_TEXTURE);
		static void rebuild_cache();
	protected:
		texture();
		explicit texture(const std::string& fname, unsigned tf);
		explicit texture(surface_ptr, unsigned tf);
	private:
		static void texture_from_surface(SDL_Surface* source, texture* tex);
		static void load_file_into_texture(const std::string& fname, texture* tex);

		std::string name_;
		GLuint tex_id_;
		GLfloat ratio_w_;
		GLfloat ratio_h_;
		unsigned flags_;
		GLfloat width_;
		GLfloat height_;
	};
}
