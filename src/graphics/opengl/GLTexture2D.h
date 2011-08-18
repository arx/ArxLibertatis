
#ifndef ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H
#define ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H

#include "graphics/opengl/OpenGLUtil.h"

#include "graphics/texture/Texture.h"
#include "graphics/texture/TextureStage.h"

class GLTextureStage;

class GLTexture2D : public Texture2D {
	
public:
	
	GLTexture2D();
	~GLTexture2D();
	
	bool Create();
	void Upload();
	void Destroy();
	
	void apply();
	inline void link(GLTextureStage * _stage) { stage = _stage; }
	inline GLTextureStage * getStage() { return stage; }
	
private:
	
	GLTextureStage * stage;
	
	GLuint tex;
	
	TextureStage::WrapMode wrapMode;
	TextureStage::FilterMode minFilter;
	TextureStage::FilterMode magFilter;
	TextureStage::FilterMode mipFilter;
	
};

#endif // ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H
