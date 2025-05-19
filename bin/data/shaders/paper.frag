#version 330

in vec2 vTexcoord;

out vec4 fragColor;

uniform sampler2D paperDiff;
uniform sampler2D paperNorm;

void main() {
	vec2 norm = texture(paperNorm, vTexcoord).rg; // Only retrieve the x y inclination
	vec4 diff = texture(paperDiff, vTexcoord);
	float height = dot(diff.rgb, vec3(0.333));
	fragColor = vec4(norm, height, 1.0f);

}
