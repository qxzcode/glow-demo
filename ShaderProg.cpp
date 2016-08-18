// ShaderProg.cpp
#include "ShaderProg.h"

#include <glm/gtc/type_ptr.hpp>
#include <vector>

static GLuint loadShader(GLenum type, std::string src) {
	GLuint sh = glCreateShader(type);
	const GLchar* const str = src.c_str();
	const GLint len = src.length();
	glShaderSource(sh, 1, &str, &len);
	glCompileShader(sh);
	GLint status;
	glGetShaderiv(sh, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLchar buf[512];
		GLsizei len;
		glGetShaderInfoLog(sh, sizeof(buf), &len, buf);
		cerr << "[compiling shader]:\n" << buf << endl;
		glDeleteShader(sh);
		return 0;
	}
	return sh;
}

ShaderProg::ShaderProg(const std::string& vertSrc, const std::string& fragSrc, const std::initializer_list<std::string>& attribs):prog(0),attribs(attribs) {
	rebuild(vertSrc, fragSrc);
}

void ShaderProg::rebuild(const std::string& vertSrc, const std::string& fragSrc) {
	// destroy any existing program
	if (prog) glDeleteProgram(prog);

	// create a new program object
	prog = glCreateProgram();

	// compile and attach the shaders
	GLuint vsh = loadShader(GL_VERTEX_SHADER, vertSrc);
	GLuint fsh = loadShader(GL_FRAGMENT_SHADER, fragSrc);
	glAttachShader(prog, vsh);
	glAttachShader(prog, fsh);
	glDeleteShader(vsh);
	glDeleteShader(fsh);

	// link the program and check for errors
	glLinkProgram(prog);
	GLint status;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLchar buf[512];
		glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
		cerr << "[linking shader]:\n" << buf << endl;
		glDeleteProgram(prog);
		prog = 0;
	}

	// get some program information
	glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTES, &numAttribs);
	totalAttribSize = 0;
	attribLocs.clear();
	attribSizes.clear();
	for (std::string attribName : attribs) {
		int loc = glGetAttribLocation(prog, attribName.c_str());
		attribLocs.push_back(loc);
		int size = getAttribSize(loc);
		attribSizes.push_back(size);
		totalAttribSize += size;
		//cout << "size of "<<attribName<<" ["<<loc<<"] = "<<size<<endl;
	}
	
	if (numAttribs != attribs.size())
		cout << "WARNING: numAttribs ("<<numAttribs<<") != attribs.size() ("<<attribs.size()<<")";
}

int ShaderProg::getAttribSize(GLuint index) {
	GLchar name[32];
	GLint size;
	GLenum type;
	glGetActiveAttrib(prog, index, sizeof(name), nullptr, &size, &type, name);
	switch (type) {
	case GL_FLOAT:
	case GL_DOUBLE:
		return size;
	case GL_FLOAT_VEC2:
	case GL_DOUBLE_VEC2:
		return 2*size;
	case GL_FLOAT_VEC3:
	case GL_DOUBLE_VEC3:
		return 3*size;
	case GL_FLOAT_VEC4:
	case GL_DOUBLE_VEC4:
		return 4*size;
	case GL_FLOAT_MAT2:
	case GL_DOUBLE_MAT2:
		return 2*2*size;
	case GL_FLOAT_MAT3:
	case GL_DOUBLE_MAT3:
		return 3*3*size;
	case GL_FLOAT_MAT4:
	case GL_DOUBLE_MAT4:
		return 4*4*size;
	default:
		return -1;
	}
}

void ShaderProg::setAttribPointers() {
	int offset = 0;
	for (int i = 0; i < numAttribs; i++) {
		glVertexAttribPointer(attribLocs[i], attribSizes[i], GL_FLOAT, false, sizeof(GLfloat)*totalAttribSize, (void*)(sizeof(GLfloat)*offset));
		offset += attribSizes[i];
	}
}

void ShaderProg::enableAttribArrays() {
	for (int i = 0; i < numAttribs; i++) {
		glEnableVertexAttribArray(attribLocs[i]);
	}
}
void ShaderProg::disableAttribArrays() {
	for (int i = 0; i < numAttribs; i++) {
		glDisableVertexAttribArray(attribLocs[i]);
	}
}

void ShaderProg::setUniform(const std::string& name, bool b) {
	use();
	glUniform1i(glGetUniformLocation(prog, name.c_str()), b);
}
void ShaderProg::setUniform(const std::string& name, int i) {
	use();
	glUniform1i(glGetUniformLocation(prog, name.c_str()), i);
}
void ShaderProg::setUniform(const std::string& name, float f) {
	use();
	glUniform1f(glGetUniformLocation(prog, name.c_str()), f);
}
void ShaderProg::setUniform(const std::string& name, const glm::mat4& m) {
	use();
	glUniformMatrix4fv(glGetUniformLocation(prog, name.c_str()), 1, GL_FALSE, glm::value_ptr(m));
}
void ShaderProg::setUniform(const std::string& name, const glm::vec2& m) {
	use();
	glUniform2f(glGetUniformLocation(prog, name.c_str()), m.x, m.y);
}
void ShaderProg::setUniform(const std::string& name, const glm::vec3& m) {
	use();
	glUniform3f(glGetUniformLocation(prog, name.c_str()), m.x, m.y, m.z);
}
