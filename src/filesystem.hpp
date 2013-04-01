#pragma once

#include <string>

namespace sys
{
	bool file_exists(const std::string& name);
	std::string read_file(const std::string& name);
	void write_file(const std::string& name, const std::string& data);
}
