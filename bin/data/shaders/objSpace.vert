#version 330

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;

in vec4 position;
in vec4 normal;
in vec4 color;
in vec2 texcoord;

out vec4 vPosition;
out vec2 vTexcoord;
out vec4 vNormal;
out vec4 vColor;

uniform float time; // Global time counter
uniform float speed; // Speed that noise updates
uniform float freq; // Sin wave frequency
uniform float tremor; // Sin wave amplitude
uniform float pixelSize; // Rough pixel size estimate to scale hand tremor
uniform vec3 cameraPos;

void main() {
	// unused; edgeFactor can be used to modululate whether the noise only occurs at object edges if desired.
	vec4 worldPos = modelViewMatrix * position;
    vec3 viewDir = normalize(cameraPos - worldPos.xyz);
    float edgeFactor = 1.0 - abs(dot(normal.xyz, viewDir)); // Will be 0 if the normal and view are perpendicular

	// Calculate "hand tremors" using sin noise
    vec4 projPos = modelViewProjectionMatrix * position;
    float noise = sin(time * speed + dot(projPos.xyz, vec3(1)));
	float offset = noise * tremor * pixelSize;

	gl_Position = projPos - (normal * offset); // Offset position using normal and calculated noise offset.
	vPosition = position;
    vTexcoord = texcoord;
	vNormal = normal;
	vColor = color;
}
