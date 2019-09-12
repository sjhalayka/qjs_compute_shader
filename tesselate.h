#ifndef TESSELATE_H
#define TESSELATE_H


#include "marching_cubes.h"
using namespace marching_cubes;

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <fstream>
using std::ifstream;
using std::ofstream;

#include <iomanip>
using std::ios_base;
using std::setprecision;

#include <algorithm>
using std::fill;


void add_field_border(vector<float> &values, 
					   const float border_value,
					   float &grid_min, float &grid_max,
					   size_t &res );

void tesselate_field(const vector<float> &values, vector<triangle> &triangle_list, const float &isovalue, const float &grid_min, const float &grid_max, const size_t &res);

bool write_triangles_to_binary_stereo_lithography_file(const vector<triangle>& triangles, const char* const file_name);

#endif
