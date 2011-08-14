
#include "graphics/opengl/GLTexture2D.h"

GLTexture2D::GLTexture2D() : tex(GL_NONE) { }
GLTexture2D::~GLTexture2D() { }

bool GLTexture2D::Create() {
	
	glGenTextures(1, &tex);
	
	// TODO color keying
	// TODO mipmapping
	
	CHECK_GL;
	
	return (tex != GL_NONE);
}

void GLTexture2D::Upload() {
	
	arx_assert(tex != GL_NONE);
	
	glBindTexture(GL_TEXTURE_2D, tex);
	
	GLint internal;
	GLenum format;
	if(mFormat == Image::Format_L8) {
		internal = GL_LUMINANCE8, format = GL_LUMINANCE;
	} else if(mFormat == Image::Format_A8) {
		internal = GL_ALPHA8, format = GL_ALPHA;
	} else if(mFormat == Image::Format_L8A8) {
		internal = GL_LUMINANCE8_ALPHA8, format = GL_LUMINANCE_ALPHA;
	} else if(mFormat == Image::Format_R8G8B8) {
		internal = GL_RGB8, format = GL_RGB;
	} else if(mFormat == Image::Format_B8G8R8) {
		internal = GL_RGB8, format = GL_BGR;
	} else if(mFormat == Image::Format_R8G8B8A8) {
		internal = GL_RGBA8, format = GL_RGBA;
	} else if(mFormat == Image::Format_B8G8R8A8) {
		internal = GL_RGBA8, format = GL_BGRA;
	} else {
		arx_assert_msg(false, "Unsupported image format");
	}
	
	glTexImage2D(GL_TEXTURE_2D, 0, internal, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, mImage.GetData());
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	CHECK_GL;
}

void GLTexture2D::Destroy() {
	
	LogInfo << "destroying " << mFileName;
	
	if(tex) {
		glDeleteTextures(1, &tex);
		CHECK_GL;
	}
}
