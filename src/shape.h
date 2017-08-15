#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <algorithm>
#include <string>
#include <sstream>
class Shape
{
public:
	virtual float distance(const glm::vec2& p, const float max_dist = 0) const =0;
};


class Line : public Shape
{


	glm::vec3 m_l;
	glm::vec2 m_p[2];
	glm::vec2 m_d;
	float m_radius;
public:


	Line(const glm::vec2& a, const glm::vec2&b, const float thickness =0.1f)
	{
		m_p[0] = a;
		m_p[1] = b;
		m_d = b-a;
		m_radius = thickness*0.5f;
		auto n = glm::normalize(m_d);
		std::swap(n.x,n.y);
		n.x *=-1;

		const auto d = glm::dot(a,n);
		m_l = glm::vec3(n,-d);

	}
	float distance(const glm::vec2 &p, const float max_dist = 0) const
	{

		if(glm::dot(p-m_p[0],m_p[1]-m_p[0]) > 0 &&
				glm::dot(p-m_p[1],m_p[0]-m_p[1]) > 0)
			return  -1.0f*(fabs(glm::dot(m_l,glm::vec3(p,1.0f)))-m_radius);

		return -1.0f*(std::min(glm::distance(p,m_p[1]),glm::distance(p,m_p[0])) - m_radius);
	}
};


class Point : public Shape
{


	glm::vec2 m_p;
	float m_radius;
public:


	Point(const glm::vec2& a, const float size =0.1f)
	{
		m_radius =size/2.0f;
		m_p = a;

	}
	float distance(const glm::vec2 &p, const float max_dist = 0) const
	{
		const auto d = fabs(glm::distance(p,m_p));
		return -1.0*(d - m_radius);
	}
};


class Glyphe : public Shape
{
	std::vector<Shape*> m_shapes;

public:
	float distance(const glm::vec2 &p, const float max_dist = 0) const
	{
		float res = std::numeric_limits<float>::lowest();
		//float curr_min = std::numeric_limits<float>::max();
		float min = 0;std::numeric_limits<float>::max();
		for(const auto s : m_shapes)
		{
			float d = s->distance(p,max_dist);
			if(d > 0 && min < d)
			{
				min = d;
				res = d;
			}
		}

		if(min == 0)
		{
			float max = std::numeric_limits<float>::lowest();
			for(const auto s : m_shapes)
			{
				float d = s->distance(p,max_dist);
				if(d < 0 && max < d)
				{
					max = d;
					res = d;
				}
			}
		}
		return res;
	}
	void add_shape(Shape* s) {m_shapes.push_back(s);}
};


// glyphe format

/**

R otate (n degrees)
T anslate x y
S cale x y

B 3 // set brush thickness is scaled by (scale.x+scale.y)/2
P 50 50 3 // point at T*(0.5,0.5,1)
L 50 50 20 20 3 // Line from 50 50 to 20 20

*/


// trim from start (in place)
static inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
	}));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}


class GParser
{
	float Rotate ;
	glm::vec2 Translate;
	glm::vec2 Scale;
	float Thickness;

	glm::mat3 m;
	float thickness;
void build_transform()
{
	m = glm::scale(glm::rotate(glm::translate(glm::mat3(1),Translate),Rotate),Scale);
	thickness = Thickness * (fabs(Scale.x)+fabs(Scale.y))/2.0f;
}
public:
Glyphe* parse(std::istream& in )
{
	std::string line;

	Translate = glm::vec2(0,0);
	Scale = glm::vec2(1,1);
	Rotate = 0.0f;
	Thickness = 0.01f;
	build_transform();
	Glyphe* res = new Glyphe;

	while(std::getline(in,line))
	{
		trim(line);
		std::stringstream str(line);
		char c = 0;
		str>> c;
		if(c == 'T')
		{
			str>>Translate.x>>Translate.y;
			build_transform();
		}
		else if(c == 'S')
		{
			str>>Scale.x>>Scale.y;
			build_transform();
		}
		else if(c == 'R')
		{
			str>>Rotate;
			build_transform();
		}
		else if(c == 'B')
		{
			str>>Thickness;
			build_transform();
		}
		else if(c == 'P')
		{
			glm::vec3 position(0,0,1);
			str>>position.x>>position.y;
			position = m * position;
			res->add_shape(new Point(glm::vec2(position),thickness));
		}
		else if(c == 'L')
		{
			glm::vec3 position[2] = {glm::vec3(0,0,1),glm::vec3(0,0,1)};
			for(uint i = 0 ; i <2 ;i++)
			{
			str>>position[i].x>>position[i].y;
			position[i] = m * position[i];
			}
			res->add_shape(new Line(glm::vec2(position[0]),glm::vec2(position[1]),thickness));
		}
	}
	return res;
}
};
