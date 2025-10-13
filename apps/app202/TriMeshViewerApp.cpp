#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>
#include <vgl/objFileIO.h>
#include <vgl/TriMesh.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <set>

#include <tinyfiledialogs.h>

class TriMeshViewerApp : public AppGLBase
{
public:
	TriMeshViewerApp(const int width, const int height) : AppGLBase(width, height) {};

	void Draw(const int width, const int height)
	{
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::infinitePerspective(glm::radians(53.1301f), width / (float)height, 0.1f);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), -GlobalViewPosition);
		view = view * glm::mat4_cast(GlobalViewRotation);

		glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

		vgl::setLight(GL_LIGHT0, L_position);
		vgl::setMaterial(GL_FRONT_AND_BACK);

		// draw triangle mesh
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_LIGHTING);
		glEnable(GL_RESCALE_NORMAL);
		vgl::drawTriMesh(vertices_subdiv, normals_subdiv, faces_subdiv);
		glDisable(GL_RESCALE_NORMAL);
		glDisable(GL_LIGHTING);

		// draw its wireframe
		if (show_wireframe)
		{
			glColor3f(0.0f, 0.0f, 0.0f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			vgl::drawTriMesh(vertices_subdiv, faces_subdiv);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glColor3f(1.0f, 1.0f, 1.0f);
		}
	}

	void Subdivide()
	{
		vertices_subdiv = vertices;
		faces_subdiv    = faces;

		for (int n = 0; n < N_subdiv; n++)
		{
			std::vector<glm::vec3> vertices_subdiv_new = vertices_subdiv;

			std::map<vgl::Edge, int> vertices_map;
			std::map<vgl::Edge, std::set<glm::uint>> adjacent_verts;

			// add new edge vertex (odd vertex)
			auto AddNewVertex = [&](int vidx0, int vidx1, int vidx2_adj) {
				vgl::Edge e01 = vgl::Edge(vidx0, vidx1);

				glm::vec3 v0 = vertices_subdiv[e01.idx0];
				glm::vec3 v1 = vertices_subdiv[e01.idx1];

				// mid-point as a temporary position
				glm::vec3 v01_mid = 0.5f * (v0 + v1);
				
				if (vertices_map.end() == vertices_map.find(e01)) {
					int vidx01 = vertices_subdiv_new.size();
					vertices_subdiv_new.push_back(v01_mid);
					vertices_map.insert(std::make_pair(e01, vidx01));
				}

				if (adjacent_verts.end() == adjacent_verts.find(e01)) {
					adjacent_verts.insert(std::make_pair(e01, std::set<glm::uint>()));
				}
				std::set<glm::uint>& verts = adjacent_verts.find(e01)->second;
				verts.insert(vidx2_adj);
			};

			// make new vertices between each edge
			for (int fidx = 0; fidx < faces_subdiv.size() / 3; fidx++)
			{
				int vidx0 = faces_subdiv[3 * fidx + 0];
				int vidx1 = faces_subdiv[3 * fidx + 1];
				int vidx2 = faces_subdiv[3 * fidx + 2];

				AddNewVertex(vidx0, vidx1, vidx2);
				AddNewVertex(vidx1, vidx2, vidx0);
				AddNewVertex(vidx2, vidx0, vidx1);
			}

			// compute valence of each vertices
			std::vector<std::set<glm::uint>> valence(vertices_subdiv.size());
			for (auto kv : vertices_map)
			{
				valence[kv.first.idx0].insert(kv.first.idx1);
				valence[kv.first.idx1].insert(kv.first.idx0);
			}

			// reposition of even vertices by weight average
			for (int vidx = 0; vidx < vertices_subdiv.size(); vidx++)
			{
				float N = (float)valence[vidx].size();
				glm::vec3 v_even = vertices_subdiv[vidx];

				float inner = 3.0f / 8.0f + 1.0f / 4.0f * glm::cos(glm::two_pi<float>() / N);;
				float beta = 1.0f / N * (5.0f / 8.0f - inner * inner);

				// [TODO]

				for (auto vidx_j : valence[vidx]) {
					glm::vec3 v_neighbor = vertices_subdiv[vidx_j];

					// [TODO]
				}
				vertices_subdiv_new[vidx] = v_even;
			}

			// reposition of odd vertices
			for (auto kv : adjacent_verts)
			{
				int vidx01 = vertices_map.find(kv.first)->second;
				glm::vec3 v_odd = vertices_subdiv_new[vidx01];

				glm::vec3 v1 = vertices_subdiv[kv.first.idx0];
				glm::vec3 v2 = vertices_subdiv[kv.first.idx1];

				if (kv.second.size() == 2) {
					auto it = kv.second.begin();
					glm::vec3 v3 = vertices_subdiv[*it];
					it++;
					glm::vec3 v4 = vertices_subdiv[*it];

					// [TODO]

					vertices_subdiv_new[vidx01] = v_odd;
				}
				else if (kv.second.size() == 1) {
					// do nothing: it is already mid point
				}
				else {
					fprintf(stderr, "ERROR: something weird happened. maybe input mesh was non-manifold.\n");
				}
			}

			std::vector<glm::uint> faces_subdiv_new;
			auto AddFace = [&](glm::uint vidx0, glm::uint vidx1, glm::uint vidx2)
			{
				faces_subdiv_new.push_back(vidx0);
				faces_subdiv_new.push_back(vidx1);
				faces_subdiv_new.push_back(vidx2);
			};

			for (int f = 0; f < faces_subdiv.size(); f += 3)
			{
				int vidx0 = faces_subdiv[f + 0];
				int vidx1 = faces_subdiv[f + 1];
				int vidx2 = faces_subdiv[f + 2];

				// add to container
				auto e01 = vgl::Edge(vidx0, vidx1);
				auto e12 = vgl::Edge(vidx1, vidx2);
				auto e20 = vgl::Edge(vidx2, vidx0);

				int vidx01 = vertices_map.find(e01)->second;
				int vidx12 = vertices_map.find(e12)->second;
				int vidx20 = vertices_map.find(e20)->second;

				// add 4 faces in new face array
				AddFace(vidx0 , vidx01, vidx20);
				AddFace(vidx01, vidx1 , vidx12);
				AddFace(vidx12, vidx20, vidx01);
				AddFace(vidx2 , vidx20, vidx12);
			}

			// transfer data for the next iteration
			vertices_subdiv = vertices_subdiv_new;
			faces_subdiv = faces_subdiv_new;
		}

		printf("subdivision level (%d): mesh (%4d, %4d)\n", N_subdiv, vertices_subdiv.size(), faces_subdiv.size() / 3);

		// in the end, compute vertex normals from triangles
		normals_subdiv = vgl::ComputeVertexNormals(vertices_subdiv, faces_subdiv);
	}

	bool Init()
	{
		glfwSetWindowTitle(window, "TriMeshViewerApp");

		TwAddButton(bar, "import_mesh", [](void* client) {
			TriMeshViewerApp* _this = (TriMeshViewerApp*)client;
			_this->LoadObjFile();
		}, this, " ");
		TwAddButton(bar, "export_mesh", [](void* client) {
			TriMeshViewerApp* _this = (TriMeshViewerApp*)client;
			_this->SaveObjFile();
		}, this, " ");
#if 1
		TwAddButton(bar, "Global-init", [](void* client) {
			TriMeshViewerApp* _this = (TriMeshViewerApp*)client;
			_this->resetGlobalView();
		}, this, "group='Global' label='init' ");
		TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation  , "group='Global' label='rot'  open");
		TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
		TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
		TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif
		TwAddVarRW(bar, "Light_dir", TwType::TW_TYPE_DIR3F, &L_position, "label='Light_dir' open");
		TwAddVarRW(bar, "show_wireframe", TwType::TW_TYPE_BOOLCPP, &show_wireframe, "label='show_wireframe'");
		TwAddVarCB(bar, "N_subdiv", TwType::TW_TYPE_INT32,
			[](const void* value, void* obj) {
				TriMeshViewerApp* _this = (TriMeshViewerApp*)obj;
				_this->N_subdiv = *(int*)value;
				_this->Subdivide();
			},
			[](void* value, void* obj) {
				TriMeshViewerApp* _this = (TriMeshViewerApp*)obj;
				*(int*)value = _this->N_subdiv;
			},
			this, " min=0 max=4");

		resetGlobalView();

		// set hidden surface removal flags
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		// set some offset trick to show wireframe clearly
		glEnable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(1.0f, 1.0f);

		return true;
	}

private:
	glm::quat GlobalViewRotation;
	glm::vec3 GlobalViewPosition;
	void resetGlobalView() {
		GlobalViewPosition = glm::vec3(0.0f, 0.0f, 4.0f);
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
	}
	glm::vec4 L_position = glm::vec4(-0.45f, 0.9f, 0.1f, 0.0f);
	bool show_wireframe = false;

	void LoadObjFile()
	{
		char const* filterPatterns[1] = { "*.obj" };
		char* filepath = tinyfd_openFileDialog(
			"Read .obj mesh file",
			"./", 1, filterPatterns, NULL, 0);

		if (filepath)
		{
			printf("filepath: %s\n", filepath);

			// clear triangle mesh data in advance
			vertices.clear();
			faces.clear();

			vgl::ReadTriMeshObj(filepath, vertices, faces);
			Subdivide();
		}
	}
	void SaveObjFile()
	{
		char const* filterPatterns[1] = { "*.obj" };
		char* filepath = tinyfd_saveFileDialog(
			"Write .obj mesh file",
			"./", 1, filterPatterns, NULL);

		if (filepath)
		{
			printf("filepath: %s\n", filepath);

			vgl::WriteTriMeshObj(filepath, vertices_subdiv, faces_subdiv);
		}
	}

	// triangle mesh
	std::vector<glm::vec3> vertices;
	std::vector<glm::uint> faces;

	// triangle mesh by subdivision
	std::vector<glm::vec3> vertices_subdiv;
	std::vector<glm::vec3> normals_subdiv;
	std::vector<glm::uint> faces_subdiv;
	int N_subdiv = 0;
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	TriMeshViewerApp app(640, 480);
	app.SetInternalProcess(true);
	app.run();
	return EXIT_SUCCESS;
}
