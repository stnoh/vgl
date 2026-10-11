#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>
#include <vgl/GLShader.h>

#include <vgl/rawFileIO.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <tinyfiledialogs.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

std::string shader_vs = R"(
#version 330 core
uniform mat4 mvp;

void main()
{
	// [TODO]
}
)";
std::string shader_fs = R"(
#version 330 core

void main()
{
	// [TODO]
}
)";

class VolumeViewerApp : public AppGLBase
{
public:
	VolumeViewerApp(const int width, const int height) : AppGLBase(width, height) {};

	void Draw(const int width, const int height)
	{
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::infinitePerspective(glm::radians(53.1301f), width / (float)height, 0.1f);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), -GlobalViewPosition);
		view = view * glm::mat4_cast(GlobalViewRotation);

		glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

		vgl::setLight(GL_LIGHT0, L_position);
		vgl::setMaterial(GL_FRONT_AND_BACK);

		// draw triangle mesh
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_LIGHTING);
		glEnable(GL_RESCALE_NORMAL);

		//shader.DrawShader([&]() {
			vgl::drawCube(1.0f);
		//});

		glDisable(GL_RESCALE_NORMAL);
		glDisable(GL_LIGHTING);
	}

	bool Init()
	{
		SetAppGLTitle("VolumeViewerApp");

		TwAddButton(bar, "import_volume", [](void* client) {
			VolumeViewerApp* _this = (VolumeViewerApp*)client;
			_this->LoadRawFile();
		}, this, " ");
#if 1
		TwAddButton(bar, "Global-init", [](void* client) {
			VolumeViewerApp* _this = (VolumeViewerApp*)client;
			_this->resetGlobalView();
		}, this, "group='Global' label='init' ");
		TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation  , "group='Global' label='rot'  open");
		TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
		TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
		TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif
		resetGlobalView();

		// set hidden surface removal flags
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		// set some offset trick to show wireframe clearly
		glEnable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(1.0f, 1.0f);

		// prepare shader
		shader = vgl::GLShader();
		shader.Compile(shader_vs, vgl::SHADER_TYPE::VERTEX);
		shader.Compile(shader_fs, vgl::SHADER_TYPE::FRAGMENT);
		shader.Link();

		return true;
	}

private:
	glm::quat GlobalViewRotation = glm::quat();
	glm::vec3 GlobalViewPosition = glm::vec3(0.0f, 0.0f, 2.0f);
	void resetGlobalView() {
		GlobalViewPosition = glm::vec3(0.0f, 0.0f, 2.0f);
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
	}
	glm::vec4 L_position = glm::vec4(-0.45f, 0.9f, 0.1f, 0.0f);

	void LoadRawFile()
	{
		char const* filterPatterns[1] = { "*.raw" };
		char* filepath = tinyfd_openFileDialog(
			"Read .raw volume file",
			"./", 1, filterPatterns, NULL, 0);

		if (filepath)
		{
			printf("filepath: %s\n", filepath);
			vgl::ReadRawFile(filepath, volume_size, volume_data);
			printf("%d %d %d\n", volume_size.x, volume_size.y, volume_size.z);

			// [TEMPORARY]
			/*
			for (int k = 0; k < volume_size.z; k++) {
				std::vector<unsigned char> data;
				data.resize(volume_size.x * volume_size.y);
				for (int j = 0; j < volume_size.y; j++)
				for (int i = 0; i < volume_size.x; i++) {
					data[i + j * volume_size.x] = volume_data[i + j * volume_size.x + k * volume_size.x * volume_size.y];
				}

				// output file
				char buf[128];
				sprintf(buf, "image%03d.bmp", k);
				stbi_write_bmp(buf, volume_size.x, volume_size.y, 1, data.data());
			}
			//*/
		}
	}

	vgl::GLShader shader;

	// volume from raw file
	glm::ivec3 volume_size;
	std::vector<glm::uint8> volume_data;
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	VolumeViewerApp app(480, 480);
	app.run();
	return EXIT_SUCCESS;
}
