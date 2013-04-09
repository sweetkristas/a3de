#pragma once

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

	class texture : public reference_counted_ptr
	{
	public:
		GLuint id() const { return tex_id_; }

		static const_texture_ptr get(const std::string& fname);
		static void rebuild_cache();
	protected:
		texture();
		explicit texture(const std::string& fname);
	private:
		std::string name_;
		GLuint tex_id_;
	};
}
