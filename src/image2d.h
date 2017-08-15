#pragma once
#include <cstdint>
#include <cstdlib>
#include <glm/glm.hpp>
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
								   static_cast<float>(height));
		return at(p.x,p.y);
	}

	glm::vec2 in_relative(const int32_t x, const int32_t y) const
	{
		return glm::vec2((static_cast<float>(x)+0.5f)/static_cast<float>(width),
						 (static_cast<float>(y)+0.5f)/static_cast<float>(height));
	}

	glm::vec2 in_pixels(const glm::vec2& p) const
	{
		return p* glm::vec2(static_cast<float>(width),
							static_cast<float>(height));
	}

	void scale(T s)
	{
		for(int i = 0 ; i< width*height*channels;i++)
			m_data[i]*=s;
	}
	template<typename Q>
	Image2D<Q> cast()
	{
		Image2D<Q>res(width,height,channels);
		for(int i = 0 ; i< width*height*channels;i++)
			res.m_data[i] = static_cast<Q>(m_data[i]);

		return  res;
	}


};
