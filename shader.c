#include <stdio.h>
#include <stdlib.h>
#define nullptr (void*)0
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shader.h"

struct stat;
struct stat* my_stat;

void raise(const char* msg, const char* log) {
	fprintf(stderr, "%s::%s", msg, log);
	fflush(stderr);
	exit(-1);
}

GLuint createShaderProgram(const GLchar* vertexShaderPath, const GLchar* fragmentShaderPath) {
	if (!fopen(vertexShaderPath, "rb")) {
		raise("ERROR::SHADER::VERTEX::FAILED_TO_OPEN_FILE", vertexShaderPath);
	}

	FILE* fp = fopen(vertexShaderPath, "rb");
	fseek(fp, 0L, SEEK_END);
	long flength = ftell(fp);

	GLchar* _vertexShaderSource = (GLchar*) calloc(sizeof(GLchar), (flength+1));
	fread(_vertexShaderSource, sizeof(GLchar), flength, fopen(vertexShaderPath, "rb"));
	const GLchar* vertexShaderSource = _vertexShaderSource;

	GLint success;
	GLchar infoLog[512];
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		raise("ERROR::SHADER::VERTEX::FAILED_TO_COMPILE_SHADER", infoLog);
	}

	if (!fopen(fragmentShaderPath, "rb")) {
		raise("ERROR::SHADER::FRAGMENT::FAILED_TO_OPEN_FILE", fragmentShaderPath);		
	}

	fp = fopen(fragmentShaderPath, "rb");
	fseek(fp, 0L, SEEK_END);
	flength = ftell(fp);

	GLchar* _fragmentShaderSource = (GLchar*) calloc(sizeof(GLchar), (flength+1));
	fread(_fragmentShaderSource, sizeof(GLchar), flength, fopen(fragmentShaderPath, "rb"));
	const GLchar* fragmentShaderSource = _fragmentShaderSource;

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		raise("ERROR::SHADER::FRAGMENT::FAILED_TO_COMPILE_SHADER", infoLog);
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		raise("ERROR::PROGRAM::FAILED_TO_LINK_PROGRAM", infoLog);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}