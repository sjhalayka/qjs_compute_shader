// Source code by Shawn Halayka
// Source code is in the public domain


#ifndef MAIN_H
#define MAIN_H


#include "marching_cubes.h"
using namespace marching_cubes;

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <cstdlib>
using namespace std;

#include <GL/glew.h>
#include <GL/glut.h>

// Automatically link in the GLUT and GLEW libraries if compiling on MSVC++
#ifdef _MSC_VER
#pragma comment(lib, "freeglut")
#pragma comment(lib, "glew32")
#endif

void compute(GLuint& tex_output, GLuint& tex_input, 
	GLint tex_w, GLint tex_h, 
	GLuint& compute_shader_program, 
	vector<float> &output_pixels, 
	const vector<float> &input_pixels,
	GLuint max_iterations,
	GLfloat threshold,
	const quaternion C)
{
	// Copy pixel array to GPU as texture 1
	glActiveTexture(GL_TEXTURE1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, &input_pixels[0]);
	glBindImageTexture(1, tex_input, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	// Pass in the input image and quaternion Julia set parameters as uniforms
	glUniform1i(glGetUniformLocation(compute_shader_program, "input_image"), 1); // use GL_TEXTURE1
	glUniform4f(glGetUniformLocation(compute_shader_program, "C"), C.x, C.y, C.z, C.w);
	glUniform1i(glGetUniformLocation(compute_shader_program, "max_iterations"), max_iterations);
	glUniform1f(glGetUniformLocation(compute_shader_program, "threshold"), threshold);

	// Run compute shader
	glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);

	// Wait for compute shader to finish
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// Copy output pixel array to CPU as texture 0
	glActiveTexture(GL_TEXTURE0);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &output_pixels[0]);
}

void init_textures(GLuint &tex_output, GLuint &tex_input, GLuint tex_w, GLuint tex_h)
{
	// Generate and allocate output texture
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex_w, tex_h, 0, GL_RED, GL_FLOAT, NULL);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	// Generate input texture
	glGenTextures(1, &tex_input);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_input);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

bool compile_and_link_compute_shader(const char *const file_name, GLuint &program)
{
	// Read in compute shader contents
	ifstream infile(file_name);

	if (infile.fail())
	{
		cout << "Could not open " << file_name << endl;
		return false;
	}

	string shader_code;
	string line;

	while (getline(infile, line))
	{
		shader_code += line;
		shader_code += "\n";
	}

	// Compile compute shader
	const char* cch = 0;
	GLint status = GL_FALSE;

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &(cch = shader_code.c_str()), NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (GL_FALSE == status)
	{
		string status_string = "Compute shader compile error.\n";
		vector<GLchar> buf(4096, '\0');
		glGetShaderInfoLog(shader, 4095, 0, &buf[0]);

		for (size_t i = 0; i < buf.size(); i++)
			if ('\0' != buf[i])
				status_string += buf[i];

		status_string += '\n';

		cout << status_string << endl;

		glDeleteShader(shader);

		return false;
	}

	// Link compute shader
	program = glCreateProgram();
	glAttachShader(program, shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (GL_FALSE == status)
	{
		string status_string = "Program link error.\n";
		vector<GLchar> buf(4096, '\0'); 
		glGetShaderInfoLog(program, 4095, 0, &buf[0]);

		for (size_t i = 0; i < buf.size(); i++)
			if ('\0' != buf[i])
				status_string += buf[i];

		status_string += '\n';

		cout << status_string << endl;

		glDetachShader(program, shader);
		glDeleteShader(shader);
		glDeleteProgram(program);

		return false;
	}

	// The shader is no longer needed now that the program
	// has been linked
	glDetachShader(program, shader);
	glDeleteShader(shader);

	return true;
}

bool init_opengl_4_3(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA);
	int glut_window_handle = glutCreateWindow("");

	if (!(GLEW_OK == glewInit() &&
		GLEW_VERSION_4_3))
	{
		return false;
	}

	return true;
}

bool init_all(int argc, char** argv, 
	GLuint& tex_output, GLuint& tex_input, 
	GLint tex_w, GLint tex_h, 
	GLuint& compute_shader_program)
{
	// Initialize OpenGL
	if (false == init_opengl_4_3(argc, argv))
	{
		cout << "OpenGL 4.3 initialization failure" << endl;
		return false;
	}
	else
	{
		cout << "OpenGL 4.3 initialization OK" << endl;
	}

	// Check for non-POT texture support
	if (!GLEW_ARB_texture_non_power_of_two)
	{
		cout << "System does not support non-POT textures" << endl;
		return false;
	}
	else
	{
		cout << "System supports non-POT textures" << endl;
	}

	// Initialize the compute shader
	compute_shader_program = 0;

	if (false == compile_and_link_compute_shader("shader.comp", compute_shader_program))
	{
		cout << "Failed to initialize compute shader" << endl;
		return false;
	}


	cout << "Texture size: " << tex_w << "x" << tex_h << endl;

	// Check that the global workgrounp count is greater than or equal to the input/output textures
	GLint global_workgroup_count[2];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &global_workgroup_count[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &global_workgroup_count[1]);

	cout << "Max global workgroup size: " << global_workgroup_count[0] << "x" << global_workgroup_count[1] << endl;

	if (tex_w > global_workgroup_count[0])
	{
		cout << "Texture width " << tex_w << " is larger than max " << global_workgroup_count[0] << endl;
		return false;
	}

	if (tex_h > global_workgroup_count[1])
	{
		cout << "Texture height " << tex_h << " is larger than max " << global_workgroup_count[1] << endl;
		return false;
	}

	init_textures(tex_output, tex_input, tex_w, tex_h);
	
	// Use the compute shader
	glUseProgram(compute_shader_program);

	return true;
}

void delete_all(GLuint& tex_output, GLuint& tex_input, GLuint& compute_shader_program)
{
	glDeleteTextures(1, &tex_output);
	glDeleteTextures(1, &tex_input);
	glDeleteProgram(compute_shader_program);
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

	cout << "Writing " << data_size / 1048576.0f << " MB of data to binary Stereo Lithography file: " << file_name << endl;

	out.write(reinterpret_cast<const char*>(&buffer[0]), data_size);
	out.close();

	return true;
}


#endif