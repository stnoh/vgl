/******************************************************************************
Helper for Wavefront obj mesh format (.obj) I/O. This is temporary code.
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vgl/objFileIO.h>
#include <sstream>
#include <iterator>
#include <functional>

namespace vgl {

void _WriteObjFile(const char* filepath, const std::vector<glm::vec3>& V,
	const std::vector<glm::vec2>* T, const std::vector<glm::vec3>* N,
	const std::vector<glm::uint>& F)
{
	std::ofstream oFile(filepath);

	// write all vertex position data (mandatory)
	{
		std::stringstream ss;
		char line[1024];

		for (int vidx = 0; vidx < V.size(); vidx++)
		{
			glm::vec3 p = V[vidx];
			std::snprintf(line, 1024, "v %f %f %f", p.x, p.y, p.z);
			ss << line << std::endl;
		}
		oFile << ss.str();
	}

	// write all optional data if exists
	if (NULL != T) {
		std::stringstream ss;
		char line[1024];

		for (int tidx = 0; tidx < T->size(); tidx++)
		{
			glm::vec2 t = (*T)[tidx];
			std::snprintf(line, 1024, "vt %f %f", t.x, t.y);
			ss << line << std::endl;
		}
		oFile << ss.str();
	}
	if (NULL != N) {
		std::stringstream ss;
		char line[1024];

		for (int nidx = 0; nidx < N->size(); nidx++)
		{
			glm::vec3 n = (*N)[nidx];
			std::snprintf(line, 1024, "vn %f %f %f", n.x, n.y, n.z);
			ss << line << std::endl;
		}
		oFile << ss.str();
	}

	// write all face data (mandatory)
	{
		auto WriteV__ = [](int vidx) -> std::string {
			char line[1024];
			std::snprintf(line, 1024, "%d//", vidx);
			return std::string(line);
		};
		auto WriteV_N = [](int vidx) -> std::string {
			char line[1024];
			std::snprintf(line, 1024, "%d//%d", vidx, vidx);
			return std::string(line);
		};
		auto WriteVT_ = [](int vidx) -> std::string {
			char line[1024];
			std::snprintf(line, 1024, "%d/%d/", vidx, vidx);
			return std::string(line);
		};
		auto WriteVTN = [](int vidx) -> std::string {
			char line[1024];
			std::snprintf(line, 1024, "%d/%d/%d", vidx, vidx, vidx);
			return std::string(line);
		};

		std::function<std::string(int)> WriteV;
		if (NULL == T && NULL == N) WriteV = WriteV__;
		if (NULL == T && NULL != N) WriteV = WriteV_N;
		if (NULL != T && NULL == N) WriteV = WriteVT_;
		if (NULL != T && NULL != N) WriteV = WriteVTN;

		for (int fidx = 0; fidx < F.size() / 3; fidx++)
		{
			// [CAUTION] obj index starts from "1", not "0"
			int vidx0 = F[3 * fidx + 0] + 1;
			int vidx1 = F[3 * fidx + 1] + 1;
			int vidx2 = F[3 * fidx + 2] + 1;

			oFile << "f " + WriteV(vidx0) + " " +  WriteV(vidx1) + " " + WriteV(vidx2) << std::endl;
		}
	}

	// EOF: need one blank at the end of file
	oFile.flush(); // sync file I/O
	oFile.close();
}

void WriteTriMeshObj(const char* filepath, const std::vector<glm::vec3>& V,
	const std::vector<glm::vec2>& T, const std::vector<glm::vec3>& N,
	const std::vector<glm::uint>& F)
{
	_WriteObjFile(filepath, V, &T, &N, F);
}

void WriteTriMeshObj(const char* filepath, const std::vector<glm::vec3>& V,
	const std::vector<glm::vec3>& N,
	const std::vector<glm::uint>& F)
{
	_WriteObjFile(filepath, V, NULL, &N, F);
}

void WriteTriMeshObj(const char* filepath, const std::vector<glm::vec3>& V,
	const std::vector<glm::uint>& F)
{
	_WriteObjFile(filepath, V, NULL, NULL, F);
}

void _ReadObjFile(const char* filepath, std::vector<glm::vec3>& V,
	std::vector<glm::vec2>* T, std::vector<glm::vec3>* N,
	std::vector<glm::uint>& F)
{
	// check the file
	std::ifstream iFile(filepath);
	if (!iFile.is_open()) {
		fprintf(stderr, "ERROR: cannot find a file: %s\n", filepath);
		return;
	}

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

			// two optional cases
			if (tokens[0] == "vt" && NULL != T) {
				glm::vec3 vt;
				vt.x = std::stof(tokens[1]);
				vt.y = std::stof(tokens[2]);
				T->push_back(vt);
			}
			if (tokens[0] == "vn" && NULL != N) {
				glm::vec3 vn;
				vn.x = std::stof(tokens[1]);
				vn.y = std::stof(tokens[2]);
				vn.z = std::stof(tokens[3]);
				N->push_back(vn);
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
	_ReadObjFile(filepath, V, &T, &N, F);
}

void ReadTriMeshObj(const char* filepath, std::vector<glm::vec3>& V, std::vector<glm::vec3>& N, std::vector<glm::uint>& F)
{
	_ReadObjFile(filepath, V, NULL, &N, F);
}

void ReadTriMeshObj(const char* filepath, std::vector<glm::vec3>& V, std::vector<glm::uint>& F)
{
	_ReadObjFile(filepath, V, NULL, NULL, F);
}

}
