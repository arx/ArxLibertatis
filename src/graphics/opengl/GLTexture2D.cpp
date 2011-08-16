
#include "graphics/opengl/GLTexture2D.h"

#include "graphics/opengl/GLTextureStage.h"

GLTexture2D::GLTexture2D() : stage(NULL), tex(GL_NONE) { }
GLTexture2D::~GLTexture2D() {
	Destroy();
}

bool GLTexture2D::Create() {
	
	arx_assert(stage == NULL);
	
	glGenTextures(1, &tex);
	
	// TODO color keying
	// TODO mipmapping
	
	// Set our state to the default OpenGL state
	wrapMode = TextureStage::WrapRepeat;
	mipFilter = TextureStage::FilterLinear;
	minFilter = TextureStage::FilterNearest;
	magFilter = TextureStage::FilterMode(-1); // TODO TextureStage::FilterLinear;
	
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
	
	CHECK_GL;
}

void GLTexture2D::Destroy() {
	
	LogInfo << "destroying " << mFileName;
	
	if(tex) {
		glDeleteTextures(1, &tex);
		CHECK_GL;
	}
	
	if(stage) {
		arx_assert(stage->tex == this);
		stage->tex = NULL, stage = NULL;
	}
}

static const GLint arxToGlWrapMode[] = {
	GL_REPEAT, // WrapRepeat,
	GL_MIRRORED_REPEAT, // WrapMirror
	GL_CLAMP // WrapClamp
};

static const GLint arxToGlFilter[][3] = {
	// Mipmap: FilterNone
	{
		-1, // FilterNone
		GL_NEAREST, // FilterNearest
		GL_LINEAR   // FilterLinear
	},
	// Mipmap: FilterNearest
	{
		-1, // FilterNone
		GL_NEAREST, // FilterNearest TODO should be GL_NEAREST_MIPMAP_NEAREST
		GL_LINEAR   // FilterLinear TODO should be GL_LINEAR_MIPMAP_NEAREST
	},
	// Mipmap: FilterLinear
	{
		-1, // FilterNone
		GL_NEAREST, // FilterNearest TODO should be GL_NEAREST_MIPMAP_LINEAR
		GL_LINEAR   // FilterLinear TODO should be GL_LINEAR_MIPMAP_LINEAR
	}
};

void GLTexture2D::apply() {
	
	arx_assert(stage != NULL);
	arx_assert(stage->tex == this);
	
	glBindTexture(GL_TEXTURE_2D, tex);
	
	if(stage->wrapMode != wrapMode) {
		GLint glwrap = arxToGlWrapMode[wrapMode = stage->wrapMode];
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glwrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glwrap);
	}
	
	if(stage->mipFilter != mipFilter || stage->minFilter != minFilter) {
		minFilter = stage->minFilter, mipFilter = stage->mipFilter;
		arx_assert(minFilter != TextureStage::FilterNone);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, arxToGlFilter[mipFilter][minFilter]);
	}
	
	if(stage->magFilter != magFilter) {
		arx_assert(stage->magFilter != TextureStage::FilterNone);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, arxToGlFilter[0][magFilter = stage->magFilter]);
	}
	
}
