#include "ShaderProgram.h"
#include "LoadFile.h"

#include <string.h>
#include <iostream>
#include <string>

ShaderProgram::ShaderProgram() {
	shader_program = glCreateProgram();
};

GLuint ShaderProgram::Compile() {
	glLinkProgram(shader_program);

	GLint success;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar log[512];
		glGetProgramInfoLog(shader_program, 512, NULL, log);
		std::cout << "Failed to link program\n" << log << std::endl;
		exit(1);
	}

	return shader_program;
}

ShaderProgram ShaderProgram::AddFrag(char* filename)  { return AddShader(GL_FRAGMENT_SHADER,		filename); }
ShaderProgram ShaderProgram::AddVert(char* filename)  { return AddShader(GL_VERTEX_SHADER,			filename); }
ShaderProgram ShaderProgram::AddGeom(char* filename)  { return AddShader(GL_GEOMETRY_SHADER,		filename); }
ShaderProgram ShaderProgram::AddTesc(char* filename)  { return AddShader(GL_TESS_CONTROL_SHADER,	filename); }
ShaderProgram ShaderProgram::AddTesev(char* filename) { return AddShader(GL_TESS_EVALUATION_SHADER, filename); }
ShaderProgram ShaderProgram::AddComp(char* filename)  { return AddShader(GL_COMPUTE_SHADER,			filename); }

ShaderProgram ShaderProgram::AddShader(GLenum type, char* filename) {
	std::string sourceStr;
	load_file(filename, &sourceStr);
	const GLchar* source = sourceStr.c_str();

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar log[512];
		glGetProgramInfoLog(shader, 512, NULL, log);
		std::cout << "Failed to compile shader" << log << std::endl;
		exit(1);
	}

	glAttachShader(shader_program, shader);
	glDeleteShader(shader);

	return *this;
}