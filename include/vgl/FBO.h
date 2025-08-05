/******************************************************************************
Off-screen rendering by GL frame buffer object
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#ifndef FRAME_BUFFER_OBJECT
#define FRAME_BUFFER_OBJECT

#include <GL/glew.h>

namespace vgl {

class FBO
{
public:
	FBO(const int width, const int height);
	FBO() : FBO(8192, 8192) {}; // maximum size as default
	~FBO();

	void Enable();
	void Disable();

	bool Resize(const int width, const int height);

private:

	GLuint fbo;
	GLuint rboColor;
	GLuint rboDepth;

	int width;
	int height;
	const int maxWidth;
	const int maxHeight;
};

}

#endif
