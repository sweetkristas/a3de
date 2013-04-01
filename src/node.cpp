#include "asserts.hpp"
#include "node.hpp"

#include "stdafx.h"

node::node()
	: type_(NODE_TYPE_NULL), i_(0), f_(0.0f)
{
}

node::node(int n)
	: type_(NODE_TYPE_INTEGER), i_(n), f_(0.0f)
{

}

node::node(float f)
	: type_(NODE_TYPE_FLOAT), i_(0), f_(f)
{
}

node::node(const std::string&)
	: type_(NODE_TYPE_STRING), i_(0), f_(0.0f)
{
}

node::node(const std::map<node,node>&)
	: type_(NODE_TYPE_MAP), i_(0), f_(0.0f)
{
}

node::node(const std::vector<node>&)
	: type_(NODE_TYPE_LIST), i_(0), f_(0.0f)
{
}

node node::from_bool(bool b)
{
	node n;
	n.type_ = NODE_TYPE_BOOL;
	n.b_ = b;
	return n;
}


std::string node::type_as_string() const
{
	switch(type_) {
	case NODE_TYPE_NULL:
		return "null";
	case NODE_TYPE_INTEGER:
		return "int";
	case NODE_TYPE_FLOAT:
		return "float";
	case NODE_TYPE_BOOL:
		return "bool";
	case NODE_TYPE_STRING:
		return "string";
	case NODE_TYPE_MAP:
		return "map";
	case NODE_TYPE_LIST:
		return "list";
	}
	ASSERT_LOG(false, "Unrecognised type converting to string: " << type_);
}

template<> int get_as<int>(const node& n)
{
	switch(n.type()) {
	case NODE_TYPE_INTEGER:
		return n.i_;
	case NODE_TYPE_FLOAT:
		return int(n.f_);
	case NODE_TYPE_BOOL:
		return (n.b_) ? 1 : 0;
	}
	ASSERT_LOG(false, "as<int> type conversion error from " << n.type_as_string() << " to int");
	return 0;
}
