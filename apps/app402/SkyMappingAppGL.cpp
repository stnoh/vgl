#include <glm/ext.hpp>

#include <vgl/AppGLBase.h>
#include <vgl/DrawGL3D.h>
#include <vgl/TriMesh.h>
#include <vgl/GLShader.h>
#include <vgl/FBO.h>

#include <tinyfiledialogs.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

const char* vertShader_modern = R"(
#version 330 
layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;

uniform mat4 Projection;
uniform mat4 ModelView;

out vec3 wPosition;
out vec3 wDirection;
out vec3 wCameraPos;

void main()
{
	gl_Position = Projection * ModelView * vec4(vPosition, 1.0);
	wDirection = normalize(vNormal);

	wPosition = vPosition;
	wCameraPos = vec3(-ModelView[3] * ModelView);
}
)";

const char* vertShader_legacy = R"(
#version 120
uniform mat4 Projection;
uniform mat4 ModelView;

varying vec3 wDirection;

void main()
{
	gl_Position = Projection * ModelView * gl_Vertex;
	wDirection = normalize(gl_Vertex.xyz);
}
)";

const char* fragShader_envmap = R"(
#version 330 
in vec3 wPosition;
in vec3 wDirection;
in vec3 wCameraPos;

out vec4 fragColor;

uniform bool isBackground;
uniform samplerCube skyboxCubemap;

void main()
{
	if (isBackground){
		fragColor = texture(skyboxCubemap, wDirection);
	}
	else{
		vec3 V = normalize(wPosition - wCameraPos);
		vec3 N = normalize(wDirection);
		vec3 R = reflect(V, N);

		fragColor = texture(skyboxCubemap, R);
	}
}
)";

const char* nishita_vs = R"(
#version 330
layout(location=0) in vec2 vert;
layout(location=1) in vec2 surfacePosAttrib;
out vec2 surfacePosition;

uniform mat4 mvp;

void main()
{
	surfacePosition = surfacePosAttrib;
	gl_Position = mvp * vec4(vert, 0.0f, 1.0f);
}
)";

// code from: https://github.com/yumcyaWiz/skydome-rendering
const char* nishita_fs = R"(
#version 330
in vec2 surfacePosition;
uniform vec3 front_dir;
uniform vec3 up_dir;

// variables
uniform vec3 sunDir;
uniform float gamma;

// constants for 32-bit floating point
const float PI = 3.1415926; 
const float eps = 1e-6;
const float inf = 1.0 / 0.0;

// earth parameters (constants)
const float atmos_radius = 6420.0 * 1000.0; // 6420 [km] = 6360 + 60 [km]
const float earth_radius = 6360.0 * 1000.0; // 6360 [km]
const vec3 beta_rayleigh = vec3(5.8e-6, 13.5e-6, 33.1e-6); // accurate value
const vec3 beta_mie      = vec3(21e-6);
const float rayleigh_scaleheight = 7400.0; // 7400 [m]
const float mie_scaleheight      = 1200.0; // 1200 [m]
const float g = 0.74; // 0.74 is Henyey-Greenstein phase function asymmetry factor
const float beta_ext = 2.1; // 2.1 is attenuation term of absorbation

const float sun_intensity = 20.0;

// hyper-parameters for rendering
const int view_samples  = 20;
const int light_samples = 10;

out vec4 fragColor;

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Sphere {
	float radius;
	vec3  center;
};

bool intersect_ray_to_sphere(Ray ray, Sphere s, out float t) {
    t = inf;
	vec3 o = ray.origin - s.center;
	vec3 d = ray.direction;

	float b = dot(d, o);
	float r = s.radius;

	float det = b * b - dot(o, o) + r * r;
	if (det < 0.0) return false; else det = sqrt(det);
	return (t = - b - det) > eps ? true : ((t = - b + det) > eps ? true : false);
}

float rayleigh_phase(vec3 rayDir, vec3 lightDir) {
	float mu  = dot(rayDir, lightDir);
    float mu2 = mu * mu;
	return 3.0 / (16.0*PI) * (1.0 + mu2);
}

float mie_phase(vec3 rayDir, vec3 lightDir, float g) {
	float mu  = dot(rayDir, lightDir);
    float mu2 = mu * mu;
    float g2 = g * g;
	return 3.0 / (8.0*PI) * ((1.0 - g2) * (1.0 + mu2))
                / ((2.0 + g2) * pow((1.0 + g2 - 2.0*g*mu), 1.5));
}

vec3 sky(Ray ray)
{
	const vec3 earth_origin = vec3(0.0);
    Sphere sphere_atmos = Sphere(atmos_radius, earth_origin);
	
    // check the first hit
    float atmos_dist;
    if (!intersect_ray_to_sphere(ray, sphere_atmos, atmos_dist)) {
    	return vec3(0.0); // didn't hit the outer bound of atmosphere
    }
	
    // integrate values
    float optical_depth_rayleigh = 0.0;
    float optical_depth_mie      = 0.0;
    vec3 color_rayleigh = vec3(0.0);
    vec3 color_mie      = vec3(0.0);
	
	// sampling to the ray direction
    float ds = atmos_dist / float(view_samples);
    for (int i = 0; i < view_samples; i++) {
        float t = ds * (i + 0.5);
        vec3 samplePos = ray.origin + t * ray.direction;
		
        float sea_level = length(samplePos - earth_origin) - earth_radius;
		
        float h_rayleigh = exp(-sea_level/rayleigh_scaleheight) * ds;
        float h_mie      = exp(-sea_level/mie_scaleheight     ) * ds;
        optical_depth_rayleigh += h_rayleigh;
        optical_depth_mie      += h_mie;
		
        // direct illumination sampling
        Ray lightRay = Ray(samplePos, sunDir);
        float atmos_dist_lightray;
        if(!intersect_ray_to_sphere(lightRay, sphere_atmos, atmos_dist_lightray)) {
			break; // didn't hit the outer bound of atmosphere
        }
		
		// optical depth for each term
        float optical_depth_rayleigh_light = 0.0;
        float optical_depth_mie_light      = 0.0;
		
		// sampling points to the light direction to compute scattering
        float ds_light = atmos_dist_lightray / float(light_samples);
        for(int j = 0; j < light_samples; j++) {
            float t2 = ds * (j + 0.5);
            vec3  samplePos_light = lightRay.origin + t2 * lightRay.direction;
            float sea_level_light = length(samplePos_light - earth_origin) - earth_radius;

            if (sea_level_light < 0.0) {
                break; // stop when ray penetrated the earth
            }
			
            optical_depth_rayleigh_light += exp(-sea_level_light / rayleigh_scaleheight) * ds_light;
            optical_depth_mie_light      += exp(-sea_level_light / mie_scaleheight     ) * ds_light;
			
			// compute scattering value at the last sample
            if(j == (light_samples - 1) ) {
                float optical_rayleigh = optical_depth_rayleigh + optical_depth_rayleigh_light;
                float optical_mie      = optical_depth_mie      + optical_depth_mie_light;
				
                vec3 tau = beta_rayleigh * optical_rayleigh + beta_mie * beta_ext * optical_mie;
                vec3 attenuation = exp(-tau);
                color_rayleigh += h_rayleigh * attenuation;
                color_mie      += h_mie      * attenuation;
            }
        }
    }
	
    // compute integrated color
    float phase_rayleigh = rayleigh_phase(ray.direction, sunDir);
    float phase_mie      = mie_phase(ray.direction, sunDir, g);
	
    vec3 term_rayleigh = color_rayleigh * beta_rayleigh * phase_rayleigh;
    vec3 term_mie      = color_mie      * beta_mie      * phase_mie     ;
    return (term_rayleigh + term_mie) * sun_intensity;
}

Ray generate_camera_ray(vec2 uv) {
	// vectors to define camera coordinate
	vec3 from  = vec3(0.0, earth_radius + 1.5,  0.0); // 1.5 [m] (around human eye level)
	vec3 front = front_dir;
	vec3 up    = up_dir;
	vec3 right = cross(up_dir, front_dir);

	// compute camera coordinate and ray for pixel
	vec3 w = normalize(front);
	vec3 u = normalize(right);
	vec3 v = normalize(up);

	// cubemap: FOV is fixed to 90 [deg]
	float d = 1.0;
	return Ray(from, normalize(d * w + uv.x * u + uv.y * v) );
}

void main(void) {
	// compute ray for this pixel: [0.0:1.0]^2 -> [-1.0:+1.0]^2
	Ray ray = generate_camera_ray(surfacePosition);

	fragColor = vec4(0.5 * ray.direction + 0.5, 1.0);

	// get color from [Nishita93]
	vec3 color = sky(ray);
	
	// gamma correction for PBR
	color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / gamma));
	fragColor = vec4(color, 1.0);
}
)";

class SkyMappingAppGL : public AppGLBase
{
public:
	std::vector<std::string> files{
		"posx.jpg",
		"negx.jpg",
		"posy.jpg",
		"negy.jpg",
		"posz.jpg",
		"negz.jpg"
	};

	std::vector<glm::vec3> front_dirs{
		glm::vec3(+1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3( 0.0f,+1.0f, 0.0f),
		glm::vec3( 0.0f,-1.0f, 0.0f),
		glm::vec3( 0.0f, 0.0f,+1.0f),
		glm::vec3( 0.0f, 0.0f,-1.0f)
	};

	std::vector<glm::vec3> up_dirs{
		glm::vec3(0.0f,+1.0f, 0.0f),
		glm::vec3(0.0f,+1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f,-1.0f),
		glm::vec3(0.0f, 0.0f,+1.0f),
		glm::vec3(0.0f,+1.0f, 0.0f),
		glm::vec3(0.0f,+1.0f, 0.0f)
	};

	SkyMappingAppGL(const int width, const int height) : AppGLBase(width, height) {};

	// mandatory callback
	void Draw(const int width, const int height)
	{
		glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 proj = glm::infinitePerspective(glm::radians(90.0f), width / (float)height, 0.1f);
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
		view = view * glm::mat4_cast(glm::inverse(GlobalViewRotation));

		glMatrixMode(GL_PROJECTION); glLoadMatrixf(glm::value_ptr(proj));
		glMatrixMode(GL_MODELVIEW);  glLoadMatrixf(glm::value_ptr(view));

		////////////////////////////////////////////////////////////
		// 1st: draw IcoSphere with shader
		////////////////////////////////////////////////////////////
		glDepthFunc(GL_LESS);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glPolygonMode(GL_FRONT, GL_FILL);

		// set cubemap as texture #0
		glEnable(GL_TEXTURE_CUBE_MAP);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

		if (show_object) {
			shader_modern.DrawShader([&]() {
				GLint loc;
				loc = shader_modern.GetUniformLocation("Projection");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
				loc = shader_modern.GetUniformLocation("ModelView");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
				loc = shader_modern.GetUniformLocation("skyboxCubemap");
				glUniform1i(loc, 0); // 0 for GL_TEXTURE0
				loc = shader_modern.GetUniformLocation("isBackground");
				glUniform1i(loc, 0);

				// set vertices with normals
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, &icosphere.vertices[0]);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, &icosphere.normals[0]);
				glEnableVertexAttribArray(1);

				vgl::drawTriMesh(icosphere.vertices, icosphere.normals, icosphere.faces);
			});
		}

		////////////////////////////////////////////////////////////
		// 2nd: draw cube for skybox
		////////////////////////////////////////////////////////////
		glDepthFunc(GL_LEQUAL);
		glPolygonMode(GL_BACK, GL_FILL);

		if (show_skybox) {
			shader_legacy.DrawShader([&]() {
				GLint loc;
				loc = shader_legacy.GetUniformLocation("Projection");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
				loc = shader_legacy.GetUniformLocation("ModelView");
				glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
				loc = shader_legacy.GetUniformLocation("skyboxCubemap");
				glUniform1i(loc, 0); // 0 for GL_TEXTURE0
				loc = shader_legacy.GetUniformLocation("isBackground");
				glUniform1i(loc, 1);

				vgl::drawCube(1e5f); // draw large enough cube
			});
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}

	void DrawSkyQuad(glm::vec3 front_dir, glm::vec3 up_dir)
	{
		const float L = -1.0f;
		const float R = +1.0f;
		const float B = +1.0f;
		const float T = -1.0f;

		shader_nishita.DrawShader([&]() {
			GLuint loc;
			loc = shader_nishita.GetUniformLocation("mvp");
			if (-1 != loc) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
			loc = shader_nishita.GetUniformLocation("front_dir");
			if (-1 != loc) glUniform3fv(loc, 1, glm::value_ptr(front_dir));
			loc = shader_nishita.GetUniformLocation("up_dir");
			if (-1 != loc) glUniform3fv(loc, 1, glm::value_ptr(up_dir));
			loc = shader_nishita.GetUniformLocation("sunDir");
			if (-1 != loc) glUniform3fv(loc, 1, glm::value_ptr(Light_dir));
			loc = shader_nishita.GetUniformLocation("gamma");
			if (-1 != loc) glUniform1f(loc, gamma);

			// [TODO] additional variables

			glBegin(GL_TRIANGLES);
			glVertexAttrib2f(1, L, B); glVertex2f(-1.0f, -1.0f);
			glVertexAttrib2f(1, R, B); glVertex2f(+1.0f, -1.0f);
			glVertexAttrib2f(1, R, T); glVertex2f(+1.0f, +1.0f);

			glVertexAttrib2f(1, L, B); glVertex2f(-1.0f, -1.0f);
			glVertexAttrib2f(1, R, T); glVertex2f(+1.0f, +1.0f);
			glVertexAttrib2f(1, L, T); glVertex2f(-1.0f, +1.0f);
			glEnd();
		});
	}

	void ComputeCubemap(bool init = false)
	{
		// get the current viewport status
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		// prepare cubemap
		glEnable(GL_TEXTURE_CUBE_MAP);

		if (init) glGenTextures(1, &cubemap);

		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

		// resize cubemap in advance
		int W = 512;
		int H = 512;
		offscreen_FBO->Resize(W, H);

		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

		// six directions for cubemap
		for (int n = 0; n < 6; n++)
		{
			offscreen_FBO->DrawFBO([&] {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, cubemap, 0);
				glActiveTexture(GL_TEXTURE0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

				DrawSkyQuad(front_dirs[n], up_dirs[n]);
			});
		}

		if (init) {
			// set texture parameters
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}

		// unbind cubemap for another process
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	}

	void SaveCubemap()
	{
		char* dirpath = tinyfd_selectFolderDialog(
			"Set directory that reserve cubemap images files", "./");

		if (dirpath)
		{
			// get the current viewport status
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			// resize cubemap in advance
			int W = 512;
			int H = 512;
			offscreen_FBO->Resize(W, H);

			for (int n = 0; n < 6; n++)
			{
				char buf[256];
				sprintf(buf, "%s\\%s", dirpath, files[n].c_str());

				offscreen_FBO->DrawFBO([&] {
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, cubemap, 0);
					glActiveTexture(GL_TEXTURE0);
					DrawSkyQuad(front_dirs[n], up_dirs[n]);
					
					offscreen_FBO->CopyColorToBuffer();
					
					stbi_write_jpg(buf, W, H, 3, offscreen_FBO->buffer_color, 100);
					printf("write: %s\n", buf);
				});
			}

			// revert rendering status
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
			glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		}
	}

	bool Init()
	{
		SetAppGLTitle("SkyMappingApp");

		// UI: AntTweakBar
		TwAddButton(bar, "Save Cubemap", [](void* client) {
			SkyMappingAppGL* _this = (SkyMappingAppGL*)client;
			_this->SaveCubemap();
			}, this, " ");

		TwAddVarCB(bar, "Light_dir", TwType::TW_TYPE_DIR3F,
			[](const void* value, void* obj) {
				SkyMappingAppGL* _this = (SkyMappingAppGL*)obj;
				glm::vec4 sunDir = *(glm::vec4*)value;
				_this->Light_dir = glm::normalize(sunDir);
			},
			[](void* value, void* obj) {
				SkyMappingAppGL* _this = (SkyMappingAppGL*)obj;
				*(glm::vec4*)value = _this->Light_dir;
				_this->ComputeCubemap();
			}, this, " label='Light_dir' open");

		TwAddVarCB(bar, "gamma", TwType::TW_TYPE_FLOAT,
			[](const void* value, void* obj) {
				SkyMappingAppGL* _this = (SkyMappingAppGL*)obj;
				float gamma = *(float*)value;
				_this->gamma = gamma;
			},
			[](void* value, void* obj) {
				SkyMappingAppGL* _this = (SkyMappingAppGL*)obj;
				*(float*)value = _this->gamma;
				_this->ComputeCubemap();
			}, this, " label='gamma' min=1.0 step=0.01");

		// global viewer
		resetGlobalView();
#if 1
		TwAddButton(bar, "Global-init", [](void* client) {
			SkyMappingAppGL* _this = (SkyMappingAppGL*)client; _this->resetGlobalView();
			}, this, "group='Global' label='init' ");
		TwAddVarRW(bar, "Global-rot", TwType::TW_TYPE_QUAT4F, &GlobalViewRotation, "group='Global' label='rot'  open");
#endif
		TwAddVarRW(bar, "show_object", TwType::TW_TYPE_BOOLCPP, &show_object, "group='Global' label='show_object'");
		TwAddVarRW(bar, "show_skybox", TwType::TW_TYPE_BOOLCPP, &show_skybox, "group='Global' label='show_skybox'");

		// prepare icosphere to test rendering
		icosphere = vgl::IcoSphere(3, 2.0f);

		// prepare shader for environment mapping with cubemap
		shader_modern = vgl::GLShader();
		shader_modern.Compile(vertShader_modern, vgl::SHADER_TYPE::VERTEX);
		shader_modern.Compile(fragShader_envmap, vgl::SHADER_TYPE::FRAGMENT);
		shader_modern.Link();

		// prepare shader for environment mapping with cubemap
		shader_legacy = vgl::GLShader();
		shader_legacy.Compile(vertShader_legacy, vgl::SHADER_TYPE::VERTEX);
		shader_legacy.Compile(fragShader_envmap, vgl::SHADER_TYPE::FRAGMENT);
		shader_legacy.Link();

		// prepare shader for environment mapping with cubemap
		shader_nishita = vgl::GLShader();
		shader_nishita.Compile(nishita_vs, vgl::SHADER_TYPE::VERTEX);
		shader_nishita.Compile(nishita_fs, vgl::SHADER_TYPE::FRAGMENT);
		shader_nishita.Link();

		// off-screen renderer for conversion from equirectangular image to cubemap
		offscreen_FBO = new vgl::FBO();
		ComputeCubemap(true);

		return true;
	}

	void End()
	{
		glDeleteTextures(1, &cubemap);

		delete offscreen_FBO;
		offscreen_FBO = nullptr;
	}

private:
	bool show_object = true;
	bool show_skybox = true;

	// global viewer
	glm::quat GlobalViewRotation = glm::quat();
	void resetGlobalView() {
		GlobalViewRotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
	}

	vgl::IcoSphere icosphere;

	glm::vec4 Light_dir = glm::normalize(glm::vec4(0, 1, 1, 0));
	float gamma = 2.2f;

	vgl::GLShader shader_modern;
	vgl::GLShader shader_legacy;
	vgl::GLShader shader_nishita;

	GLuint cubemap = 0;

	vgl::FBO* offscreen_FBO = nullptr;
};


///////////////////////////////////////////////////////////////////////////////
// entry point
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	SkyMappingAppGL app(640, 480);
	app.run();
	return EXIT_SUCCESS;
}
