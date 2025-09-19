#include <vgl/AppGLBase.h>

class TestAppGL : public AppGLBase
{
public:
	TestAppGL(const int width, const int height) : AppGLBase(width, height) {};

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
	TestAppGL app(640, 480);
	app.SetInternalProcess(false); // false: only updated when it is "dirty"
	app.run();
	return EXIT_SUCCESS;
}
