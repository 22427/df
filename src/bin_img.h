#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "shape.h"
#include "image2d.h"
class BinImg : public Shape, public Image2D<uint8_t>
{

public:

	BinImg(const int32_t w, const int32_t h)
		:Image2D<uint8_t>(w,h,1)
	{

	}



	glm::vec2 closest_to_pixelborder(const glm::vec2& pix_cent, const glm::vec2& i) const
	{
		const auto d = glm::normalize(i-pix_cent);
		const auto ad = glm::abs(d);
		float t;
		if(ad.x>ad.y)
		{
			t=0.5f/ad.x;
		}
		else
		{
			t=0.5f/ad.y;
		}

		return pix_cent+t*d;
	}

	float dist_to_pixelborder(const glm::vec2& pix_cent, const glm::vec2&i) const
	{
		return glm::distance(closest_to_pixelborder(pix_cent,i),i);
	}


	float distance(const glm::vec2 &p, const float max_dist = 0) const
	{
		int mw = max_dist * width;
		int mh = max_dist * height;
		if(!max_dist)
		{
			mw = width;
			mh = height;
		}
		const glm::vec2 position_in_pixels = p* glm::vec2(width,height);
		uint8_t search = this->nearest(p)[0]?0:1;


		float min = std::numeric_limits<float>::max();
		if(!search)
			min = std::min(std::min(position_in_pixels.x,position_in_pixels.y),
						   std::min(fabsf(width-position_in_pixels.x),fabsf(height-position_in_pixels.y)));
		int start_y = std::max(0.0f,position_in_pixels.y-mh);
		int start_x = std::max(0.0f,position_in_pixels.x-mw);

		int end_y = std::min(static_cast<float>(height),position_in_pixels.y+mh);
		int end_x = std::min(static_cast<float>(width),position_in_pixels.x+mw);


		for(int32_t y =  start_y; y < end_y; y++)
			for(int32_t x = start_x ; x < end_x; x++)
			{
				if((at(x,y)[0] && search) || (!at(x,y)[0] && !search))
					min = std::min(min,dist_to_pixelborder(glm::vec2(x,y)+glm::vec2(0.5f),
														   position_in_pixels));
			}

		if(min == std::numeric_limits<float>::max())
		{
			printf("oink! %f %f\n",position_in_pixels.x,position_in_pixels.y);
		}
		if(search)
			min*=-1;
		return min;


	}
};
