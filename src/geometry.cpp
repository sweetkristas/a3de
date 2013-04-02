#include "graphics.hpp"
#include <sstream>
#include <boost/lexical_cast.hpp>

#include "geometry.hpp"
#include "utils.hpp"

point::point(const std::string& str)
{
	std::vector<std::string> buf = utils::split(str, ",");
	if(buf.size() != 2) {
		x = y = 0;
	} else {
		x = strtol(buf[0].c_str(), NULL, 10);
		y = strtol(buf[1].c_str(), NULL, 10);
	}
}

point::point(int x, int y) 
	: x(x), y(y)
{
}

point::point(const std::vector<int>& v)
{
	if(v.empty()) {
		x = y = 0;
	} else if(v.size() == 1) {
		x = v[0];
		y = 0;
	} else {
		x = v[0];
		y = v[1];
	}
}

std::string point::to_string() const
{
	std::stringstream ss;
	ss << x << "," << y;
	return ss.str();
}

rect rect::from_coordinates(int x1, int y1, int x2, int y2)
{
	if(x1 > x2+1) {
		std::swap(x1, x2);
	}

	if(y1 > y2+1) {
		std::swap(y1, y2);
	}
	return rect(x1, y1, x2-x1+1, y2-y1+1);
}

rect::rect(const std::string& str)
{
	if(str.empty()) {
		*this = rect();
		return;
	}

	int items[4];
	int num_items = 0;
	std::vector<std::string> buf = utils::split(str, ",");
	for(int n = 0; n != 4 && n != buf.size(); ++n) {
		items[num_items++] = boost::lexical_cast<int>(buf[n]);
	}

	switch(num_items) {
	case 2:
		*this = rect::from_coordinates(items[0], items[1], 1, 1);
		break;
	case 3:
		*this = rect::from_coordinates(items[0], items[1], items[2], 1);
		break;
	case 4:
		*this = rect::from_coordinates(items[0], items[1], items[2], items[3]);
		break;
	default:
		*this = rect();
		break;
	}
}

rect::rect(int x, int y, int w, int h)
	: top_left_(std::min(x, x+w), std::min(y, y+h)),
	bottom_right_(std::max(x, x+w), std::max(y, y+h))
{}

rect::rect(const std::vector<int>& v)
{
	switch(v.size()) {
	case 2:
		*this = rect::from_coordinates(v[0], v[1], v[0], v[1]);
		break;
	case 3:
		*this = rect::from_coordinates(v[0], v[1], v[2], v[1]);
		break;
	case 4:
		*this = rect::from_coordinates(v[0], v[1], v[2], v[3]);
		break;
	default:
		*this = rect();
		break;
	}
}

rect& rect::operator+= (const point& p)
{
	top_left_.x += p.x;
	top_left_.y += p.y;
	bottom_right_.x += p.x;
	bottom_right_.y += p.y;
	return *this;
}

std::string rect::to_string() const
{
	std::stringstream ss;
	ss << x() << "," << y() << "," << (x2()-1) << "," << (y2()-1);
	return ss.str();
}
