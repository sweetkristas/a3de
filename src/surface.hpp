#pragma once

#include "graphics.hpp"
#include "ref_counted_ptr.hpp"

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#	define SURFACE_MASK 0xFF,0xFF00,0xFF0000,0xFF000000
#	define SURFACE_MASK_RGB 0xFF,0xFF00,0xFF0000,0x0
#	define SURFACE_MASK_R 0xff
#	define SURFACE_MASK_G 0xff00
#	define SURFACE_MASK_B 0xff0000
#	define SURFACE_MASK_A 0xff000000
#else
#	define SURFACE_MASK 0xFF000000,0xFF0000,0xFF00,0xFF
#	define SURFACE_MASK_RGB 0xFF0000,0xFF00,0xFF,0x0
#	define SURFACE_MASK_R 0xff000000
#	define SURFACE_MASK_G 0xff0000
#	define SURFACE_MASK_B 0xff00
#	define SURFACE_MASK_A 0xff
#endif

namespace graphics
{
	class surface : public reference_counted_ptr
	{
	public:
		surface(const std::string& fname);
		virtual ~surface();

		SDL_Surface* get() { return surf_; }
	private:
		SDL_Surface* surf_;
	};

	typedef boost::intrusive_ptr<surface> surface_ptr;
}
