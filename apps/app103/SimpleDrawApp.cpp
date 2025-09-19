#include "SimpleDrawApp.h"


///////////////////////////////////////////////////////////////////////////////
// mandatory member function
///////////////////////////////////////////////////////////////////////////////
void SimpleDrawApp::Draw(const int width, const int height)
{
	glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(ortho_proj));
	glMatrixMode(GL_MODELVIEW);  glLoadIdentity();

	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, +0.0f);
	glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(+0.0f, +0.5f, +0.0f);
	glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(+0.5f, -0.5f, +0.0f);
	glEnd();
}


///////////////////////////////////////////////////////////////////////////////
// override member functions
///////////////////////////////////////////////////////////////////////////////
bool SimpleDrawApp::Init()
{
	// just use simple orthogonal projection for 2D drawing
	ortho_proj = glm::ortho(-1.0f, +1.0f, -1.0f, +1.0f);

	return true;
}
void SimpleDrawApp::End()
{

}


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
