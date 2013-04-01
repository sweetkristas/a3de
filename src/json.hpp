#pragma once

#include "node.hpp"


namespace json
{
	class parse_error : public std::runtime_error
	{
	public:
		parse_error(const std::string& error)
			: std::runtime_error(error)
		{}
	};

	node parse(const std::string& s);
	node parse_from_file(const std::string& fname);
	void write(const std::string fnanme, const node& n);
}
