#include "main.h"


// The following code computes a quaternion Julia set by offloading the iterative function
// onto the graphics processing unit (GPU). This is done because the GPU has far more cores
// than a standard central processing unit (CPU), and can thus run more copies of the 
// iterative function at once. This is accomplished by using an OpenGL compute shader. The
// output of the compute shader is then run through the Marching Cubes algorithm, which 
// produces a set of triangles that defines the 2D surface of a quaternion Julia set. This 
// triangle set is saved to STL format, which is widely supported (like, by MeshLab).
//
// - Shawn Halayka - 2019 - sjhalayka@gmail.com


int main(int argc, char **argv)
{
	// Set up grid parameters
	const size_t res = 300; // res must be 2 or greater, and not necessarily a power-of-two
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

	// Set up output/input data
	const size_t num_output_channels = 1;
	vector<float> output_pixels(res * res * num_output_channels, 0.0f);
	const size_t num_input_channels = 4;
	vector<float> input_pixels(res * res * num_input_channels, 0.0f);

	// We must keep track of both the current and the previous slices, 
	// so that they can be used as input for the Marching Cubes algorithm
	vector<float> previous_slice;

	// The result of the Marching Cubes algorithm is triangles
	vector<triangle> triangles;

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

		// Use the Marching Cubes algorithm to convert the output data
		// into triangles, that is, if this isn't the first loop iteration
		if(z > 0)
		{
			tesselate_adjacent_xy_slice_pair(
				previous_slice, output_pixels,
				z - 1,
				triangles,
				threshold, // Use threshold as isovalue
				grid_min, grid_max, res,
				grid_min, grid_max, res,
				grid_min, grid_max, res);
		}

		// Tehcnically, a swap could be used here, which should be faster
		// than an assignment
		previous_slice = output_pixels;
	}

	// Write triangles to file using the common STL format
	write_triangles_to_binary_stereo_lithography_file(triangles, "out.stl");

	delete_all(tex_output, tex_input, compute_shader_program);

	return 0;
}
