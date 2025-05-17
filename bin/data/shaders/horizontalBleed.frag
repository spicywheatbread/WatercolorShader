#version 330

uniform sampler2DRect colorImage;
uniform sampler2DRect depthImage;
uniform sampler2DRect controlImage;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 controlImageOutput;

const float gaussianKernel[21] = {0.0439582, 0.0450147, 0.0459815, 0.0468518, 0.0476193, 0.0482786, 0.0488248, 0.0492539, 0.0495627, 0.0497489, 0.0498111, 0.0497489, 0.0495627, 0.0492539, 0.0488248, 0.0482786, 0.0476193, 0.0468518, 0.0459815, 0.0450147, 0.0439582};

void main() {
	fragColor = vec4(0, 0, 0.5, 1);
	controlImageOutput = vec4(0, 0.5, 0, 1);
}
