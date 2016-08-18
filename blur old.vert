#version 120

uniform bool direction;

attribute vec2 position;

varying vec2 vTexCoords[15];
varying float weights[8];

#define PI (3.141592653589793238462643383279)
float gaussian(float x, float s) {
	return ( 1.0 / sqrt(2*PI*s) ) * exp( -(x*x)/(2*s) );
}

void main() {
	vTexCoords[7] = position*0.5 + 0.5;
	float wSum = 0.0;
	for (int i=0; i<15; i++) {
		float off = (i-7)*0.006;
		vTexCoords[i] = vTexCoords[7] + (direction?vec2(0,off):vec2(off,0));
		if (i < 8) {
			weights[i] = gaussian(i*0.1, 0.5*0.35);
			wSum += weights[i];
			if (i!=0) wSum += weights[i];
		}
	}
	for (int i=0; i<8; i++) {
		weights[i] /= wSum;
	}
	gl_Position = vec4(position, 0.0, 1.0);
}
