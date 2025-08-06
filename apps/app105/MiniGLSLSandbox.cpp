#include <vgl/AppGLBase.h>
#include <vgl/GLShader.h>

#include <fstream>
#include <sstream>
#include <tinyfiledialogs.h>

std::string shader_vs = R"(
#version 120

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
)";
std::string shader_fs = R"(
#version 120

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main()
{
	//gl_FragColor = vec4(gl_TexCoord[0].x, gl_TexCoord[0].y, 0.0, 1.0);

	vec2 position = gl_TexCoord[0].xy + mouse / 4.0;

	float color = 0.0;
	color += sin( position.x * cos( time / 15.0 ) * 80.0 ) + cos( position.y * cos( time / 15.0 ) * 10.0 );
	color += sin( position.y * sin( time / 10.0 ) * 40.0 ) + cos( position.x * sin( time / 25.0 ) * 40.0 );
	color += sin( position.x * sin( time / 5.0 ) * 10.0 ) + sin( position.y * sin( time / 35.0 ) * 80.0 );
	color *= sin( time / 10.0 ) * 0.5;

	gl_FragColor = vec4(color, color * 0.5, sin( color + time / 3.0 ) * 0.75, 1.0);
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

		if (use_shader) {
			shader->Enable();
			
			// pass data to shader ...
			GLuint loc;
			loc = shader->GetUniformLocation("time");
			glUniform1f(loc, time);
			//printf("time: %f\r", time);
			time += 0.001f; // [TEMPORARY]

			loc = shader->GetUniformLocation("mouse");
			glUniform2f(loc, mouse_this_x / (float)width, mouse_this_y / (float)height);
			//printf("mouse: %f %f\r", mouse_this_x / (float)width, mouse_this_y / (float)height);

			loc = shader->GetUniformLocation("resolution");
			glUniform2f(loc, width, height);
			//printf("resolution: %f %f\r", (float)width, (float)height);
		}

		glBegin(GL_TRIANGLES);
		glColor3f(1.0, 0.0f, 1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, -size, 0.0f); // left bottom
		glColor3f(0.0, 0.0f, 1.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(+size, -size, 0.0f); // right bottom
		glColor3f(0.0, 1.0f, 1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(+size, +size, 0.0f); // right top

		glColor3f(1.0, 0.0f, 1.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, -size, 0.0f); // left bottom
		glColor3f(0.0, 1.0f, 1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(+size, +size, 0.0f); // right top
		glColor3f(1.0, 1.0f, 1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, +size, 0.0f); // left top
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

		time = 0.0f;

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
		time = 0.0f;
	}

	bool use_shader = true;
	float time = 0.0f;

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
