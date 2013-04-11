#pragma once

#include "filesystem.hpp"

namespace module
{
	void load_module(const std::string& modname);
	std::string map_file(const std::string& file);
	void get_unique_files(const std::string& path, sys::file_path_map& fpm);
}
