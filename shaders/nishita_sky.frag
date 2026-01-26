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

// code from: https://github.com/yumcyaWiz/skydome-rendering
vec3 sunDir = normalize(vec3(0, 1.0, -1.0));
vec3 earth_origin = vec3(0.0);

float atmos_radius = 6420.0 * 1000.0; // 6420 [km] = 6360 + 60 [km]
float earth_radius = 6360.0 * 1000.0; // 6360 [km]

// [CHECK ME LATER] constants or variables
float rayleigh_scaleheight = 7400.0; // [CHECK ME] unit?
float mie_scaleheight      = 1200.0; // [CHECK ME] unit?
float sun_intensity        = 20.0;
vec3 beta_rayleigh = vec3(3.8e-6, 13.5e-6, 33.1e-6);
vec3 beta_mie      = vec3(21e-6);

int view_samples  = 20;
int light_samples = 10;

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
    Sphere sphere_atmos = Sphere(atmos_radius, earth_origin);

    // check the first hit
    float res;
    if (!intersect_ray_to_sphere(ray, sphere_atmos, res)) {
    	return vec3(0.0); // didn't hit the outer bound of atmosphere
    }
    float atmos_dist = res;

    float ds = atmos_dist / float(view_samples);
    float phase_rayleigh = rayleigh_phase(ray.direction, sunDir);
    float phase_mie      = mie_phase(ray.direction, sunDir, 0.74); // [CHECK ME] what is 0.74

    // integrate values
    float optical_depth_rayleigh = 0.0;
    float optical_depth_mie      = 0.0;
    vec3 color_rayleigh = vec3(0.0);
    vec3 color_mie      = vec3(0.0);

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
        float res2;
        if(!intersect_ray_to_sphere(lightRay, sphere_atmos, res2)) {
            return vec3(0.0); // didn't hit the outer bound of atmosphere
        }

        float atmos_dist_lightray = res2;
        float ds_light = atmos_dist_lightray / float(light_samples);
        float optical_depth_rayleigh_light = 0.0;
        float optical_depth_mie_light      = 0.0;

        for(int j = 0; j < light_samples; j++) {
            float t2 = ds * (j + 0.5);
            vec3  samplePos_light = lightRay.origin + t2 * lightRay.direction;
            float sea_level_light = length(samplePos_light - earth_origin) - earth_radius;

            if (sea_level_light < 0.0) {
                break; // light ray penetrated the earth
            }

            optical_depth_rayleigh_light += exp(-sea_level_light / rayleigh_scaleheight) * ds_light;
            optical_depth_mie_light      += exp(-sea_level_light / mie_scaleheight     ) * ds_light;

            if(j == (light_samples - 1) ) {
                float optical_rayleigh = optical_depth_rayleigh + optical_depth_rayleigh_light;
                float optical_mie      = optical_depth_mie      + optical_depth_mie_light;

                vec3 tau = beta_rayleigh * optical_rayleigh + beta_mie * 2.1 * optical_mie; // [CHECK ME] what is 2.1
                vec3 attenuation = exp(-tau);
                color_rayleigh += h_rayleigh * attenuation;
                color_mie      += h_mie      * attenuation;
            }
        }
    }

    // compute integrated color
    vec3 term_rayleigh = color_rayleigh * beta_rayleigh * phase_rayleigh;
    vec3 term_mie      = color_mie      * beta_mie      * phase_mie     ;
    return (term_rayleigh + term_mie) * sun_intensity;
}

Ray generate_camera_ray(vec2 uv) {
	// vectors to define camera coordinate
	vec3 from  = vec3(0.0, earth_radius + 1.0,  0.0);
	vec3 front = vec3(0.0, 0.0, -1.0);
    vec3 right = cross(front, vec3(0.0, 1.0, 0.0));
    vec3 up    = cross(right, front); // [CAUTION]

	// compute camera coordinate and ray for pixel
	vec3 w = normalize(front);
	vec3 u = normalize(right);
	vec3 v = normalize(up);
	
    // [CAUTION] just follow the original code
    float fov = 90.0; // this value is actually handled as radian
	float d = 1.0 / tan(fov/2.0);
	return Ray(from, normalize(d * w + resolution.x / resolution.y * uv.x * u + uv.y * v) );
}

void main(void) {
	// compute ray for this pixel: [0.0:1.0]^2 -> [-1.0:+1.0]^2
	vec2 uv = 2.0 * gl_FragCoord.xy / resolution.xy - 1.0;
	Ray ray = generate_camera_ray(uv);

	// get color from volumetric ray casting
	vec3 color = sky(ray);
	
	// gamma correction for PBR
	float gamma = 2.2;
	color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / gamma));
	gl_FragColor = vec4(color, 1.0);
}
