#include <sstream>
#include "asserts.hpp"
#include "node.hpp"

namespace node 
{
	node::node()
		: type_(NODE_TYPE_NULL), i_(0), f_(0.0f)
	{
	}

	node::node(int64_t n)
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

	int64_t node::as_int() const
	{
		switch(type()) {
		case NODE_TYPE_INTEGER:
			return i_;
		case NODE_TYPE_FLOAT:
			return int64_t(f_);
		case NODE_TYPE_BOOL:
			return b_ ? 1 : 0;
		}
		ASSERT_LOG(false, "as_int() type conversion error from " << type_as_string() << " to int");
		return 0;
	}

	std::string node::as_string() const
	{
		switch(type()) {
		case NODE_TYPE_STRING:
			return s_;
		case NODE_TYPE_INTEGER: {
			std::stringstream s;
			s << i_;
			return s.str();
		}
		case NODE_TYPE_FLOAT: {
			std::stringstream s;
			s << f_;
			return s.str();
		}
		}
		ASSERT_LOG(false, "as_string() type conversion error from " << type_as_string() << " to int");
		return "";
	}

	float node::as_float() const
	{
		switch(type()) {
		case NODE_TYPE_INTEGER:
			return float(i_);
		case NODE_TYPE_FLOAT:
			return f_;
		case NODE_TYPE_BOOL:
			return b_ ? 1.0f : 0.0f;
		}
		ASSERT_LOG(false, "as_float() type conversion error from " << type_as_string() << " to int");
		return 0;
	}

	bool node::as_bool() const
	{
		switch(type()) {
		case NODE_TYPE_INTEGER:
			return i_ ? true : false;
		case NODE_TYPE_FLOAT:
			return f_ == 0.0f ? false : true;
		case NODE_TYPE_BOOL:
			return b_;
		case NODE_TYPE_STRING:
			return s_.empty() ? false : true;
		case NODE_TYPE_LIST:
			return l_.empty() ? false : true;
		case NODE_TYPE_MAP:
			return m_.empty() ? false : true;
		}
		ASSERT_LOG(false, "as_bool() type conversion error from " << type_as_string() << " to int");
		return 0;
	}

	const node_list& node::as_list() const
	{
		ASSERT_LOG(type() == NODE_TYPE_LIST, "as_list() type conversion error from " << type_as_string() << " to int");
		return l_;
	}

	const node_map& node::as_map() const
	{
		ASSERT_LOG(type() == NODE_TYPE_MAP, "as_map() type conversion error from " << type_as_string() << " to int");
		return m_;
	}

	node_list& node::as_mutable_list()
	{
		ASSERT_LOG(type() == NODE_TYPE_LIST, "as_mutable_list() type conversion error from " << type_as_string() << " to int");
		return l_;
	}

	node_map& node::as_mutable_map()
	{
		ASSERT_LOG(type() == NODE_TYPE_MAP, "as_mutable_map() type conversion error from " << type_as_string() << " to int");
		return m_;
	}

	bool node::operator<(const node& n) const
	{
		if(type() != n.type()) {
			return type_ < n.type();
		}
		switch(type()) {
		case NODE_TYPE_NULL:
			return true;
		case NODE_TYPE_BOOL:
			return b_ < n.b_;
		case NODE_TYPE_INTEGER:
			return i_ < n.i_;
		case NODE_TYPE_FLOAT:
			return f_ < n.f_;
		case NODE_TYPE_STRING:
			return s_ < n.s_;
		case NODE_TYPE_MAP:
			return m_.size() < n.m_.size();
		case NODE_TYPE_LIST:
			for(int i = 0; i != l_.size() && i != n.l_.size(); ++i) {
				if(l_[i] < n.l_[i]) {
					return true;
				} else if(l_[i] > n.l_[i]) {
					return false;
				}
			}
			return l_.size() < n.l_.size();
		}
		ASSERT_LOG(false, "operator< unknown type: " << type_as_string());
		return false;
	}

	bool node::operator>(const node& n) const
	{
		return !(*this < n);
	}
}