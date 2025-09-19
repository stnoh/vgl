/******************************************************************************
Application base class with GL context using GLFW with AntTweakBar
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vgl/AppGLBase.h>

///////////////////////////////////////////////////////////////////////////////
// [C++ limitation] custom callback
///////////////////////////////////////////////////////////////////////////////
void windowRefreshFun(GLFWwindow* window)
{
	AppGLBase* appGL = (AppGLBase*)glfwGetWindowUserPointer(window);

	// clean the whole window as black color
	glViewport(0, 0, appGL->width, appGL->height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	appGL->Draw(appGL->width, appGL->height);

	// routine
	TwDraw(); // draw AntTweakBar before swapbuffer
	glfwSwapBuffers(window);
	appGL->is_dirty = false;
}


///////////////////////////////////////////////////////////////////////////////
// ctor & dtor
///////////////////////////////////////////////////////////////////////////////
AppGLBase::AppGLBase(const int width, const int height)
: width(width), height(height), is_dirty(true), internalProcess(false)
{
	// initialize GLFW
	glfwSetErrorCallback([](int errCode, const char* desc) {
		fprintf(stderr, "GLFW error, id:%d, description:%s\n", errCode, desc);
	});
	if (!glfwInit())
	{
		fprintf(stderr, "Cannot initialize GLFW.\n");
		exit(1);
	}

	// main GLFW window
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	window = glfwCreateWindow(width, height, "", nullptr, nullptr);
	if (!window)
	{
		fprintf(stderr, "Cannot create GLFW window.\n");
		exit(1);
	}
	glfwSetWindowUserPointer(window, this); // register the pointer to instance

	// get the OpenGL version & show it
	int iOpenGLMajor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
	int iOpenGLMinor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
	int iOpenGLRevision = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
	printf("Status: Using GLFW Version %s\n", glfwGetVersionString());
	printf("Status: Using OpenGL Version: %i.%i, Revision: %i\n",
		iOpenGLMajor, iOpenGLMinor, iOpenGLRevision);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	// initialize GL extension wrangler
	const GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "GLEW Error:%s\n", glewGetErrorString(err));
		glfwDestroyWindow(window);
		exit(1);
	}

	// attach default AntTweakBar
	TwInit(TW_OPENGL, nullptr);
	TwWindowSize(width, height);
	bar = TwNewBar("Bar");
	TwDefine("TW_HELP visible=false"); // no help as default
	TwDefine("GLOBAL fontsize=3"); // large fontsize
	TwDefine("Bar position='5 5' size='250 450' label='Functions' refresh=0.5 alpha=0 iconified=true"); // 

	// set GLFW-only callbacks
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int w, int h)
	{
		AppGLBase* appGL = (AppGLBase*)glfwGetWindowUserPointer(window);
		appGL->width = w;
		appGL->height = h > 0 ? h : 1;
		appGL->is_dirty = true;
	});
	glfwSetWindowRefreshCallback(window, windowRefreshFun);

	// set Tweakbar-related callbacks
	glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height)
	{
		AppGLBase* appGL = (AppGLBase*)glfwGetWindowUserPointer(window);
		TwWindowSize(width, height);
		appGL->is_dirty = true;
	});
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
	{
		AppGLBase* appGL = (AppGLBase*)glfwGetWindowUserPointer(window);

		// reserve mouse button status
		if (0 == action) appGL->mouse_button[button] = 0;
		else appGL->mouse_button[button] = action + mods;

		TwEventMouseButtonGLFW3(window, button, action, mods);
		appGL->is_dirty = true;
	});
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos)
	{
		AppGLBase* appGL = (AppGLBase*)glfwGetWindowUserPointer(window);

		// reserve mouse cursor position
		appGL->mouse_this_x = (int)xpos;
		appGL->mouse_this_y = (int)ypos;

		TwEventCursorPosGLFW3(window, xpos, ypos);
		appGL->is_dirty = true;
	});
	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
	{
		AppGLBase* appGL = (AppGLBase*)glfwGetWindowUserPointer(window);
		TwEventScrollGLFW3(window, xoffset, yoffset);
		appGL->is_dirty = true;
	});
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		AppGLBase* appGL = (AppGLBase*)glfwGetWindowUserPointer(window);
		appGL->GetKey(key, action);
		TwEventKeyGLFW3(window, key, scancode, action, mods);
		appGL->is_dirty = true;
	});
	glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint)
	{
		AppGLBase* appGL = (AppGLBase*)glfwGetWindowUserPointer(window);
		TwEventCharModsGLFW3(window, codepoint, 0); // [FIX ME LATER]
		appGL->is_dirty = true;
	});
}
AppGLBase::~AppGLBase()
{
	if (bar) TwDeleteBar(bar);
	TwTerminate();

	if(window) glfwWindowShouldClose(window);
	glfwTerminate();
}


///////////////////////////////////////////////////////////////////////////////
// main routine
///////////////////////////////////////////////////////////////////////////////
bool AppGLBase::run(const bool continousUpdate)
{
	if (!Init()) {
		End();
		return false;
	}

	for (;;)
	{
		bool updated = Update();
		if (is_dirty || updated) windowRefreshFun(window);

		// update
		if(continousUpdate) glfwPollEvents();
		else glfwWaitEvents();

		if (glfwWindowShouldClose(window)) break;
	}

	End();
	return true;
}
