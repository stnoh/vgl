/******************************************************************************
Application base class with GL context using GLFW with AntTweakBar
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef APP_GL_BASE
#define APP_GL_BASE

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <AntTweakBar.h>

class AppGLBase
{
public:
	AppGLBase(const int width, const int height);
	virtual ~AppGLBase();

	bool run(const bool continousUpdate = true);

	// [MANDOTORY]
	virtual void Draw(const int width, const int height) = 0;
	int width, height;
	bool is_dirty;

	// [OPTIONAL (1)] initialize & finalize
	virtual bool Init() { return true; };
	virtual void End()  { };

	// [OPTIONAL (2)] control internal process
	virtual void GetKey(int key, int action) { };
	virtual bool Update() {
		if (!internalProcess) return false;
		return true;
	};
	void SetInternalProcess(bool turnOn){ internalProcess = turnOn; }

protected:
	GLFWwindow *window;
	TwBar *bar;

	bool internalProcess;

	int mouse_button[3] = { 0, 0, 0 }; // left, right, middle button
	int mouse_this_x = 0, mouse_this_y = 0;
};

#endif
