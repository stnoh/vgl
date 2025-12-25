uniform vec2 resolution;
uniform float time;

float seed = 0.0;
float rand() { return fract(sin(seed++) * 43758.5453123); }

void main(void) {
	seed = time + resolution.y * gl_FragCoord.x / resolution.x + gl_FragCoord.y / resolution.y;

	float r = rand();
	float g = rand();
	float b = rand();
	gl_FragColor = vec4(r, g, b, 1.0);
}
