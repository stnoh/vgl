#ifndef CAMERA_FRUSTUM_VIEWER_APP
#define CAMERA_FRUSTUM_VIEWER_APP

#include <vgl/AppGLBase.h>

#include <glm/glm.hpp>
#include <vgl/GLCamera.h>

class CameraFrustumViewerApp : public AppGLBase
{
public:
	CameraFrustumViewerApp(const int width, const int height) : AppGLBase(width, height) { };

	// mandatory member function
	void Draw(const int width, const int height);

	// override member functions
	bool Init();
	void End();

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
		rendercam->position = glm::vec3(0.0f, 1.5f, 1.5f);
		rendercam->rotation = glm::quat(glm::radians(glm::vec3(-45.0f, 0.0f, 0.0f)));
	}

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
