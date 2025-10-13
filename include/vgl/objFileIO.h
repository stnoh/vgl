/******************************************************************************
Helper for Wavefront obj mesh format (.obj) I/O. This is temporary code.
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef OBJ_FILE_IO
#define OBJ_FILE_IO

#include <fstream>
#include <vector>

#include <glm/glm.hpp>

namespace vgl {

void WriteTriMeshObj(const char* filepath,
	const std::vector<glm::vec3>& V, const std::vector<glm::vec2>& T, const std::vector<glm::vec3>& N,
	const std::vector<glm::uint>& F);
void WriteTriMeshObj(const char* filepath,
	const std::vector<glm::vec3>& V, const std::vector<glm::vec3>& N,
	const std::vector<glm::uint>& F);
void WriteTriMeshObj(const char* filepath,
	const std::vector<glm::vec3>& V,
	const std::vector<glm::uint>& F);

void ReadTriMeshObj(const char* filepath, 
	std::vector<glm::vec3>& V, std::vector<glm::vec2>& T, std::vector<glm::vec3>& N,
	std::vector<glm::uint>& F);

void ReadTriMeshObj(const char* filepath,
	std::vector<glm::vec3>& P, std::vector<glm::vec3>& N, 
	std::vector<glm::uint>& F);

void ReadTriMeshObj(const char* filepath,
	std::vector<glm::vec3>& P,
	std::vector<glm::uint>& F);

}

#endif
