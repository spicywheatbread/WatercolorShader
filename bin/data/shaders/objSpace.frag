#version 330

in vec4 vPosition;
in vec2 vTexcoord;
in vec4 vNormal;
in vec4 vColor;

out vec4 fragColor;

uniform sampler2DRect materialTex;

uniform vec3 lightPosition;

// Converter functions sourced from https://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
// All components are in the range [0…1], including hue.
vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
	// Implement custom reflectance model from paper
	// Custom control variables
	// TODO: Turn these into uniforms, just testing for now.
	float diluteArea = 1; // (0, 1]; Affects overall brightness I think?
	float dilute = 0.2; // [0, 1]
	float cangiante = 0.5; // [0, 1] ; Affects interpolation towards hue
	// vec4 paperColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // White paper color
	 vec4 paperColor = vec4(241.0f / 255.0f, 230.0f / 255.0f, 207.0f / 255.0f, 1.0f); // Parchment Paper color

	// Step 1: Dilute Area
	vec3 lightDir = normalize(lightPosition - vPosition.xyz);
	float diff = max(dot(lightDir, vNormal.xyz), 0.0f);
	float da = (diff + diluteArea - 1) / (diluteArea);

	vec4 baseColor = texture(materialTex, vTexcoord);
	vec4 cangianteColor = baseColor + (da * cangiante);
	fragColor = dilute * da * (paperColor - cangianteColor) + cangianteColor;

	// Pigment Turbulence / Paper Fade: We use the alpha var to switch between these behaviors.
	if(vColor.a < 0.5) { // Exponential attenuation
		fragColor = pow(fragColor, vec4(3 - (vColor.a * 4)));
	} else { //
		fragColor = (vColor.a - 0.5) * 2 * (paperColor - fragColor) + fragColor;
	}

}
