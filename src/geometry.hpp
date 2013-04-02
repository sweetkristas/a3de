#pragma once

#include <string>
#include <vector>

#include "graphics.hpp"

struct point 
{
	explicit point(const std::string& str);
	explicit point(int x=0, int y=0);
	explicit point(const std::vector<int>& v);

	std::string to_string() const;

	union 
	{
		struct { int x, y; };
		int buf[2];
	};
};

class rect 
{
public:
	static rect from_coordinates(int x1, int y1, int x2, int y2);
	explicit rect(const std::string& str);
	explicit rect(int x=0, int y=0, int w=0, int h=0);
	explicit rect(const std::vector<int>& v);
	int x() const { return top_left_.x; }
	int y() const { return top_left_.y; }
	int x2() const { return bottom_right_.x; }
	int y2() const { return bottom_right_.y; }
	int w() const { return bottom_right_.x - top_left_.x; }
	int h() const { return bottom_right_.y - top_left_.y; }

	GLfloat xf() const { return GLfloat(top_left_.x); }
	GLfloat yf() const { return GLfloat(top_left_.y); }
	GLfloat x2f() const { return GLfloat(bottom_right_.x); }
	GLfloat y2f() const { return GLfloat(bottom_right_.y); }
	GLfloat wf() const { return GLfloat(bottom_right_.x - top_left_.x); }
	GLfloat hf() const { return GLfloat(bottom_right_.y - top_left_.y); }

	int mid_x() const { return (x() + x2())/2; }
	int mid_y() const { return (y() + y2())/2; }

	point& top_left() { return top_left_; }
	point& bottom_right() { return bottom_right_; }
	const point& top_left() const { return top_left_; }
	const point& bottom_right() const { return bottom_right_; }

	rect& operator+= (const point& p);
	std::string to_string() const;
	SDL_Rect sdl_rect() const { SDL_Rect r = {x(), y(), w(), h()}; return r; }

	bool empty() const { return w() == 0 || h() == 0; }
private:
	point top_left_, bottom_right_;
};
