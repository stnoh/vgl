/******************************************************************************
Utility functions for 3D triangle mesh with GL
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <map>
#include <set>
#include <vector>
#include <glm/glm.hpp>

namespace vgl {

class Edge {
public:
	Edge(int _idx0, int _idx1) :
		idx0(std::min(_idx0, _idx1)),
		idx1(std::max(_idx0, _idx1)) {}

	int idx0; // smaller vertex index
	int idx1; // larger  vertex index

	bool operator< (const Edge& n) const {
		if (this->idx0 < n.idx0) return true;
		if (this->idx0 == n.idx0) {
			if (this->idx1 < n.idx1) return true;
		}
		return false;
	}
};

class IcoSphere
{
public:
	IcoSphere(int level = 0, float radius = 1.5f);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::uint> faces;

private:
	void subdivide();
};

std::pair<std::vector<glm::vec3>, std::map<glm::uint, std::set<glm::uint>>> ComputeFaceNormals(std::vector<glm::vec3> vertices, std::vector<glm::uint> faces);
std::vector<glm::vec3> ComputeVertexNormals(std::vector<glm::vec3> vertices, std::vector<glm::uint> faces);

}
