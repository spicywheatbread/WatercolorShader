#version 330

uniform sampler2D colorImage;
uniform sampler2D blurImage;
uniform sampler2D bleedImage;
uniform sampler2D controlImage;
uniform sampler2D paperImage;

uniform vec2 pixelSize;

in vec2 vTexcoord;

out vec4 fragColor;

void main() {
	// Retrieve information stored in paper image: rg stores normal xy
	vec4 paperValue = texture(paperImage, vTexcoord);
	vec2 paperDistortTexcoord = vTexcoord + (pixelSize * paperValue.rg * 10);

	// Calculate diff in bleed & color, combine with control image to alter color image
	vec4 difference = texture(bleedImage, paperDistortTexcoord) - texture(colorImage, paperDistortTexcoord);
	fragColor = texture(controlImage, paperDistortTexcoord).b * difference + texture(colorImage, paperDistortTexcoord);

	// Edge darkening effect with minimal difference of gaussians
	vec4 blurDiff = texture(blurImage, paperDistortTexcoord) - texture(colorImage, paperDistortTexcoord);
	float max = max(blurDiff.r, max(blurDiff.g, max(blurDiff.b, blurDiff.a)));
	max = smoothstep(0.02, 0.1, max);

	fragColor = pow(fragColor, vec4(1 + texture(controlImage, paperDistortTexcoord).b * max));

	// Paper granulation interpolates between color transmittance and color subtraction based on saturation of final color
	float invertedPaper = 1.0f - paperValue.b; // blue channel holds height map
	vec4 transmittance = fragColor * (fragColor - invertedPaper);
	float density = 1.0f * invertedPaper;
	fragColor = transmittance + (vec4(1.0) - fragColor) * pow(fragColor, 1 + texture(controlImage, vTexcoord) * density);
}
