#version 330

in vec4 vPosition;
in vec2 vTexcoord;
in vec4 vNormal;
in vec4 vColor;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 vertexColor;
layout (location = 2) out vec4 alphaColor;

uniform sampler2D tex0;

uniform vec3 lightPosition;


void main() {
	float diluteArea = 1; // (0, 1]; Affects diffuse factor
	float dilute = 0.2; // [0, 1]
	float cangiante = 0.5; // [0, 1] ; Affects interpolation towards hue

	 vec4 paperColor = vec4(241.0f / 255.0f, 230.0f / 255.0f, 207.0f / 255.0f, 1.0f); // Parchment Paper color

	// Step 1: Dilute Area
	vec3 lightDir = normalize(lightPosition - vPosition.xyz);
	float diff = max(dot(lightDir, vNormal.xyz), 0.0f);
	float da = (diff + diluteArea - 1) / (diluteArea);

	vec4 baseColor = texture(tex0, vTexcoord);
	vec4 cangianteColor = baseColor + (da * cangiante);
	fragColor = dilute * da * (paperColor - cangianteColor) + cangianteColor;

	// Pigment Turbulence / Paper Fade: We use the alpha var to switch between these behaviors.
	if(vColor.a < 0.5) { // Exponential attenuation
		fragColor = pow(fragColor, vec4(3 - (vColor.a * 4)));
	} else { // Fade to paper color
		fragColor = (vColor.a - 0.5) * 2 * (paperColor - fragColor) + fragColor;
	}
	vertexColor = vColor;
	alphaColor = vec4(vColor.a, vColor.a, vColor.a, 1.0f);
}
