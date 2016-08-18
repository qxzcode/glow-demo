#version 140
#define blurAmount (@)

uniform sampler2D tex;
uniform float gaussian[blurAmount];
uniform float alpha;
uniform bool doBlur;

varying vec2 vTexCoords[blurAmount];

void main() {
	if (doBlur) {
		@
			gl_FragColor += texture(tex, vTexCoords[_i_]) * gaussian[_i_];
		@
	} else {
		gl_FragColor = texture(tex, vTexCoords[0]);
		gl_FragColor.a *= alpha;
	}
}
