#include <vgl/AppGLBase.h>
#include <vgl/GLShader.h>

#include <glm/ext.hpp>

#include <chrono>
#include <iostream>

#include <fstream>
#include <sstream>
#include <tinyfiledialogs.h>

std::string shader_vs = R"(
#version 330 core
layout(location=0) in vec2 vert;
layout(location=1) in vec2 surfacePosAttrib;
out vec2 surfacePosition;

uniform mat4 mvp;

void main()
{
	surfacePosition = surfacePosAttrib;
	gl_Position = mvp * vec4(vert, 0.0f, 1.0f);
}
)";
std::string shader_fs = R"(
#version 330 core

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
		float aspect = (float)width / (float)height;

		GLuint loc = -1;

		if (use_shader) {
			shader->Enable();

			// default projection * view * model
			glm::mat4 identity = glm::mat4(1.0f);

			// elapsed time in seconds
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now - time_stamp);
			float time = 1e-6f * microseconds.count();

			// normalized mouse position
			float x = mouse_this_x / (float)width;
			float y = 1.0f - mouse_this_y / (float)height;

			// pass data to shader
			loc = shader->GetUniformLocation("mvp");
			if (-1 != loc) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(identity));

			loc = shader->GetUniformLocation("time");
			if (-1 != loc) glUniform1f(loc, time);
			//printf("time: %f [sec]\r", time);

			loc = shader->GetUniformLocation("mouse");
			if (-1 != loc) glUniform2f(loc, x, y);
			//printf("mouse: %7.3f %7.3f\r", x, y);

			loc = shader->GetUniformLocation("resolution");
			if (-1 != loc) glUniform2f(loc, (float)width, (float)height);
			//printf("resolution: %4d %4d\r", width, height);

			loc = shader->GetUniformLocation("surfaceSize");
			if (-1 != loc) glUniform2f(loc, aspect, 1.0f);
			//printf("surfaceSize: %7.3f %7.3f\r", width, 1.0f);

			// [CAUTION] last query should be "backbuffer"
			loc = shader->GetUniformLocation("backbuffer");
			if (-1 != loc && 0 != renderTexture) {
				// the first texture
				glActiveTexture(GL_TEXTURE0 + 0); // [IMPORTANT]
				glBindTexture(GL_TEXTURE_2D, renderTexture);
				glUniform1i(loc, 0);
			}
		}

		// compute 
		float L = center_x - 0.5f * fov * aspect;
		float R = center_x + 0.5f * fov * aspect;
		float B = center_y - 0.5f * fov;
		float T = center_y + 0.5f * fov;

		// bluish color to show non-shaded image
		auto DrawQuad = [&] {
			glBegin(GL_TRIANGLES);
			glColor3f(0.0, 0.0f, 0.5f); glVertexAttrib2f(1, L, B); glVertex2f(-size, -size); // left bottom
			glColor3f(1.0, 0.0f, 0.5f); glVertexAttrib2f(1, R, B); glVertex2f(+size, -size); // right bottom
			glColor3f(1.0, 1.0f, 0.5f); glVertexAttrib2f(1, R, T); glVertex2f(+size, +size); // right top

			glColor3f(0.0, 0.0f, 0.5f); glVertexAttrib2f(1, L, B); glVertex2f(-size, -size); // left bottom
			glColor3f(1.0, 1.0f, 0.5f); glVertexAttrib2f(1, R, T); glVertex2f(+size, +size); // right top
			glColor3f(0.0, 1.0f, 0.5f); glVertexAttrib2f(1, L, T); glVertex2f(-size, +size); // left top
			glEnd();
		};

		DrawQuad();

		if (use_shader) {

			// render again for "backbuffer"
			if (-1 != loc) {
				if (0 == renderTexture || width != tex_width || height != tex_height) {
					if (0 != renderTexture) glDeleteTextures(1, &renderTexture);

					tex_width  = width;
					tex_height = height;

					// create texture with settings
					renderTexture = 0;
					glGenTextures(1, &renderTexture);
					glBindTexture(GL_TEXTURE_2D, renderTexture);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					
					// allocate texture
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
				}

				// render to texture for "backbuffer"
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
					std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

				DrawQuad();
				glBindFramebuffer(GL_FRAMEBUFFER, 0); // revert target to display
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			shader->Disable();
		}
	}

	////////////////////////////////////////
	// ctor & dtor
	////////////////////////////////////////
	bool Init()
	{
		// shader loader
		shader = new vgl::GLShader();
		shader->Compile(shader_vs, vgl::SHADER_TYPE::VERTEX);
		shader->Compile(shader_fs, vgl::SHADER_TYPE::FRAGMENT);
		shader->Link();

		// prepare FBO for "backbuffer"
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // revert target to display

		// initialize interaction data
		InitUserInteraction();
		glfwSetWindowTitle(window, "MiniGLSLSandbox: default");

		TwDefine("Bar size='150 250'");
		TwAddVarRW(bar, "UseShader", TwType::TW_TYPE_BOOLCPP, &use_shader, "key=SPACE");
		TwAddButton(bar, "LoadShader", [](void* client) {
			((MiniGLSLSandbox*)client)->LoadShader();
		}, this, "key=L");
		TwAddVarRW(bar, "fov", TwType::TW_TYPE_FLOAT, &fov, "min=0.001 step=0.001");
		TwAddVarRW(bar, "center_x", TwType::TW_TYPE_FLOAT, &center_x, "step=0.001");
		TwAddVarRW(bar, "center_y", TwType::TW_TYPE_FLOAT, &center_y, "step=0.001");

		return true;
	}
	void End()
	{
		if (nullptr != shader) {
			delete shader;
			shader = nullptr;
		}

		if (renderTexture) {
			glDeleteTextures(1, &renderTexture);
			renderTexture = 0;
		}

		if (fbo) {
			glDeleteFramebuffers(1, &fbo);
			fbo = 0;
		}
	}

	bool Update()
	{
		float aspect = (float)width / (float)height;

		int diff_x = mouse_this_x - mouse_prev_x;
		int diff_y = mouse_this_y - mouse_prev_y;

		// left drag
		if (mouse_button[0]) {
			center_x -= fov * (float)diff_x / (float)height;
			center_y += fov * (float)diff_y / (float)width * aspect;
		}

		// middle drag
		if (mouse_button[2]) {
			fov *= pow(0.997, diff_x + diff_y);
		}

		mouse_prev_x = mouse_this_x;
		mouse_prev_y = mouse_this_y;
		return true;
	}

private:
	void LoadShader()
	{
		char const* filterPatterns[1] = { "*.frag" };

		char* filepath = tinyfd_openFileDialog(
			"Read .frag shader for GLSL fragment shader",
			"./", 1, filterPatterns, NULL, 0);

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

				glfwSetWindowTitle(window, "MiniGLSLSandbox: default");
			}
			else {
				InitUserInteraction();
				char buf[256];
				sprintf(buf, "MiniGLSLSandbox: %s", filepath);
				glfwSetWindowTitle(window, buf);
			}
		}
	}

	bool use_shader = true;


	////////////////////////////////////////
	// user interaction
	////////////////////////////////////////
	void InitUserInteraction()
	{
		// initialize user interaction data
		center_x = 0.0f;
		center_y = 0.0f;
		fov = 1.0f;
		time_stamp = std::chrono::system_clock::now();
	}
	float center_x;
	float center_y;
	float fov;

	// reserve previous mouse position
	int mouse_prev_x;
	int mouse_prev_y;

	std::chrono::system_clock::time_point time_stamp;

	vgl::GLShader* shader = nullptr;

	// FBO and texture for "backbuffer"
	GLuint fbo = 0;
	GLuint renderTexture = 0;
	int tex_width  = -1;
	int tex_height = -1;
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
