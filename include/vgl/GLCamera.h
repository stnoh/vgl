/******************************************************************************
GL camera class that manages projection and modelview matrices
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef GL_CAMERA
#define GL_CAMERA

#include <GL/glew.h>
#include <vector>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace vgl {

class GLCamera
{
public:

	////////////////////////////////////////////////////////////
	// intrinsic (FOV): projection matrix
	////////////////////////////////////////////////////////////
	const glm::mat4 GetProjMatrix();
	
	// get approximated angle FoV
	const float GetCameraFoV(bool isVertical=true)
	{
		if (is_ortho) return FLT_MAX;

		glm::mat4 proj = GetProjMatrix();

		// vertical FoV (FoVy) as default, horizontal FoV (FoVx) if needed
		float value = (isVertical) ? proj[1][1] : proj[0][0];

		float AFoV = 2.0f * glm::degrees( atan2(1.0f, value) );
		return AFoV;
	}
	const std::vector<float> GetCameraMatrixCV()
	{
		float fx = 1.0f / (R - L);
		float fy = 1.0f / (T - B);
		float cx = -L * fx;
		float cy = +T * fy;
		return std::vector<float>{fx, fy, cx, cy};
	}

	// set projection matrix by normalized camera matrix [0.0:1.0]
	void SetCameraMatrixCV(const std::vector<float>& camParams4x1, float z_near);
	void SetCameraMatrixCV(const std::vector<float>& camParams4x1, float z_near, float z_far);

	void SetCameraFoV(float FoVy_deg, float width, float height, float z_near, float z_far)
	{
		float FoVy = glm::radians(FoVy_deg);
		glm::mat4 proj = glm::perspectiveFov(FoVy, width, height, z_near, z_far);
		L = (proj[2][0] - 1.0f) / proj[0][0];
		R = (proj[2][0] + 1.0f) / proj[0][0];
		T = (proj[2][1] + 1.0f) / proj[1][1];
		B = (proj[2][1] - 1.0f) / proj[1][1];

		this->z_near = z_near;
		this->z_far  = z_far;
		is_ortho = false;
	}
	void SetCameraFoV(float FoVy, float width, float height, float z_near)
	{
		SetCameraFoV(FoVy, width, height, z_near, z_far);
	}

	// default values for camera frustum
	bool is_ortho = false;
	bool is_inf   = true;

	float z_near =   0.1f;
	float z_far  = 100.0f;
	float L = -0.5f;
	float R = +0.5f;
	float B = -0.5f;
	float T = +0.5f;


	////////////////////////////////////////////////////////////
	// extrinsic (RT): rotation and translation
	////////////////////////////////////////////////////////////
	const glm::mat4 GetViewMatrix() { return glm::inverse(GetPoseMatrix()); }
	const glm::mat4 GetPoseMatrix()
	{
		glm::mat4 R = glm::mat4_cast(rotation);
		glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
		return T * R;
	}
	void SetPoseMatrixGL(const glm::mat4& poseGL)
	{
		rotation = glm::quat(poseGL);
		position = glm::vec3(poseGL[3]);
	}

	// extrinsic (RT)
	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::quat();


	////////////////////////////////////////////////////////////
	// set camera coordinate (=pose matrix)
	////////////////////////////////////////////////////////////
	void SetCameraCoord(std::function<void(void)> func, bool drawFrustum=false);
};

}

#endif
