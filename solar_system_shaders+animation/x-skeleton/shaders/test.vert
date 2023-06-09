#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texCoord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform vec3 cameraPos;
uniform float specular;
uniform float diffuse;
uniform float ambient;

out vec3 fragPos;
out vec3 fragColor;
out vec3 n;
out vec2 tc;
out float ambientStrength;
out float diffuseStrength;
out float specularStrength;
out vec3 viewPos;

void main() {
	ambientStrength = ambient;
	diffuseStrength = diffuse;
	specularStrength = specular;
	tc = texCoord;
	fragPos = vec3(M* vec4(pos,1.0));
	fragColor = color;
	n = mat3(transpose(inverse(M))) * normal;
	viewPos = cameraPos;
	//n = normal;
	gl_Position = P * V * M * vec4(pos, 1.0);
}
