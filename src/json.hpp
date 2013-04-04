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

	node::node parse(const std::string& s);
	node::node parse_from_file(const std::string& fname);
	void write(std::ostream& os, const node::node& n, bool pretty=true);
}
