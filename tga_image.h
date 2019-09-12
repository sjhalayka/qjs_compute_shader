// http://www.paulbourke.net/dataformats/tga/

#define _CRT_SECURE_NO_WARNINGS


#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;

class header {
public:
    char  idlength;
    char  colourmaptype;
    char  datatypecode;
    short int colourmaporigin;
    short int colourmaplength;
    char  colourmapdepth;
    short int x_origin;
    short int y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
};

class pixel {
public:
    unsigned char r,g,b,a;
};


class tga_32bit_image
{
public:
    void MergeBytes(pixel *pxl, unsigned char *p, int bytes);
    void save(const char *const filename);
    void load(const char *const filename);
	void load_single_channel_float(unsigned short width, unsigned short height, const vector<float> &buffer, const float limit);
    
    vector<pixel> pixels;
    header hdr;
    unsigned int width, height;
};
