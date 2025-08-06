#include <vgl/AppGLBase.h>
#include <vgl/GLShader.h>

#include <fstream>
#include <sstream>
#include <tinyfiledialogs.h>

std::string shader_vs = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

out vec4 vertexColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vertexColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";
std::string shader_fs = R"(
#version 330 core
out vec4 FragColor;

in vec4 vertexColor;

void main()
{
    FragColor = vertexColor;
}
)";

class MiniGLSLSandbox : public AppGLBase
{
public:
	MiniGLSLSandbox(const int width, const int height) : AppGLBase(width, height) {};

	// mandatory callback
	void Draw(const int width, const int height)
	{
		const float size = 1.0f;

		if (use_shader) shader->Enable();

		glBegin(GL_TRIANGLES);
		glColor3f(0.0, 1.0f, 0.0f); glVertex2f(-size, -size); // left bottom
		glColor3f(1.0, 1.0f, 0.0f); glVertex2f(+size, -size); // right bottom
		glColor3f(1.0, 0.0f, 0.0f); glVertex2f(+size, +size); // right top

		glColor3f(0.0, 1.0f, 0.0f); glVertex2f(-size, -size); // left bottom
		glColor3f(1.0, 0.0f, 0.0f); glVertex2f(+size, +size); // right top
		glColor3f(0.0, 0.0f, 0.0f); glVertex2f(-size, +size); // left top
		glEnd();

		if (use_shader) shader->Disable();
	}

	////////////////////////////////////////
	// ctor & dtor
	////////////////////////////////////////
	bool Init()
	{
		shader = new vgl::GLShader();
		shader->Compile(shader_vs, vgl::SHADER_TYPE::VERTEX);
		shader->Compile(shader_fs, vgl::SHADER_TYPE::FRAGMENT);
		shader->Link();

		TwDefine("Bar size='100 50'");

		TwAddVarRW(bar, "UseShader", TwType::TW_TYPE_BOOLCPP, &use_shader, "key=SPACE");

		TwAddButton(bar, "Reload", [](void* client) {
			((MiniGLSLSandbox*)client)->ReloadShader();
		}, this, "key=L");

		return true;
	}
	void End()
	{
		if (nullptr != shader) {
			delete shader;
			shader = nullptr;
		}
	}

private:
	void ReloadShader()
	{
		// [TODO]
	}

	bool use_shader = true;
	vgl::GLShader* shader = nullptr;
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	MiniGLSLSandbox app(640, 480);
	app.SetInternalProcess(true);
	app.run();
	return EXIT_SUCCESS;
}
