#version 140

attribute vec3 position;
attribute vec3 aColor;
attribute vec3 aNormal;
attribute vec2 aTexCoords;

uniform mat4 worldMat;
uniform mat4 mvpMat;
uniform vec3 lightDir;
uniform bool useLighting;

varying vec2 vTexCoords;
varying vec3 vColor;

void main() {
	vTexCoords = aTexCoords;
	vec3 normal = mat3(worldMat) * aNormal;
	vColor = aColor;
	if (useLighting) {
		vColor *= dot(lightDir, normal)/2 + 0.5;
	}
	gl_Position = mvpMat * vec4(position, 1.0);
}
