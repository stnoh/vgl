/******************************************************************************
Helper for Wavefront obj mesh format (.obj) I/O. This is temporary code.
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vgl/objFileIO.h>
#include <sstream>
#include <iterator>

namespace vgl {

void _ReadObjFile(std::ifstream& iFile, std::vector<glm::vec3>& V,
	const bool existT, std::vector<glm::vec2>& T,
	const bool existN, std::vector<glm::vec3>& N,
	std::vector<glm::uint>& F)
{
	std::string line;

	while (std::getline(iFile, line)) {
		std::istringstream iss(line);
		//printf("%s\n",line.c_str());

		// tokenize this line
		std::vector<std::string> tokens;
        std::string token;
		while (iss >> token) {
			tokens.push_back(token);
		}

		// parse each line
		if (!tokens.empty()) {
			if (tokens[0] == "#" || tokens[0][0] == '#') continue;

			// vertices
			if (tokens[0] == "v") {
				glm::vec3 v;
				v.x = std::stof(tokens[1]);
				v.y = std::stof(tokens[2]);
				v.z = std::stof(tokens[3]);
				V.push_back(v);
			}
			if (tokens[0] == "vt" && existT) {
				glm::vec3 vt;
				vt.x = std::stof(tokens[1]);
				vt.y = std::stof(tokens[2]);
				T.push_back(vt);
			}
			if (tokens[0] == "vn" && existN) {
				glm::vec3 vn;
				vn.x = std::stof(tokens[1]);
				vn.y = std::stof(tokens[2]);
				vn.z = std::stof(tokens[3]);
				N.push_back(vn);
			}

			// faces
			if (tokens[0] == "f") {
				glm::uint f0 = std::stoi(tokens[1]) - 1;
				glm::uint f1 = std::stoi(tokens[2]) - 1;
				glm::uint f2 = std::stoi(tokens[3]) - 1;
				F.push_back(f0);
				F.push_back(f1);
				F.push_back(f2);
			}
		}
	}

	//printf("#V=%d #VT=%d #VN=%d, #F=%d\n", V.size(), T.size(), N.size(), F.size());
}

void ReadTriMeshObj(const char* filepath, std::vector<glm::vec3>& V, std::vector<glm::vec2>& T, std::vector<glm::vec3>& N, std::vector<glm::uint>& F)
{
	// check the file
	std::ifstream iFile(filepath);
	if (!iFile.is_open()) {
		fprintf(stderr, "ERROR: cannot find a file: %s\n", filepath);
		return;
	}

	_ReadObjFile(iFile, V, true, T, true, N, F);
}

void ReadTriMeshObj(const char* filepath, std::vector<glm::vec3>& V, std::vector<glm::vec3>& N, std::vector<glm::uint>& F)
{
	// check the file
	std::ifstream iFile(filepath);
	if (!iFile.is_open()) {
		fprintf(stderr, "ERROR: cannot find a file: %s\n", filepath);
		return;
	}

	std::vector<glm::vec2> T;
	_ReadObjFile(iFile, V, false, T, true, N, F);
}

void ReadTriMeshObj(const char* filepath, std::vector<glm::vec3>& V, std::vector<glm::uint>& F)
{
	// check the file
	std::ifstream iFile(filepath);
	if (!iFile.is_open()) {
		fprintf(stderr, "ERROR: cannot find a file: %s\n", filepath);
		return;
	}

	std::vector<glm::vec2> T;
	std::vector<glm::vec3> N;
	_ReadObjFile(iFile, V, false, T, false, N, F);
}

}
