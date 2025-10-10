#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>
#include <vgl/TriMesh.h>

#include <vgl/GLShader.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/constants.hpp>

const char* vertShader = R"(
#version 330 
layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 surfacePosAttrib;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

out vec2 surfacePosition;

void main()
{
	vec3 v = vec3(Model * vec4(vPosition, 1.0));
	surfacePosition = surfacePosAttrib;

	gl_Position = Projection * View * vec4(v, 1.0);
}
)";

const char* fragShader = R"(
#version 330
uniform mat4 Model;
uniform vec4 LightPosition;
uniform vec4 DiffuseProduct;
uniform sampler2D normalMap;

in vec2 surfacePosition;

out vec4 fragColor;

void main()
{
	mat3 NormalMatrix = mat3(transpose(inverse(Model)));

	// revert from [0.0:+1.0]^3 to [-1.0:1.0]^3
	vec3 N = texture(normalMap, surfacePosition).rgb;
	N = 2.0 * N - 1.0;
	N = normalize(NormalMatrix * N);

	vec3 L = normalize(LightPosition.xyz);
	float NdotL = max(dot(N,L), 0.0);

	vec4 diffuse  = DiffuseProduct * NdotL;

	fragColor = diffuse;
}
)";

class SimpleTextureApp : public AppGLBase
{
public:
	SimpleTextureApp(const int width, const int height) : AppGLBase(width, height) {};

	void Draw(const int width, const int height)
	{
		glm::mat4 proj = glm::perspective(glm::radians(53.1301f), width / (float)height, 0.1f, 100.0f);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), -GlobalViewPosition);
		view = view * glm::mat4_cast(GlobalViewRotation);

		if (is_ortho) {
			float zoom = 1.75f;
			float aspect = width / (float)height;
			proj = glm::ortho(-zoom * aspect, zoom * aspect, -zoom, +zoom);
			view = glm::mat4_cast(glm::quat(glm::radians(glm::vec3(90.0f, 0.0f, 0.0f))));
		}

		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

		glm::vec4 lightPos = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		glm::vec4 diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);

		vgl::setLight(GL_LIGHT0, lightPos,
			glm::vec4(0.0f),
			diffuse,
			glm::vec4(0.0f) );

		glColor3f(1.0f, 1.0f, 1.0f);

		// 1) left: fine mesh
		glm::mat4 model_L = glm::translate(glm::mat4(1.0f), glm::vec3(-1.25f, 0.0f, 0.0f));;
		model_L *= glm::mat4_cast(glm::quat(glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f))));
		{
			glEnable(GL_LIGHTING);

			glPushMatrix();
			glMultMatrixf(glm::value_ptr(model_L));

			vgl::drawTriMesh(plane_xy.vertices, plane_xy.normals, plane_xy.faces);

			glPopMatrix();
			glDisable(GL_LIGHTING);
		}

		const float size = 1.0f;
		glm::mat4 model_R = glm::translate(glm::mat4(1.0f), glm::vec3(1.25f, 0.0f, 0.0f));;
		model_R *= glm::mat4_cast(glm::quat(glm::radians(glm::vec3(-90.0f, 0.0f, 0.0f))));

		// 2-1) right: draw a quad with normalmap
		if (!show_normalcolor)
		{
			glEnable(GL_LIGHTING);
			glPushMatrix();
			glMultMatrixf(glm::value_ptr(model_R));

			glEnable(GL_TEXTURE_2D);

			shader.DrawShader([&] {
				GLuint loc = -1;

				loc = shader.GetUniformLocation("Projection");
				if (-1 != loc) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
				loc = shader.GetUniformLocation("View");
				if (-1 != loc) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
				loc = shader.GetUniformLocation("Model");
				if (-1 != loc) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model_R));

				loc = shader.GetUniformLocation("LightPosition");
				if (-1 != loc) glUniform4fv(loc, 1, glm::value_ptr(lightPos));
				loc = shader.GetUniformLocation("DiffuseProduct");
				if (-1 != loc) glUniform4fv(loc, 1, glm::value_ptr(diffuse));

				loc = shader.GetUniformLocation("normalMap");
				if (-1 != loc) {
					glActiveTexture(GL_TEXTURE0 + 0);
					glBindTexture(GL_TEXTURE_2D, tex2D);
					glUniform1i(loc, 0);
				}

				glBegin(GL_QUADS);
				glVertexAttrib2f(1, 0.0f, 0.0f); glVertex3f(-size, +size, 0.0f);
				glVertexAttrib2f(1, 0.0f, 1.0f); glVertex3f(-size, -size, 0.0f);
				glVertexAttrib2f(1, 1.0f, 1.0f); glVertex3f(+size, -size, 0.0f);
				glVertexAttrib2f(1, 1.0f, 0.0f); glVertex3f(+size, +size, 0.0f);
				glEnd();
			});
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			
			glPopMatrix();
			
			glDisable(GL_LIGHTING);
		}

		// 2-2) right: draw a quad with normal color
		if (show_normalcolor)
		{
			glPushMatrix();
			glMultMatrixf(glm::value_ptr(model_R));

			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex2D);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, +size, 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, -size, 0.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(+size, -size, 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(+size, +size, 0.0f);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, 0);

			glDisable(GL_TEXTURE_2D);
			glPopMatrix();
		}

		if (!is_ortho) {
			glColor3f(0.0f, 0.0f, 0.0f);
			vgl::drawGridXZ(5.0f, 20);
		}

		glDisable(GL_LIGHT0);
	}

	bool Init()
	{
		glfwSetWindowTitle(window, "SimpleTextureApp");

		// global viewer
#if 1
		resetGlobalView();
		TwAddVarRW(bar, "Global-ortho", TwType::TW_TYPE_BOOLCPP, &is_ortho, "group='Global' label='ortho' ");
		TwAddButton(bar, "Global-init", [](void* client) {
			SimpleTextureApp* _this = (SimpleTextureApp*)client; _this->resetGlobalView();
			}, this, "group='Global' label='init' ");
		TwAddVarRW(bar, "Global-rot" , TwType::TW_TYPE_QUAT4F, &GlobalViewRotation , "group='Global' label='rot'  open");
		TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
		TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
		TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
		TwAddVarRW(bar, "show_normalcolor", TwType::TW_TYPE_BOOLCPP, &show_normalcolor, "label='show_normalcolor' ");
		TwAddVarRW(bar, "show_wireframe", TwType::TW_TYPE_BOOLCPP, &show_wireframe, "label='show_wireframe' ");
#endif

		// enable hidden surface removal frags
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		// prepare shader to get image as normal information
		shader = vgl::GLShader();
		shader.Compile(vertShader, vgl::SHADER_TYPE::VERTEX);
		shader.Compile(fragShader, vgl::SHADER_TYPE::FRAGMENT);
		shader.Link();

		// compute vertex normal & normalmap
		const int TEX_WIDTH = 128;
		std::vector<glm::vec3> texture = std::vector<glm::vec3>(TEX_WIDTH * TEX_WIDTH);

		plane_xy = vgl::PlaneXY(TEX_WIDTH-1, 2.0f);
		for (int j = 0; j < TEX_WIDTH; j++)
		for (int i = 0; i < TEX_WIDTH; i++){
			glm::vec3& v = plane_xy.vertices[i + j * TEX_WIDTH];
			glm::vec3& n = plane_xy.normals [i + j * TEX_WIDTH];

			float x = v.x;
			float y = v.y;

			n.x = -glm::cos(glm::two_pi<float>() * x);
			n.y = +glm::sin(glm::two_pi<float>() * y);
			n = glm::normalize(n);

			texture[i + j * TEX_WIDTH] = 0.5f * (glm::vec3(n.x, n.y, n.z) + 1.0f); // [-1.0:1.0]^3 -> [0.0:+1.0]^3

			v.z = 0.125f * glm::sin(glm::two_pi<float>() * x) * glm::cos(glm::two_pi<float>() * y);
		}

		// create depth map as 2D texture
		glGenTextures(1, &tex2D);
		glBindTexture(GL_TEXTURE_2D, tex2D);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		float borderColor[] = { 0.0, 0.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_FLOAT, &texture[0]);

		return true;
	}

	void End()
	{
		if (NULL != tex2D) {
			glDeleteTextures(1, &tex2D);
			tex2D = NULL;
		}
	}

private:

	// shader
	vgl::GLShader shader;
	vgl::PlaneXY plane_xy;

	GLuint tex2D = NULL;

	bool is_ortho = false;
	bool show_normalcolor = true;
	bool show_wireframe = false;

	// global viewer
	glm::quat GlobalViewRotation;
	glm::vec3 GlobalViewPosition;
	void resetGlobalView() {
		GlobalViewPosition = glm::vec3(0.0f, 0.0f, 4.0f);
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(30.0f, 0.0f, 0.0f)));
	}
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	SimpleTextureApp app(640, 480);
	app.SetInternalProcess(true);
	app.run();
	return EXIT_SUCCESS;
}
