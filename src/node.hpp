#pragma once

#include <map>
#include <string>
#include <vector>
#include <cstdint>

namespace node 
{
	class node;
	typedef std::map<node,node> node_map;
	typedef std::vector<node> node_list;

	class node
	{
	public:
		enum node_type
		{
			NODE_TYPE_NULL,
			NODE_TYPE_BOOL,
			NODE_TYPE_INTEGER,
			NODE_TYPE_FLOAT,
			NODE_TYPE_STRING,
			NODE_TYPE_MAP,
			NODE_TYPE_LIST,
		};

		node();
		explicit node(int64_t);
		explicit node(float);
		explicit node(const std::string&);
		explicit node(const node_map&);
		explicit node(const node_list&);

		node_type type() const { return type_; }
		std::string type_as_string() const;

		static node from_bool(bool b);

		std::string as_string() const;
		int64_t as_int() const;
		float as_float() const;
		bool as_bool() const;
		const node_list& as_list() const;
		const node_map& as_map() const;

		node_list& as_mutable_list();
		node_map& as_mutable_map();

		bool operator<(const node&) const;
		bool operator>(const node&) const;
	protected:
	private:
		node_type type_;

		bool b_;
		int64_t i_;
		float f_;
		std::string s_;
		node_map m_;
		node_list l_;
	};
}
