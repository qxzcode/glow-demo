// main.cpp
#include "stdafx.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
using std::min;
using std::max;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ShaderProg.h"
#include "Chunk.h"
#include "geom.h"
#include "RTT.h"

#include "shader_sources.h"


GLFWwindow* win;

Chunk* test = nullptr;
ShaderProg* worldShader = nullptr;
ShaderProg* blurShader = nullptr;
RTT* glowMapMS = nullptr;
RTT* glowTex1 = nullptr;
RTT* glowTex2 = nullptr;
VAO* fullVAO = nullptr;
constexpr int GLOW_TEX_SCALE = 15;
constexpr int GLOW_SIZE = 15;
constexpr int GLOW_MULTISAMPLES = 8;
int blurAmount = 1, maxBlurAmount;

float gaussian(float x, float s) {
	return float( ( 1.0 / sqrt(2*M_PI*s) ) * exp( -(x*x)/(2*s) ) );
}
constexpr float blurScale = 1.0;
constexpr float blurStrength = 1.0;
void calcGaussian() {
	float halfBlur = blurAmount * 0.5f;
	float deviation = halfBlur * 0.35f;
	deviation *= deviation;

	float* gaussArr = new float[blurAmount];
	float mSum = 0.0;
	for (int i=0; i<blurAmount; i++) {
		float off = (i-halfBlur) * blurScale;
		mSum += gaussArr[i] = gaussian(off*blurStrength, deviation);
	}
	for (int i=0; i<blurAmount; i++) {
		gaussArr[i] /= mSum*0.90f;
	}
	glUniform1fv(glGetUniformLocation(blurShader->prog,"gaussian"), blurAmount, gaussArr);
	delete[] gaussArr;
}
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	// source:
	// http://stackoverflow.com/a/17620909/1848578
	if(from.empty())
		return;
	std::string wsRet;
	wsRet.reserve(str.length());
	size_t start_pos = 0, pos;
	while((pos = str.find(from, start_pos)) != std::string::npos) {
		wsRet += str.substr(start_pos, pos - start_pos);
		wsRet += to;
		pos += from.length();
		start_pos = pos;
	}
	wsRet += str.substr(start_pos);
	str.swap(wsRet); // faster than str = wsRet;
}
std::string blurSrc(std::string src) {
	size_t i1 = src.find('@');
	std::string src2 = src.substr(0, i1);
	src2 += std::to_string(blurAmount);

	size_t i2 = i1;
	while ((i1 = src.find('@', i2+1)) != -1) {
		src2 += src.substr(i2+1, i1-i2-1);
		i2 = src.find('@', i1+1);
		std::string toLoop = src.substr(i1+1, i2-i1-1);
		for (int n=0; n<blurAmount; n++) {
			std::string curLoop = toLoop;
			replaceAll(curLoop, "_i_", std::to_string(n));
			if (n==0) replaceAll(curLoop, "+=", "=");
			src2 += curLoop;
		}
	}
	src2 += src.substr(i2+1);

	//cout<<"--------\n"<<src2<<"--------\n";
	return src2;
}
void setBlurAmount(int gw, int gh) {
	blurAmount = (gw+gh)/GLOW_SIZE;
	if (blurAmount < 1) blurAmount = 1;
	if (blurAmount > maxBlurAmount) blurAmount = maxBlurAmount;

	blurShader->rebuild(blurSrc(blur_vert), blurSrc(blur_frag));
	blurShader->use();
	cout << "rebuilt blur shader\n";
	calcGaussian();
	blurShader->setUniform("tex", 0);
	blurShader->setUniform("blurScale", blurScale);
	blurShader->setUniform("texelSize", glm::vec2{1.0f/gw,1.0f/gh});
}
void init() {
	cout << "Init" << endl;
	GLint val;
	glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &val);
	cout << "max vertex shader outs: " << val << endl;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

	worldShader = new ShaderProg(world_vert, world_frag, {"position","aColor","aNormal","aTexCoords"});
	worldShader->setUniform("tex", 0);
	glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &maxBlurAmount);
	maxBlurAmount = (maxBlurAmount-4)/2;
	cout << "maxBlurAmount = " << maxBlurAmount << endl;
	blurShader = new ShaderProg(blurSrc(blur_vert), blurSrc(blur_frag), {"position"});

	test = new Chunk(0, 0, worldShader);
	test->generate();
	test->buildVAO();

	// these get resized on window resize
	GLint ms;
	glGetIntegerv(GL_MAX_SAMPLES, &ms);
	if (ms > GLOW_MULTISAMPLES) ms = GLOW_MULTISAMPLES;
	glowMapMS = new RTT(1,1, true, false, ms);
	glowTex1 = new RTT(1,1, false, false);
	glowTex2 = new RTT(1,1, false, false);
	fullVAO = new VAO(GL_TRIANGLE_FAN, blurShader);
	fullVAO->setData({ -1,-1, 1,-1, 1,1, -1,1 });
	
	//...
}

double lastMX, lastMY;
float mouseDX() {
	double curMX;
	glfwGetCursorPos(win, &curMX, nullptr);
	double dx = curMX - lastMX;
	lastMX = curMX;
	return float(dx);
}
float mouseDY() {
	double curMY;
	glfwGetCursorPos(win, nullptr, &curMY);
	double dy = curMY - lastMY;
	lastMY = curMY;
	return float(dy);
}

glm::mat4 mvMat(1), viewMat, projMat;
glm::vec3 playerPos{Chunk::SIZE/2, 5, Chunk::SIZE/2};
float camY = 0, camP = 0;
int debugMode = 0;
void draw(float t, float dt) {
	// update game state
	glm::vec3 mv;
	float amt = 10.0f * dt;
	if (glfwGetKey(win,GLFW_KEY_W)) mv.z -= amt;
	if (glfwGetKey(win,GLFW_KEY_S)) mv.z += amt;
	if (glfwGetKey(win,GLFW_KEY_D)) mv.x += amt;
	if (glfwGetKey(win,GLFW_KEY_A)) mv.x -= amt;
	if (glfwGetKey(win,GLFW_KEY_SPACE)) mv.y += amt;
	if (glfwGetKey(win,GLFW_KEY_LEFT_SHIFT)) mv.y -= amt;
	playerPos += glm::mat3(glm::rotate(glm::mat4(1), camY, {0,1,0}))*mv;
	camY -= mouseDX()/600;
	camP -= mouseDY()/600;
	if (camP<-M_PI_2) camP = float(-M_PI_2);
	if (camP>M_PI_2) camP = float(M_PI_2);
	
	// set transform matrices
	viewMat = glm::mat4(1);
	viewMat = glm::rotate(viewMat, -camP, {1,0,0});
	viewMat = glm::rotate(viewMat, -camY, {0,1,0});
	viewMat = glm::translate(viewMat, -playerPos);
	worldShader->setUniform("worldMat", mvMat);
	worldShader->setUniform("mvpMat", projMat * viewMat * mvMat);
	worldShader->setUniform("useTex", false);
	glm::mat4 lightM = glm::mat4(1);//glm::rotate(glm::mat4(1), t, {1,1,1});
	worldShader->setUniform("lightDir", glm::normalize(glm::vec3(lightM*glm::vec4{1,3,2,0})));

	// draw glow scene to glowMapMS
	glClearColor(0, 0, 0, 1);
	glDisable(GL_BLEND);
	glowMapMS->startRender();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColorMask(false,false,false,false);
	test->draw();
	glColorMask(true,true,true,true);
	worldShader->setUniform("useLighting", false);
	test->draw2();
	glowMapMS->stopRender();
	
	// blit glowMapMS onto glowTex1, resolving antialiasing
	glBindFramebuffer(GL_READ_FRAMEBUFFER, glowMapMS->framebuffer);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glowTex1->framebuffer);
	glBlitFramebuffer(0,0,glowMapMS->width,glowMapMS->height, 0,0,glowMapMS->width,glowMapMS->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render glowTex1 onto glowTex2, blurring in one direction
	glowTex2->startRender();
	glClear(GL_COLOR_BUFFER_BIT);
	blurShader->use();
	blurShader->setUniform("alpha", 1.0f);
	blurShader->setUniform("doBlur", debugMode!=1);
	blurShader->setUniform("direction", false);
	glowTex1->bindTex();
	fullVAO->draw();
	glowTex2->stopRender();

	// render glowTex2 back onto glowTex1, blurring in the other direction
	glowTex1->startRender();
	glClear(GL_COLOR_BUFFER_BIT);
	blurShader->setUniform("direction", true);
	glowTex2->bindTex();
	fullVAO->draw();
	glowTex1->stopRender();

	// render the normal scene onto the screen
	glClearColor(0, 0, 0.2f, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	worldShader->use();
	worldShader->setUniform("useLighting", true);
	test->draw2();
	test->draw();
	
	// render glowTex1 onto the screen, blending the glow effect in
	blurShader->use();
	blurShader->setUniform("doBlur", false);
	blurShader->setUniform("alpha", 1);// - 0.7f*(sinf(t*1.5f)/2+0.5f));
	glowTex1->bindTex();
	glBlendFunc(GL_SRC_ALPHA, true?GL_ONE:GL_ONE_MINUS_SRC_COLOR);
	if (debugMode==0) glEnable(GL_BLEND);
	if (debugMode!=3) fullVAO->draw();

}

void destroy() {
	delete test;
	delete worldShader;
	delete blurShader;
	delete glowMapMS;
	delete glowTex1;
	delete glowTex2;
}

int main() {
	cout << "Program started" << endl;

	// init GLFW
	glfwSetErrorCallback([] (int err, const char* desc) {
		cerr << "GLFW error " << err << ": " << desc << endl;
	});
	if (!glfwInit()) return -1;

	// create the window
	constexpr float ASPECT_RATIO = 3.0/2.0;
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int wByW = int(vidmode->width*0.75);
	int wByH = int(vidmode->height*0.75 * ASPECT_RATIO);
	int winWidth = min(wByW, wByH);
	int winHeight = int(winWidth/ASPECT_RATIO);
	win = glfwCreateWindow(winWidth, winHeight, "OpenGL Game", nullptr, nullptr);
	if (!win) {
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(win, (vidmode->width-winWidth)/2, (vidmode->height-winHeight)/2);
	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);
	glewExperimental = GL_TRUE;
	glewInit();

	// set event callbacks
	glfwSetFramebufferSizeCallback(win, [] (GLFWwindow* win, int w, int h) {
		if (w>0 && h>0) {
			cout << "Window resized to " << w << "x" << h << endl;
			glViewport(0, 0, w, h);
			projMat = glm::perspective(glm::radians(45.0f), float(w)/h, 0.1f, 1000.0f);
			int gw = w/GLOW_TEX_SCALE, gh = h/GLOW_TEX_SCALE;
			glowMapMS->resize(gw, gh);
			glowTex1->resize(gw, gh);
			glowTex2->resize(gw, gh);
			setBlurAmount(gw, gh);
			cout << "blurAmount = " << blurAmount << endl;
		}
	});
	glfwSetKeyCallback(win, [] (GLFWwindow* win, int key, int scancode, int action, int mods) {
		if (action == GLFW_RELEASE) {
			switch (key) {
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(win, GLFW_TRUE);
				break;
			case GLFW_KEY_X:
				glfwSetInputMode(win, GLFW_CURSOR, glfwGetInputMode(win,GLFW_CURSOR)==GLFW_CURSOR_DISABLED? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
				break;
			case GLFW_KEY_Z:
				debugMode = (debugMode+1)%4;
				break;
			}
		}
	});

	// init resources, etc.
	init();

	// show the window
	glfwShowWindow(win);

	// main loop
	double lastFrame = glfwGetTime() - 1.0/60.0;
	double lastPrint = lastFrame;
	double flAvg = 0;
	int flCount = 0;
	glfwGetCursorPos(win, &lastMX, &lastMY);
	while (!glfwWindowShouldClose(win)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		double curT = glfwGetTime();
		double dt = curT - lastFrame;
		lastFrame = curT;
		draw(float(curT), float(dt));
		double frameLen = dt;//glfwGetTime() - curT;
		flAvg = (flAvg*flCount + frameLen) / (flCount+1);
		flCount++;
		lastPrint += dt;
		if (lastPrint > 1.0) {
			cout << int(flAvg*1000+0.5) << " ms; " << int(1/flAvg+0.5) << " FPS (over " << flCount << " frames)" << endl;
			lastPrint = 0.0;
			flCount = 0;
		}

		glfwSwapBuffers(win);
		glfwPollEvents();
	}

	destroy();
	glfwTerminate();
	return 0;
}
