#include <vgl/AppGLBase.h>
#include <vgl/GLShader.h>

#include <chrono>

#include <fstream>
#include <sstream>
#include <tinyfiledialogs.h>

std::string shader_vs = R"(
void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
)";
std::string shader_fs = R"(
uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main()
{
	gl_FragColor = vec4(gl_FragCoord.xy / resolution.xy, 0.0, 1.0);
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

			// elapsed time in seconds
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - time_stamp);
			float time = 1e-6f * microseconds.count();

			// normalized mouse position
			float x = mouse_this_x / (float)width;
			float y = 1.0f - mouse_this_y / (float)height;

			// pass data to shader
			GLuint loc;
			loc = shader->GetUniformLocation("time");
			if (-1 != loc) glUniform1f(loc, time);
			//printf("time: %f [sec]\r", time);

			loc = shader->GetUniformLocation("mouse");
			if (-1 != loc) glUniform2f(loc, x, y);
			//printf("mouse: %7.3f %7.3f\r", x, y);

			loc = shader->GetUniformLocation("resolution");
			if (-1 != loc) glUniform2f(loc, (float)width, (float)height);
			//printf("resolution: %4d %4d\r", width, height);

			// [TODO] "surfaceSize"

			// [TODO] "backbuffer"
		}

		// bluish color to show non-shaded image
		glBegin(GL_TRIANGLES);
		glColor3f(0.0, 0.0f, 0.5f); glVertex2f(-size, -size); // left bottom
		glColor3f(1.0, 0.0f, 0.5f); glVertex2f(+size, -size); // right bottom
		glColor3f(1.0, 1.0f, 0.5f); glVertex2f(+size, +size); // right top

		glColor3f(0.0, 0.0f, 0.5f); glVertex2f(-size, -size); // left bottom
		glColor3f(1.0, 1.0f, 0.5f); glVertex2f(+size, +size); // right top
		glColor3f(0.0, 1.0f, 0.5f); glVertex2f(-size, +size); // left top
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

		time_stamp = std::chrono::system_clock::now();

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
		char* filepath = tinyfd_openFileDialog(
			"Read .glsl shader for pixel shader",
			"./", 0, NULL, NULL, 0);

		if (filepath)
		{
			printf("filepath: %s\n", filepath);

			std::ifstream shader_fs_file;
			shader_fs_file.open(filepath);
			std::stringstream shader_fs_stream;
			shader_fs_stream << shader_fs_file.rdbuf();
			shader_fs_file.close();

			shader->Compile(shader_vs, vgl::SHADER_TYPE::VERTEX);
			shader->Compile(shader_fs_stream.str(), vgl::SHADER_TYPE::FRAGMENT);

			if (!shader->Link())
			{
				fprintf(stderr, "ERROR: revert to default shader.\n");

				// revert to default shader
				shader->Compile(shader_vs, vgl::SHADER_TYPE::VERTEX);
				shader->Compile(shader_fs, vgl::SHADER_TYPE::FRAGMENT);
				shader->Link();
			}
			else {
				time_stamp = std::chrono::system_clock::now();
			}
		}

	}

	bool use_shader = true;
	std::chrono::system_clock::time_point time_stamp;

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
