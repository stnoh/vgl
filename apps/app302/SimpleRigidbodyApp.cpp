#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class SimpleRigidbodyApp : public AppGLBase
{
public:
	SimpleRigidbodyApp(const int width, const int height) : AppGLBase(width, height) {};

	void Draw(const int width, const int height)
	{
		float aspect = width / (float)height;
		glm::mat4 proj = glm::ortho(0.0f, aspect, 0.0f, +1.0f);

		if (aspect < 1.0f) {
			proj = glm::ortho(0.0f, +1.0f, 0.0f, 1.0f / aspect);
		}

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity(); glMultMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(2.0f);
		glDisable(GL_LIGHTING);
		glColor3f(1.0f, 0.0f, 1.0f);
		glBegin(GL_LINES);
		glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, 0.0f); glVertex3f(1.0f, 1.0f, 0.0f);
		glVertex3f(1.0f, 1.0f, 0.0f); glVertex3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 1.0f, 0.0f); glVertex3f(0.0f, 0.0f, 0.0f);
		glEnd();

		glEnable(GL_LIGHTING);
		vgl::setLight(GL_LIGHT0, glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));

		// draw sphere
		int N = position.size();
		for (int i = 0; i < N; i++)
		{
			glm::vec2 p = position[i];
			glm::vec2 v = velocity[i];

			glm::mat4 tr = glm::translate(glm::mat4(1.0f), glm::vec3(p.x, p.y, 1.0f));
			{
				glPushMatrix();
				glMultMatrixf(glm::value_ptr(tr));
				vgl::drawSphere(radius, color[i]);
				glPopMatrix();
			}
		}
	}

	bool Update()
	{
		// impulse-based simulation
		if (updated) {
			int N = position.size();

			// time integration
			for (int i = 0; i < N; i++)
			{
				glm::vec2 p = position[i];
				glm::vec2 v = velocity[i];
				position[i] = p + v * dt;
			}
			
			// simple collision detection
			for (int j = 0; j < N - 1; j++)
			{
				glm::vec2 p_j = position[j];
				
				for (int i = j + 1; i < N; i++)
				{
					glm::vec2 p_i = position[i];
					glm::vec2 diff = p_i - p_j;
					float d_p = 2.0f * radius - glm::length(diff);

					// impulse-based collision simulation
					if (0.0f < d_p) {
						glm::vec2 n = glm::normalize(diff);

						glm::vec2 v_i = velocity[i];
						glm::vec2 v_j = velocity[j];
						glm::vec2 v_ij = v_i - v_j;

						float v_dot_n = glm::dot(v_ij, n);

						velocity[i] = v_i - v_dot_n * n;
						velocity[j] = v_j + v_dot_n * n;

						// position correction to avoid penetration
						position[i] += 0.5f * d_p * n;
						position[j] -= 0.5f * d_p * n;
					}
				}
			}

			// simple reflection for the boundary
			auto reflect = [](glm::vec2 v, glm::vec2 n) {
				float VdotN = glm::min(glm::dot(v, n), 0.0f);
				return v - 2.0f * VdotN / glm::dot(n, n) * n;
			};

			for (int i = 0; i < N; i++)
			{
				glm::vec2 p_i = position[i];

				if (p_i.x < 0.0f + radius) velocity[i] = reflect(velocity[i], glm::vec2(+1.0f,  0.0f));
				if (1.0f - radius < p_i.x) velocity[i] = reflect(velocity[i], glm::vec2(-1.0f,  0.0f));
				if (p_i.y < 0.0f + radius) velocity[i] = reflect(velocity[i], glm::vec2( 0.0f, +1.0f));
				if (1.0f - radius < p_i.y) velocity[i] = reflect(velocity[i], glm::vec2( 0.0f, -1.0f));
			}
		}

		return updated;
	}

	void Initialize()
	{
		// (re)initialize array in advance
		position = std::vector<glm::vec2>(N);
		velocity = std::vector<glm::vec2>(N);
		color    = std::vector<glm::vec4>(N);

		for (int i = 0; i < N; i++) {

			// set initial position inside of the boundary
			position[i] = glm::linearRand(glm::vec3(0.0 + 2.0f * radius), glm::vec3(1.0 - 2.0f * radius));
				
			// set initial speed
			float vx = glm::gaussRand(0.0f, 1.0f);
			float vy = glm::gaussRand(0.0f, 1.0f);
			velocity[i] = glm::normalize(glm::vec2(vx, vy));

			// set color of each sphere
			color[i] = glm::linearRand(glm::vec4(0.0f), glm::vec4(1.0f));
			color[i].a = 1.0f;
		}
	}

	bool Init()
	{
		glfwSetWindowTitle(window, "SimpleRigidbodyApp");
		TwDefine("Bar iconified=true"); // hide TwBar at initial

		TwAddVarRW(bar, "updated", TwType::TW_TYPE_BOOLCPP, &updated, "label='updated' ");
		TwAddVarRW(bar, "dt", TwType::TW_TYPE_FLOAT, &dt, "label='dt' min=1e-6 max=1e-2 step=1e-6");

		TwAddVarCB(bar, "N_subdiv", TwType::TW_TYPE_INT32,
			[](const void* value, void* obj) {
				SimpleRigidbodyApp* _this = (SimpleRigidbodyApp*)obj;
				_this->N = *(int*)value;
				_this->Initialize();
			},
			[](void* value, void* obj) {
				SimpleRigidbodyApp* _this = (SimpleRigidbodyApp*)obj;
				*(int*)value = _this->N;
			},
			this, " min=1 max=32");

		Initialize();

		return true;
	}

private:
	bool updated;
	float dt = 2e-4f;

	float radius = 0.04f;

	int N = 2;

	std::vector<glm::vec2> position;
	std::vector<glm::vec2> velocity;
	std::vector<glm::vec4> color;
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	SimpleRigidbodyApp app(512, 512);
	app.SetInternalProcess(true);
	app.run();
	return EXIT_SUCCESS;
}
