// RTT.h
#pragma once

#include <GL/glew.h>

class RTT {
public:
	static void checkFramebuffer(bool silenceGood = false) {
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			cerr << "Bad framebuffer status: " << status;
			switch (status) {
			case GL_FRAMEBUFFER_UNDEFINED: cout << " (GL_FRAMEBUFFER_UNDEFINED)";break;
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: cout << " (GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)";break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: cout << " (GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)";break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: cout << " (GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)";break;
			}
			cerr << endl;
		} else if (!silenceGood) {
			cout << "Framebuffer status complete" << endl;
		}
	}

	RTT(GLsizei w, GLsizei h, bool depth, bool alpha = true, GLsizei multisamples = 0):width(w),height(h),alpha(alpha),multisamples(multisamples) {
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		if (depth) {
			glGenRenderbuffers(1, &renderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
			if (multisamples)
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisamples, GL_DEPTH_COMPONENT, w, h);
			else
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
		}

		glGenTextures(1, &tex);
		glBindTexture(multisamples?GL_TEXTURE_2D_MULTISAMPLE:GL_TEXTURE_2D, tex);
		if (multisamples)
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisamples, alpha?GL_RGBA:GL_RGB, w, h, true);
		else {
			glTexImage2D(GL_TEXTURE_2D, 0, alpha?GL_RGBA:GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		}
		if (multisamples)
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
		else
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0);

		GLenum bufs[] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, bufs);

		checkFramebuffer(true);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	~RTT() {
		glDeleteFramebuffers(1, &framebuffer);
		glDeleteTextures(1, &tex);
		glDeleteRenderbuffers(1, &renderbuffer);
	}

	void startRender() {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glGetIntegerv(GL_VIEWPORT, savedViewport);
		glViewport(0, 0, width, height);
	}
	void stopRender() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(savedViewport[0],savedViewport[1],savedViewport[2],savedViewport[3]);
	}
	void bindTex() {
		glBindTexture(GL_TEXTURE_2D, tex);
	}
	void resize(int w, int h) {
		width = w;
		height = h;
		glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
		if (multisamples)
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisamples, GL_DEPTH_COMPONENT, w, h);
		else
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
		glBindTexture(multisamples?GL_TEXTURE_2D_MULTISAMPLE:GL_TEXTURE_2D, tex);
		if (multisamples)
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisamples, alpha?GL_RGBA:GL_RGB, w, h, true);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, alpha?GL_RGBA:GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}

	// non-copyable 'cuz I'm lazy
	RTT(const RTT& x) = delete;
	RTT& operator=(const RTT& x) = delete;

	GLsizei width, height;
	bool alpha;
	GLsizei multisamples;
	GLuint framebuffer=0, renderbuffer=0, tex=0;
	GLint savedViewport[4];
};