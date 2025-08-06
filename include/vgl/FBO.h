/******************************************************************************
Off-screen rendering by GL frame buffer object
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef FRAME_BUFFER_OBJECT
#define FRAME_BUFFER_OBJECT

#include <GL/glew.h>

#include <functional>
#include <vector>
#include <set>
#include <glm/glm.hpp>

namespace vgl {

class FBO
{
public:
	FBO(const int width, const int height);
	FBO() : FBO(8192, 8192) {}; // maximum size as default
	~FBO();

	void Enable();
	void Disable();

	// lambda guard for enable/disable
	void DrawFBO(std::function<void(void)> func)
	{
		Enable();
		func();
		Disable();
	}

	bool Resize(const int width, const int height);

	void CopyColorToBuffer();
	void CopyDepthToBuffer();

	std::vector<glm::vec3> ConvertDepthImage2PointCloud(
		const glm::mat4 proj_inv, const bool full = false);

	std::pair<std::vector<glm::vec3>, std::vector<glm::u8vec3>> 
		ConvertDepthImage2PointCloudWithColor(
			const glm::mat4 proj_inv, const bool full = false);

	std::set<glm::uint> GetVisibleVertexIndices(
		glm::mat4 ProjViewModel, const std::vector<glm::vec3>& model_verts,
		const float threshold = 5e-3);

private:

	GLuint fbo;
	GLuint rboColor;
	GLuint rboDepth;

	GLubyte* buffer_color = nullptr;
	GLfloat* buffer_depth = nullptr;

	int width;
	int height;
	const int maxWidth;
	const int maxHeight;
};

}

#endif
