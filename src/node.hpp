#pragma once

#include <map>
#include <string>
#include <vector>

class node;
typedef std::map<node,node> node_map;
typedef std::vector<node> node_list;

template<class T, class S>
struct as_if {
    explicit as_if(const node& node_): n(n_) {}
    const node& n;
        
    const T operator()(const S& fallback) const 
	{
        T t;
        if(convert<T>::decode(node, t))
            return t;
        return fallback;
    }
};

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
	explicit node(int);
	explicit node(float);
	explicit node(const std::string&);
	explicit node(const node_map&);
	explicit node(const node_list&);

	node_type type() const { return type_; }
	std::string type_as_string() const;

	static node from_bool(bool b);

	template<typename T, typename S> friend struct as_if;
	template<class T> inline const T node::as() const;

protected:
private:
	node_type type_;

	bool b_;
	int i_;
	float f_;
	std::string s_;
	node_map m_;
	node_list l_;
};

template<typename T>
inline const T node::as() const
{
	return as_if<T, void>(*this)();
}
