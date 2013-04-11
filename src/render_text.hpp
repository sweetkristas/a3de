#pragma once

#include "color.hpp"
#include "fonts.hpp"
#include "shaders.hpp"
#include "texture.hpp"

namespace graphics
{
	namespace renderer
	{
		class text
		{
		public:
			explicit text(const std::string& str, const std::string& font, int size, SDL_Color& color);
			virtual ~text();
			void draw() const;
		protected:
		private:
			text();
			std::string str_;
			color color_;
			font::font_ptr font_;
			const_texture_ptr tex_;
		};
	}
}
