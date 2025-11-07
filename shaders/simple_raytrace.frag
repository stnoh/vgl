#version 330

uniform vec2 resolution;

#define NUM_SPHERES 3

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Sphere {
	float radius;
	vec3 center;
	vec3 emission;
	vec3 color;
};

Sphere sphere(int i) {
	if (i--==0) return	Sphere(1e5 , vec3(50., -1e5, 81.6), vec3(0.), vec3( 0.75, 0.75, 0.75) );
	if (i--==0) return	Sphere(25.0, vec3(40., 25.0, 47.0), vec3(0.), vec3( 0.85, 0.525, 0.125) );
 	return 				Sphere(1.  , vec3(50., 81.6, 81.6), vec3(1.0), vec3(0.)); // light source, also
}

float intersect_ray_to_sphere(Ray r, Sphere s) {
	const float epsilon = 1e-3;

	vec3 oc = r.origin - s.center;
	float t;
	float b = dot(r.direction, oc);
	float det = b * b - dot(oc, oc) + s.radius * s.radius;
	if (det < 0.) return 0.; else det = sqrt(det);
	return (t = - b - det) > epsilon ? t : ((t = - b + det) > epsilon ? t : 0.);
}

int intersect_all(Ray r, out float t, out Sphere s, int avoid) {
	int id = -1;
	t = 1e5;
	s = sphere(0);

	for (int i = 0; i < NUM_SPHERES; i++) { 
		Sphere S = sphere(i);
		float d = intersect_ray_to_sphere(r, S);
		if (i != avoid && d != 0. && d<t) { t = d; id = i; s=S; }
	}
	return id;
}

vec3 raytrace(Ray r)
{
	float t;
	Sphere obj;
	int id = -1;
	if ((id = intersect_all(r, t, obj, id)) < 0) return vec3(0.0);

	if (id==NUM_SPHERES-1) return vec3(obj.emission);

	vec3 p = r.origin + t * r.direction;
	vec3 n = normalize(p - obj.center);

	// ray to the single light source
	Sphere light = sphere(NUM_SPHERES-1);
	vec3 light_p = light.center;

	// shadow ray
	Ray r_l = Ray(p, normalize(light_p - p));
	Sphere obj2;
	id = intersect_all(r_l, t, obj2, id);

	if (id != NUM_SPHERES-1) return vec3(0.0);

	return light.emission * obj.color; // [TODO] something is missing!
}

void main( void ) {
	vec2 uv = 2. * gl_FragCoord.xy / resolution.xy - 1.; // [-0.5:+0.5]^2

	vec3 from = vec3(50.0, 40.8, 169.0);
	vec3 to   = vec3(50.0, 40.0,  81.6);
	vec3 up   = vec3( 0.0,  1.0,   0.0);

	vec3 w = normalize(from - to);
	vec3 u = normalize(cross(up, w));
	vec3 v = normalize(cross(w, u));

	// roughly 
	Ray ray = Ray(from, normalize(0.53135 * (resolution.x / resolution.y * uv.x * u + uv.y * v) - w) );
	vec3 color = raytrace(ray);

	// gamma correction for PBR
	float gamma = 2.2;
	gl_FragColor = vec4(pow(clamp(color, 0., 1.), vec3(1./gamma)), 1.0);
}
 