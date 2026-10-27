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
layout(location=0) in vec3 vPosition;
uniform mat4 Projection;
uniform mat4 ModelView;

out vec3 rayStart;
out vec3 rayEnd;

void main()
{
	mat4 invModelView = inverse(ModelView);
	vec4 camPosLocal = invModelView * vec4(0.0, 0.0, 0.0, 1.0);
	vec3 camPos = camPosLocal.xyz / camPosLocal.w;
	
	// Naive "forward" ray (from camera origin to inside face)
	rayStart = camPos + vec3(0.5);
	rayEnd   = vPosition + vec3(0.5);

	gl_Position = Projection * ModelView * vec4(vPosition, 1.0); // mandatory
}
)";
std::string shader_fs = R"(
#version 330 core
uniform sampler3D volume_data;
uniform float iso_value;

in vec3 rayStart;
in vec3 rayEnd;

out vec4 FragColor;

const int MAX_STEP = 256;

void main()
{
	// determine search range
	vec3 rayDir = normalize(rayEnd - rayStart);
	float t_max = length(rayStart - rayEnd);
	
	// Naive ray marching
	float iso_prev = texture(volume_data, rayStart).r - iso_value;
	float dt = t_max / float(MAX_STEP);

	for (int i=1;i<MAX_STEP;i++) {
		float it = float(i);
		vec3 pos = rayStart + it * dt * rayDir;
		float iso_this = texture(volume_data, pos).r - iso_value;
		
		// check intersection
		if (iso_this * iso_prev <= 0.0f) {
			it -= abs( iso_this / (iso_prev - iso_this) ); // move back to accurate intersected position
			pos = rayStart + it * dt * rayDir;
			FragColor = vec4(pos, 1.0);
			return;
		}

		iso_prev = iso_this;
	}
	
	discard; // if there was no intersection
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

		// draw triangle mesh
		glColor3f(1.0f, 1.0f, 1.0f);

		// set backface culling to draw "internal" face
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		// draw volume with shader
		if (loaded) {
			// set volume data as texture #0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, texID);

			shader.DrawShader([&]() {
				GLint loc = -1;
				loc = shader.GetUniformLocation("Projection");
				if (-1 != loc) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
				loc = shader.GetUniformLocation("ModelView");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
				loc = shader.GetUniformLocation("volume_data");
				if (-1 != loc) glUniform1i(loc, 0);
				loc = shader.GetUniformLocation("iso_value");
				if (-1 != loc) glUniform1f(loc, iso_value);

				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);

				vgl::drawCube();
			});

			// disable texture #0
			glBindTexture(GL_TEXTURE_3D, 0);
		}
		else
		{
			// just draw inverted cube (default)
			glEnable(GL_LIGHTING);
			vgl::setLight(GL_LIGHT0, L_position, L_ambient, L_diffuse, L_specular);
			vgl::setMaterial(GL_FRONT_AND_BACK, M_ambient, M_diffuse, M_specular, glm::vec4(0.0f), M_shininess);

			vgl::drawCube();

			glDisable(GL_LIGHT0);
			glDisable(GL_LIGHTING);
		}

		// disable backface culling
		glCullFace(GL_BACK);
		glDisable(GL_CULL_FACE);
	}

	bool Init()
	{
		SetAppGLTitle("VolumeViewerApp");

		TwAddButton(bar, "import_volume", [](void* client) {
			VolumeViewerApp* _this = (VolumeViewerApp*)client;
			_this->LoadRawFile();
		}, this, " ");
		TwAddButton(bar, "export_images", [](void* client) {
			VolumeViewerApp* _this = (VolumeViewerApp*)client;
			_this->SavePngFiles();
		}, this, " ");

		TwAddVarRW(bar, "iso_value", TwType::TW_TYPE_FLOAT, &iso_value, "step=0.001 min=0.0 max=1.0");

#if 1
		TwAddButton(bar, "Global-init", [](void* client) {
			VolumeViewerApp* _this = (VolumeViewerApp*)client;
			_this->resetGlobalView();
		}, this, "group='Global' label='init' ");
		TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation, "group='Global' label='rot'  open");
		TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
		TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
		TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif
		resetGlobalView();

		//TwAddVarRW(bar, "Light_dir", TwType::TW_TYPE_DIR3F, &L_position, "group='Global' label='Light_dir' open"); // to visualize

		// prepare shader
		shader = vgl::GLShader();
		shader.Compile(shader_vs, vgl::SHADER_TYPE::VERTEX);
		shader.Compile(shader_fs, vgl::SHADER_TYPE::FRAGMENT);
		shader.Link();

		// set 3D texture setting in advance
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_3D, texID);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_3D, 0);

		return true;
	}

private:
	glm::quat GlobalViewRotation = glm::quat();
	glm::vec3 GlobalViewPosition = glm::vec3(0.0f, 0.0f, 2.0f);
	void resetGlobalView() {
		GlobalViewPosition = glm::vec3(0.0f, 0.0f, 2.0f);
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
	}

	// light properties
	glm::vec4 L_position = glm::vec4(0.25f, -0.6f, 0.75f, 0.0f);
	glm::vec4 L_ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 L_diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 L_specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// material properties
	glm::vec4 M_ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 M_diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 M_specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float M_shininess = 32.0f;

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

			loaded = true;
		}

		// set volume data as 3D texture
		glBindTexture(GL_TEXTURE_3D, texID);

		glTexImage3D(GL_TEXTURE_3D, 0,
			GL_R8, volume_size.x, volume_size.y, volume_size.z, 0,
			GL_RED, GL_UNSIGNED_BYTE, &volume_data[0]);

		glBindTexture(GL_TEXTURE_3D, 0);
	}

	void SavePngFiles()
	{
		if (!loaded) return;

		char* dirpath = tinyfd_selectFolderDialog(
			"Set directory where export as multiple PNG images", "./");

		if (dirpath)
		{
			int image_size = volume_size.x * volume_size.y;

			for (int k = 0; k < volume_size.z; k++) {
				std::vector<unsigned char> data;
				data.resize(image_size);
				for (int j = 0; j < volume_size.y; j++)
				for (int i = 0; i < volume_size.x; i++) {
					int id = i + j * volume_size.x;
					data[id] = volume_data[id + k * image_size];
				}

				// output as PNG image
				char buf[128];
				sprintf(buf, "%s/image%03d.png", dirpath, k);
				stbi_write_png(buf, volume_size.x, volume_size.y, 1, data.data(), 0);
			}
		}
	}

	vgl::GLShader shader;

	// volume from raw file
	glm::ivec3 volume_size = glm::ivec3(0);
	std::vector<glm::uint8> volume_data;
	float iso_value = 0.175;

	bool loaded = false;

	GLuint texID = 0;
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
