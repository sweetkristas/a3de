#include "asserts.hpp"
#include "geometry.hpp"
#include "profile_timer.hpp"
#include "render.hpp"
#include "render_text.hpp"

namespace graphics
{
	namespace renderer
	{
		text::text(const std::string& str, const std::string& font, int size, const SDL_Color& color)
			: str_(str), color_(color)
		{
			profile::manager pman("text::text");
			// XXX handle changes and stuff
			font_ = font::get_font(font, size);
			surface_ptr surf = surface_ptr(TTF_RenderUTF8_Blended(font_.get(), str_.c_str(), color_.as_sdl_color()), SDL_FreeSurface);
			ASSERT_LOG(surf != NULL, "Couldn't render text into texture");
			tex_ = texture::get(surf);
		}

		text::text()
		{}

		text::~text()
		{}

		void text::draw(render& render_obj) const
		{
			// XXX draw texture to display using orthographic projection.
			// Probably want to make a simple shader for this case.
			render_obj.blit_2d_texture(tex_, 0.0f, 0.0f);
		}


		void text::temp_draw(render& render_obj, 
			GLfloat x, 
			GLfloat y, 
			const std::string& str, 
			const std::string& font, 
			int size, 
			const color& c)
		{
			SDL_Color bg = {0, 0, 0, 255};
			surface_ptr surf = surface_ptr(TTF_RenderUTF8_Shaded(font::get_font(font, size).get(), str.c_str(), c.as_sdl_color(), bg), SDL_FreeSurface);
			ASSERT_LOG(surf != NULL, "Couldn't render text into texture");
			render_obj.blit_2d_texture(texture::get(surf), x, y);
		}
	}
}
