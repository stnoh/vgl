#ifndef SIMPLE_SHADER_APP
#define SIMPLE_SHADER_APP

#include <vgl/AppGLBase.h>

#include <vgl/FBO.h>

class SimpleDrawApp : public AppGLBase
{
public:
	SimpleDrawApp(const int width, const int height) : AppGLBase(width, height) { };

	// mandatory member function
	void Draw(const int width, const int height);

	// override member functions
	bool Init();
	void End();

private:

};

#endif
