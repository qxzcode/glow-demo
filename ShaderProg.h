// ShaderProg.h
#pragma once
#include "stdafx.h"

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/mat4x4.hpp>

class ShaderProg {
public:
	ShaderProg(const std::string& vertSrc, const std::string& fragSrc, const std::initializer_list<std::string>& attribs);
	~ShaderProg() {
		glDeleteProgram(prog);
	}

	void use() {
		glUseProgram(prog);
	}

	void rebuild(const std::string& vertSrc, const std::string& fragSrc);

	int getAttribSize(GLuint index);
	void setAttribPointers();
	void enableAttribArrays();
	void disableAttribArrays();
	void setUniform(const std::string& name, bool b);
	void setUniform(const std::string& name, int i);
	void setUniform(const std::string& name, float f);
	void setUniform(const std::string& name, const glm::mat4& m);
	void setUniform(const std::string& name, const glm::vec2& m);
	void setUniform(const std::string& name, const glm::vec3& m);

	// non-copyable 'cuz I'm lazy
	ShaderProg(const ShaderProg& x) = delete;
	ShaderProg& operator=(const ShaderProg& x) = delete;

	GLuint prog;
	GLint numAttribs;
	std::vector<GLint> attribLocs;
	std::vector<int> attribSizes;
	int totalAttribSize;
	std::vector<std::string> attribs;
};