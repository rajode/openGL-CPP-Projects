#version 330 core

in vec3 fragPos;
in vec3 fragColor;
in vec3 n;
in vec2 tc;
in float ambientStrength;
in float diffuseStrength;
in float specularStrength;
in vec3 viewPos;

uniform sampler2D sampler;
uniform vec3 lightPos = vec3(0,0,0);
uniform vec3 ambientLightColor = vec3(1,1,1);
uniform vec3 lightColor = vec3(1,1,1);

out vec4 color;

void main() {
	//color = vec4(fragColor, 1.0);
	vec4 d = texture(sampler, tc);

	vec3 ambient = ambientStrength * ambientLightColor;
	
	vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(n, lightDir), 0);
	vec3 diffuse = diffuseStrength * diff * lightColor;

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, n);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	vec4 res = vec4((ambient+diffuse+specular),1);
	color = res * d;
}
