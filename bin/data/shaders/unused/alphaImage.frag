#version 330

in vec4 vColor;

out vec4 fColor;

void main() {
	fColor = vec4(vColor.a, vColor.a, vColor.a, 1);
}
