#version 330

uniform sampler2DRect colorImage;
uniform sampler2DRect depthImage;
uniform sampler2DRect controlImage;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 controlImageOutput;

in vec2 vTexcoord;

const float gaussianKernel[21] = float[21](0.0439582, 0.0450147, 0.0459815, 0.0468518, 0.0476193, 0.0482786, 0.0488248, 0.0492539, 0.0495627, 0.0497489, 0.0498111, 0.0497489, 0.0495627, 0.0492539, 0.0488248, 0.0482786, 0.0476193, 0.0468518, 0.0459815, 0.0450147, 0.0439582);
const float depthThreshold = 0.001;

// Reminders:
// ControlImage.b -> Controls color bleeding
// FOR NOW, we're not going to account for normalized tex coords. May try to refactor later,
// but this is the most convenient at cost of performance.
void main() {
	fragColor = vec4(0, 0, 0, 1);
	for(int i = -10; i <= 10; i++) { // Range of vertices we will sample for the bleed
		vec2 offsetTexcoord = vec2(vTexcoord.x, vTexcoord.y + i);
		if(texture(controlImage, vTexcoord).b > 0 || texture(controlImage, offsetTexcoord).b > 0) { // Control mask blue channel indicates if we allow bleeding
			bool bleed = false;
			bool sourceBehind = false;
			if(texture(depthImage, vTexcoord).r - depthThreshold > texture(depthImage, offsetTexcoord).r) {
				sourceBehind = true;
				if(texture(controlImage, offsetTexcoord).b > 0) {
					bleed = true;
				}
			}
			if(texture(controlImage, vTexcoord).b > 0 && !sourceBehind) {
				bleed = true;
			}
			if(bleed) {
				fragColor = fragColor + texture(colorImage, offsetTexcoord) * gaussianKernel[i + 10];
				controlImageOutput = texture(controlImage, vTexcoord) + vec4(0, 0, 0.5, 1);
			} else {
				fragColor = fragColor + texture(colorImage, vTexcoord) * gaussianKernel[i + 10];
				controlImageOutput = texture(controlImage, vTexcoord);
			}
		} else {
			fragColor = fragColor + texture(colorImage, vTexcoord) * gaussianKernel[i + 10];
			controlImageOutput = texture(controlImage, vTexcoord);
		}
	}
}
