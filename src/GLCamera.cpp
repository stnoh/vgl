/******************************************************************************
GL camera class that manages projection and modelview matrices
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vgl/GLCamera.h>
#include <vgl/DrawGL3D.h>

using namespace vgl;

///////////////////////////////////////////////////////////////////////////////
// related to projection matrix
///////////////////////////////////////////////////////////////////////////////
const glm::mat4 GLCamera::GetProjMatrix()
{
	glm::mat4 proj(0.0f);

	// orthogonal projection
	if (is_ortho) {
		proj = glm::ortho(L, R, B, T, z_near, z_far);
	}
	// perspective projection
	else {
		proj[0][0] = 2.0f / (R - L);
		proj[1][1] = 2.0f / (T - B);
		proj[2][0] = (R + L) / (R - L);
		proj[2][1] = (T + B) / (T - B);
		proj[2][3] = -1.0f; // in case of perspective matrix

		if (is_inf) {
			proj[2][2] = -1.0f;
			proj[3][2] = -2.0f * z_near;
		}
		else {
			proj[2][2] = -(z_far + z_near) / (z_far - z_near);
			proj[3][2] = -2.0f * (z_far * z_near) / (z_far - z_near);
		}
	}

	return proj;
}

void GLCamera::SetCameraMatrixCV(const std::vector<float>& camParams4x1, float z_near)
{
	is_inf = true;
	this->z_near = z_near;

	// get normalized camera parameters
	float fx = camParams4x1[0];
	float fy = camParams4x1[1];
	float cx = camParams4x1[2];
	float cy = camParams4x1[3];

	// convert to normalized frustum values
	L = -cx / fx;
	R = +(1.0f - cx) / fx;
	B = -(1.0f - cy) / fy;
	T = +cy / fy;
}
void GLCamera::SetCameraMatrixCV(const std::vector<float>& camParams4x1, float z_near, float z_far)
{
	SetCameraMatrixCV(camParams4x1, z_near);

	// overwrite
	is_inf = false;
	this->z_far = z_far;
}


///////////////////////////////////////////////////////////////////////////////
// set camera coordinate (=pose matrix), draw frustum as option
///////////////////////////////////////////////////////////////////////////////
void GLCamera::SetCameraCoord(std::function<void(void)> func, bool drawFrustum)
{
	glm::mat4 proj = GetProjMatrix();
	glm::mat4 pose = GetPoseMatrix();

	glPushMatrix();
	glMultMatrixf(glm::value_ptr(pose));
	if (drawFrustum) vgl::drawCameraFrustum(proj);

	func();

	glPopMatrix();
}
