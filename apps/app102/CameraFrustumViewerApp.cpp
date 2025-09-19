#include "CameraFrustumViewerApp.h"
#include <vgl/plyFileIO.h>
#include <tinyfiledialogs.h>


///////////////////////////////////////////////////////////////////////////////
// mandatory member function
///////////////////////////////////////////////////////////////////////////////
void CameraFrustumViewerApp::Draw(const int width, const int height)
{
	// left / right
	int w_2 = width / 2;

	// viewport settings ( 0:left / 1:right )
	int x[2] = { 0, w_2 };
	int y[2] = { 0, height - w_2 };
	int w[2] = { w_2, w_2 };
	int h[2] = { height, w_2 }; // keep the same aspect (1:1)

	glEnable(GL_SCISSOR_TEST); // to cut left/right viewport
	for (int id = 0; id < 2; id++)
	{
		glViewport(x[id], y[id], w[id], h[id]);
		glScissor (x[id], y[id], w[id], h[id]);

		glClearColor(BgColor[id].r, BgColor[id].g, BgColor[id].b, BgColor[id].a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = rendercam->GetProjMatrix();
		glm::mat4 view = rendercam->GetViewMatrix();

		// for global viewer (id:0)
		if (0 == id) {
			proj = glm::infinitePerspective(glm::radians(53.1301f), w[0] / (float)h[0], 0.1f);
			view = glm::translate(glm::mat4(1.0f), -GlobalViewPosition);
			view = view * glm::mat4_cast(GlobalViewRotation);
		}

		glEnable(GL_DEPTH_TEST);
		drawView3D(proj, view);
		glDisable(GL_DEPTH_TEST);
	}
	glDisable(GL_SCISSOR_TEST);
}

void CameraFrustumViewerApp::drawView3D(glm::mat4 proj, glm::mat4 view)
{
	glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
	glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));
	vgl::setLight(GL_LIGHT0, glm::vec4(-1.0f, -1.0f, -1.0f, 0.0f)); // default light

	// model matrix
	glm::mat4 model = glm::translate(glm::mat4(1.0f), ModelPosition) * glm::mat4(ModelRotation);
	model = glm::scale(model, glm::vec3(ModelUniScale));

	////////////////////////////////////////////////////////////
	// model space
	////////////////////////////////////////////////////////////
	glEnable(GL_RESCALE_NORMAL);
	{
		glPushMatrix(); glMultMatrixf(glm::value_ptr(model));

		// draw 3D axes
		glEnable(GL_LIGHTING);
		if (V.empty()) vgl::drawAxes(1.0f);
		glDisable(GL_LIGHTING);

		// draw original mesh with normal map color
		vgl::drawTriMesh(V, N, C, F);

		// draw visible submesh with yellow color
		glColor3f(1.0f, 1.0f, 0.0f);

		glEnable(GL_POLYGON_OFFSET_FILL); // for superimpose
		glPolygonOffset(-0.5f, 0.5f);
		vgl::drawTriMesh(V, F_visible);
		glDisable(GL_POLYGON_OFFSET_FILL);

		glPopMatrix();
	}
	glDisable(GL_RESCALE_NORMAL);


	////////////////////////////////////////////////////////////
	// camera space
	////////////////////////////////////////////////////////////
	glColor3f(1.0f, 1.0f, 0.0f);
	glLineWidth(2.0f);
	rendercam->SetCameraCoord([&] {

		// draw visible point cloud with cyan color
		glColor3f(0.0f, 1.0f, 1.0f);
		vgl::drawPointCloud(P_visible);

	}, true);
}


///////////////////////////////////////////////////////////////////////////////
// override member functions
///////////////////////////////////////////////////////////////////////////////
bool CameraFrustumViewerApp::Init()
{
	// instantiate camera object
	rendercam = new vgl::GLCamera();
	rendercam->SetCameraMatrixCV({ 1.0f, 1.0f, 0.5f, 0.5f }, 1.0f, 30.0f);
	rendercam->z_near = 0.1f;
	rendercam->z_far  = 3.0f;

	resetGlobalView();
	resetCameraView();
	resetModelMatrix();

	// instantiate frame buffer object
	offscreenFBO = new vgl::FBO();

	////////////////////////////////////////////////////////////
	// UI: AntTweakBar
	////////////////////////////////////////////////////////////
	TwDefine("Bar size='250 550'");

	// global viewer
#if 1
	TwAddButton(bar, "Global-init", [](void *client) {
		CameraFrustumViewerApp* _this = (CameraFrustumViewerApp*)client; _this->resetGlobalView();
	}, this, "group='Global' label='init' ");
	TwAddVarRW(bar, "Global-rot" , TwType::TW_TYPE_QUAT4F, &GlobalViewRotation , "group='Global' label='rot'  open");
	TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
	TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
	TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif

	// model coordinate
#if 1
	TwAddButton(bar, "Model-LoadMesh",
		[](void* client) {
		CameraFrustumViewerApp* _this = (CameraFrustumViewerApp*)client;
		_this->LoadMesh();
	}, this, "group='Model' label='LoadMesh' key=F3");

	TwAddButton(bar, "Model-init", [](void* client) {
		CameraFrustumViewerApp* _this = (CameraFrustumViewerApp*)client; _this->resetModelMatrix();
	}, this, "group='Model' label='init' ");
	TwAddVarRW(bar, "Model-rot", TwType::TW_TYPE_QUAT4F, &ModelRotation, "group='Model' label='rot'   open");
	TwAddVarRW(bar, "Model-posX", TwType::TW_TYPE_FLOAT, &ModelPosition.x, "group='Model' label='posX'  step=0.01");
	TwAddVarRW(bar, "Model-posY", TwType::TW_TYPE_FLOAT, &ModelPosition.y, "group='Model' label='posY'  step=0.01");
	TwAddVarRW(bar, "Model-posZ", TwType::TW_TYPE_FLOAT, &ModelPosition.z, "group='Model' label='posZ'  step=0.01");
	TwAddVarRW(bar, "Model-scale", TwType::TW_TYPE_FLOAT, &ModelUniScale, "group='Model' label='scale' step=0.01");
#endif

	// view matrix related properties (local camera)
#if 1
	TwAddButton(bar, "Local-init", [](void *client) {
		CameraFrustumViewerApp* _this = (CameraFrustumViewerApp*)client; _this->resetCameraView();
	}, this, "group='Local' label='init' ");
	TwAddVarRW(bar, "Local-rot" , TwType::TW_TYPE_QUAT4F, &rendercam->rotation  , "group='Local' label='rot'  open");
	TwAddVarRW(bar, "Local-posX", TwType::TW_TYPE_FLOAT , &rendercam->position.x, "group='Local' label='posX' step=0.01");
	TwAddVarRW(bar, "Local-posY", TwType::TW_TYPE_FLOAT , &rendercam->position.y, "group='Local' label='posY' step=0.01");
	TwAddVarRW(bar, "Local-posZ", TwType::TW_TYPE_FLOAT , &rendercam->position.z, "group='Local' label='posZ' step=0.01");

	TwDefine("Bar/Local opened=false"); // close group as default
#endif

	// draw visible faces computed from depth image
#if 1
	TwAddButton(bar, "GetVisibleFaces",
		[](void* client) {
		CameraFrustumViewerApp* _this = (CameraFrustumViewerApp*)client;
		_this->GetVisibleFaces();
	}, this, "key=SPACE");
	TwAddButton(bar, "GetVisiblePointCloud",
		[](void* client) {
		CameraFrustumViewerApp* _this = (CameraFrustumViewerApp*)client;
		_this->GetVisiblePointCloud();
	}, this, "key=P");
#endif

	// projection matrix related properties
#if 1
	TwAddVarRW(bar, "Camera-ortho"   , TwType::TW_TYPE_BOOLCPP, &rendercam->is_ortho, "group='Camera' label='ortho'");
	TwAddVarRW(bar, "Camera-infinity", TwType::TW_TYPE_BOOLCPP, &rendercam->is_inf  , "group='Camera' label='infinity'");
	TwAddVarRW(bar, "Camera-z_near"  , TwType::TW_TYPE_FLOAT  , &rendercam->z_near  , "group='Camera' label='z_near' step=0.01");
	TwAddVarRW(bar, "Camera-z_far"   , TwType::TW_TYPE_FLOAT  , &rendercam->z_far   , "group='Camera' label='z_far'  step=0.01");

	TwAddVarCB(bar, "Camera-FoV", TwType::TW_TYPE_FLOAT, 
		[](const void* value, void* obj) {
			vgl::GLCamera* rendercam = (vgl::GLCamera*)obj;
			rendercam->SetCameraFoV(*(float*)value, 1.0f, 1.0f, rendercam->z_near);
		},
		[](void* value, void* obj) {
			vgl::GLCamera* rendercam = (vgl::GLCamera*)obj;
			*(float*)value = rendercam->GetCameraFoV();
		},
		rendercam, "group='Camera' label='FoV' min=1.0 max=179.0");

	TwAddVarRW(bar, "Camera-L", TwType::TW_TYPE_FLOAT, &rendercam->L, "group='Camera' label='Left'   step=0.01");
	TwAddVarRW(bar, "Camera-R", TwType::TW_TYPE_FLOAT, &rendercam->R, "group='Camera' label='Right'  step=0.01");
	TwAddVarRW(bar, "Camera-B", TwType::TW_TYPE_FLOAT, &rendercam->B, "group='Camera' label='Bottom' step=0.01");
	TwAddVarRW(bar, "Camera-T", TwType::TW_TYPE_FLOAT, &rendercam->T, "group='Camera' label='Top'    step=0.01");

	TwDefine("Bar/Camera opened=false"); // close group as default
#endif

	return true;
}
void CameraFrustumViewerApp::End()
{
	if (rendercam) {
		delete rendercam;
		rendercam = nullptr;
	}
	if (offscreenFBO) {
		delete offscreenFBO;
		offscreenFBO = nullptr;
	}
}


///////////////////////////////////////////////////////////////////////////////
// user-defined function
///////////////////////////////////////////////////////////////////////////////
void CameraFrustumViewerApp::LoadMesh()
{
	char const* filterPatterns[1] = {"*.ply"};

	char* filepath = tinyfd_openFileDialog(
		"Read .ply mesh",
		"./", 1, filterPatterns, NULL, 0);

	if (filepath) {
		printf("filepath: %s\n", filepath);

		// load ply mesh
		vgl::ReadTriMeshPly(filepath, V, N, F);
		printf("[DONE]\n");
		printf("(V, N, F) = (%d, %d, %d)\n", V.size(), N.size(), F.size() / 3);

		C = vgl::GetNormalColors(N); // get normalmap color from vertex normal
	}
}
void CameraFrustumViewerApp::GetVisiblePointCloud()
{
	P_visible.clear();

	////////////////////////////////////////
	// offscreen rendering to generate data
	////////////////////////////////////////
	int w = width / 2;
	int h = w;

	offscreenFBO->Resize(w, h);
	offscreenFBO->DrawFBO( [&] {
		// draw 3D scene
		glm::mat4 proj = rendercam->GetProjMatrix();
		glm::mat4 view = rendercam->GetViewMatrix();
		glm::mat4 model = glm::translate(glm::mat4(1.0f), ModelPosition) * glm::mat4(ModelRotation);
		model = glm::scale(model, glm::vec3(ModelUniScale));

		glClearColor(BgColor[1].r, BgColor[1].g, BgColor[1].b, BgColor[1].a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		drawView3D(proj, view);
		glDisable(GL_DEPTH_TEST);

		offscreenFBO->CopyDepthToBuffer();

		// get visible vertex indices from mesh
		P_visible = offscreenFBO->ConvertDepthImage2PointCloud(glm::inverse(proj));
	});

	printf("# visible points V = %d\r", P_visible.size() );
}
void CameraFrustumViewerApp::GetVisibleFaces()
{
	// clear submesh information in advance
	F_visible.clear();

	////////////////////////////////////////
	// offscreen rendering to generate data
	////////////////////////////////////////
	int w = width / 2;
	int h = w;

	std::set<glm::uint> V_indices;

	offscreenFBO->Resize(w, h);
	offscreenFBO->DrawFBO([&] {
		// draw 3D scene
		glm::mat4 proj = rendercam->GetProjMatrix();
		glm::mat4 view = rendercam->GetViewMatrix();
		glm::mat4 model = glm::translate(glm::mat4(1.0f), ModelPosition) * glm::mat4(ModelRotation);
		model = glm::scale(model, glm::vec3(ModelUniScale));

		glClearColor(BgColor[1].r, BgColor[1].g, BgColor[1].b, BgColor[1].a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		drawView3D(proj, view);
		glDisable(GL_DEPTH_TEST);

		offscreenFBO->CopyDepthToBuffer();

		// get visible vertex indices from mesh
		V_indices = offscreenFBO->GetVisibleVertexIndices(proj * view * model, V);
	});

	// extract faces which contain visible vertex ***************************** [TEMPORARY]
	auto GetSubmeshFaceIndicesFromVertexIndices = [](
		const std::vector<glm::uint>& original_faces,
		const std::set<glm::uint>& v_indices, const bool strict = false)->std::set<glm::uint>
	{
		std::set<glm::uint> submesh_f_indices;

		for (int fidx = 0; fidx < original_faces.size() / 3; fidx++) {
			int vidx0 = original_faces[3 * fidx + 0];
			int vidx1 = original_faces[3 * fidx + 1];
			int vidx2 = original_faces[3 * fidx + 2];

			std::set<GLuint>::iterator it0 = v_indices.find(vidx0);
			std::set<GLuint>::iterator it1 = v_indices.find(vidx1);
			std::set<GLuint>::iterator it2 = v_indices.find(vidx2);

			// strict: all vertices should be in the set.
			if (strict) {
				if (it0 != v_indices.end() &&
					it1 != v_indices.end() &&
					it2 != v_indices.end())
					submesh_f_indices.insert(fidx);
			}
			// non-strict: at least one vertex is in the set.
			else {
				if (it0 != v_indices.end() ||
					it1 != v_indices.end() ||
					it2 != v_indices.end())
					submesh_f_indices.insert(fidx);
			}
		}

		return submesh_f_indices;
	};
	std::set<glm::uint> F_indices = GetSubmeshFaceIndicesFromVertexIndices(F, V_indices);

	for (auto fidx : F_indices) {
		F_visible.push_back(F[3 * fidx + 0]);
		F_visible.push_back(F[3 * fidx + 1]);
		F_visible.push_back(F[3 * fidx + 2]);
	}

	printf("# visible (V,F) = (%d,%d)\r", V_indices.size(), F_visible.size());
}

///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	CameraFrustumViewerApp app(1280, 640);

	app.SetInternalProcess(true); // update when it is dirty
	app.run();

	return EXIT_SUCCESS;
}
