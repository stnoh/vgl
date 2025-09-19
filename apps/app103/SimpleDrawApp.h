#ifndef SIMPLE_SHADER_APP
#define SIMPLE_SHADER_APP

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vgl/AppGLBase.h>

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
	glm::mat4 ortho_proj;
};

#endif
