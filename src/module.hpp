#pragma once

#include <string>

namespace module
{
	void load_module(const std::string& modname);
	std::string map_file(const std::string& file);
}
