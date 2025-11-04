#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

// code from "CDROM/code/solver.c" in CDROM_GDC03.zip, written by Jos Stam
#define IX(i,j) ((i)+(N+2)*(j))
#define SWAP(x0,x) {float * tmp=x0;x0=x;x=tmp;}
#define FOR_EACH_CELL for ( i=1 ; i<=N ; i++ ) { for ( j=1 ; j<=N ; j++ ) {
#define END_FOR }}

void add_source(int N, float* x, float* s, float dt)
{
	int i, size = (N + 2) * (N + 2);
	for (i = 0; i < size; i++) x[i] += dt * s[i];
}

void set_bnd(int N, int b, float* x)
{
	int i;

	for (i = 1; i <= N; i++) {
		x[IX(0, i)] = b == 1 ? -x[IX(1, i)] : x[IX(1, i)];
		x[IX(N + 1, i)] = b == 1 ? -x[IX(N, i)] : x[IX(N, i)];
		x[IX(i, 0)] = b == 2 ? -x[IX(i, 1)] : x[IX(i, 1)];
		x[IX(i, N + 1)] = b == 2 ? -x[IX(i, N)] : x[IX(i, N)];
	}
	x[IX(0, 0)] = 0.5f * (x[IX(1, 0)] + x[IX(0, 1)]);
	x[IX(0, N + 1)] = 0.5f * (x[IX(1, N + 1)] + x[IX(0, N)]);
	x[IX(N + 1, 0)] = 0.5f * (x[IX(N, 0)] + x[IX(N + 1, 1)]);
	x[IX(N + 1, N + 1)] = 0.5f * (x[IX(N, N + 1)] + x[IX(N + 1, N)]);
}

void lin_solve(int N, int b, float* x, float* x0, float a, float c)
{
	int i, j, k;

	for (k = 0; k < 20; k++) {
		FOR_EACH_CELL
			x[IX(i, j)] = (x0[IX(i, j)] + a * (x[IX(i - 1, j)] + x[IX(i + 1, j)] + x[IX(i, j - 1)] + x[IX(i, j + 1)])) / c;
		END_FOR
			set_bnd(N, b, x);
	}
}

void diffuse(int N, int b, float* x, float* x0, float diff, float dt)
{
	float a = dt * diff * N * N;
	lin_solve(N, b, x, x0, a, 1 + 4 * a);
}

void advect(int N, int b, float* d, float* d0, float* u, float* v, float dt)
{
	int i, j, i0, j0, i1, j1;
	float x, y, s0, t0, s1, t1, dt0;

	dt0 = dt * N;
	FOR_EACH_CELL
		x = i - dt0 * u[IX(i, j)]; y = j - dt0 * v[IX(i, j)];
	if (x < 0.5f) x = 0.5f; if (x > N + 0.5f) x = N + 0.5f; i0 = (int)x; i1 = i0 + 1;
	if (y < 0.5f) y = 0.5f; if (y > N + 0.5f) y = N + 0.5f; j0 = (int)y; j1 = j0 + 1;
	s1 = x - i0; s0 = 1 - s1; t1 = y - j0; t0 = 1 - t1;
	d[IX(i, j)] = s0 * (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) +
		s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
	END_FOR
		set_bnd(N, b, d);
}

void project(int N, float* u, float* v, float* p, float* div)
{
	int i, j;

	FOR_EACH_CELL
		div[IX(i, j)] = -0.5f * (u[IX(i + 1, j)] - u[IX(i - 1, j)] + v[IX(i, j + 1)] - v[IX(i, j - 1)]) / N;
	p[IX(i, j)] = 0;
	END_FOR
		set_bnd(N, 0, div); set_bnd(N, 0, p);

	lin_solve(N, 0, p, div, 1, 4);

	FOR_EACH_CELL
		u[IX(i, j)] -= 0.5f * N * (p[IX(i + 1, j)] - p[IX(i - 1, j)]);
	v[IX(i, j)] -= 0.5f * N * (p[IX(i, j + 1)] - p[IX(i, j - 1)]);
	END_FOR
		set_bnd(N, 1, u); set_bnd(N, 2, v);
}

void dens_step(int N, float* x, float* x0, float* u, float* v, float diff, float dt)
{
	add_source(N, x, x0, dt);
	SWAP(x0, x); diffuse(N, 0, x, x0, diff, dt);
	SWAP(x0, x); advect(N, 0, x, x0, u, v, dt);
}

void vel_step(int N, float* u, float* v, float* u0, float* v0, float visc, float dt)
{
	add_source(N, u, u0, dt); add_source(N, v, v0, dt);
	SWAP(u0, u); diffuse(N, 1, u, u0, visc, dt);
	SWAP(v0, v); diffuse(N, 2, v, v0, visc, dt);
	project(N, u, v, u0, v0);
	SWAP(u0, u); SWAP(v0, v);
	advect(N, 1, u, u0, u0, v0, dt); advect(N, 2, v, v0, u0, v0, dt);
	project(N, u, v, u0, v0);
}

class StableFluidsApp : public AppGLBase
{
public:
	StableFluidsApp(const int width, const int height) : AppGLBase(width, height) {};

	void Draw_Grid()
	{
		glColor3f(0.25f, 0.25f, 0.25f);

		glBegin(GL_LINES);

		// X-parallel lines
		for (int j = 0; j <= N; j++) {
			float y = j / (float)N;
			glVertex3f(0.0f, y, 0.0f);
			glVertex3f(1.0f, y, 0.0f);
		}

		// Y-parallel lines
		for (int i = 0; i <= N; i++) {
			float x = i / (float)N;
			glVertex3f(x, 0.0f, 0.0f);
			glVertex3f(x, 1.0f, 0.0f);
		}

		glEnd();
	}

	void Draw_Velocity()
	{
		glLineWidth(1.0f);

		glBegin(GL_LINES);

		float h = 1.0f / N;

		for (int i = 1; i <= N; i++) {
			float x = (i - 0.5f) * h;
			for (int j = 1; j <= N; j++) {
				float y = (j - 0.5f) * h;
				glColor3f(1.0f, 1.0f, 0.0f); glVertex2f(x, y);
				glColor3f(1.0f, 0.0f, 0.0f); glVertex2f(x + u[IX(i, j)], y + v[IX(i, j)]);
			}
		}

		glEnd();
	}

	void Draw_Density()
	{
		// simple color conversion from single value
		auto getColor = [](float density)->glm::vec3 {
			return glm::vec3(density, density, density);
		};

		float h = 1.0f / N;

		glBegin(GL_QUADS);

		for (int i = 0; i <= N; i++) {
			float x = (i - 0.5f) * h;
			for (int j = 0; j <= N; j++) {
				float y = (j - 0.5f) * h;

				float d00 = dens[IX(i, j)];
				float d01 = dens[IX(i, j + 1)];
				float d10 = dens[IX(i + 1, j)];
				float d11 = dens[IX(i + 1, j + 1)];

				glColor3f(d00, d00, d00); glVertex2f(x, y);
				glColor3f(d10, d10, d10); glVertex2f(x + h, y);
				glColor3f(d11, d11, d11); glVertex2f(x + h, y + h);
				glColor3f(d01, d01, d01); glVertex2f(x, y + h);
			}
		}

		glEnd();
	}

	void Draw(const int width, const int height)
	{
		glm::mat4 proj = glm::ortho(0.0f, 1.0f, 0.0f, +1.0f);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity(); glMultMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (show_density ) Draw_Density();
		if (show_velocity) Draw_Velocity();
		if (show_grid    ) Draw_Grid();
	}

	void get_from_UI(float* d, float* u, float* v)
	{
		int i, j, size = (N + 2) * (N + 2);

		for (i = 0; i < size; i++) {
			u[i] = v[i] = d[i] = 0.0f;
		}

		if (!mouse_button[1] && !mouse_button[2]) return;

		i = (int)((mouse_this_x / (float)width) * N + 1);
		j = (int)(((height - mouse_this_y) / (float)height) * N + 1);

		if (i<1 || i>N || j<1 || j>N) return;

		if (mouse_button[1]) {
			u[IX(i, j)] = force * (mouse_this_x - mouse_prev_x);
			v[IX(i, j)] = force * (mouse_prev_y - mouse_this_y);
		}

		if (mouse_button[2]) {
			d[IX(i, j)] = source;
		}

		mouse_prev_x = mouse_this_x;
		mouse_prev_y = mouse_this_y;
	}

	bool Update()
	{
		if (update) {
			get_from_UI(dens_prev, u_prev, v_prev);
			vel_step(N, u, v, u_prev, v_prev, visc, dt);
			dens_step(N, dens, dens_prev, u, v, diff, dt);
		}

		return update;
	}

	void free_data()
	{
		if (u) free(u);
		if (v) free(v);
		if (u_prev) free(u_prev);
		if (v_prev) free(v_prev);
		if (dens) free(dens);
		if (dens_prev) free(dens_prev);
	}

	void allocate_data()
	{
		int size = (N + 2) * (N + 2);

		u = (float*)malloc(size * sizeof(float));
		v = (float*)malloc(size * sizeof(float));
		u_prev = (float*)malloc(size * sizeof(float));
		v_prev = (float*)malloc(size * sizeof(float));
		dens = (float*)malloc(size * sizeof(float));
		dens_prev = (float*)malloc(size * sizeof(float));
	}

	void clear_data()
	{
		int size = (N + 2) * (N + 2);

		for (int i = 0; i < size; i++) {
			u[i] = v[i] = u_prev[i] = v_prev[i] = dens[i] = dens_prev[i] = 0.0f;
		}
	}

	bool Init()
	{
		glfwSetWindowTitle(window, "StableFluidsApp");

		TwAddButton(bar, "clear_data", [](void* client) {
			StableFluidsApp* _this = (StableFluidsApp*)client; _this->clear_data();
		}, this, "group='Local' label='clear_data' ");

		TwAddVarRW(bar, "show-density" , TwType::TW_TYPE_BOOLCPP, &show_density , " label='show_density'");
		TwAddVarRW(bar, "show-velocity", TwType::TW_TYPE_BOOLCPP, &show_velocity, " label='show_velocity'");
		TwAddVarRW(bar, "show-grid"    , TwType::TW_TYPE_BOOLCPP, &show_grid    , " label='show_grid'");

		TwAddVarRW(bar, "update", TwType::TW_TYPE_BOOLCPP, &update, " label='update'");

		TwAddVarRW(bar, "dt"    , TwType::TW_TYPE_FLOAT, &dt    , "label='dt' step=1e-3 min=1e-3");
		TwAddVarRW(bar, "visc"  , TwType::TW_TYPE_FLOAT, &visc  , "label='visc' step=0.1 min=0.0");
		TwAddVarRW(bar, "force" , TwType::TW_TYPE_FLOAT, &force , "label='force' step=0.1 min=0.0");
		TwAddVarRW(bar, "source", TwType::TW_TYPE_FLOAT, &source, "label='source' step=0.1 min=0.0");

		// initialize in advance
		allocate_data();
		clear_data();

		return true;
	}

	void End()
	{
		free_data();
	}

private:
	bool update = true;

	bool show_density  = true;
	bool show_velocity = true;
	bool show_grid     = true;

	int mouse_prev_x, mouse_prev_y;

	// default simulation parameters
	const int N = 64;
	float dt = 0.1f;
	float diff = 0.0f;
	float visc = 0.0f;
	float force = 5.0f;
	float source = 100.0f; 

	// simulation region (volume)
	float *u, *v, *u_prev, *v_prev;
	float *dens, *dens_prev;
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	StableFluidsApp app(512, 512);
	app.SetInternalProcess(true);
	app.run();
	return EXIT_SUCCESS;
}
