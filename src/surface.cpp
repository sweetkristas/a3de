#include "asserts.hpp"
#include "surface.hpp"

namespace graphics
{
	surface::surface(const std::string& fname)
	{
		surf_ = IMG_Load(fname.c_str());
		ASSERT_LOG(surf_ != NULL, "Failed to load image: " << fname << " : " << IMG_GetError());
	}

	surface::~surface()
	{
		ASSERT_LOG(surf_ != NULL, "Error destructing surface. Is NULL");
		SDL_FreeSurface(surf_);
		surf_ = NULL;
	}
}
