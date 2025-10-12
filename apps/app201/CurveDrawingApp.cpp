#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class CurveDrawingApp : public AppGLBase
{
public:
	CurveDrawingApp(const int width, const int height) : AppGLBase(width, height) {};

	void Draw(const int width, const int height)
	{
		float aspect = width / (float)height;
		glm::mat4 proj = glm::ortho(-zoom * aspect, zoom * aspect, -zoom, +zoom);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(center_x, center_y, 0.0f));

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

		// draw 2D-grid
		glLineWidth(1.0f);
		glColor3f(0.4f, 0.4f, 0.4f);
		vgl::drawGridXY(2.0f, 20); // [-1.0:+1.0]^2

		// draw control lines
		glLineWidth(5.0f);
		glColor3f(1.0f, 0.5f, 0.0f);
		glBegin(GL_LINE_STRIP);
		for (auto p : control_points) glVertex3fv(glm::value_ptr(p));
		glEnd();

		// draw curve line 
		glLineWidth(3.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINE_STRIP);
		for (auto p : curve_points) glVertex3fv(glm::value_ptr(p));
		glEnd();

		// draw curve point
		glPointSize(5.0f);
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_POINTS);
		for (auto p : curve_points) glVertex3fv(glm::value_ptr(p));
		glEnd();

		// draw control point
		glPointSize(8.0f);
		glColor3f(0.0f, 1.0f, 1.0f);
		glBegin(GL_POINTS);
		for (auto p : control_points) glVertex3fv(glm::value_ptr(p));
		glEnd();
	}

	void RecomputeCurve()
	{
		// clear computed curve in advance
		curve_points.clear();
		if (control_points.size() <= 1) return;

		// very simple linear interpolation
		int N_CPs = control_points.size() - 1;
		for (int n = 0; n < N_CPs; n++)
		{
			auto CP0 = control_points[n + 0];
			auto CP1 = control_points[n + 1];

			for (int i = 0; i < N_curve; i++)
			{
				float t0 = i / (float)(N_curve - 1);
				float t1 = 1.0f - t0;

				glm::vec3 p = t1 * CP0 + t0 * CP1;
				curve_points.push_back(p);
			}
		}
	}

	bool Update()
	{
		bool updated = AppGLBase::Update();

		auto GetPos = [&](int mouse_x, int mouse_y)->glm::vec3
		{
			float aspect = width / (float)height;
			float pos_x = +2.0f * (mouse_x / (float)width  - 0.5f) * zoom * aspect - center_x;
			float pos_y = -2.0f * (mouse_y / (float)height - 0.5f) * zoom          - center_y;
			return glm::vec3(pos_x, pos_y, 0.0f);
		};

		////////////////////////////////////////
		// left click: add control point
		////////////////////////////////////////
		if (is_left_drag) {
			if (!mouse_button[0]) is_left_drag = false;
		}
		else {
			if (mouse_button[0]) {

				// Left Click
				if (mouse_button[0] == 1) {
					// pick the nearest point here
					glm::vec3 pos = GetPos(mouse_this_x, mouse_this_y);
					auto closest = std::min_element(control_points.begin(), control_points.end(),
						[&](glm::vec3 a, glm::vec3 b) {
							return glm::length(pos - a) < glm::length(pos - b);
						}
					);
					drag_point_index = std::distance(control_points.begin(), closest);;

					const float threshold = 0.1f;
					if (glm::length(pos - *closest) > threshold) drag_point_index = -1;
				}

				// Control + Left Click
				if (mouse_button[0] == 3) {
					control_points.push_back(GetPos(mouse_this_x, mouse_this_y));
					RecomputeCurve();
				}

				is_left_drag = true;
			}
		}

		////////////////////////////////////////
		// left drag: move control point
		////////////////////////////////////////
		if (is_left_drag) {
			if (0 <= drag_point_index && drag_point_index < control_points.size()) {
				control_points[drag_point_index] = GetPos(mouse_this_x, mouse_this_y);
				RecomputeCurve();
			}
		}

		////////////////////////////////////////
		// right drag: move
		////////////////////////////////////////
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

		////////////////////////////////////////
		// middle drag: zoom in/out
		////////////////////////////////////////
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

		return updated;
	}

	bool Init()
	{
		glfwSetWindowTitle(window, "CurveDrawingApp");
		TwDefine("Bar iconified=true"); // hide TwBar at initial 
#if 1
		// initialize interaction data
		InitView();

		TwDefine("Bar size='150 250'");
		TwAddButton(bar, "InitView", [](void *client) {
			CurveDrawingApp* _this = (CurveDrawingApp*)client; _this->InitView();
		}, this, "");
		TwAddButton(bar, "RemoveLastCP", [](void* client) {
			CurveDrawingApp* _this = (CurveDrawingApp*)client;
			_this->control_points.pop_back();
			_this->RecomputeCurve();
		}, this, "");
		TwAddVarRW(bar, "fov", TwType::TW_TYPE_FLOAT, &zoom, "min=0.001 step=0.001");
		TwAddVarRW(bar, "center_x", TwType::TW_TYPE_FLOAT, &center_x, "step=0.001");
		TwAddVarRW(bar, "center_y", TwType::TW_TYPE_FLOAT, &center_y, "step=0.001");
		TwAddVarCB(bar, "vertices_in_curve", TwType::TW_TYPE_INT32, 
			[](const void* value, void* obj) {
				CurveDrawingApp* _this = (CurveDrawingApp*)obj;
				_this->N_curve = *(int*)value;
				_this->RecomputeCurve();
			},
			[](void* value, void* obj) {
				CurveDrawingApp* _this = (CurveDrawingApp*)obj;
				*(int*)value = _this->N_curve;
			},
			this, " min=16 ");
#endif

		// initialize control points
		control_points = {
			glm::vec3(-0.5f, -0.5f, 0.0f),
			glm::vec3(+0.0f, +0.5f, 0.0f),
			glm::vec3(+0.5f, -0.5f, 0.0f)
		};
		RecomputeCurve();

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

	std::vector<glm::vec3> control_points;
	int drag_point_index = -1;

	std::vector<glm::vec3> curve_points;
	int N_curve = 16;

	bool is_left_drag   = false;
	bool is_right_drag  = false;
	bool is_middle_drag = false;
	int mouse_prev_x;
	int mouse_prev_y;
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	CurveDrawingApp app(480, 480);
	app.SetInternalProcess(true);
	app.run();
	return EXIT_SUCCESS;
}
