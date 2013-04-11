#include "asserts.hpp"
#include "render_text.hpp"

namespace graphics
{
	namespace render
	{
		text::text(const std::string& str, const std::string& font, int size, SDL_Color& color)
			: str_(str), color_(color)
		{
			// XXX handle changes and stuff
			font_ = font::get_font(font, size);
			surface_ptr surf = surface_ptr(TTF_RenderUTF8_Blended(font_.get(), str_.c_str(), color_), SDL_FreeSurface);
			ASSERT_LOG(surf != NULL, "Couldn't render text into texture");
			tex_ = texture::get(surf);
		}

		text::text()
		{}

		text::~text()
		{}

		void text::draw() const
		{
			// XXX draw texture to display using orthographic projection.
			// Probably want to make a simple shader for this case.
		}

	}
}
