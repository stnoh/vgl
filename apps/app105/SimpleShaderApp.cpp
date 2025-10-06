#include <glm/ext.hpp>

#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>
#include <vgl/TriMesh.h>
#include <vgl/GLShader.h>

const char* vertexShader = R"(
#version 330 
layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;

uniform mat4 Projection;
uniform mat4 ModelView;
uniform vec4 LightPosition;

uniform vec4 AmbientProduct;
uniform vec4 DiffuseProduct;
uniform vec4 SpecularProduct;
uniform float SpecularShininess;

out vec4 vertexColor;

void main()
{
	mat3 NormalMatrix = mat3(transpose(inverse(ModelView)));

	vec3 v = vec3(ModelView * vec4(vPosition, 1.0));
	vec3 N = normalize(NormalMatrix * vNormal);
	vec3 L = normalize(LightPosition.xyz - LightPosition.w * v);
	vec3 R = normalize(reflect(-L, N));
	float NdotL = max(dot(N,L), 0.0);

	vec4 ambient  = AmbientProduct;
	vec4 diffuse  = DiffuseProduct * NdotL;
	vec4 specular = vec4(0.0);

	vec3 E = vec3(0.0, 0.0, 1.0);
	float RdotE = max(dot(R,E), 0.0);
	if (NdotL > 0.0) specular = SpecularProduct * pow(RdotE, 0.3 * SpecularShininess);

	// output
	vertexColor = ambient + diffuse + specular;
	gl_Position = Projection * vec4(v, 1.0); // clipped position
}
)";

const char* fragmentShader = R"(
#version 330 
in vec4 vertexColor;
out vec4 fragColor;

void main()
{
    fragColor = vertexColor;
}
)";

class SimpleShaderAppGL : public AppGLBase
{
public:
	SimpleShaderAppGL(const int width, const int height) : AppGLBase(width, height) {};

	// mandatory callback
	void Draw(const int width, const int height)
	{
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::infinitePerspective(glm::radians(53.1301f), width / (float)height, 0.1f);
		glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 4), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

		glEnable(GL_DEPTH_TEST);

		////////////////////////////////////////////////////////////
		// 1st: draw faces with GL_FILL
		////////////////////////////////////////////////////////////
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		glPolygonMode(GL_FRONT, GL_FILL);

		if (use_shader)
		{
			shader->DrawShader([&](){
				GLint loc;
				loc = shader->GetUniformLocation("Projection");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
				loc = shader->GetUniformLocation("ModelView");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));

				loc = shader->GetUniformLocation("LightPosition");
				glUniform4fv(loc, 1, glm::value_ptr(L_position));

				loc = shader->GetUniformLocation("AmbientProduct");
				glUniform4fv(loc, 1, glm::value_ptr(L_ambient * M_ambient));
				loc = shader->GetUniformLocation("DiffuseProduct");
				glUniform4fv(loc, 1, glm::value_ptr(L_diffuse * M_diffuse));
				loc = shader->GetUniformLocation("SpecularProduct");
				glUniform4fv(loc, 1, glm::value_ptr(L_specular * M_specular));

				loc = shader->GetUniformLocation("SpecularShininess");
				glUniform1f(loc, M_shininess);

				// set vertices with normals
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &icosphere.vertices[0]);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, &icosphere.normals[0]);
				glEnableVertexAttribArray(1);

				vgl::drawTriMesh(icosphere.vertices, icosphere.normals, icosphere.faces);
			});
		}
		else {
			glEnable(GL_LIGHTING);
			vgl::setLight(GL_LIGHT0, L_position, L_ambient, L_diffuse, L_specular);
			vgl::setMaterial(GL_FRONT_AND_BACK, M_ambient, M_diffuse, M_specular, glm::vec4(0.0f), M_shininess);

			vgl::drawTriMesh(icosphere.vertices, icosphere.normals, icosphere.faces);

			glDisable(GL_LIGHT0);
			glDisable(GL_LIGHTING);
		}

		////////////////////////////////////////////////////////////
		// 2nd: draw triangle with GL_LINE without GL_LIGHTING
		////////////////////////////////////////////////////////////
		if (show_wireframe) {
			glColor3f(0.25f, 0.25f, 0.25f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			vgl::drawTriMesh(icosphere.vertices, icosphere.faces);
		}

		glDisable(GL_DEPTH_TEST);
	}

	bool Init()
	{
		glfwSetWindowTitle(window, "SimpleShaderApp");

		// UI: AntTweakBar
		TwAddVarRW(bar, "Light_dir", TwType::TW_TYPE_DIR3F, &L_position, "group='Global' label='Light_dir' open");

		TwAddVarRW(bar, "use_shader", TwType::TW_TYPE_BOOLCPP, &use_shader, "group='Global' label='use_shader'");
		TwAddVarRW(bar, "show_wire", TwType::TW_TYPE_BOOLCPP, &show_wireframe, "group='Global' label='show_wire'");

		// prepare icosphere to test rendering
		icosphere = vgl::IcoSphere(2);

		// prepare shader
		shader = new vgl::GLShader();
		shader->Compile(vertexShader  , vgl::SHADER_TYPE::VERTEX);
		shader->Compile(fragmentShader, vgl::SHADER_TYPE::FRAGMENT);
		shader->Link();

		return true;
	}

	void End()
	{
		if (NULL != shader)
		{
			delete shader;
			shader = NULL;
		}
	}

	vgl::IcoSphere icosphere;

private:
	bool use_shader = true;
	bool show_wireframe = false;

	// light properties
	glm::vec4 L_position = glm::vec4(1, 1, 1, 0);
	glm::vec4 L_ambient  = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 L_diffuse  = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	glm::vec4 L_specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// material properties
	glm::vec4 M_ambient  = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 M_diffuse  = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	glm::vec4 M_specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float M_shininess = 32.0f;

	vgl::GLShader* shader = nullptr;
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	SimpleShaderAppGL app(640, 480);
	app.SetInternalProcess(false);
	app.run();
	return EXIT_SUCCESS;
}
