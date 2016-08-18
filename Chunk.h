#pragma once

#include "VAO.h"

class Chunk {
public:
	static constexpr int SIZE = 128;

	Chunk(int cx, int cy, ShaderProg* sp):cx(cx),cy(cy),vao(GL_TRIANGLES, sp), vao2(GL_TRIANGLES, sp) {

	}

	void generate();
	void buildVAO();

	void draw() {
		vao.draw();
	}
	void draw2() {
		vao2.draw();
	}

	int cx, cy;
	float hmap[SIZE][SIZE];
	VAO vao, vao2;
};