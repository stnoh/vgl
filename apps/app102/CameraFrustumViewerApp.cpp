#include <vgl/Util.h>
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
		vgl::drawAxes(1.0f);
		glDisable(GL_LIGHTING);

		// draw grid on Y-plane as black color
		glColor3f(0.0f, 0.0f, 0.0f);
		glLineWidth(1.0f);
		vgl::drawGridXZ(10.0f, 10);

		glPopMatrix();
	}
	glDisable(GL_RESCALE_NORMAL);


	////////////////////////////////////////////////////////////
	// camera space
	////////////////////////////////////////////////////////////
	glColor3f(1.0f, 1.0f, 0.0f);
	glLineWidth(2.0f);
	rendercam->SetCameraCoord([&] {
		//vgl::ShowMatrix4x4(rendercam->GetProjMatrix());
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
}


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	CameraFrustumViewerApp app(1280, 640);

	app.SetInternalProcess(false); // update only when it is dirty
	app.run();

	return EXIT_SUCCESS;
}
