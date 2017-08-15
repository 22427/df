#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <string>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <cstring>
#include <iostream>

#include <stb_image.h>
#include <stb_image_write.h>
#include <sstream>
#include "bin_img.h"
#include "shape.h"
#include "image2d.h"




Image2D<float> sample_distance_field(const Glyphe& g,int width, int height, float belt_radius =5)
{
	Image2D<float> df(width,height,1);
//	belt_radius--;
	int br = belt_radius;
	belt_radius = belt_radius/std::min(width,height);
#pragma omp parallel for
	for(int y = 0; y<height;y++)
		for(int x = 0; x<width;x++)
		{
			const auto d = g.distance(df.in_relative(x,y),belt_radius*4);

			df(x,y)[0] =d;
		}

	float max= std::numeric_limits<float>::lowest();

	for(int y = 0; y<height;y++)
		for(int x = 0; x<width;x++)
		{
			bool use = false;
			if(df(x,y)[0] == std::numeric_limits<float>::lowest())
				continue;
			int y_s = std::max(0,y-br);
			int x_s = std::max(0,x-br);
			int y_e = std::max(height,y+br);
			int x_e = std::max(width,x+br);
			for(int yy = y_s; yy<=y_e;yy++)
				for(int xx = x_s; xx<=x_e;xx++)
				{
					if(df(xx,yy)[0] * df(x,y)[0] <0)
					{
						use = true;
						break;
					}
				}
			if(use)
				max = std::max(max,fabsf(df(x,y)[0]));
		}


	printf("normalizing with %f\n",max);
	for(int i = 0 ; i< width * height ; i++)
	{
		df(i)[0] = ((df(i)[0]/max)+1.0f)*0.5f;
		df(i)[0] = df(i)[0]<0? 0:df(i)[0];
		df(i)[0] = df(i)[0]>1? 1:df(i)[0];
	}

	return  df;
}



void save_image(const Image2D<uint8_t>& img,const std::string& path)
{
	stbi_write_png(path.c_str(),img.width,img.height,img.channels,img(0),0);
}

Image2D<uint8_t> load_image(const std::string& path)
{
	int w, h , c;
	auto data = stbi_load(path.c_str(),&w,&h,&c,0);
	Image2D<uint8_t> img;
	img.set_data_ptr(data,true,w,h,c);
	return img;
}

BinImg* to_binary(const Image2D<uint8_t>& img, uint c)
{
	BinImg* res = new BinImg(img.width,img.height);
	for(int i  =0 ; i < img.num_pix();i++)
	{
		res->at(i)[0] = img.at(i)[c]>127?1:0;
	}
	return res;
}


#include "fstream"
int main(int argc, char** argv)
{

	Glyphe g;

	GParser gp;

	for(int i = 1; i< argc-4;i++)
	{
		std::string arg = argv[i];
		const auto ending = arg.substr(arg.find_last_of('.'));
		if(ending == ".txt")
		{
			std::ifstream f(arg);
			g.add_shape(gp.parse(f));
		}
		else
		{
			auto img = load_image(arg);
			auto bin_img = to_binary(img,0);
			g.add_shape(bin_img);
		}
	}

	int w,h,b;
	w = atoi(argv[argc-1-3]);
	h = atoi(argv[argc-1-2]);
	b = atoi(argv[argc-1-1]);
	std::string path = argv[argc-1];


	auto img = sample_distance_field(g,w,h,b);
	img.scale(255);
	auto r = img.cast<uint8_t>();
	save_image(r,path);


}


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
