#version 330

uniform sampler2D tex0;
uniform vec2 pixelSize;

in vec2 vTexcoord;

out vec4 fragColor;

const float kernel[25] = float[]( // Normalized gaussian blur kernel
		1.0 / 273.0,  4.0 / 273.0,  7.0 / 273.0,  4.0 / 273.0, 1.0 / 273.0,
		4.0 / 273.0, 16.0 / 273.0, 26.0 / 273.0, 16.0 / 273.0, 4.0 / 273.0,
		7.0 / 273.0, 26.0 / 273.0, 41.0 / 273.0, 26.0 / 273.0, 7.0 / 273.0,
		4.0 / 273.0, 16.0 / 273.0, 26.0 / 273.0, 16.0 / 273.0, 4.0 / 273.0,
		1.0 / 273.0,  4.0 / 273.0,  7.0 / 273.0,  4.0 / 273.0, 1.0 / 273.0
	);
void main() {
	vec3 result = vec3(0.0);
	int index = 0;

	for (int y = -2; y <= 2; y++) {
		for (int x = -2; x <= 2; x++) {
			vec2 offset = vec2(float(x), float(y)) * pixelSize;
			result += texture(tex0, (vTexcoord + offset)).rgb * kernel[index];
			index++;
		}
	}

	fragColor = vec4(result, 1.0);
}
