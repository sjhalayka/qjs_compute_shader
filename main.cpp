#include "main.h"


int main(int argc, char **argv)
{
	// Initialize OpenGL
	if (false == init_opengl_4_3(argc, argv))
	{
		cout << "OpenGL 4.3 initialization failure" << endl;
		return 0;
	}
	else
	{
		cout << "OpenGL 4.3 initialization OK" << endl;
	}

	// Check for non-POT texture support
	if (!GLEW_ARB_texture_non_power_of_two)
	{
		cout << "System does not support non-POT textures" << endl;
		return 0;
	}
	else
	{
		cout << "System supports non-POT textures" << endl;
	}

	// Initialize the compute shader
	GLuint compute_shader_program = 0;

	if (false == compile_and_link_compute_shader("shader.comp", compute_shader_program))
	{
		cout << "Failed to initialize compute shader" << endl;
		return 0;
	}

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

	// Initialize the textures
	GLuint tex_output = 0, tex_input = 0;
	init_textures(tex_output, tex_input, x_res, y_res);

	cout << "Texture size: " << x_res << "x" << y_res << endl;

	// Check that the global workgrounp count is greater than or equal to the input/output textures
	int global_workgroup_count[2];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &global_workgroup_count[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &global_workgroup_count[1]);

	cout << "Max global workgroup size: " << global_workgroup_count[0] << "x" << global_workgroup_count[1] << endl;

	if (x_res > global_workgroup_count[0])
	{
		cout << "Texture size " << x_res << " is larger than max " << global_workgroup_count[0] << endl;
		return 0;
	}

	if (y_res > global_workgroup_count[1])
	{
		cout << "Texture size " << y_res << " is larger than max " << global_workgroup_count[1] << endl;
		return 0;
	}

	// Set up more grid parameters
	const float x_step_size = (x_grid_max - x_grid_min) / (x_res - 1);
	const float y_step_size = (y_grid_max - y_grid_min) / (y_res - 1);
	const float z_step_size = (z_grid_max - z_grid_min) / (z_res - 1);

	// Set up set parameters
	const quaternion C(0.3f, 0.5f, 0.4f, 0.2f);
	const int max_iterations = 8;
	const float threshold = 4.0;
	const float z_w = 0;

	quaternion Z(x_grid_min, y_grid_min, z_grid_min, z_w);

	// For each z slice...
	for (size_t z = 0; z < z_res; z++, Z.z += z_step_size)
	{
		cout << "Z slice " << z + 1 << " of " << z_res << endl;

		Z.x = x_grid_min;

		vector<float> input_pixels(x_res * y_res * 4, 0.0f);

		// Create pixel array
		for (size_t x = 0; x < x_res; x++, Z.x += x_step_size)
		{
			Z.y = y_grid_min;

			for (size_t y = 0; y < y_res; y++, Z.y += y_step_size)
			{
				size_t index = 4*(y * x_res + x);

				input_pixels[index + 0] = Z.x;
				input_pixels[index + 1] = Z.y;
				input_pixels[index + 2] = Z.z; 
				input_pixels[index + 3] = Z.w;
			}
		}

		
		// Copy pixel array to GPU as texture 1
		glActiveTexture(GL_TEXTURE1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, x_res, y_res, 0, GL_RGBA, GL_FLOAT, &input_pixels[0]);
		glBindImageTexture(1, tex_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

		glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		// Pass in the input image as a uniform
		glUniform1i(glGetUniformLocation(compute_shader_program, "input_image"), 1); // use GL_TEXTURE1
		
		glUniform4f(glGetUniformLocation(compute_shader_program, "C"), C.x, C.y, C.z, C.w);
		glUniform1i(glGetUniformLocation(compute_shader_program, "max_iterations"), max_iterations);
		glUniform1f(glGetUniformLocation(compute_shader_program, "threshold"), threshold);

		// Use the compute shader
		glUseProgram(compute_shader_program);

		// Run compute shader
		glDispatchCompute((GLuint)x_res, (GLuint)y_res, 1);

		// Wait for compute shader to finish
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// Copy output pixel array to CPU as texture 0
		vector<float> output_pixels(x_res * y_res * 4, 0.0f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_output);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &output_pixels[0]);

		// Print slice
		for (size_t x = 0; x < x_res; x++)
		{
			for (size_t y = 0; y < y_res; y++)
			{
				size_t index = 4 * (y * x_res + x);

				//cout << "0 " << input_pixels[index + 0] << " " << output_pixels[index + 0] << endl;
				//cout << "1 " << input_pixels[index + 1] << " " << output_pixels[index + 1] << endl;
				//cout << "2 " << input_pixels[index + 2] << " " << output_pixels[index + 2] << endl;
				//cout << "3 " << input_pixels[index + 3] << " " << output_pixels[index + 3] << endl;

				float magnitude = output_pixels[index + 0];

				if (magnitude < threshold)
					cout << "*";
				else
					cout << ".";
			}

			cout << endl;
		}

		cout << endl << endl;
	}

	return 0;
}
