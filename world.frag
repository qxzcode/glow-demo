#version 140

uniform sampler2D tex;
uniform bool useTex;

varying vec2 vTexCoords;
varying vec3 vColor;

void main() {
	vec4 texColor = useTex? texture(tex, vTexCoords) : vec4(1, 1, 1, 1.0);
	gl_FragColor = texColor * vec4(vColor, 1.0);
}
