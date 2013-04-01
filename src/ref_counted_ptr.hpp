#pragma once

#include <boost/intrusive_ptr.hpp>

template<class T>
inline void intrusive_ptr_add_ref(T* expr)
{
	expr->add_reference();
}

template<class T>
inline void intrusive_ptr_release(T* expr)
{
	expr->release_reference();
}

class reference_counted_ptr
{
public:
	reference_counted_ptr() 
		: references_(0) 
	{}
	virtual ~reference_counted_ptr()
	{}
	void add_reference() { ++references_; }
	void release_reference() 
	{ 
			if(--references_ == 0) { 
				delete this;
			}
	}
private:
	long references_;
};
