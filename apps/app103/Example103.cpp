#include "Example103.h"


///////////////////////////////////////////////////////////////////////////////
// mandatory member function
///////////////////////////////////////////////////////////////////////////////
void Example103::Draw(const int width, const int height)
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

void Example103::drawView3D(glm::mat4 proj, glm::mat4 view)
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

		glEnable(GL_LIGHTING);

		// draw 3D axes
		if (V.empty()) vgl::drawAxes(1.0f);

		// draw original mesh
		vgl::drawTriMesh(V, N, F);
		glDisable(GL_LIGHTING);

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
	glm::mat4 cam_proj = rendercam->GetProjMatrix();
	glm::mat4 cam_pose = rendercam->GetPoseMatrix();

	glColor3f(1.0f, 1.0f, 0.0f);
	{
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(cam_pose));

		// camera frustum
		glLineWidth(2.0f);
		vgl::drawCameraFrustum(cam_proj);

		glPopMatrix();
	}
}


///////////////////////////////////////////////////////////////////////////////
// override member functions
///////////////////////////////////////////////////////////////////////////////
bool Example103::Init()
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
		Example103* _this = (Example103*)client; _this->resetGlobalView();
	}, this, "group='Global' label='init' ");
	TwAddVarRW(bar, "Global-rot" , TwType::TW_TYPE_QUAT4F, &GlobalViewRotation , "group='Global' label='rot'  open");
	TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
	TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
	TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif

	// model coordinate
#if 1
	TwAddButton(bar, "Model-init", [](void* client) {
		Example103* _this = (Example103*)client; _this->resetModelMatrix();
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
		Example103* _this = (Example103*)client; _this->resetCameraView();
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
void Example103::End()
{
	// [TODO]
}


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	Example103 app(1280, 640);

	app.SetInternalProcess(true); // update when it is dirty
	app.run();

	return EXIT_SUCCESS;
}
