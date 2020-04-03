#pragma once
#include "include/glad.h"

class ShaderProgram {
public:
	ShaderProgram();
	GLuint Compile();
	ShaderProgram AddFrag(char* filename);
	ShaderProgram AddVert(char* filename);
	ShaderProgram AddGeom(char* filename);
	ShaderProgram AddTesc(char* filename);
	ShaderProgram AddTesev(char* filename);
	ShaderProgram AddComp(char* filename);
private:
	GLuint shader_program;
	ShaderProgram AddShader(GLenum type, char* filename);
};