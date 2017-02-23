#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/gtx/norm.hpp>


#include <glm/glm.hpp>
#include <stdint.h>
#include <algorithm>
#include <cstring>


#include <stb_image.h>
#include <stb_image_write.h>

class Image2D
{
public:
	uint8_t* data;
	int32_t width;
	int32_t height;
	int32_t channels;


	const uint8_t* at(const int32_t x, const int32_t y) const
	{
		return data + (width*y+x)*channels;
	}
};



glm::vec2 closest_to_pixelborder(const glm::vec2& p, const glm::vec2& i)
{
	const auto d = glm::normalize(i-p);
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

	return p+t*d;
}

float dist2_to_pixelborder(const glm::vec2& p, const glm::vec2&i)
{
	return glm::distance2(closest_to_pixelborder(p,i),i);
}

float dist2_other_border(const Image2D& img, const glm::vec2& i,int32_t max)
{
	const glm::ivec2 it(static_cast<int32_t>(i.x),static_cast<int32_t>(i.y));
	auto me = img.at(it.x,it.y)[0];

	float min = static_cast<float>(max)*sqrtf(2.0f);
	for(int32_t y = std::max(0,it.y-max) ; y < std::min(img.height,it.y+max); y++)
		for(int32_t x = std::max(0,it.x-max) ; x<std::min(img.width,it.x+max); x++)
		{
			if(img.at(x,y)[0] != me)
				min = std::min(min,dist2_to_pixelborder(glm::vec2(0.5f+x,0.5f+y),i));
		}
	if(me == 0)
		return  -sqrtf(min);
	else
		return  sqrtf(min);
}


float* calc_dist(const Image2D& img, const uint32_t w, const uint32_t h, const int max)
{
	float* res = new float[w*h];
	glm::vec2 factor(static_cast<float>(img.width)/w,static_cast<float>(img.height)/h);
	#pragma omp parallel for
	for(uint32_t y = 0 ; y < h;y++)
		for(uint32_t x = 0 ; x < w;x++)
		{
			glm::vec2 i(0.5f+x,0.5f+y);
			i*=factor;

			res[y*w+x] = dist2_other_border(img,i,max);
		}
	return  res;
}

struct settings
{
	std::string input;
	std::string output;
	int out_width;
	int out_height;
	int max_dist;
	settings()
	{
		input ="";
		output="";
		out_width = -1;
		out_height = -1;
		max_dist = 25;
	}

};
void print_help()
{
	settings def;
	fprintf(stderr,"-if\tSet input file name.\n");
	fprintf(stderr,"-of\tSet output file name.\n");
	fprintf(stderr,"-w \tSet output width.\n");
	fprintf(stderr,"-h \tSet output height.\n	");
	fprintf(stderr,"-max \tSet max distance.(Default: %d)\n",def.max_dist);
	exit(0);
}
settings parse_argv(int argc, char** argv)
{
	settings s;
	for(int i = 1 ; i<argc;i++)
	{
		if(strcmp(argv[i],"-if") == 0)
		{
			s.input = argv[++i];
		}
		if(strcmp(argv[i],"-of") == 0)
		{
			s.output = argv[++i];
		}

		if(strcmp(argv[i],"-w") == 0)
		{
			s.out_width = atoi(argv[++i]);
		}
		if(strcmp(argv[i],"-h") == 0)
		{
			s.out_height = atoi(argv[++i]);
		}
		if(strcmp(argv[i],"-max") == 0)
		{
			s.max_dist = atoi(argv[++i]);
		}
	}
	if(s.input.empty() || s.output.empty())
		print_help();

	return s;
}

int main(int argc, char** argv)
{
	settings s = parse_argv(argc,argv);
	int w,h,c;
	auto i = stbi_load(s.input.c_str(),&w,&h,&c,1);
	if(!i)
	{
		fprintf(stderr,"Error loading input!\n");
		exit(0);
	}

	if(s.out_height <=0 && s.out_width <=0)
	{
		s.out_width = w;
		s.out_height = h;
	}
	else if(s.out_height <=0 )
	{
		float ar = static_cast<float>(w)/h;
		s.out_height = static_cast<int>(1.0f/ar*s.out_width);
	}
	else if(s.out_width <=0 )
	{
		float ar = static_cast<float>(w)/h;
		s.out_width = static_cast<int>(ar*s.out_height);
	}


	Image2D img;
	img.channels = 1;
	img.width = w;
	img.height =h;
	img.data = i;

	auto df = calc_dist(img,
						static_cast<uint32_t>(s.out_width),
						static_cast<uint32_t>(s.out_height),
						s.max_dist);

	unsigned char* r = new unsigned char[s.out_height*s.out_width];

	float mmax = static_cast<float>(s.max_dist)*sqrtf(2.0f);


	for(int j = 0 ; j< w*h;j++)
	{
		r[j] = static_cast<unsigned char>(255.0f * std::min(1.0f,std::max(0.0f,(0.5f+df[j]/(2.0f*mmax)))));
	}

	stbi_write_png(s.output.c_str(),s.out_width,s.out_height,1,r,0);

	delete [] r;
	delete [] df;

}


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>
