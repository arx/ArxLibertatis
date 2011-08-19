
#ifndef ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H
#define ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H

#include "graphics/opengl/OpenGLUtil.h"

#include "graphics/texture/Texture.h"
#include "graphics/texture/TextureStage.h"

class OpenGLRenderer;
class GLTextureStage;

class GLTexture2D : public Texture2D {
	
public:
	
	GLTexture2D(OpenGLRenderer * renderer);
	~GLTexture2D();
	
	bool Create();
	void Upload();
	void Destroy();
	
	void apply(GLTextureStage * stage);
	
private:
	
	OpenGLRenderer * renderer;
	
	GLuint tex;
	
	TextureStage::WrapMode wrapMode;
	TextureStage::FilterMode minFilter;
	TextureStage::FilterMode magFilter;
	TextureStage::FilterMode mipFilter;
	
	friend class GLTextureStage;
};

#endif // ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H
