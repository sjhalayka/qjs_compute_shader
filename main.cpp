#include "main.h"


int main(int argc, char **argv)
{
	// Set up grid parameters
	const size_t res = 750; // res must be 2 or greater
	const float grid_max = 1.5;
	const float grid_min = -grid_max;
	const float step_size = (grid_max - grid_min) / (res - 1);

	// Initialize OpenGL / compute shader / textures
	GLuint compute_shader_program = 0;
	GLuint tex_output = 0, tex_input = 0;

	if (false == init_all(argc, argv, tex_output, tex_input, res, res, compute_shader_program))
	{
		cout << "Aborting" << endl;
		return -1;
	}

	// Set up quaternion Julia set parameters
	const quaternion C(0.3f, 0.5f, 0.4f, 0.2f);
	const int max_iterations = 8;
	const float threshold = 4.0f;

	// Set up input quaternion
	quaternion Z(grid_min, grid_min, grid_min, 0.0);

	// Set up input/output data
	const size_t num_input_channels = 4;
	const size_t num_output_channels = 1;
	vector<float> input_pixels(res * res * num_input_channels, 0.0f);
	vector<float> output_pixels(res * res * num_output_channels, 0.0f);

	// Set up slice data, for use as input into the Marching Cubes algorithm
	vector<float> slices(output_pixels.size() * res, 0.0f);
	size_t slices_index = 0;

	// For each z slice
	for (size_t z = 0; z < res; z++, Z.z += step_size)
	{
		cout << "Z slice " << z + 1 << " of " << res << endl;

		Z.x = grid_min;

		// Create pixel array to be used as input for the compute shader
		for (size_t x = 0; x < res; x++, Z.x += step_size)
		{
			Z.y = grid_min;

			for (size_t y = 0; y < res; y++, Z.y += step_size)
			{
				const size_t index = num_input_channels * (y * res + x);

				input_pixels[index + 0] = Z.x;
				input_pixels[index + 1] = Z.y;
				input_pixels[index + 2] = Z.z; 
				input_pixels[index + 3] = Z.w;
			}
		}

		// Run the compute shader
		compute(tex_output, tex_input,
			res, res,
			compute_shader_program,
			output_pixels,
			input_pixels,
			max_iterations,
			threshold,
			C); 

		// Store output data for later use
		for (size_t i = 0; i < output_pixels.size(); i++)
		{
			slices[slices_index] = output_pixels[i];
			slices_index++;
		}
	}

	// Convert slice data to triangle mesh, save to STL file
	vector<triangle> triangles; 
	tesselate_field(slices, triangles, threshold, grid_min, grid_max, res);
	write_triangles_to_binary_stereo_lithography_file(triangles, "out.stl");
	return 0;
}
