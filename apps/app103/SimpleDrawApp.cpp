#include <vgl/Util.h>
#include <vgl/AppGLBase.h>
#include <glm/gtc/type_ptr.hpp>

class SimpleDrawApp : public AppGLBase
{
public:
	SimpleDrawApp(const int width, const int height) : AppGLBase(width, height) { };

	// mandatory member function
	void Draw(const int width, const int height)
	{
		glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// just use simple orthogonal projection for 2D drawing
		if (false) {
			// Legacy GL
			glMatrixMode(GL_PROJECTION); glLoadIdentity();
		}
		else {
			glm::mat4 ortho_proj = glm::ortho(-1.0f, +1.0f, -1.0f, +1.0f, +1.0f, -1.0f); // [CAUTION] z-near and z-far
			vgl::ShowMatrix4x4(ortho_proj);
			glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(ortho_proj));
		}

		glMatrixMode(GL_MODELVIEW);  glLoadIdentity();

		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, +0.0f);
		glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(+0.0f, +0.5f, +0.0f);
		glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(+0.5f, -0.5f, +0.0f);
		glEnd();
	}

	// override member functions
	bool Init()
	{
		TwDefine("Bar iconified=true");

		return true;
	}
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	SimpleDrawApp  app(640, 640);
	
	app.SetInternalProcess(false);
	app.run();

	return EXIT_SUCCESS;
}
