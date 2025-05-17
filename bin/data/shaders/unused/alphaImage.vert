#version 330

uniform mat4 modelViewProjectionMatrix;

in vec4 color;
in vec4 position;

out vec4 vColor;

void main() {
	gl_Position = modelViewProjectionMatrix * position;
	vColor = color;
}
