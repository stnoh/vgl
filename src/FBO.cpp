/******************************************************************************
Off-screen rendering by GL frame buffer object
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vgl/FBO.h>
#include <iostream>

using namespace vgl;


///////////////////////////////////////////////////////////////////////////////
// ctor / dtor
///////////////////////////////////////////////////////////////////////////////
FBO::FBO(const int width, const int height)
: maxWidth(width), maxHeight(height)
{
	// resolution
	this->width = width;
	this->height = height;

	// render buffer for color image
	glGenRenderbuffers(1, &rboColor);
	glBindRenderbuffer(GL_RENDERBUFFER, rboColor);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, width, height); // for single-precision floating color

	// render buffer for depth image
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// frame buffer
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboColor);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// check validity of framebuffer (should be OK)
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
FBO::~FBO()
{
	Disable(); // deallocate before removing

	// release render buffers & frame buffer
	glDeleteRenderbuffers(1, &rboColor);
	glDeleteRenderbuffers(1, &rboDepth);
	glDeleteFramebuffers(1, &fbo);
}


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
void FBO::Enable()
{
	// enable to use framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, width, height);
}
void FBO::Disable()
{
	// disable framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// release buffers if exist
	if (nullptr != buffer_color)
	{
		delete buffer_color;
		buffer_color = nullptr;
	}
	if (nullptr != buffer_depth)
	{
		delete buffer_depth;
		buffer_depth = nullptr;
	}
}

bool FBO::Resize(const int width, const int height)
{
	bool failed = false;

	if (0 > width || maxWidth < width) {
		fprintf(stderr, "Failed to set FBO width: %d", width);
		failed = true;
	}
	else this->width = width;

	if (0 > height || maxHeight < height) {
		fprintf(stderr, "Failed to set FBO height: %d", height);
		failed = true;
	}
	else this->height = height;

	return !failed;
}


///////////////////////////////////////////////////////////////////////////////
// copy data from GPU memory
///////////////////////////////////////////////////////////////////////////////
void FBO::CopyColorToBuffer()
{
	// release buffer if exists
	if (nullptr != buffer_color) {
		delete buffer_color;
		buffer_color = nullptr;
	}

	// allocate buffer & copy
	buffer_color = new GLubyte[(size_t)width * height * 3];
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer_color);
}
void FBO::CopyDepthToBuffer()
{
	// release buffer if exists
	if (nullptr != buffer_depth) {
		delete buffer_depth;
		buffer_depth = nullptr;
	}

	// allocate buffer & copy
	buffer_depth = new GLfloat[(size_t)width * height];
	glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buffer_depth);

	// convert depth to 3D data
	for (int j = 0; j < height; j++)
	for (int i = 0; i < width ; i++)
	{
		float z_b = buffer_depth[i + j* width];
		buffer_depth[i + j* width] = (1.0f == z_b) ? 0.0f : z_b; // <inf> -> 0.0
	}
}


///////////////////////////////////////////////////////////////////////////////
// convert depth image to point cloud (in GL coordinate)
// similar to glm::unProject, but it provides more efficient routine
///////////////////////////////////////////////////////////////////////////////
inline glm::vec3 convertDepth2Point(const glm::mat4& proj_inv, glm::vec3 pt_img, float width, float height)
{
	// normalize point on 2D image plane
	pt_img.x = (pt_img.x + 0.5f) / width;
	pt_img.y = (pt_img.y + 0.5f) / height;

	// convert from NDC: ( [-1.0:+1.0], [-1.0:+1.0], [-1.0:+1.0] )
	glm::vec4 pt_NDC = glm::vec4(2.0f * pt_img - 1.0f, 1.0f);
	glm::vec4 pt3d = proj_inv * pt_NDC;

	// perspective-awareness
	return pt3d / pt3d.w;
}
std::vector<glm::vec3> FBO::ConvertDepthImage2PointCloud(
	const glm::mat4 proj_inv, const bool full)
{
	std::vector<glm::vec3> point;

	if (nullptr == buffer_depth) {
		fprintf(stderr, "ERROR: depth buffer does not exist.");
		return point;
	}

	if (full) {
		// same with image size
		point = std::vector<glm::vec3>(width * height);

		for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
		{
			float d = buffer_depth[i + j * width];
			glm::vec3 pt2d(i, j, d);
			point[i + j * width] = convertDepth2Point(proj_inv, pt2d, width, height);
		}
	}
	else {
		for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
		{
			float d = buffer_depth[i + j * width];
			if (0.0f == d) continue; // filter un-rendered pixel

			glm::vec3 pt2d(i, j, d);
			point.push_back(convertDepth2Point(proj_inv, pt2d, width, height));
		}
	}

	return point;
}
std::pair<std::vector<glm::vec3>, std::vector<glm::u8vec3>> 
FBO::ConvertDepthImage2PointCloudWithColor(
	const glm::mat4 proj_inv, const bool full)
{
	std::vector<glm::vec3> point;
	std::vector<glm::u8vec3> color;

	if (nullptr == buffer_color || nullptr == buffer_depth) {
		fprintf(stderr, "ERROR: depth buffer does not exist.");
		return std::pair<std::vector<glm::vec3>, std::vector<glm::u8vec3>>(point, color);
	}

	if (full) {
		// same with image size
		point = std::vector<glm::vec3>(width * height);
		color = std::vector<glm::u8vec3>(width * height);

		for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
		{
			float d = buffer_depth[i + j * width];
			glm::vec3 pt2d(i, j, d);
			point[i + j * width] = convertDepth2Point(proj_inv, pt2d, width, height);

			GLubyte r = buffer_color[3 * (i + j * width) + 0];
			GLubyte g = buffer_color[3 * (i + j * width) + 1];
			GLubyte b = buffer_color[3 * (i + j * width) + 2];
			color[i + j * width] = glm::u8vec3(r, g, b);
		}
	}
	else {
		for (int j = 0; j < height; j++)
		for (int i = 0; i < width; i++)
		{
			float d = buffer_depth[i + j * width];
			if (0.0f == d) continue; // filter un-rendered pixel

			glm::vec3 pt2d(i, j, d);
			point.push_back(convertDepth2Point(proj_inv, pt2d, width, height));

			GLubyte r = buffer_color[3 * (i + j * width) + 0];
			GLubyte g = buffer_color[3 * (i + j * width) + 1];
			GLubyte b = buffer_color[3 * (i + j * width) + 2];
			color.push_back(glm::u8vec3(r, g, b));
		}
	}

	return std::pair<std::vector<glm::vec3>, std::vector<glm::u8vec3>>(point, color);
}

std::set<glm::uint> FBO::GetVisibleVertexIndices(
	glm::mat4 ProjViewModel, const std::vector<glm::vec3>& model_verts,
	const float threshold)
{
	std::set<glm::uint> v_indices;

	if (nullptr == buffer_depth) {
		fprintf(stderr, "ERROR: depth buffer does not exist.");
		return v_indices;
	}

	// convert buffer value to camera space Z
	for (int vidx = 0; vidx < model_verts.size(); vidx++)
	{
		glm::vec4 pt_proj = ProjViewModel * glm::vec4(model_verts[vidx], 1.0f);
		glm::vec3 pt_NDC = pt_proj / pt_proj.w;

		// filter 1) out of screen
		if (pt_NDC.x < -1.0 || +1.0 < pt_NDC.x ||
			pt_NDC.y < -1.0 || +1.0 < pt_NDC.y) continue;

		// filter 2) compare to rendered depth
		glm::vec2 pt2d = glm::vec2(0.5f * (pt_NDC + 1.0f));
		int i = width *  pt2d.x;
		int j = height * pt2d.y;
		float d = buffer_depth[i + j * width];
		float pixel_z = 2.0f * d - 1.0f;

		if (glm::length(pt_NDC.z - pixel_z) > threshold) continue;

		v_indices.insert(vidx);
	}

	return v_indices;
}
