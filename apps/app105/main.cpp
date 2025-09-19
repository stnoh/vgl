#include <vgl/AppGLBase.h>

class SimpleShaderAppGL : public AppGLBase
{
public:
	SimpleShaderAppGL(const int width, const int height) : AppGLBase(width, height) {};

	// mandatory callback
	void Draw(const int width, const int height)
	{
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// override callback
	void GetKey(int key, int action) {
		if (GLFW_KEY_TAB == key && GLFW_PRESS == action) {
			SetInternalProcess(!internalProcess);
		}
	};
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	SimpleShaderAppGL app(640, 480);
	app.SetInternalProcess(true);
	app.run();
	return EXIT_SUCCESS;
}
