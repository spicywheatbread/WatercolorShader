#version 330

uniform sampler2DRect colorImage;
uniform sampler2DRect blurImage;
uniform sampler2DRect bleedImage;
uniform sampler2DRect controlImage;

in vec2 vTexcoord;

out vec4 fragColor;

void main() {
	vec4 difference = texture(bleedImage, vTexcoord) - texture(colorImage, vTexcoord);
	fragColor = texture(controlImage, vTexcoord).b * difference + texture(colorImage, vTexcoord);
	vec4 blurDiff = texture(blurImage, vTexcoord) - texture(colorImage, vTexcoord);
	float max = max(blurDiff.r, max(blurDiff.g, max(blurDiff.b, blurDiff.a)));
	max = smoothstep(0.02, 0.1, max);
	fragColor = pow(fragColor, vec4(1 + texture(controlImage, vTexcoord).b * max));
}
