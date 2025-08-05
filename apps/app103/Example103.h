#ifndef APP_GL_EXAMPLE_103
#define APP_GL_EXAMPLE_103

#include <vgl/AppGLBase.h>

#include <vgl/DrawGL3D.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vgl/GLCamera.h>
#include <vgl/FBO.h>

class Example103 : public AppGLBase
{
public:
	Example103(const int width, const int height) : AppGLBase(width, height) { };

	// mandatory member function
	void Draw(const int width, const int height);

	// override member functions
	bool Init();
	void End();

	// user-defined function
	void LoadMesh();

private:
	void drawView3D(glm::mat4 proj, glm::mat4 view);

	// member objects & variables
	glm::fvec4 BgColor[2] = { glm::fvec4(0.5, 0.5, 0.5, 1.0), glm::fvec4(0.2, 0.3, 0.5, 1.0) };

	// global viewer
	glm::quat GlobalViewRotation;
	glm::vec3 GlobalViewPosition;
	void resetGlobalView() {
		GlobalViewPosition = glm::vec3(0.0f, 0.0f, 4.0f);
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(45.0f, -37.5f, -30.0f)));
	}

	// camera object
	vgl::GLCamera* rendercam = nullptr;
	void resetCameraView() {
		// oblique view as default
		rendercam->position = glm::vec3(0.0f, 0.0f, 2.0f);
		rendercam->rotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
	}

	// offscreen renderer
	vgl::FBO* offscreenFBO = nullptr;

	// container for 3D mesh
	std::vector<glm::vec3> V, N; // vertex/normal
	std::vector<glm::u8vec3> C;  // vertex color
	std::vector<glm::uint> F;

	std::vector<glm::uint> F_visible; // visible faces

	// model matrix for 3D mesh
	glm::quat ModelRotation;
	glm::vec3 ModelPosition;
	float     ModelUniScale;
	void resetModelMatrix()
	{
		ModelUniScale = 1.0f;
		ModelPosition = glm::vec3(0.0f, 0.0f, 0.0f);;
		ModelRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
	}
};

#endif
