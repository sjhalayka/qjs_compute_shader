#include "main.h"


int main(int argc, char **argv)
{
	// Set up grid parameters
	const size_t res = 30;
	const float x_grid_max = 1.5;
	const float y_grid_max = 1.5;
	const float z_grid_max = 1.5;
	const float x_grid_min = -x_grid_max;
	const float y_grid_min = -y_grid_max;
	const float z_grid_min = -z_grid_max;
	const size_t x_res = res;
	const size_t y_res = res;
	const size_t z_res = res;
	const float x_step_size = (x_grid_max - x_grid_min) / (x_res - 1);
	const float y_step_size = (y_grid_max - y_grid_min) / (y_res - 1);
	const float z_step_size = (z_grid_max - z_grid_min) / (z_res - 1);

	// Initialize OpenGL / compute shader / textures
	GLuint compute_shader_program = 0;
	GLuint tex_output = 0, tex_input = 0;

	if (false == init_all(argc, argv, tex_output, tex_input, x_res, y_res, compute_shader_program))
	{
		cout << "Aborting" << endl;
		return 0;
	}

	// Set up quaternion Julia set parameters
	const quaternion C(0.3f, 0.5f, 0.4f, 0.2f);
	const int max_iterations = 8;
	const float threshold = 4.0f;

	quaternion Z(x_grid_min, y_grid_min, z_grid_min, 0.0);

	vector<float> input_pixels(x_res * y_res * 4, 0.0f);
	vector<float> output_pixels(x_res * y_res * 4, 0.0f);

	// For each z slice...
	for (size_t z = 0; z < z_res; z++, Z.z += z_step_size)
	{
		cout << "Z slice " << z + 1 << " of " << z_res << endl;

		Z.x = x_grid_min;

		// Create pixel array
		for (size_t x = 0; x < x_res; x++, Z.x += x_step_size)
		{
			Z.y = y_grid_min;

			for (size_t y = 0; y < y_res; y++, Z.y += y_step_size)
			{
				const size_t index = 4*(y * x_res + x);

				input_pixels[index + 0] = Z.x;
				input_pixels[index + 1] = Z.y;
				input_pixels[index + 2] = Z.z; 
				input_pixels[index + 3] = Z.w;
			}
		}

		// Run the compute shader
		compute(tex_output, tex_input,
			x_res, y_res,
			compute_shader_program,
			input_pixels,
			output_pixels,
			max_iterations,
			threshold,
			C); 

		//// Print slice
		//for (size_t x = 0; x < x_res; x++)
		//{
		//	for (size_t y = 0; y < y_res; y++)
		//	{
		//		const size_t index = 4 * (y * x_res + x);

		//		float magnitude = output_pixels[index];

		//		if (magnitude < threshold)
		//			cout << "*";
		//		else
		//			cout << ".";
		//	}

		//	cout << endl;
		//}

		//cout << endl << endl;
	}

	return 0;
}
