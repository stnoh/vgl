/******************************************************************************
Utility functions for 3D triangle mesh with GL
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vgl/TriMesh.h>

namespace vgl {

void AddFace(std::vector<glm::uint>& faces, glm::uint vidx0, glm::uint vidx1, glm::uint vidx2)
{
	faces.push_back(vidx0);
	faces.push_back(vidx1);
	faces.push_back(vidx2);
}

///////////////////////////////////////////////////////////////////////////////
// IcoSphere obtained from icosahedron subdivision
///////////////////////////////////////////////////////////////////////////////
IcoSphere::IcoSphere(int level, float radius)
{
	// golden ratio
	const float tau = (1.0f + sqrtf(5.0f)) / 2.0f;

	////////////////////////////////////////
	// icosahedron: 12 vertices and 20 faces
	////////////////////////////////////////

	// rectangular in XY
	vertices.push_back(glm::vec3(-1.0, +tau, 0));
	vertices.push_back(glm::vec3(+1.0, +tau, 0));
	vertices.push_back(glm::vec3(-1.0, -tau, 0));
	vertices.push_back(glm::vec3(+1.0, -tau, 0));

	// rectangular in YZ
	vertices.push_back(glm::vec3(0, -1.0, +tau));
	vertices.push_back(glm::vec3(0, +1.0, +tau));
	vertices.push_back(glm::vec3(0, -1.0, -tau));
	vertices.push_back(glm::vec3(0, +1.0, -tau));

	// rectangular in ZX
	vertices.push_back(glm::vec3(+tau, 0, -1.0));
	vertices.push_back(glm::vec3(+tau, 0, +1.0));
	vertices.push_back(glm::vec3(-tau, 0, -1.0));
	vertices.push_back(glm::vec3(-tau, 0, +1.0));

	AddFace(faces, 0, 11, 5);
	AddFace(faces, 0, 5, 1);
	AddFace(faces, 0, 1, 7);
	AddFace(faces, 0, 7, 10);
	AddFace(faces, 0, 10, 11);

	AddFace(faces, 1, 5, 9);
	AddFace(faces, 5, 11, 4);
	AddFace(faces, 11, 10, 2);
	AddFace(faces, 10, 7, 6);
	AddFace(faces, 7, 1, 8);

	AddFace(faces, 3, 9, 4);
	AddFace(faces, 3, 4, 2);
	AddFace(faces, 3, 2, 6);
	AddFace(faces, 3, 6, 8);
	AddFace(faces, 3, 8, 9);

	AddFace(faces, 4, 9, 5);
	AddFace(faces, 2, 4, 11);
	AddFace(faces, 6, 2, 10);
	AddFace(faces, 8, 6, 7);
	AddFace(faces, 9, 8, 1);

	for (int n = 0; n < level; n++) subdivide();

	for (int n = 0; n < vertices.size(); n++) vertices[n] = radius * glm::normalize(vertices[n]);

	for (auto v : vertices) normals.push_back(glm::normalize(v));
}

void IcoSphere::subdivide() {

	std::map<vgl::Edge, int> vertices_map;

	auto AddNewVertex = [&](int vidx0, int vidx1) {
		vgl::Edge e01 = vgl::Edge(vidx0, vidx1);

		glm::vec3 v0 = vertices[e01.idx0];
		glm::vec3 v1 = vertices[e01.idx1];

		glm::vec3 v01_mid = 0.5f * (v0 + v1);
		if (vertices_map.end() == vertices_map.find(e01)) {
			int vidx01 = vertices.size();
			vertices.push_back(v01_mid);
			vertices_map.insert(std::make_pair(e01, vidx01));
		}
	};

	// make new vertices between each edge
	for (int f = 0; f < faces.size(); f += 3)
	{
		int vidx0 = faces[f + 0];
		int vidx1 = faces[f + 1];
		int vidx2 = faces[f + 2];

		AddNewVertex(vidx0, vidx1);
		AddNewVertex(vidx1, vidx2);
		AddNewVertex(vidx2, vidx0);
	}

	std::vector<glm::uint> new_faces;

	for (int f = 0; f < faces.size(); f += 3)
	{
		int vidx0 = faces[f + 0];
		int vidx1 = faces[f + 1];
		int vidx2 = faces[f + 2];

		// add to container
		auto e01 = vgl::Edge(vidx0, vidx1);
		auto e12 = vgl::Edge(vidx1, vidx2);
		auto e20 = vgl::Edge(vidx2, vidx0);

		int vidx01 = vertices_map.find(e01)->second;
		int vidx12 = vertices_map.find(e12)->second;
		int vidx20 = vertices_map.find(e20)->second;

		// add 4 faces in new face array
		AddFace(new_faces, vidx0 , vidx01, vidx20);
		AddFace(new_faces, vidx01, vidx1 , vidx12);
		AddFace(new_faces, vidx12, vidx20, vidx01);
		AddFace(new_faces, vidx2 , vidx20, vidx12);
	}

	faces = new_faces;
}

vgl::PlaneXY::PlaneXY(int subdiv, float scale)
{
	for (int j = 0; j <= subdiv; j++)
	for (int i = 0; i <= subdiv; i++)
	{
		float x = scale * ((i + 0) / (float)subdiv - 0.5f);
		float y = scale * ((j + 0) / (float)subdiv - 0.5f);

		vertices.push_back(glm::vec3(x, y, 0.0f));
		normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
	}

	for (int j = 0; j < subdiv; j++)
	for (int i = 0; i < subdiv; i++)
	{
		int i0 = i;
		int i1 = i + 1;
		int j0 = j;
		int j1 = j + 1;

		int vidx0 = i0 + j0 * subdiv;
		int vidx1 = i1 + j0 * subdiv;
		int vidx2 = i0 + j1 * subdiv;
		int vidx3 = i1 + j1 * subdiv;

		AddFace(faces, vidx0, vidx1, vidx3);
		AddFace(faces, vidx0, vidx3, vidx2);
	}
}


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
std::pair<std::vector<glm::vec3>, std::map<glm::uint, std::set<glm::uint>>> ComputeFaceNormals(std::vector<glm::vec3> vertices, std::vector<glm::uint> faces)
{
	std::vector<glm::vec3> face_normals;
	std::map<glm::uint, std::set<glm::uint>> vidx_to_face_indices;

	// compute face normals and vertex-to-face indices
	for (int f = 0; f < faces.size(); f += 3)
	{
		// get vertex indices of this triangle
		int vidx0 = faces[f + 0];
		int vidx1 = faces[f + 1];
		int vidx2 = faces[f + 2];

		if (vidx_to_face_indices.end() == vidx_to_face_indices.find(vidx0)) vidx_to_face_indices.insert(std::make_pair(vidx0, std::set<glm::uint>()));
		if (vidx_to_face_indices.end() == vidx_to_face_indices.find(vidx1)) vidx_to_face_indices.insert(std::make_pair(vidx1, std::set<glm::uint>()));
		if (vidx_to_face_indices.end() == vidx_to_face_indices.find(vidx2)) vidx_to_face_indices.insert(std::make_pair(vidx2, std::set<glm::uint>()));

		vidx_to_face_indices.find(vidx0)->second.insert(f);
		vidx_to_face_indices.find(vidx1)->second.insert(f);
		vidx_to_face_indices.find(vidx2)->second.insert(f);

		glm::vec3 v0 = vertices[vidx0];
		glm::vec3 v1 = vertices[vidx1];
		glm::vec3 v2 = vertices[vidx2];

		face_normals.push_back(glm::cross(v1 - v0, v2 - v0));
	}

	return { face_normals, vidx_to_face_indices };
}
std::vector<glm::vec3> ComputeVertexNormals(std::vector<glm::vec3> vertices, std::vector<glm::uint> faces)
{
	std::vector<glm::vec3> face_normals;
	std::map<glm::uint, std::set<glm::uint>> vertex_to_faces;
	
	// limitation until C++14
	auto data = ComputeFaceNormals(vertices, faces);
	face_normals = data.first;
	vertex_to_faces = data.second;

	std::vector<glm::vec3> vertex_normals;

	// average vertex normals
	for (int n = 0; n < vertices.size(); n++)
	{
		glm::vec3 normal = glm::vec3();

		std::set<glm::uint> f_indices = vertex_to_faces.find(n)->second;

		for (int f = 0; f < f_indices.size(); f++)
		{
			normal += face_normals[f];
		}

		vertex_normals.push_back(normal / (float)f_indices.size());
	}

	return vertex_normals;
}

}
