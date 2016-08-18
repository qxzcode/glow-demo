#version 140
#define blurAmount (@)

uniform bool direction;
uniform vec2 texelSize;
uniform float blurScale;
uniform bool doBlur;

attribute vec2 position;

varying vec2 vTexCoords[blurAmount];

void main() {
	vec2 tc = position*0.5 + 0.5;

	if (doBlur) {
		vec2 off = direction? vec2(0,blurScale*texelSize.y) : vec2(blurScale*texelSize.x,0);
		const float halfBlur = blurAmount * 0.5;
		for (int i=0; i<blurAmount; i++) {
			vTexCoords[i] = tc + (i-halfBlur)*off;
		}
	} else {
		vTexCoords[0] = tc;
	}

	gl_Position = vec4(position, 0.0, 1.0);
}
