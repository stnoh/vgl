/******************************************************************************
Helper for drawing simple but general 3D objects with GL
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef DRAW_GL_3D
#define DRAW_GL_3D

#include <GL/glew.h>

#include <glm/glm.hpp>

#include <vector>

namespace vgl {

// simple primitive (GLU)
void drawAxes(double length);
void drawSphere(float radius, glm::vec4 color);

// simple primitive (GL_LINES)
void drawGridXZ(float length, int step);
void drawAABB(glm::vec3 minAB, glm::vec3 maxAB);

// camera frustum
//void drawPerspCamera(glm::mat4 proj); // no need to expose
//void drawOrthoCamera(glm::mat4 proj); // no need to expose
void drawCameraFrustum(glm::mat4 proj);

// 3D data
void drawPointCloud(const std::vector<glm::vec3>& points);
void drawPointCloud(const std::vector<glm::vec3>& points, const std::vector<glm::u8vec3>& colors);

void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::uint>& F);
void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::vec3>& N, const std::vector<glm::uint>& F);
void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::u8vec3>& C, const std::vector<glm::uint>& F);
void drawTriMesh(const std::vector<glm::vec3>& V, const std::vector<glm::vec3>& N, const std::vector<glm::u8vec3>& C, const std::vector<glm::uint>& F);

// lighting
void setLight(GLenum lightNum, glm::vec4 lightPos,
	const glm::vec4 ambient  = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 
	const glm::vec4 diffuse  = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	const glm::vec4 specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

// material
void setMaterial(GLenum faceType, 
	const glm::vec4 ambient  = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
	const glm::vec4 diffuse  = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
	const glm::vec4 specular = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	const glm::vec4 emission = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
	const float shininess=0.0f);

// convert normal to normalmap color
//inline glm::u8vec3 getNormalColor(const glm::vec3& normal); // no need to expose
std::vector<glm::u8vec3> GetNormalColors(const std::vector<glm::vec3>& normal);

}

#endif
