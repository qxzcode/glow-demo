// VAO.h
#pragma once

#include <GL/glew.h>
#include <vector>

#include "ShaderProg.h"

class VAO {
public:
	VAO(GLenum mode, ShaderProg* sp = nullptr):mode(mode) {
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		setShader(sp);
	}
	void setShader(ShaderProg* sp) {
		shader = sp;
		if (shader) {
			bindVAO();
			bindVBO();
			shader->setAttribPointers();
		}
	}

	~VAO() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}

	void bindVAO() {
		glBindVertexArray(vao);
	}
	void bindVBO() {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}

	void setData(const GLfloat* buf, size_t num) {
		bindVBO();
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*num, buf, GL_STATIC_DRAW);
		if (shader) count = num / shader->totalAttribSize;
	}
	void setData(const std::vector<GLfloat>& buf) {
		setData(buf.data(), buf.size());
	}

	void draw(int count, bool enableArrays = true) {
		this->count = count;
		draw(enableArrays);
	}
	void draw(bool enableArrays = true) {
		bindVAO();
		bindVBO();

		if (shader && enableArrays) shader->enableAttribArrays();
		glDrawArrays(mode, 0, count);
		if (shader && enableArrays) shader->disableAttribArrays();
	}

	// non-copyable 'cuz I'm lazy
	VAO(const VAO& x) = delete;
	VAO& operator=(const VAO& x) = delete;

	GLuint vao, vbo;
	GLenum mode;
	GLsizei count = 0;
	ShaderProg* shader;
};