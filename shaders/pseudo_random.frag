uniform vec2 resolution;
uniform float time;

float random(vec2 uv)
{
	// https://www.npmjs.com/package/glsl-random?activeTab=code
	return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453 * time);
}

void main(void) {
	gl_FragColor = vec4(vec3(random(gl_FragCoord.xy/resolution.xy)), 1.0);
}
