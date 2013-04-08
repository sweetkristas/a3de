#pragma once

#include "graphics.hpp"
#include "ref_counted_ptr.hpp"

namespace graphcs
{
	class texture : public reference_counted_ptr
	{
	public:
		texture();
		explicit texture(const std::string& fname);

		GLuint id() const { return tex_id_; }
	protected:
	private:
		std::string name_;
		GLuint tex_id_;
	};
}
