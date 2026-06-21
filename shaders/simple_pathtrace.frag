#version 330

// modified version of Zavie's path tracer
// https://www.shadertoy.com/view/4sfGDB

uniform sampler2D backbuffer;
uniform float time;
uniform vec2 resolution;

const int SPP = 16;
const int MAXDEPTH = 6;
const bool useNEE = false;

const float PI     = 3.1415926; // 32-bit floating point
const float TWO_PI = 2.0 * PI;
const float eps = 1e-6;
const float inf = 1.0 / 0.0;

const int DIFFUSE = 0;
const int MIRROR  = 1;
const int GLASS   = 2;

const float eta = 1.5; // refraction index for "glass"

// pseudo random generator for GLSL shader
float seed = 0.0;
float rand() { return fract(sin(seed++) * 43758.5453123); }

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Sphere {
	float radius;
	vec3  center;
	vec3  emission;
	vec3  color;
	int   material;
};

const int NUM_SPHERES = 9;
const int LIGHT_ID = 8;

Sphere sphere(int i) {
	if (i--==0) return Sphere(1e5, vec3( 1e5+1 ,40.8,81.6), vec3(0.0), vec3(0.75,0.25,0.25), DIFFUSE);
	if (i--==0) return Sphere(1e5, vec3(-1e5+99,40.8,81.6), vec3(0.0), vec3(0.25,0.25,0.75), DIFFUSE);
	if (i--==0) return Sphere(1e5, vec3(50,40.8, 1e5),      vec3(0.0), vec3(0.75), DIFFUSE);
	if (i--==0) return Sphere(1e5, vec3(50,40.8,-1e5+170),  vec3(0.0), vec3(0.00), DIFFUSE);
	if (i--==0) return Sphere(1e5, vec3(50, 1e5, 81.6),     vec3(0.0), vec3(0.75), DIFFUSE);
	if (i--==0) return Sphere(1e5, vec3(50,-1e5+81.6,81.6), vec3(0.0), vec3(0.75), DIFFUSE);
	if (i--==0) return Sphere(16.5,vec3(27,16.5,47),        vec3(0.0), vec3(1.0), MIRROR);
	if (i--==0) return Sphere(16.5,vec3(73,16.5,78),        vec3(0.0), vec3(0.7, 1.0, 0.9), GLASS);
				return Sphere(600.,vec3(50., 681.33, 81.6), vec3(12.), vec3(0.0), DIFFUSE);
}

Ray generate_camera_ray(vec2 uv) {
	// vectors to define camera coordinate
	vec3 from = vec3(50.0, 40.8, 169.0);
	vec3 to   = vec3(50.0, 40.0,  81.6);
	vec3 up   = vec3( 0.0,  1.0,   0.0);

	// compute camera coordinate and ray for pixel
	vec3 w = normalize(from - to);
	vec3 u = normalize(cross(up, w));
	vec3 v = normalize(cross(w, u));
	
	const float fov_half = 0.5135; // camera's fov (54.36-degree/2 = 27.18-degree)
	return Ray(from, normalize(fov_half * (resolution.x / resolution.y * uv.x * u + uv.y * v) - w) );
}

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

int trace(Ray ray, out float t_min, int avoid) {
	int id_min = -1;
	t_min = inf;
	
	for (int i = 0; i < NUM_SPHERES; i++) {
		if (avoid == i) continue;
		Sphere S = sphere(i);
		float t;
		if (intersect_ray_to_sphere(ray, S, t) && t < t_min) {
			t_min = t;
			id_min = i;
		}
	}
	return id_min;
}

vec3 align_to_vector(vec3 z_axis, vec3 local_dir) {
	vec3 helper = abs(z_axis.x) > 0.9 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
	vec3 x_axis = normalize(cross(helper, z_axis));
	vec3 y_axis = cross(z_axis, x_axis);

	return local_dir.x * x_axis + local_dir.y * y_axis + local_dir.z * z_axis;
}

float get_light_pdf(vec3 p, Sphere light, out float cos_theta, out vec3 z_axis) {
	vec3 to_light_center = light.center - p;
	float d2 = dot(to_light_center, to_light_center);
	float r2 = light.radius * light.radius;

	if (d2 <= r2) return 0.0;

	// normalized direction and maximum opening angle to the cone
	z_axis = to_light_center / sqrt(d2);
	cos_theta = sqrt(1.0 - r2 / d2);
	return 1.0 / (TWO_PI * (1.0 - cos_theta));
}

vec3 NEE(vec3 p, vec3 n, int id, int id_light, out vec3 out_dir, out float out_pdf) {

	// initialize output
	out_dir = vec3(0.0);
	out_pdf = 0.0;

	Sphere light = sphere(id_light);

	// get the maximum opening angle of cone in the hemisphere
	float cos_theta_max;
	vec3 z_axis;
	out_pdf = get_light_pdf(p, light, cos_theta_max, z_axis);

	if (out_pdf <= 0.0) return vec3(0.0);

	// uniform sampling in the cone
	float u1 = rand();
	float u2 = rand();
	float cos_theta = 1.0 - u1 * (1.0 - cos_theta_max);
	float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
	float phi = TWO_PI * u2;

	vec3 local_dir = vec3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
	out_dir = align_to_vector(z_axis, local_dir);

	if (dot(n, out_dir) <= 0.0) return vec3(0.0);

	// check shadow ray
	Ray ray = Ray(p, out_dir);
	float t;
	id = trace(ray, t, id);
	if (id_light != id) return vec3(0.0);

	// compute pdf and radiance by NEE
	return light.emission * cos_theta_max / out_pdf;
}

float mis_weight(float pdf_a, float pdf_b) {
	// Veach's power heuristic
	float  pdf_a2 = pdf_a * pdf_a;
	float  pdf_b2 = pdf_b * pdf_b;
	return pdf_a2 / (pdf_a2 + pdf_b2);
}

vec3 sample(vec3 d, float phi, float sin_a, float cos_a) {
	// random sample point (=direction) on a hemisphere
	vec3 w = normalize(d); // hemisphere's Z(up)-vector
	vec3 u = normalize(cross(w.yzx, w));
	vec3 v = cross(w, u);

	return sin_a * (u * cos(phi) + v * sin(phi)) + cos_a * w;
}

vec3 radiance(Ray ray) {
	vec3 color = vec3(0.0); // radiance accumulator
	vec3 beta  = vec3(1.0); // pass throughput weight
	int id = -1;

	// additional variables for NEE
	bool  last_ray_was_specular = true; 
	float last_bsdf_pdf = 1.0; 

	// bounded loop ray tracing instead of its recursive version
	for (int depth=0; depth < MAXDEPTH; depth++) {
		float t;
		id = trace(ray, t, id);
		if (-1==id) break;

		// get next intersection point and its surface normal
		Sphere obj = sphere(id);
		vec3 p = ray.origin + t * ray.direction;
		vec3 n = normalize(p - obj.center);
		vec3 nl = sign(-dot(n, ray.direction)) * n;

		// stop ray tracing at the emitted object
		if (id == LIGHT_ID) {
			
			if (!useNEE || last_ray_was_specular) {
				color += beta * obj.emission;
			}
			else{
				// cancel the doubled light integration
				float cos_theta;
				vec3  z_axis;
				float pdf_NEE = get_light_pdf(p, sphere(LIGHT_ID), cos_theta, z_axis);

				float weight = mis_weight(last_bsdf_pdf, pdf_NEE);
				color += beta * obj.emission * weight;
			}

			break;
		}

		if (DIFFUSE==obj.material) {

			// next event estimation
			if (useNEE){
				vec3  dir_light;
				float pdf_NEE;

				vec3  e_NEE = NEE(p, nl, id, LIGHT_ID, dir_light, pdf_NEE);

				if (0.0 < pdf_NEE) {
					float cos_theta = clamp(dot(nl, dir_light), 0.0, 1.0);
					float pdf_bsdf  = cos_theta / PI;

					float weight = mis_weight(pdf_NEE, pdf_bsdf);
					color += beta * e_NEE / PI * obj.color * weight;
				}
			}

			beta *= obj.color;

			// determine next ray direction from random uniform sampling
			float r2 = rand();
			float sin_a = sqrt(r2);
			float cos_a = sqrt(1.0-r2);
			vec3 dir = sample(nl, TWO_PI * rand(), sin_a, cos_a);

			// update ray and other data
			ray = Ray(p, dir);
			last_ray_was_specular = false; 
			last_bsdf_pdf = cos_a / PI; 
		}
		else if (MIRROR==obj.material) {
			beta *= obj.color;

			ray = Ray(p, reflect(ray.direction, n));
			last_ray_was_specular = true;
		}
		else if (GLASS==obj.material) {
			// incident angle
			float a = dot(n, ray.direction);
			float ddn = abs(a);
			
			float nc = 1.0; // vacuum's refraction index
			float nt = eta; // t denotes "transmission"
			float nnt = (a > 0.0) ? nt / nc : nc / nt;

			float cos2_t = 1.0 - nnt * nnt * (1.0 - ddn * ddn);

			// reflection ray: total reflection or two pathes
			ray = Ray(p, reflect(ray.direction, n));

			// two pathes: reflection and refraction
			if (cos2_t > 0.0) {
				vec3 dir_t = normalize(nnt * ray.direction + sign(a)*(ddn*nnt+sqrt(cos2_t))*n);

				float R0 = (nt-nc) / (nt+nc);
				R0 = R0 * R0;
				float c = (a > 0.0) ? dot(dir_t, n) : ddn;
				c = 1.0 - c;

				float Re = R0 + (1.0 - R0) * pow(c, 5.0);

				// randomly select a single path to suppress divergent branch
				float P = 0.25 + 0.5*Re;
				if (rand() < P) {
					// path 1) reflection
					float RP = Re / P;
					beta *= RP;
				}
				else {
					// path 2) refraction
					float TP = (1.0 - Re) / (1.0 - P);
					beta *= obj.color * TP;
					ray = Ray(p, dir_t);
				}
			}

			last_ray_was_specular = true;
		}
		else {
			break; // fallback
		}
	}

	return color;
}

void main(void) {
	// set seed for pseudo random number generator
	seed = time + resolution.y * gl_FragCoord.x / resolution.x + gl_FragCoord.y / resolution.y;

	// compute ray for this pixel
	vec2 uv = 2.0 * gl_FragCoord.xy / resolution.xy - 1.0; // [0.0:1.0]^2 -> [-1.0:+1.0]^2
	Ray ray = generate_camera_ray(uv);

	// integration with sample-per-pixel
	vec3 color = vec3(0.0);
	for (int i=0;i<SPP;i++) {
		color += radiance(ray);
	}
	
	const float gamma = 2.2;
	color /= float(SPP);

	// [quick checking] only show the result of this frame
	if (false) {
		gl_FragColor = vec4(pow(clamp(color, 0.0, 1.0), vec3(1.0 / gamma)), 1.0);
		return;
	}

	// integration with previously computed color
	vec4 previous = texture2D(backbuffer, gl_FragCoord.xy / resolution.xy);
	float weight = clamp(255.0 * previous.a, 0.0, 254.0); // [0.0:254.0]
	
	// gamma correction for PBR
	color = (color + pow(previous.rgb, vec3(gamma)) * weight) / (1.0 + weight);
	color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / gamma));
	gl_FragColor = vec4(color, (weight + 1.0) / 255.0);
}
