#include "stdafx.h"
#include "Chunk.h"

#include "geom.h"

static inline float randf() {
	return float(rand())/RAND_MAX;
}

void Chunk::generate() {
	cout << "Generating chunk...";
	for (int x = 0; x < SIZE; x++) {
		for (int z = 0; z < SIZE; z++) {
			hmap[x][z] = randf() + sinf(x*0.2f)*sinf(z*0.2f)*4.0f;
		}
	}
	cout << " Done" << endl;
}

void Chunk::buildVAO() {
	cout << "Building VAO...";
	geom::floatVec buf;
	bool in2[SIZE][SIZE];
	glm::vec3 color = {0.35,0.2,0.4};
	for (int x = 0; x < SIZE; x++) {
		for (int z = 0; z < SIZE; z++) {
			if (in2[x][z] = rand()%9==0) continue;

			float h = hmap[x][z];
			geom::makeQuad(buf,color, {x,h,z},
			{x,h,z+1},
			{x+1,h,z+1},
			{x+1,h,z});
			float h2;
			if (x>0 && (h2=hmap[x-1][z])<h) { // -x
				geom::makeQuad(buf,color, {x,h,z},
				{x,h2,z},
				{x,h2,z+1},
				{x,h,z+1});
			}
			if (x<SIZE-1 && (h2=hmap[x+1][z])<h) { // +x
				geom::makeQuad(buf,color, {x+1,h,z},
				{x+1,h,z+1},
				{x+1,h2,z+1},
				{x+1,h2,z});
			}
			if (z>0 && (h2=hmap[x][z-1])<h) { // -z
				geom::makeQuad(buf,color, {x,h,z},
				{x+1,h,z},
				{x+1,h2,z},
				{x,h2,z});
			}
			if (z<SIZE-1 && (h2=hmap[x][z+1])<h) { // +z
				geom::makeQuad(buf,color, {x,h,z+1},
				{x,h2,z+1},
				{x+1,h2,z+1},
				{x+1,h,z+1});
			}
		}
	}
	vao.setData(buf);

	buf.clear();
	for (int x = 0; x < SIZE; x++) {
		for (int z = 0; z < SIZE; z++) {
			if (!in2[x][z]) continue;

			float a=randf(), b=1.0f;
			if (rand()%2) std::swap(a,b);
			int c = rand()%3;
			if (c==0) color = {a,b,0};
			if (c==1) color = {0,a,b};
			if (c==2) color = {a,0,b};

			float h = hmap[x][z];
			geom::makeQuad(buf,color, {x,h,z},
			{x,h,z+1},
			{x+1,h,z+1},
			{x+1,h,z});
			float h2;
			if (x>0 && (h2=hmap[x-1][z])<h) { // -x
				geom::makeQuad(buf,color, {x,h,z},
				{x,h2,z},
				{x,h2,z+1},
				{x,h,z+1});
			}
			if (x<SIZE-1 && (h2=hmap[x+1][z])<h) { // +x
				geom::makeQuad(buf,color, {x+1,h,z},
				{x+1,h,z+1},
				{x+1,h2,z+1},
				{x+1,h2,z});
			}
			if (z>0 && (h2=hmap[x][z-1])<h) { // -z
				geom::makeQuad(buf,color, {x,h,z},
				{x+1,h,z},
				{x+1,h2,z},
				{x,h2,z});
			}
			if (z<SIZE-1 && (h2=hmap[x][z+1])<h) { // +z
				geom::makeQuad(buf,color, {x,h,z+1},
				{x,h2,z+1},
				{x+1,h2,z+1},
				{x+1,h,z+1});
			}
		}
	}
	vao2.setData(buf);
	cout << " Done" << endl;
}