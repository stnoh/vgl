/******************************************************************************
Helper for stanford mesh format (.ply) I/O
This is temporary code. I have a plan to re-write or dispose this code soon.
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef PLY_FILE_IO
#define PLY_FILE_IO

#include <fstream>
#include <vector>

#include <glm/glm.hpp>

namespace vgl {

// write ply file
void WritePointCloudPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::vec3> N, std::vector<glm::u8vec3> C);
void WritePointCloudPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::vec3> N);
void WritePointCloudPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::u8vec3> C);
void WritePointCloudPly(const char* filepath, std::vector<glm::vec3> P);

void WriteTriMeshPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::vec3> N, std::vector<glm::u8vec3> C, std::vector<glm::uint> F);
void WriteTriMeshPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::vec3> N, std::vector<glm::uint> F);
void WriteTriMeshPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::u8vec3> C, std::vector<glm::uint> F);
void WriteTriMeshPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::uint> F);

// read ply file
void ReadPointCloudPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::vec3>& N, std::vector<glm::u8vec3>& C);
void ReadPointCloudPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::vec3>& N);
void ReadPointCloudPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::u8vec3>& C);
void ReadPointCloudPly(const char* filepath, std::vector<glm::vec3>& P);

void ReadTriMeshPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::vec3>& N, std::vector<glm::u8vec3>& C, std::vector<glm::uint>& F);
void ReadTriMeshPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::vec3>& N, std::vector<glm::uint>& F);
void ReadTriMeshPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::u8vec3>& C, std::vector<glm::uint>& F);
void ReadTriMeshPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::uint>& F);

}

#endif
