/******************************************************************************
Off-screen rendering by GL frame buffer object
Author: Seung-Tak Noh (seungtak.noh [at] gmail.com)
******************************************************************************/
#include <vgl/FBO.h>
#include <iostream>

using namespace vgl;


///////////////////////////////////////////////////////////////////////////////
// ctor / dtor
///////////////////////////////////////////////////////////////////////////////
FBO::FBO(const int width, const int height)
: maxWidth(width), maxHeight(height)
{
	// resolution
	this->width = width;
	this->height = height;

	// render buffer for color image
	glGenRenderbuffers(1, &rboColor);
	glBindRenderbuffer(GL_RENDERBUFFER, rboColor);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, width, height); // for single-precision floating color

	// render buffer for depth image
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// frame buffer
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboColor);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// check validity of framebuffer (should be OK)
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
FBO::~FBO()
{
	// release render buffers & frame buffer
	glDeleteRenderbuffers(1, &rboColor);
	glDeleteRenderbuffers(1, &rboDepth);
	glDeleteFramebuffers(1, &fbo);
}


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////
void FBO::Enable()
{
	// enable to use framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, width, height);
}
void FBO::Disable()
{
	// disable framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool FBO::Resize(const int width, const int height)
{
	bool failed = false;

	if (0 > width || maxWidth < width) {
		fprintf(stderr, "Failed to set FBO width: %d", width);
		failed = true;
	}
	else this->width = width;

	if (0 > height || maxHeight < height) {
		fprintf(stderr, "Failed to set FBO height: %d", height);
		failed = true;
	}
	else this->height = height;

	return !failed;
}
