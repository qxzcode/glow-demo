#version 140

uniform sampler2D tex;
uniform float alpha;

varying vec2 vTexCoords[15];
varying float weights[8];

const float dist[] = {
	/*0.195125494208,
	0.18881539691,
	0.171083282897,
	0.145152501388,
	0.115315650454,
	0.0857825123286,
	0.0597524956983,
	0.0389726661165*/
	0.0666666666667,
	0.0666666666667,
	0.0666666666667,
	0.0666666666667,
	0.0666666666667,
	0.0666666666667,
	0.0666666666667,
	0.0666666666667
};

void main() {
	gl_FragColor = vec4(0.0);
	gl_FragColor += texture(tex, vTexCoords[0])  * weights[7];
	gl_FragColor += texture(tex, vTexCoords[1])  * weights[6];
	gl_FragColor += texture(tex, vTexCoords[2])  * weights[5];
	gl_FragColor += texture(tex, vTexCoords[3])  * weights[4];
	gl_FragColor += texture(tex, vTexCoords[4])  * weights[3];
	gl_FragColor += texture(tex, vTexCoords[5])  * weights[2];
	gl_FragColor += texture(tex, vTexCoords[6])  * weights[1];
	gl_FragColor += texture(tex, vTexCoords[7])  * weights[0];
	gl_FragColor += texture(tex, vTexCoords[8])  * weights[1];
	gl_FragColor += texture(tex, vTexCoords[9])  * weights[2];
	gl_FragColor += texture(tex, vTexCoords[10]) * weights[3];
	gl_FragColor += texture(tex, vTexCoords[11]) * weights[4];
	gl_FragColor += texture(tex, vTexCoords[12]) * weights[5];
	gl_FragColor += texture(tex, vTexCoords[13]) * weights[6];
	gl_FragColor += texture(tex, vTexCoords[14]) * weights[7];
	gl_FragColor.a *= alpha;
}
