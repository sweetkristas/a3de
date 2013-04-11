#pragma once

#include "fonts.hpp"
#include "texture.hpp"

namespace graphics
{
	namespace render
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
			SDL_Color color_;
			font::font_ptr font_;
			texture_ptr tex_;
		};
	}
}
