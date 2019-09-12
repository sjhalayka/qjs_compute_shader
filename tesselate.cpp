#include "tesselate.h"

void add_field_border(vector<float> &values, const float border_value, float &grid_min, float &grid_max, size_t &res)
{
	const float step_size = (grid_max - grid_min) / (res - 1);
	const float new_grid_min = grid_min - step_size;
	const float new_grid_max = grid_max + step_size;
	const size_t new_res = res + 2;

	vector<float> new_values(new_res*new_res*new_res, border_value);

	// Stuff new vector with old vector values
	for(size_t x = 0; x < res; x++)
		for(size_t y = 0; y < res; y++)
			for(size_t z = 0; z < res; z++)
				new_values[(x + 1)*new_res*new_res + (y + 1)*new_res + (z + 1)] = values[x*res*res + y*res + z];

	values = new_values;
	grid_min = new_grid_min;
	grid_max = new_grid_max;
	res = new_res;
}

void tesselate_field(const vector<float> &values, vector<triangle> &triangle_list, const float &isovalue, const float &grid_min, const float &grid_max, const size_t &res)
{
	triangle_list.clear();

	const float step_size = (grid_max - grid_min) / (res - 1);

	for(size_t x = 0; x < res - 1; x++)
	{
		cout << "Tesselation step " << x + 1 << " of " << res - 1 << endl;

		for(size_t y = 0; y < res - 1; y++)
		{
			for(size_t z = 0; z < res - 1; z++)
			{
				grid_cube temp_cube;

				size_t x_offset = 0;
				size_t y_offset = 0;
				size_t z_offset = 0;

				// Setup vertex 0
				x_offset = 0;
				y_offset = 0;
				z_offset = 0;
				temp_cube.vertex[0].x = grid_min + ((x+x_offset) * step_size);
				temp_cube.vertex[0].y = grid_min + ((y+y_offset) * step_size);
				temp_cube.vertex[0].z = grid_min + ((z+z_offset) * step_size);
				temp_cube.value[0] = values[(x + x_offset)*(res)*(res) + (y + y_offset)*(res) + (z + z_offset)];

				// Setup vertex 1
				x_offset = 1;
				y_offset = 0;
				z_offset = 0;
				temp_cube.vertex[1].x = grid_min + ((x+x_offset) * step_size);
				temp_cube.vertex[1].y = grid_min + ((y+y_offset) * step_size);
				temp_cube.vertex[1].z = grid_min + ((z+z_offset) * step_size);
				temp_cube.value[1] = values[(x + x_offset)*(res)*(res) + (y + y_offset)*(res) + (z + z_offset)];

				// Setup vertex 2
				x_offset = 1;
				y_offset = 0;
				z_offset = 1;
				temp_cube.vertex[2].x = grid_min + ((x+x_offset) * step_size);
				temp_cube.vertex[2].y = grid_min + ((y+y_offset) * step_size);
				temp_cube.vertex[2].z = grid_min + ((z+z_offset) * step_size);
				temp_cube.value[2] = values[(x + x_offset)*(res)*(res) + (y + y_offset)*(res) + (z + z_offset)];

				// Setup vertex 3
				x_offset = 0; 
				y_offset = 0;
				z_offset = 1;
				temp_cube.vertex[3].x = grid_min + ((x+x_offset) * step_size);
				temp_cube.vertex[3].y = grid_min + ((y+y_offset) * step_size);
				temp_cube.vertex[3].z = grid_min + ((z+z_offset) * step_size);
				temp_cube.value[3] = values[(x + x_offset)*(res)*(res) + (y + y_offset)*(res) + (z + z_offset)];

				// Setup vertex 4
				x_offset = 0;
				y_offset = 1;
				z_offset = 0;
				temp_cube.vertex[4].x = grid_min + ((x+x_offset) * step_size);
				temp_cube.vertex[4].y = grid_min + ((y+y_offset) * step_size);
				temp_cube.vertex[4].z = grid_min + ((z+z_offset) * step_size);
				temp_cube.value[4] = values[(x + x_offset)*(res)*(res) + (y + y_offset)*(res) + (z + z_offset)];

				// Setup vertex 5
				x_offset = 1;
				y_offset = 1;
				z_offset = 0;
				temp_cube.vertex[5].x = grid_min + ((x+x_offset) * step_size);
				temp_cube.vertex[5].y = grid_min + ((y+y_offset) * step_size);
				temp_cube.vertex[5].z = grid_min + ((z+z_offset) * step_size);
				temp_cube.value[5] = values[(x + x_offset)*(res)*(res) + (y + y_offset)*(res) + (z + z_offset)];

				// Setup vertex 6
				x_offset = 1;
				y_offset = 1;
				z_offset = 1;
				temp_cube.vertex[6].x = grid_min + ((x+x_offset) * step_size);
				temp_cube.vertex[6].y = grid_min + ((y+y_offset) * step_size);
				temp_cube.vertex[6].z = grid_min + ((z+z_offset) * step_size);
				temp_cube.value[6] = values[(x + x_offset)*(res)*(res) + (y + y_offset)*(res) + (z + z_offset)];

				// Setup vertex 7
				x_offset = 0;
				y_offset = 1;
				z_offset = 1;
				temp_cube.vertex[7].x = grid_min + ((x+x_offset) * step_size);
				temp_cube.vertex[7].y = grid_min + ((y+y_offset) * step_size);
				temp_cube.vertex[7].z = grid_min + ((z+z_offset) * step_size);
				temp_cube.value[7] = values[(x + x_offset)*(res)*(res) + (y + y_offset)*(res) + (z + z_offset)];

				// Generate triangles from cube
				triangle temp_triangle_array[5];

				short unsigned int number_of_triangles_generated = tesselate_grid_cube(isovalue, temp_cube, temp_triangle_array);

				for(short unsigned int i = 0; i < number_of_triangles_generated; i++)
				{
					triangle_list.push_back(temp_triangle_array[i]);
				}
			}
		}
	}
}

bool write_triangles_to_binary_stereo_lithography_file(const vector<triangle>& triangles, const char* const file_name)
{
	cout << "Triangle count: " << triangles.size() << endl;

	if (0 == triangles.size())
		return false;

	// Write to file.
	ofstream out(file_name, ios_base::binary);

	if (out.fail())
		return false;

	const size_t header_size = 80;
	vector<char> buffer(header_size, 0);
	const unsigned int num_triangles = static_cast<unsigned int>(triangles.size()); // Must be 4-byte unsigned int.
	vertex_3 normal;

	// Write blank header.
	out.write(reinterpret_cast<const char*>(&(buffer[0])), header_size);

	// Write number of triangles.
	out.write(reinterpret_cast<const char*>(&num_triangles), sizeof(unsigned int));

	// Copy everything to a single buffer.
	// We do this here because calling ofstream::write() only once PER MESH is going to 
	// send the data to disk faster than if we were to instead call ofstream::write()
	// thirteen times PER TRIANGLE.
	// Of course, the trade-off is that we are using 2x the RAM than what's absolutely required,
	// but the trade-off is often very much worth it (especially so for meshes with millions of triangles).
	cout << "Generating normal/vertex/attribute buffer" << endl;

	// Enough bytes for twelve 4-byte floats plus one 2-byte integer, per triangle.
	const size_t data_size = (12 * sizeof(float) + sizeof(short unsigned int)) * num_triangles;
	buffer.resize(data_size, 0);

	// Use a pointer to assist with the copying.
	// Should probably use std::copy() instead, but memcpy() does the trick, so whatever...
	char* cp = &buffer[0];

	for (vector<triangle>::const_iterator i = triangles.begin(); i != triangles.end(); i++)
	{
		// Get face normal.
		vertex_3 v0 = i->vertex[1] - i->vertex[0];
		vertex_3 v1 = i->vertex[2] - i->vertex[0];
		normal = v0.cross(v1);
		normal.normalize();

		memcpy(cp, &normal.x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &normal.y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &normal.z, sizeof(float)); cp += sizeof(float);

		memcpy(cp, &i->vertex[0].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[0].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[0].z, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[1].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[1].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[1].z, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[2].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[2].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[2].z, sizeof(float)); cp += sizeof(float);

		cp += sizeof(short unsigned int);
	}

	cout << "Writing " << data_size / 1048576 << " MB of data to binary Stereo Lithography file: " << file_name << endl;

	out.write(reinterpret_cast<const char*>(&buffer[0]), data_size);
	out.close();

	return true;
}
