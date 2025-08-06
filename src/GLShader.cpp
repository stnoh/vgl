/******************************************************************************
Simple GLSL Shader management code
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vgl/GLShader.h>

using namespace vgl;


///////////////////////////////////////////////////////////////////////////////
// ctor / dtor
///////////////////////////////////////////////////////////////////////////////
GLShader::GLShader()
{
	program = 0;
	shader_vert = 0;
	shader_frag = 0;
}
GLShader::~GLShader()
{
	Clear();
}


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
void GLShader::Clear()
{
	if (0 != shader_vert) {
		glDetachShader(program, shader_vert);
		glDeleteShader(shader_vert);
	}
	shader_vert = 0;

	if (0 != shader_frag) {
		glDetachShader(program, shader_frag);
		glDeleteShader(shader_frag);
	}
	shader_frag = 0;

	if (0 != program) {
		glDeleteProgram(program);
	}
	program = 0;
}
void GLShader::Compile(const std::string& shaderCode, SHADER_TYPE type)
{
	GLuint shader_type;
	GLuint* hShader;

	if (SHADER_TYPE::VERTEX == type) {
		hShader = &shader_vert;
		shader_type = GL_VERTEX_SHADER;
	}
	else if (SHADER_TYPE::FRAGMENT == type) {
		hShader = &shader_frag;
		shader_type = GL_FRAGMENT_SHADER;
	}
	else {
		fprintf(stderr, "ERROR: incompatible shader type.\n");
		return;
	}
	*hShader = glCreateShader(shader_type);

	const char* code = shaderCode.c_str();
	//printf("%s", code);
	glShaderSource(*hShader, 1, &code, NULL);
	glCompileShader(*hShader);

	GLint success;
	glGetShaderiv(*hShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[1024];
		glGetShaderInfoLog(*hShader, 1024, NULL, infoLog);
		fprintf(stderr, "ERROR: %s", infoLog);

		glDeleteShader(*hShader);
		*hShader = 0;
	}
}
bool GLShader::Link()
{
	if (0 == shader_vert || 0 == shader_frag) {
		fprintf(stderr, "ERROR: vertex or fragment shader is not loaded yet.\n");
		return false;
	}

	if (0 != program) {
		glDeleteProgram(program);
	}
	program = glCreateProgram();

	glAttachShader(program, shader_vert);
	glAttachShader(program, shader_frag);
	glLinkProgram(program);

	// check shader
	GLint success;
	glGetShaderiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[1024];
		glGetShaderInfoLog(program, 1024, NULL, infoLog);
		fprintf(stderr, "ERROR: %s", infoLog);

		Clear();
		return false;
	}

	printf("Loaded: shader program.\n");
	return true;
}
