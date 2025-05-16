#version 330

in vec2 vTexcoord;
out vec4 fragColor;

uniform sampler2DRect image;

void main() {
	float kernel[25] = float[]( // Normalized gaussian blur kernel
		1.0 / 273.0,  4.0 / 273.0,  7.0 / 273.0,  4.0 / 273.0, 1.0 / 273.0,
		4.0 / 273.0, 16.0 / 273.0, 26.0 / 273.0, 16.0 / 273.0, 4.0 / 273.0,
		7.0 / 273.0, 26.0 / 273.0, 41.0 / 273.0, 26.0 / 273.0, 7.0 / 273.0,
		4.0 / 273.0, 16.0 / 273.0, 26.0 / 273.0, 16.0 / 273.0, 4.0 / 273.0,
		1.0 / 273.0,  4.0 / 273.0,  7.0 / 273.0,  4.0 / 273.0, 1.0 / 273.0
	);
	vec3 result = vec3(0.0);
	int index = 0;

	for (int y = -2; y <= 2; y++) {
		for (int x = -2; x <= 2; x++) {
			// Because we're using arb tex, tex coord are actually pixel instead of normalized.
			// Therefore, we can offset by whole pixel values.
			vec2 offset = vec2(x, y);
			result += texture(image, 2 * (vTexcoord + offset)).rgb * kernel[index]; // Mult by 2 b/c blurFBO is half size
			index++;
		}
	}

	fragColor = vec4(result, 1.0);
}
