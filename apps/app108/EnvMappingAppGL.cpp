#include <glm/ext.hpp>

#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>
#include <vgl/TriMesh.h>
#include <vgl/GLShader.h>

#include <tinyfiledialogs.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const char* vertShader_modern = R"(
#version 330 
layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;

uniform mat4 Projection;
uniform mat4 ModelView;

out vec3 wPosition;
out vec3 wDirection;
out vec3 wCameraPos;

void main()
{
	gl_Position = Projection * ModelView * vec4(vPosition, 1.0);
	wDirection = normalize(vNormal);

	wPosition = vPosition;
	wCameraPos = vec3(-ModelView[3] * ModelView);
}
)";

const char* vertShader_legacy = R"(
#version 120
uniform mat4 Projection;
uniform mat4 ModelView;

varying vec3 wDirection;

void main()
{
	gl_Position = Projection * ModelView * gl_Vertex;
	wDirection = normalize(gl_Vertex.xyz);
}
)";

const char* fragShader_envmap = R"(
#version 330 
in vec3 wPosition;
in vec3 wDirection;
in vec3 wCameraPos;

out vec4 fragColor;

uniform bool isBackground;
uniform samplerCube skyboxCubemap;

void main()
{
	if (isBackground){
		fragColor = texture(skyboxCubemap, wDirection);
	}
	else{
		vec3 V = normalize(wPosition - wCameraPos);
		vec3 N = normalize(wDirection);
		vec3 R = reflect(V, N);

		//fragColor = vec4(0.5 * wDirection + 0.5, 1);
		//fragColor = vec4(0.5 * R + 0.5, 1);
		fragColor = texture(skyboxCubemap, R);
	}
}
)";

class EnvMappingAppGL : public AppGLBase
{
public:
	EnvMappingAppGL(const int width, const int height) : AppGLBase(width, height) {};

	// mandatory callback
	void Draw(const int width, const int height)
	{
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::infinitePerspective(glm::radians(53.1301f), width / (float)height, 0.1f);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
		view = view * glm::mat4_cast(GlobalViewRotation);

		glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

		////////////////////////////////////////////////////////////
		// 1st: draw IcoSphere with shader
		////////////////////////////////////////////////////////////
		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glPolygonMode(GL_FRONT, GL_FILL);

		glEnable(GL_TEXTURE_CUBE_MAP);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

		if (show_object) {
			shader_modern.DrawShader([&]() {
				GLint loc;
				loc = shader_modern.GetUniformLocation("Projection");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
				loc = shader_modern.GetUniformLocation("ModelView");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
				loc = shader_modern.GetUniformLocation("skyboxCubemap");
				glUniform1i(loc, 0);
				loc = shader_modern.GetUniformLocation("isBackground");
				glUniform1i(loc, 0);

				// set vertices with normals
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &icosphere.vertices[0]);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, &icosphere.normals[0]);
				glEnableVertexAttribArray(1);

				vgl::drawTriMesh(icosphere.vertices, icosphere.normals, icosphere.faces);
			});
		}

		////////////////////////////////////////////////////////////
		// 2nd: draw empty pixels by skybox
		////////////////////////////////////////////////////////////
		glDepthFunc(GL_LEQUAL);
		glPolygonMode(GL_BACK, GL_FILL);

		if (show_skybox) {
			shader_legacy.DrawShader([&]() {
				GLint loc;
				loc = shader_legacy.GetUniformLocation("Projection");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
				loc = shader_legacy.GetUniformLocation("ModelView");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
				loc = shader_legacy.GetUniformLocation("skyboxCubemap");
				glUniform1i(loc, 0);
				loc = shader_legacy.GetUniformLocation("isBackground");
				glUniform1i(loc, 1);

				vgl::drawCube(1e5f); // draw large enough cube
			});
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}

	void InitCubemap()
	{
		// prepare cubemap
		glEnable(GL_TEXTURE_CUBE_MAP);
		glGenTextures(1, &cubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

		for (int n = 0; n < 6; n++)
		{
			const int block_sz = 32;
			const int W = 256;
			const int H = 256;
			GLubyte* data = (GLubyte*)malloc(W * H * 3 * sizeof(GLubyte));

			for (int j = 0; j < H; j++) 
			for (int i = 0; i < W; i++)
			{
				int bx = i / block_sz;
				int by = j / block_sz;

				// determine pixel color
				GLubyte R, G, B;
				GLubyte vMax = 255;
				GLubyte vMin = 0;

				switch (n) {
				case 0: R =    vMax; G = 255 - j; B = 255 - i; break;
				case 1: R =    vMin; G = 255 - j; B =       i; break;
				case 2: R =       i; G =    vMax; B =       j; break;
				case 3: R =       i; G =    vMin; B = 255 - j; break;
				case 4: R =       i; G = 255 - j; B =    vMax; break;
				case 5: R = 255 - i; G = 255 - j; B =    vMin; break;
				}

				// darkgray check pattern
				if (0 == (bx + by) % 2) R = G = B = 64;

				// fill the pixel values
				int pidx = i + j * W;
				data[3 * pidx + 0] = R;
				data[3 * pidx + 1] = G;
				data[3 * pidx + 2] = B;
			}

			// set texture
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, 0, GL_RGB,
				W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);

			// release image data
			free(data);
			data = NULL;
		}

		// set texture parameters
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// unbind cubemap for another process
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}

	void LoadCubemap()
	{
		char* dirpath = tinyfd_selectFolderDialog(
			"Set directory that contains cubemap images files", "./");

		if (dirpath)
		{
			// [TEMPORARY] naming for cubemap images
			std::vector<std::string> files{
				"posx.jpg",
				"negx.jpg",
				"posy.jpg",
				"negy.jpg",
				"posz.jpg",
				"negz.jpg"
			};

			// prepare cubemap
			glEnable(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

			for (int n = 0; n < files.size(); n++)
			{
				char buf[256];
				sprintf_s(buf, "%s\\%s", dirpath, files[n].c_str());
				printf("load file: %s\n", buf);

				// load image and set texture
				int w, h, c;
				unsigned char* data = stbi_load(buf, &w, &h, &c, STBI_rgb);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, &data[0]);
				stbi_image_free(data);
			}

			// set texture parameters
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			// unbind cubemap for another process
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glDisable(GL_TEXTURE_CUBE_MAP);
		}
	}

	bool Init()
	{
		SetAppGLTitle("EnvMappingApp");

		TwAddButton(bar, "load_cubemap", [](void* client) {
			EnvMappingAppGL* _this = (EnvMappingAppGL*)client;
			_this->LoadCubemap();
		}, this, " ");

		// global viewer
		resetGlobalView();
#if 1
		TwAddButton(bar, "Global-init", [](void* client) {
			EnvMappingAppGL* _this = (EnvMappingAppGL*)client; _this->resetGlobalView();
			}, this, "group='Global' label='init' ");
		TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation, "group='Global' label='rot'  open");
#endif

		TwAddVarRW(bar, "show_object", TwType::TW_TYPE_BOOLCPP, &show_object, "group='Global' label='show_object'");
		TwAddVarRW(bar, "show_skybox", TwType::TW_TYPE_BOOLCPP, &show_skybox, "group='Global' label='show_skybox'");

		// prepare icosphere to test rendering
		icosphere = vgl::IcoSphere(3);

		// prepare shader for environment mapping with cubemap
		shader_modern = vgl::GLShader();
		shader_modern.Compile(vertShader_modern, vgl::SHADER_TYPE::VERTEX);
		shader_modern.Compile(fragShader_envmap, vgl::SHADER_TYPE::FRAGMENT);
		shader_modern.Link();

		// prepare shader for cubemap rendering
		shader_legacy = vgl::GLShader();
		shader_legacy.Compile(vertShader_legacy, vgl::SHADER_TYPE::VERTEX);
		shader_legacy.Compile(fragShader_envmap, vgl::SHADER_TYPE::FRAGMENT);
		shader_legacy.Link();

		InitCubemap();

		return true;
	}

	void End()
	{
		glDeleteTextures(1, &cubemap);
	}

	vgl::IcoSphere icosphere;

private:
	bool show_object = true;
	bool show_skybox = true;

	GLuint cubemap = 0;

	// global viewer
	glm::quat GlobalViewRotation = glm::quat();
	void resetGlobalView() {
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
	}

	vgl::GLShader shader_modern; // environment mapping (for modern GL)
	vgl::GLShader shader_legacy; // environment mapping (for legacy GL)
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	EnvMappingAppGL app(640, 480);
	app.run();
	return EXIT_SUCCESS;
}
