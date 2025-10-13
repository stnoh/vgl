#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>

#include <vgl/Util.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct Joint {
	float length;
	float angle;
	Joint(float _length, float _angle) {
		length = _length;
		angle  = _angle;
	}
};

class KinematicViewerApp : public AppGLBase
{
public:
	KinematicViewerApp(const int width, const int height) : AppGLBase(width, height) {};

	void Draw(const int width, const int height)
	{
		float aspect = width / (float)height;
		glm::mat4 proj = glm::ortho(-zoom * aspect, zoom * aspect, -zoom, +zoom);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(center_x, center_y, 0.0f));

		glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

		// draw 2D-grid
		glLineWidth(1.0f);
		glColor3f(0.4f, 0.4f, 0.4f);
		vgl::drawGridXY(2.0f, 20);

		std::vector<glm::vec3> pts = GetPositions();

		// draw control point
		glLineWidth(5.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_STRIP);
		for (auto p : pts) glVertex3fv(glm::value_ptr(p));
		glEnd();

		// draw control point
		glPointSize(10.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_POINTS);
		for (auto p : pts) glVertex3fv(glm::value_ptr(p));
		glEnd();

		// show target
		glPointSize(5.0f);
		glColor3f(1.0f, 0.0f, 0.0f);
		if (is_left_drag) {
			glBegin(GL_POINTS);
			glm::vec3 pos = GetPos(mouse_this_x, mouse_this_y);
			glVertex3fv(glm::value_ptr(pos));
			glEnd();
		}
	}

	glm::vec3 GetPos(int mouse_x, int mouse_y)
	{
		float aspect = width / (float)height;
		float pos_x = +2.0f * (mouse_x / (float)width  - 0.5f) * zoom * aspect - center_x;
		float pos_y = -2.0f * (mouse_y / (float)height - 0.5f) * zoom          - center_y;
		return glm::vec3(pos_x, pos_y, 0.0f);
	};

	void ComputeIK(glm::vec3 target_pos)
	{
		int N = joints.size();
		for (int i = N - 1; i > 0; i--)
		{
			std::vector<glm::vec3> updated = GetPositions();
			
			glm::vec3 v1 = updated[N] - updated[i-1];
			glm::vec3 v2 = target_pos - updated[i-1];

			// [TODO] compute some CCD here!
		}
	}

	bool Update()
	{
		bool updated = AppGLBase::Update();

		////////////////////////////////////////
		// left click: add control point
		////////////////////////////////////////
		if (is_left_drag) {
			if (!mouse_button[0]) is_left_drag = false;
		}
		else {
			// Control + Left Click
			if (mouse_button[0] == 3) {
				is_left_drag = true;
			}
		}

		if (is_left_drag) {
			glm::vec3 pos = GetPos(mouse_this_x, mouse_this_y);
			ComputeIK(pos);
		}

		// right drag: move
#if 1
		if (is_right_drag) {
			if (!mouse_button[1]) is_right_drag = false;
		}
		else {
			if (mouse_button[1]) {
				is_right_drag = true;
				mouse_prev_x = mouse_this_x;
				mouse_prev_y = mouse_this_y;
			}
		}

		if (is_right_drag) {
			glm::vec3 diff = GetPos(mouse_this_x, mouse_this_y) - GetPos(mouse_prev_x, mouse_prev_y);
			center_x += diff.x;
			center_y += diff.y;

			mouse_prev_x = mouse_this_x;
			mouse_prev_y = mouse_this_y;
		}
#endif

		// middle drag: zoom in/out
#if 1
		if (is_middle_drag) {
			if (!mouse_button[2]) is_middle_drag = false;
		}
		else {
			if (mouse_button[2]) {
				is_middle_drag = true;
				mouse_prev_x = mouse_this_x;
				mouse_prev_y = mouse_this_y;
			}
		}

		if (is_middle_drag) {
			zoom *= pow(0.997, mouse_this_y - mouse_prev_y);

			mouse_prev_x = mouse_this_x;
			mouse_prev_y = mouse_this_y;
		}
#endif

		return updated;
	}

	bool Init()
	{
		glfwSetWindowTitle(window, "KinematicViewerApp");
		TwDefine("Bar iconified=true"); // hide TwBar at initial 
#if 1
		InitView();
		TwDefine("Bar size='150 250'");
		TwAddButton(bar, "InitView", [](void *client) {
			KinematicViewerApp* _this = (KinematicViewerApp*)client; _this->InitView();
		}, this, "");
		TwAddVarRW(bar, "fov", TwType::TW_TYPE_FLOAT, &zoom, "min=0.001 step=0.001");
		TwAddVarRW(bar, "center_x", TwType::TW_TYPE_FLOAT, &center_x, "step=0.001");
		TwAddVarRW(bar, "center_y", TwType::TW_TYPE_FLOAT, &center_y, "step=0.001");
#endif

#if 1
		for (int i = 0; i < joints.size(); i++) {
			Joint& joint = joints[i];
			std::string name0 = "Joint-Length" + std::to_string(i);
			std::string name1 = "Joint-Angle" + std::to_string(i);
			std::string def0 = "group='Joint' label='Length" + std::to_string(i) + "' step=0.01 ";
			std::string def1 = "group='Joint' label='Angle"  + std::to_string(i) + "' ";

			TwAddVarRW(bar, name0.c_str(), TwType::TW_TYPE_FLOAT, &joint.length, def0.c_str());
			TwAddVarRW(bar, name1.c_str(), TwType::TW_TYPE_FLOAT, &joint.angle, def1.c_str());
		}
#endif
		return true;
	}

private:

	////////////////////////////////////////
	// user interaction
	////////////////////////////////////////
	void InitView()
	{
		center_x = 0.0f;
		center_y = 0.0f;
		zoom = 1.0f;
	}
	float center_x = 0.0f;
	float center_y = 0.0f;
	float zoom = 1.0f;

	bool is_left_drag = false;

	bool is_right_drag = false;
	bool is_middle_drag = false;
	int mouse_prev_x;
	int mouse_prev_y;

	std::vector<Joint> joints = {
		Joint(0.30f, 30.0f),
		Joint(0.25f, 20.0f),
		Joint(0.20f, 10.0f),
		Joint(0.15f,  5.0f)
	};

	std::vector<glm::vec3> GetPositions()
	{
		std::vector<glm::vec3> pts;
		glm::mat4 transform_now = glm::mat4(1.0f);
		pts.push_back(glm::vec3(0.0f));

		float angle_sum = 0.0f;
		glm::vec3 pt = glm::vec3(0.0f);
		for (int i = 0; i < joints.size(); i++)
		{
			angle_sum += joints[i].angle;
			pt.x += joints[i].length * glm::cos(glm::radians(angle_sum));
			pt.y += joints[i].length * glm::sin(glm::radians(angle_sum));

			pts.push_back(pt);
		}

		return pts;
	}
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	KinematicViewerApp app(480, 480);
	app.SetInternalProcess(true);
	app.run();
	return EXIT_SUCCESS;
}
