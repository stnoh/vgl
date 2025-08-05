/******************************************************************************
Helper for stanford mesh format (.ply) I/O
This is temporary code. I have a plan to re-write or dispose this code soon.
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vgl/plyFileIO.h>
#include <sstream>
#include <iterator>

namespace vgl {

///////////////////////////////////////////////////////////////////////////////
// internal method: write lines of header, vertices and faces
///////////////////////////////////////////////////////////////////////////////
std::stringstream _WritePlyHeader(const int nofVerts, const bool existN, const bool existC, const int nofFaces)
{
	std::stringstream ss;

	// header
	ss << "ply" << std::endl;
	ss << "format ascii 1.0" << std::endl;

	// vertex information
	ss << "element vertex " << nofVerts << std::endl;

	// vertex position (default)
	ss << "property float x" << std::endl;
	ss << "property float y" << std::endl;
	ss << "property float z" << std::endl;

	// vertex normal (if exists)
	if (existN) {
		ss << "property float nx" << std::endl;
		ss << "property float ny" << std::endl;
		ss << "property float nz" << std::endl;
	}

	// vertex color (if exists)
	if (existC) {
		ss << "property uchar red" << std::endl;
		ss << "property uchar green" << std::endl;
		ss << "property uchar blue" << std::endl;
	}

	// face information
	if (0 != nofFaces) {
		ss << "element face " << nofFaces << std::endl;
		ss << "property list uchar int vertex_indices" << std::endl;
	}

	ss << "end_header" << std::endl;

	return ss;
}

std::stringstream _WriteVertices(const std::vector<glm::vec3> P,
	const bool existN, const std::vector<glm::vec3> N,
	const bool existC, const std::vector<glm::u8vec3> C)
{
	std::stringstream ss;
	for (int i = 0; i < P.size(); i++) {
		char line[1024];

		// vertex position as default
		glm::vec3 p = P[i];
		std::snprintf(line, 1024, "%f %f %f", p.x, p.y, p.z);
		ss << line;

		// vertex normal
		if (existN) {
			glm::vec3 n = N[i];
			std::snprintf(line, 1024, " %f %f %f", n.x, n.y, n.z);
			ss << line;
		}

		// vertex color
		if (existC) {
			glm::u8vec3 c = C[i];
			std::snprintf(line, 1024, " %d %d %d", c.r, c.g, c.b);
			ss << line;
		}

		// end of this row
		ss << std::endl;
	}
	return ss;
}

std::stringstream _WriteTriFaces(const std::vector<glm::uint> F)
{
	std::stringstream ss;
	for (int i = 0; i < F.size(); i+=3) {
		int idx0 = F[i + 0];
		int idx1 = F[i + 1];
		int idx2 = F[i + 2];

		char line[1024];
		std::snprintf(line, 1024, "3 %d %d %d", idx0, idx1, idx2);
		ss << line << std::endl;
	}
	return ss;
}

void _WritePly(const char* filepath, const std::vector<glm::vec3> P,
	const bool existN, const std::vector<glm::vec3> N,
	const bool existC, const std::vector<glm::u8vec3> C,
	const bool existF, const std::vector<glm::uint> F)
{
	// check vertex normal size
	if (!existN) {
		if ( !N.empty() && P.size() != N.size()) {
			fprintf(stderr, "ERROR: normal data exists, but it does not match with size of position data.\n");
			return;
		}
	}

	// check vertex color size
	if (!existC) {
		if ( !C.empty() && P.size() != C.size()) {
			fprintf(stderr, "ERROR: color data exists, but it does not match with size of position data.\n");
			return;
		}
	}

	// check face size
	if (!existF) {
		if ( !F.empty() && 0 != F.size() % 3) {
			fprintf(stderr, "ERROR: face information exists, but it has an error in size.\n");
			return;
		}
	}

	std::ofstream oFile(filepath);

	int nofFaces = (existF) ? F.size()/3 : 0 ;
	oFile << _WritePlyHeader(P.size(), existN, existC, nofFaces).str();

	oFile << _WriteVertices(P, existN, N, existC, C).str();

	if (existF) {
		oFile << _WriteTriFaces(F).str();
	}

	// EOF: need one blank at the end of file
	oFile << " " << std::endl;
	oFile.flush(); // sync file I/O
	oFile.close();
}


///////////////////////////////////////////////////////////////////////////////
// public method : write ply file
///////////////////////////////////////////////////////////////////////////////
void WritePointCloudPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::vec3> N, std::vector<glm::u8vec3> C)
{
	std::vector<glm::uint> F;
	_WritePly(filepath, P, true, N, true, C, false, F);
}
void WritePointCloudPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::vec3> N)
{
	std::vector<glm::u8vec3> C;
	std::vector<glm::uint> F;
	_WritePly(filepath, P, true, N, false, C, false, F);
}
void WritePointCloudPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::u8vec3> C)
{
	std::vector<glm::vec3> N;
	std::vector<glm::uint> F;
	_WritePly(filepath, P, false, N, true, C, false, F);
}
void WritePointCloudPly(const char* filepath, std::vector<glm::vec3> P)
{
	std::vector<glm::vec3> N;
	std::vector<glm::u8vec3> C;
	std::vector<glm::uint> F;
	_WritePly(filepath, P, false, N, false, C, false, F);
}

void WriteTriMeshPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::vec3> N, std::vector<glm::u8vec3> C, std::vector<glm::uint> F)
{
	_WritePly(filepath, P, true, N, true, C, true, F);
}
void WriteTriMeshPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::vec3> N, std::vector<glm::uint> F)
{
	std::vector<glm::u8vec3> C;
	_WritePly(filepath, P, true, N, false, C, true, F);
}
void WriteTriMeshPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::u8vec3> C, std::vector<glm::uint> F)
{
	std::vector<glm::vec3> N;
	_WritePly(filepath, P, false, N, true, C, true, F);
}
void WriteTriMeshPly(const char* filepath, std::vector<glm::vec3> P, std::vector<glm::uint> F)
{
	std::vector<glm::vec3> N;
	std::vector<glm::u8vec3> C;
	_WritePly(filepath, P, false, N, false, C, true, F);
}


///////////////////////////////////////////////////////////////////////////////
// internal method: read lines of header, vertices and faces
///////////////////////////////////////////////////////////////////////////////
void _ReadPlyHeader(std::ifstream& iFile, int& nofVerts, int& nofFaces)
{
	// 1. check first line "ply"
	std::string line;
	std::getline(iFile, line);
	if (0 < line.compare("ply")) return;
//	printf("ply\n");

	// 2. check "format ascii 1.0"
	std::getline(iFile, line);
	if (0 < line.compare("format ascii 1.0")) return;
//	printf("format ascii 1.0\n");

	// 3. get num of verts from "element vertex {#ofVertices}"
	int offset;
	while (std::getline(iFile, line)) {
		offset = line.find("element vertex");
		if (0 <= offset) break;
	}

	std::string num = line.substr(14, line.size() - 14); // element vertex
	nofVerts = std::stoi(num);
//	printf("nofVerts = %d\n", nofVerts);

	// 4. get vertex properties *********************************************** [TODO] get vertex properties from file

	// 5. get num of faces from "element vertex {#ofFaces}" ******************* [TODO] check trimesh or not

	// 6. find "end_header"
	while (std::getline(iFile, line)) {
		if (0 == line.find("end_header")) break;

		offset = line.find("element face");
		if (0 <= offset) {
			std::string num = line.substr(12, line.size() - 12); // element vertex
			nofFaces = std::stoi(num);
		}
	}
}

void _ReadVertices(std::ifstream& iFile, std::vector<glm::vec3>& P,
	const bool existN, std::vector<glm::vec3>& N,
	const bool existC, std::vector<glm::u8vec3>& C)
{
	for (int i = 0; i < P.size(); i++) {
		int count = 0;

		std::string line;
		std::getline(iFile, line);

		// string to separated strings
		std::stringstream strstr(line);
		std::istream_iterator<std::string> it(strstr);
		std::istream_iterator<std::string> end;
		std::vector<std::string> results(it, end);

		// vertex position as default
		float x = std::stof(results[count++]);
		float y = std::stof(results[count++]);
		float z = std::stof(results[count++]);
		P[i] = glm::vec3(x, y, z);

		// vertex normal
		if (existN) {
			float nx = std::stof(results[count++]);
			float ny = std::stof(results[count++]);
			float nz = std::stof(results[count++]);
			N[i] = glm::vec3(nx, ny, nz);
		}

		// vertex color
		if (existC) {
			unsigned char r = std::stoi(results[count++]);
			unsigned char g = std::stoi(results[count++]);
			unsigned char b = std::stoi(results[count++]);
			C[i] = glm::vec3(r, g, b);
		}
	}
}

void _ReadTriFaces(std::ifstream& iFile, int nofFaces, std::vector<glm::uint>& F)
{
	// face data : suppose {3 index0 index1 index2}
	for (int i = 0; i < nofFaces; i++) {
		std::string line;
		std::getline(iFile, line);

		// string to separated strings
		std::stringstream strstr(line);
		std::istream_iterator<std::string> it(strstr);
		std::istream_iterator<std::string> end;
		std::vector<std::string> results(it, end);

		unsigned int n = std::stoi(results[0]);
		unsigned int i0 = std::stoi(results[1]);
		unsigned int i1 = std::stoi(results[2]);
		unsigned int i2 = std::stoi(results[3]);

		if (3 != n) {
			fprintf(stderr, "ERROR: this file has non-triangle face.\n");;
		}

		F[3 * i + 0] = i0;
		F[3 * i + 1] = i1;
		F[3 * i + 2] = i2;
	}
}

void _ReadPly(const char* filepath, std::vector<glm::vec3>& P,
	const bool existN, std::vector<glm::vec3>& N,
	const bool existC, std::vector<glm::u8vec3>& C,
	const bool existF, std::vector<glm::uint>& F)
{
	// check the file
	std::ifstream iFile(filepath);
	if (!iFile.is_open()) {
		fprintf(stderr, "ERROR: cannot find a file: %s\n", filepath);
		return;
	}

	// read header
	int nofVerts, nofFaces;
	_ReadPlyHeader(iFile, nofVerts, nofFaces);

	// read vertices
	P.resize(nofVerts);
	if (existN) N.resize(nofVerts);
	if (existC) C.resize(nofVerts);
	_ReadVertices(iFile, P, existN, N, existC, C);

	// read faces
	if (existF) {
		F.resize(nofFaces * 3);
		_ReadTriFaces(iFile, nofFaces, F);
	}

	// check last line with '{blank}' ***************************************** [TODO] need to check
	iFile.close();
}


///////////////////////////////////////////////////////////////////////////////
// public method : read ply file
///////////////////////////////////////////////////////////////////////////////
void ReadPointCloudPly(const char* filepath, std::vector<glm::vec3> &P, std::vector<glm::vec3> &N, std::vector<glm::u8vec3> &C)
{
	std::vector<glm::uint> F;
	_ReadPly(filepath, P, true, N, true, C, false, F);
}
void ReadPointCloudPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::vec3>& N)
{
	std::vector<glm::u8vec3> C;
	std::vector<glm::uint> F;
	_ReadPly(filepath, P, true, N, false, C, false, F);
}
void ReadPointCloudPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::u8vec3>& C)
{
	std::vector<glm::vec3> N;
	std::vector<glm::uint> F;
	_ReadPly(filepath, P, false, N, true, C, false, F);
}
void ReadPointCloudPly(const char* filepath, std::vector<glm::vec3>& P)
{
	std::vector<glm::vec3> N;
	std::vector<glm::u8vec3> C;
	std::vector<glm::uint> F;
	_ReadPly(filepath, P, false, N, false, C, false, F);
}

void ReadTriMeshPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::vec3>& N, std::vector<glm::u8vec3>& C, std::vector<glm::uint>& F)
{
	_ReadPly(filepath, P, true, N, true, C, true, F);
}
void ReadTriMeshPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::vec3>& N, std::vector<glm::uint>& F)
{
	std::vector<glm::u8vec3> C;
	_ReadPly(filepath, P, true, N, false, C, true, F);
}
void ReadTriMeshPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::u8vec3>& C, std::vector<glm::uint>& F)
{
	std::vector<glm::vec3> N;
	_ReadPly(filepath, P, false, N, true, C, true, F);
}
void ReadTriMeshPly(const char* filepath, std::vector<glm::vec3>& P, std::vector<glm::uint>& F)
{
	std::vector<glm::vec3> N;
	std::vector<glm::u8vec3> C;
	_ReadPly(filepath, P, false, N, false, C, true, F);
}

}
