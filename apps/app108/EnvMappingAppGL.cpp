#include <glm/ext.hpp>

#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>
#include <vgl/TriMesh.h>
#include <vgl/GLShader.h>
#include <vgl/FBO.h>

#include <tinyfiledialogs.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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

		//fragColor = vec4(0.5 * wDirection + 0.5, 1); // normal vector from object
		//fragColor = vec4(0.5 * R + 0.5, 1); // computed reflection vector
		fragColor = texture(skyboxCubemap, R);
	}
}
)";

const char* vertShader_equirect = R"(
#version 120
varying vec2 surfacePosition;

void main()
{
	gl_Position = gl_Vertex;
	surfacePosition = gl_Vertex.xy;
}
)";

const char* fragShader_equirect = R"(
#version 330 
in vec2 surfacePosition;

out vec4 fragColor;

uniform mat4 rot;
uniform sampler2D equirect;

void main()
{
	// compute direction
	vec4 dir = vec4(surfacePosition, -1.0, 1.0);
	dir = rot * dir;
	dir = normalize(dir);
	
	// from equirectanglar coordinate to spherical direction
	const float PI = 3.1415926;
	float Lat  = acos(-dir.y) / PI; // [CAUTION] Y-down for image
	float Long = atan(dir.x, dir.z) / (2.0 * PI) + 0.5;
	
	fragColor = texture(equirect, vec2(Long, Lat));
}
)";

class EnvMappingAppGL : public AppGLBase
{
public:

	// synchronize naming in https://www.humus.name/index.php?page=Textures
	std::vector<std::string> files{
		"posx.jpg",
		"negx.jpg",
		"posy.jpg",
		"negy.jpg",
		"posz.jpg",
		"negz.jpg"
	};

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

		// set cubemap as texture #0
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
				glUniform1i(loc, 0); // 0 for GL_TEXTURE0
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
				glUniform1i(loc, 0); // 0 for GL_TEXTURE0
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

				switch (n) {
				case 0: R = 255; G = 255 - j; B = 255 - i; break;
				case 1: R = 0; G = 255 - j; B = i; break;
				case 2: R = i; G = 255; B = j; break;
				case 3: R = i; G = 0; B = 255 - j; break;
				case 4: R = i; G = 255 - j; B = 255; break;
				case 5: R = 255 - i; G = 255 - j; B = 0; break;
				}

				// darkgray check pattern
				if (0 == (bx + by) % 2) R = G = B = 64;

				// fill the pixel values
				int pidx = i + j * W;
				data[3 * pidx + 0] = R;
				data[3 * pidx + 1] = G;
				data[3 * pidx + 2] = B;
			}

			// set texture for the face
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, 0, GL_RGB,
				W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			//stbi_write_jpg(files[n].c_str(), W, H, 3, data, 100); // [TEMPORARY] output rendered image if needed

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
			// cubemap
			glEnable(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

			for (int n = 0; n < files.size(); n++)
			{
				char buf[256];
				sprintf_s(buf, "%s\\%s", dirpath, files[n].c_str());
				printf("load file: %s\n", buf);

				// load image and set texture
				int w, h, c;
				GLubyte* data = stbi_load(buf, &w, &h, &c, STBI_rgb);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				stbi_image_free(data);
			}

			// unbind cubemap for another process
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glDisable(GL_TEXTURE_CUBE_MAP);
		}
	}

	void LoadEquirect()
	{
		// read image filepath from tiny filedialog
		char const* filterPatterns[3] = { "*.bmp", "*.jpg", "*.png" };
		char* filepath = tinyfd_openFileDialog(
			"Read equirectagular image file",
			"./", 3, filterPatterns, NULL, 0);

		if (filepath)
		{
			// load image from file
			int w, h, c;
			GLubyte* data = stbi_load(filepath, &w, &h, &c, STBI_rgb);

			GLuint equirect;
			glEnable(GL_TEXTURE_2D);
			glGenTextures(1, &equirect);
			glBindTexture(GL_TEXTURE_2D, equirect);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);

			// resize cubemap in advance
			int W = 512;
			int H = 512;
			offscreen_FBO->Resize(W, H);

			glEnable(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
			for (int n = 0; n < 6; n++)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
			}

			// Legacy GL style
			glMatrixMode(GL_PROJECTION); glLoadIdentity();
			glMatrixMode(GL_TEXTURE);    glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);  glLoadIdentity();

			for (int n = 0; n < 6; n++)
			{
				offscreen_FBO->DrawFBO([&] {

					// set render target as cube map texture
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, cubemap, 0);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, equirect);

					// [CAUTION] Y and Z direction in pos/neg-Y
					glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
					glm::vec3 up = glm::vec3(0.0f, +1.0f, 0.0f);
					switch (n) {
					case 0: forward = glm::vec3(+1.0f, 0.0f, 0.0f); break;
					case 1: forward = glm::vec3(-1.0f, 0.0f, 0.0f); break;
					case 2: forward = glm::vec3(0.0f, -1.0f, 0.0f); up = glm::vec3(0.0f, 0.0f, +1.0f); break;
					case 3: forward = glm::vec3(0.0f, +1.0f, 0.0f); up = glm::vec3(0.0f, 0.0f, -1.0f); break;
					case 4: forward = glm::vec3(0.0f, 0.0f, +1.0f); break;
					case 5: forward = glm::vec3(0.0f, 0.0f, -1.0f); break;
					}
					glm::mat4 rot = glm::lookAt(glm::vec3(0.0f), forward, up);

					shader_equirect.DrawShader([&]() {

						GLint loc;
						loc = shader_equirect.GetUniformLocation("rot");
						glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(rot));
						loc = shader_equirect.GetUniformLocation("equirect");
						glUniform1i(loc, 1); // 1 for GL_TEXTURE1

						glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

						glBegin(GL_QUADS);
						glVertex2f(-1.0f, -1.0f);
						glVertex2f(-1.0f, +1.0f);
						glVertex2f(+1.0f, +1.0f);
						glVertex2f(+1.0f, -1.0f);
						glEnd();
					});

					// [TEMPORARY] output rendered image if needed
					/*
					offscreen_FBO->CopyColorToBuffer();
					stbi_write_jpg(files[n].c_str(), W, H, 3, offscreen_FBO->buffer_color, 100);
					//*/
				});
			}

			// release texture and image
			stbi_image_free(data);
			glBindTexture(GL_TEXTURE_2D, 0);
			glDeleteTextures(1, &equirect);
			glDisable(GL_TEXTURE_2D);

			// unbind cubemap for another process
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glDisable(GL_TEXTURE_CUBE_MAP);
		}
	}

	bool Init()
	{
		SetAppGLTitle("EnvMappingApp");

		TwAddButton(bar, "Load Cubemap", [](void* client) {
			EnvMappingAppGL* _this = (EnvMappingAppGL*)client;
			_this->LoadCubemap();
			}, this, " ");

		TwAddButton(bar, "Load Equirectangle image", [](void* client) {
			EnvMappingAppGL* _this = (EnvMappingAppGL*)client;
			_this->LoadEquirect();
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

		// prepare shader for cubemap rendering
		shader_equirect = vgl::GLShader();
		shader_equirect.Compile(vertShader_equirect, vgl::SHADER_TYPE::VERTEX);
		shader_equirect.Compile(fragShader_equirect, vgl::SHADER_TYPE::FRAGMENT);
		shader_equirect.Link();

		// off-screen renderer for conversion from equirectangular image to cubemap
		offscreen_FBO = new vgl::FBO();

		InitCubemap();

		return true;
	}

	void End()
	{
		glDeleteTextures(1, &cubemap);

		delete offscreen_FBO;
		offscreen_FBO = nullptr;
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
	vgl::GLShader shader_equirect; // conversion equirectangular to cubemap

	vgl::FBO* offscreen_FBO = nullptr;
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
