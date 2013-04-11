#pragma once

#include <boost/shared_ptr.hpp>
#include "graphics.hpp"

namespace font
{
	class manager
	{
	public:
		manager();
		virtual ~manager();
	};

	typedef boost::shared_ptr<TTF_Font> font_ptr;
	font_ptr get_font(const std::string& font_name, int size);
}
