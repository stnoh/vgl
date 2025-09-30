#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class DepthTestAppGL : public AppGLBase
{
public:
	DepthTestAppGL(const int width, const int height) : AppGLBase(width, height) {};

	// mandatory callback
	void Draw(const int width, const int height)
	{
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::infinitePerspective(glm::radians(53.1301f), width / (float)height, 0.1f);
		glm::mat4  view = glm::translate(glm::mat4(1.0f), -GlobalViewPosition);
		view = view * glm::mat4_cast(GlobalViewRotation);

		glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

		// draw XZ floor plane
		glColor3f(0.0f, 0.0f, 0.0f);
		glLineWidth(1.0f);
		vgl::drawGridXZ(20.0f, 20);

		// set 
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		if (cull_face ) glEnable(GL_CULL_FACE);
		if (depth_test) glEnable(GL_DEPTH_TEST);

		glLineWidth(2.0f);

		// draw cube
		auto drawCubeWithOffset = [](glm::vec3 tr) {
			glPushMatrix();
			{
				glTranslatef(tr.x, tr.y, tr.z);

				glColor3f(0.0f, 1.0f, 1.0f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				vgl::drawCube(0.5f);

				glColor3f(0.0f, 0.0f, 0.0f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				vgl::drawCube(0.5f);
			}
			glPopMatrix();
		};

		// offsets for cube drawing
		glm::vec3 offset1 = glm::vec3(0.0f, 0.5f, 0.0f);
		glm::vec3 offset2 = glm::vec3(-1.2f, 0.5f, +1.2f);
		glm::vec3 offset3 = glm::vec3(+1.2f, 0.5f, -1.2f);

		std::vector<glm::vec3> offsets;

		switch (draw_order)
		{
		case 0: offsets = { offset1, offset2, offset3 }; break;
		case 1: offsets = { offset1, offset3, offset2 }; break;
		case 2: offsets = { offset2, offset1, offset3 }; break;
		case 3: offsets = { offset2, offset3, offset1 }; break;
		case 4: offsets = { offset3, offset1, offset2 }; break;
		case 5: offsets = { offset3, offset2, offset1 }; break;
		default: break;
		}

		for (auto it : offsets)
			drawCubeWithOffset(it);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// override callback
	void GetKey(int key, int action) {
		if (GLFW_KEY_TAB == key && GLFW_PRESS == action) {
			SetInternalProcess(!internalProcess);
		}
	};

	bool Init() {

		resetGlobalView();

		// global viewer
#if 1
		TwAddButton(bar, "Global-init", [](void* client) {
			DepthTestAppGL* _this = (DepthTestAppGL*)client; _this->resetGlobalView();
			}, this, "group='Global' label='init' ");
		TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation, "group='Global' label='rot'  open");
		TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
		TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
		TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif
		// 1) back face culling
		TwAddVarRW(bar, "cull_face" , TwType::TW_TYPE_BOOLCPP, &cull_face , "label='cull_face' ");

		// 2) painter's algorithm
		TwEnumVal drawOrderEV[] = {
			{0, "DRAW123"},
			{1, "DRAW132"},
			{2, "DRAW213"},
			{3, "DRAW231"},
			{4, "DRAW312"},
			{5, "DRAW321"}
		};

		TwType drawOrderType = TwDefineEnum("DrawOrder", drawOrderEV, 6);
		TwAddVarRW(bar, "draw_order", drawOrderType, &draw_order, NULL);

		// 3) z-buffer
		TwAddVarRW(bar, "depth_test", TwType::TW_TYPE_BOOLCPP, &depth_test, "label='depth_test' ");

		return true;
	}

	void End() {

	}

private:

	int draw_order = 0;
	bool cull_face = false;
	bool depth_test = false;

	// global viewer
	glm::quat GlobalViewRotation;
	glm::vec3 GlobalViewPosition;
	void resetGlobalView() {
		GlobalViewPosition = glm::vec3(0.0f, 0.0f, 4.0f);
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(45.0f, -37.5f, -30.0f)));
	}
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	DepthTestAppGL app(640, 480);
	app.SetInternalProcess(false);
	app.run();
	return EXIT_SUCCESS;
}
