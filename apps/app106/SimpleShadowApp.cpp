#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>
#include <vgl/TriMesh.h>

#include <vgl/GLShader.h>
#include <vgl/FBO.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

const char* vertShader = R"(
#version 330 
layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;

uniform mat4 Projection;
uniform mat4 Model;
uniform mat4 View;
uniform vec4 LightPosition;

uniform vec3 SpotDirection;
uniform float SpotCutoff;
uniform float SpotExponent;

uniform vec4 DiffuseProduct;

uniform mat4 LightSpaceMatrix;

out vec4 vertexColor;
out vec4 FragPosLightSpace;

void main()
{
	mat3 NormalMatrix = mat3(transpose(inverse(Model)));

	vec3 v = vec3(Model * vec4(vPosition, 1.0));
	vec3 N = normalize(NormalMatrix * vNormal);
	vec3 L = LightPosition.xyz - LightPosition.w * v;
	float d = length(L);
	L = normalize(L);

	vec3 R = normalize(reflect(-L, N));
	float NdotL = max(dot(N,L), 0.0);

	float attenuation = 1.0;
	//attenuation = 1.0 / (d * d); // quadratic attenuation

    float spotEffect = dot(normalize(-SpotDirection), L);
    if(spotEffect > SpotCutoff)
    {
        attenuation *= pow(spotEffect, SpotExponent);
    }
    else
    {
        attenuation = 0.0; // out of spot area
    }

	vec4 diffuse  = DiffuseProduct * NdotL * attenuation;

	// output
	vertexColor = diffuse;
	FragPosLightSpace = LightSpaceMatrix * vec4(v, 1.0);

	gl_Position = Projection * View * vec4(v, 1.0); // clipped position
}
)";

const char* fragShader = R"(
#version 330 
uniform sampler2D shadowMap;

in vec4 vertexColor;
in vec4 FragPosLightSpace;

out vec4 fragColor;

float ShadowCalculation(vec4 fragPosLight)
{
    vec3 projCoords = fragPosLight.xyz / fragPosLight.w;
    projCoords = projCoords * 0.5 + 0.5;

	// out of depthmap area
    if(projCoords.z > 1.0)
        return 0.0;
	
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.005;

    return (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
}

void main()
{
    float shadow = ShadowCalculation(FragPosLightSpace);

	vec3 vertexColorWithShadow = vec3(vertexColor);

    fragColor = vec4(vertexColorWithShadow * (1.0 - shadow), 1.0);
}
)";

class SimpleShadowApp : public AppGLBase
{
public:
	SimpleShadowApp(const int width, const int height) : AppGLBase(width, height) {};

	void DrawScene(glm::mat4 proj, glm::mat4 view, bool pass_one, glm::mat4 light_space)
	{
		glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // [IMPORTANT] clear in advance

		glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

		glEnable(GL_LIGHTING);
		vgl::setLight(GL_LIGHT0, L_position, glm::vec4(0.0f), L_diffuse, glm::vec4(0.0f));

		// additional properties for spotlight
		glm::vec3 spot_dir = -glm::normalize(glm::vec3(L_position));

		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, glm::value_ptr(spot_dir));
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, spot_cutoff);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, spot_exponent);

		// set material
		vgl::setMaterial(GL_FRONT_AND_BACK, glm::vec4(0.0f), M_diffuse, glm::vec4(0.0f), glm::vec4(0.0f), 0.0f);

		// sphere: simple drawing
		{
			const float tau = (1.0f + sqrtf(5.0f)) / 2.0f;

			glm::mat4 scale = glm::scale(glm::mat4(1.0), 0.5f * glm::vec3(1.0f / tau));
			glm::mat4 offset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.925f * 0.5f, 0.0f));
			glm::mat4 model = offset * scale;

			glPushMatrix();
			glMultMatrixf(glm::value_ptr(model));
			vgl::drawTriMesh(icosphere.vertices, icosphere.normals, icosphere.faces);
			glPopMatrix();
		}

		// floor: "shadow" in the 2nd pass
		{
			glm::mat4 offset = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
			glm::mat4 model = offset;

			if (pass_one)
			{
				vgl::drawTriMesh(vertices, normals, faces);
			}
			else {
				shader.DrawShader([&] {
					GLint loc;
					loc = shader.GetUniformLocation("Projection");
					glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
					loc = shader.GetUniformLocation("View");
					glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
					loc = shader.GetUniformLocation("Model");
					glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

					loc = shader.GetUniformLocation("LightPosition");
					glUniform4fv(loc, 1, glm::value_ptr(L_position));

					loc = shader.GetUniformLocation("SpotDirection");
					glUniform3fv(loc, 1, glm::value_ptr(spot_dir));

					loc = shader.GetUniformLocation("SpotCutoff");
					glUniform1f(loc, glm::cos(glm::radians(spot_cutoff)));
					loc = shader.GetUniformLocation("SpotExponent");
					glUniform1f(loc, spot_exponent);

					loc = shader.GetUniformLocation("DiffuseProduct");
					glUniform4fv(loc, 1, glm::value_ptr(L_diffuse * M_diffuse));

					loc = shader.GetUniformLocation("LightSpaceMatrix");
					glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(light_space));

					loc = shader.GetUniformLocation("shadowMap");
					if (-1 != loc) {
						glActiveTexture(GL_TEXTURE0 + 0);
						glBindTexture(GL_TEXTURE_2D, depthMap);
						glUniform1i(loc, 0);
					}

					// set vertices with normals
					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &vertices[0]);
					glEnableVertexAttribArray(0);
					glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, &normals[0]);
					glEnableVertexAttribArray(1);

					vgl::drawTriMesh(vertices, normals, faces);

					glDisableVertexAttribArray(1);
					glDisableVertexAttribArray(0);
				});
			}
		}

		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHTING);
	}

	// mandatory callback
	void Draw(const int width, const int height)
	{
		glm::mat4 proj = glm::perspective(glm::radians(53.1301f), width / (float)height, 0.1f, 100.0f);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), -GlobalViewPosition);
		view = view * glm::mat4_cast(GlobalViewRotation);

		// 1st pass: render from "light"
		glm::mat4 proj_light = glm::perspective(2.0f * glm::radians(spot_cutoff), 1.0f, 0.1f, 100.0f);
		glm::mat4 view_light = glm::lookAt(glm::vec3(L_position), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // [CAUTION]

		// texture for "depth"
		fbo->DrawFBO([&] {
			DrawScene(proj_light, view_light, true, NULL);
		});

		// 2nd pass: render from "camera"
		glViewport(0, 0, width, height); // reset viewport since fbo has it's own viewport
		DrawScene(proj, view, false, proj_light * view_light); // shader version
		//DrawScene(proj, view, false, NULL); // legacy pipeline (no shadow!)

		// additionally, draw lines on the floor
		glColor3f(0.0f, 0.0f, 0.0f);
		vgl::drawGridXZ(10.0f, 50);
	}

	bool Init()
	{
		glfwSetWindowTitle(window, "SimpleShadowApp");

		// global viewer
#if 1
		resetGlobalView();
		TwAddButton(bar, "Global-init", [](void* client) {
			SimpleShadowApp* _this = (SimpleShadowApp*)client; _this->resetGlobalView();
			}, this, "group='Global' label='init' ");
		TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation, "group='Global' label='rot'  open");
		TwAddVarRW(bar, "Global-posX", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.x, "group='Global' label='posX' step=0.01");
		TwAddVarRW(bar, "Global-posY", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.y, "group='Global' label='posY' step=0.01");
		TwAddVarRW(bar, "Global-posZ", TwType::TW_TYPE_FLOAT, &GlobalViewPosition.z, "group='Global' label='posZ' step=0.01");
#endif
		// UI: AntTweakBar
		TwAddVarRW(bar, "Light_pos", TwType::TW_TYPE_DIR3F, &L_position, "group='Global' label='Light_pos' open");

		TwAddVarRW(bar, "cutoff", TwType::TW_TYPE_FLOAT, &spot_cutoff, "group='Global' label='cutoff' min=30 max=60 ");
		TwAddVarRW(bar, "exponent", TwType::TW_TYPE_FLOAT, &spot_exponent, "group='Global' label='exponent' min=0 max=20 ");

		// enable hidden surface removal frags
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		// prepare more smooth icosphere
		icosphere = vgl::IcoSphere(4);

		// prepare very simple mesh for floor
		const float x_scale = 10.0f;
		const float z_scale = 10.0f;

		const int subdiv = 100;
		int cnt = 0;
		for (int j = 0; j < subdiv; j++)
		for (int i = 0; i < subdiv; i++)
		{
			float x0 = x_scale * ( (i + 0) / (float)subdiv - 0.5f);
			float x1 = x_scale * ( (i + 1) / (float)subdiv - 0.5f);
			float z0 = z_scale * ( (j + 0) / (float)subdiv - 0.5f);
			float z1 = z_scale * ( (j + 1) / (float)subdiv - 0.5f);

			vertices.push_back(glm::vec3(x0, 0.0f, z0));
			vertices.push_back(glm::vec3(x0, 0.0f, z1));
			vertices.push_back(glm::vec3(x1, 0.0f, z1));
			vertices.push_back(glm::vec3(x1, 0.0f, z0));

			normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
			normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
			normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
			normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

			faces.push_back(4 * cnt); faces.push_back(4 * cnt + 1); faces.push_back(4 * cnt + 2);
			faces.push_back(4 * cnt); faces.push_back(4 * cnt + 2); faces.push_back(4 * cnt + 3);
			cnt++;
		}

		// prepare shader
		shader = vgl::GLShader();
		shader.Compile(vertShader, vgl::SHADER_TYPE::VERTEX);
		shader.Compile(fragShader, vgl::SHADER_TYPE::FRAGMENT);
		shader.Link();

		// offscreen renderer
		const int shadow_width  = 1024;
		const int shadow_height = 1024;
		fbo = new vgl::FBO(shadow_width, shadow_height); // shadow map size
		fbo->Enable();

		// create depth map as 2D texture
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width, shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// set texture as a rendering target of FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		fbo->Disable();

		return true;
	}

	void End()
	{
		if (nullptr != fbo) {
			delete fbo;
			fbo = nullptr;
		}
	}

private:
	vgl::IcoSphere icosphere;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::uint> faces;

	vgl::FBO *fbo = nullptr;
	GLuint depthMap = 0;

	// light properties
	glm::vec4 L_position = glm::vec4(0, 3, 0.1, 1); // as point light
	glm::vec4 L_diffuse  = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// additional properties for spotlight
	float spot_cutoff = 30.0f;
	float spot_exponent = 12.0f;

	// material properties
	glm::vec4 M_diffuse  = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// shader
	vgl::GLShader shader;

	// global viewer
	glm::quat GlobalViewRotation;
	glm::vec3 GlobalViewPosition;
	void resetGlobalView() {
		GlobalViewPosition = glm::vec3(0.0f, 1.0f, 4.0f);
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(45.0f, -37.5f, -30.0f)));
	}
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	SimpleShadowApp app(640, 480);
	app.SetInternalProcess(true);
	app.run();
	return EXIT_SUCCESS;
}
