#version 330

uniform vec2 resolution;

const float PI = 3.1415926; // 32-bit floating point
const float eps = 1e-6;
const float inf = 1.0 / 0.0;

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Sphere {
	float radius;
	vec3  center;
	vec3  emission;
	vec3  color;
};

const int NUM_SPHERES = 3;

Sphere sphere(int i) {
	if (i--==0) return	Sphere(1e5 , vec3(50., -1e5, 81.6), vec3(0.0), vec3( 0.75, 0.75, 0.75) );
	if (i--==0) return	Sphere(25.0, vec3(40., 25.0, 47.0), vec3(0.0), vec3( 0.85, 0.52, 0.12) );
				return	Sphere(1.0 , vec3(50., 81.6, 81.6), vec3(1.0), vec3( 1.00, 1.00, 1.00) ); // light source
}

Ray generate_camera_ray(vec2 uv)
{
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

vec3 shade(Ray ray, int id, float t)
{
	if (-1 == id) return vec3(0.029, 0.071, 0.218); // sky color for gamma 2.2 (map to [0.2, 0.3, 0.5])
	
	Sphere S = sphere(id);

	if (0.0 < dot(S.emission, S.emission) ) return S.emission;

	vec3 color = vec3(0.0);

	// get intersected position and its normal
	vec3 p = ray.origin + t * ray.direction;
	vec3 n = normalize(p - S.center);

	// [HARD-CODED]
	int light_id = NUM_SPHERES-1;
	Sphere light = sphere(light_id);
	vec3 light_p = light.center;

	// ray for light/shadow
	Ray ray_l = Ray(p, normalize(light_p - p));
	float t_l;

	if (light_id==trace(ray_l, t_l, id)) {
		color += light.emission * S.color;
	}

	return color;
}

void main(void) {
	// compute ray for this pixel
	vec2 uv = 2.0 * gl_FragCoord.xy / resolution.xy - 1.0; // [0.0:1.0]^2 -> [-1.0:+1.0]^2
	Ray ray = generate_camera_ray(uv);

	// raytrace ray and get color from its shading
	float t;
	int id = trace(ray, t, -1);
	vec3 color = shade(ray, id, t);
	
	// gamma correction for PBR
	float gamma = 2.2;
	color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / gamma));
	gl_FragColor = vec4(color, 1.0);
}
