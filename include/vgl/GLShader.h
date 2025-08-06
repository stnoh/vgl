/******************************************************************************
Simple GLSL Shader management code
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef GL_SHADER_MANAGEMENT
#define GL_SHADER_MANAGEMENT

#include <GL/glew.h>

#include <string>
#include <functional>

namespace vgl {

enum SHADER_TYPE { VERTEX=0, FRAGMENT, GEOMETRY, TESSELATION };

class GLShader
{
public:
	GLShader();
	~GLShader();

	void Clear();
	void Compile(const std::string& shaderCode, SHADER_TYPE type);
	bool Link();

	void Enable()  { glUseProgram(program); }
	void Disable() { glUseProgram(0); }

	// lambda guard for enable/disable
	void DrawShader(std::function<void(void)> func)
	{
		Enable();
		func();
		Disable();
	}

	GLint GetUniformLocation(const char* variable){
		GLint loc = glGetUniformLocation(program, variable);
		return loc;
	}

private:
	GLuint program;
	GLuint shader_vert;
	GLuint shader_frag;
};

}

#endif
