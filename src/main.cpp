#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <cstring>


#include <stb_image.h>
#include <stb_image_write.h>

template<typename T = uint8_t>
class Image2D
{
	T* m_data;
	bool m_free_data;
public:
template<typename> friend struct Image2D;
	int32_t width;
	int32_t height;
	int32_t channels;

	Image2D():m_data(nullptr),width(0),height(0),channels(0){}

	Image2D(const int32_t w, const int32_t h, const int32_t c = 3)
		:m_data(nullptr),width(w),height(h),channels(c)
	{
		m_data = (T*)malloc(w*h*c*sizeof(T));
		m_free_data = true;
	}

	~Image2D()
	{
		if(m_free_data && m_data)
		{
			free(m_data);
			m_data = nullptr;
		}
	}

	int32_t num_pix()const {return width*height;}
	void set_data_ptr(T* d,
					  bool free_data = false,
					  const int32_t w=0,
					  const int32_t h=0,
					  const int32_t c=0)
	{
		if(free_data && m_data)
			free(m_data);
		m_data = d;
		m_free_data = free_data;
		if(w) width = w;
		if(h) height = h;
		if(c) channels = c;

	}

	const T* operator()(const int32_t x, const int32_t y) const
	{
		return  this->at(x,y);
	}

	T* operator()(const int32_t x, const int32_t y)
	{
		return  this->at(x,y);
	}


	const T* operator()(const int32_t id) const
	{
		return this->at(id);
	}

	T* operator()(const int32_t id)
	{
		return this->at(id);
	}


	const T* at(const int32_t x, const int32_t y) const
	{
		return m_data + ((width*y+x)*channels);
	}
	T* at(const int32_t x, const int32_t y)
	{
		return m_data + ((width*y+x)*channels);
	}

	const T* at(const int32_t id) const
	{
		return m_data +(id*channels);
	}
	T* at(const int32_t id)
	{
		return m_data +(id*channels);
	}

	const T* nearest(const glm::vec2& v) const
	{
		const auto p = v*glm::vec2(static_cast<float>(width),
								   static_cast<float>(height))+ glm::vec2(0.5f);
		return at(p.x,p.y);
	}

	glm::vec2 in_relative(const int32_t x, const int32_t y) const
	{
		return glm::vec2((static_cast<float>(x)+0.5f)/static_cast<float>(width),
						 (static_cast<float>(y)+0.5f)/static_cast<float>(height));
	}

	void scale(T s)
	{
		for(uint i = 0 ; i< width*height*channels;i++)
			m_data[i]*=s;
	}
	template<typename Q>
	Image2D<Q> cast()
	{
		Image2D<Q>res(width,height,channels);
		for(uint i = 0 ; i< width*height*channels;i++)
			res.m_data[i] = static_cast<Q>(m_data[i]);

		return  res;
	}
};






class Shape
{
public:
	virtual float distance(const glm::vec2& p) const = 0;
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
	float distance(const glm::vec2 &p)const
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
	float distance(const glm::vec2 &p)const
	{
		const auto d = fabs(glm::distance(p,m_p));
		return -1.0*(d - m_radius);
	}
};


class Glyphe : public Shape
{
	std::vector<Shape*> m_shapes;

public:
	float distance(const glm::vec2 &p) const
	{
		float res = 0;
		float curr_min = 2;
		for(const auto s : m_shapes)
		{
			float d = s->distance(p);
			if(fabs(d) < curr_min)
			{
				res =d;
				curr_min = fabs(d);
			}
		}
		return res;
	}
	void add_shape(Shape* s) {m_shapes.push_back(s);}
};


Image2D<float> sample_distance_field(const Glyphe& g,int width, int height, float max_dist =20)
{
	Image2D<float> df(width,height,1);
	float max= std::numeric_limits<float>::lowest();
	for(int y = 0; y<height;y++)
		for(int x = 0; x<width;x++)
		{
			const auto d = g.distance(df.in_relative(x,y));
			if(max < fabs(d))
			{
				max = fabs(d);
			}
			df(x,y)[0] =d;
		}


	if(max > max_dist)
		max = max_dist;

	for(uint i = 0 ; i< width * height ; i++)
	{
		df(i)[0] = ((df(i)[0]/max_dist)+1.0f)*0.5f;
		df(i)[0] = df(i)[0]<0? 0:df(i)[0];
		df(i)[0] = df(i)[0]>1? 1:df(i)[0];
	}

	return  df;
}


#include <string>
void save_image(const Image2D<uint8_t>& img,const std::string& path)
{
	stbi_write_png(path.c_str(),img.width,img.height,img.channels,img(0),0);
}


int main(int argc, char** argv)
{
	Glyphe g;
	g.add_shape(new Point(glm::vec2(0.5,0.5),0.2));

	auto img = sample_distance_field(g,256,256,0.1);
	img.scale(255);
	auto r = img.cast<uint8_t>();
	save_image(r,"tst.png");


}


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
