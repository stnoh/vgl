#version 330

// modified version of Zavie's path tracer
// https://www.shadertoy.com/view/4sfGDB

uniform sampler2D backbuffer;
uniform float time;
uniform vec2 resolution;

const int SPP = 16;
const int MAXDEPTH = 6;

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
			color += beta * obj.emission;
			break;
		}

		if (DIFFUSE==obj.material) {
			beta *= obj.color;

			// determine next ray direction from random uniform sampling
			float r2 = rand();
			float sin_a = sqrt(r2);
			float cos_a = sqrt(1.0-r2);
			vec3 dir = sample(nl, TWO_PI * rand(), sin_a, cos_a);
			ray = Ray(p, dir);
		}
		else if (MIRROR==obj.material) {
			beta *= obj.color;

			ray = Ray(p, reflect(ray.direction, n));
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

	// integration with previously computed color
	vec4 previous = texture2D(backbuffer, gl_FragCoord.xy / resolution.xy);
	float weight = clamp(255.0 * previous.a, 0.0, 254.0); // [0.0:254.0]
	
	// gamma correction for PBR
	float gamma = 2.2;
	color = (color / float(SPP) + pow(previous.rgb, vec3(gamma)) * weight) / (1.0 + weight);
	color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / gamma));
	gl_FragColor = vec4(color, (weight + 1.0) / 255.0);
}
