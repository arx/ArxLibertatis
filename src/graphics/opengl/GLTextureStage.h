
#ifndef ARX_GRAPHICS_OPENGL_GLTEXTURESTAGE_H
#define ARX_GRAPHICS_OPENGL_GLTEXTURESTAGE_H

#include "graphics/opengl/OpenGLUtil.h"
#include "graphics/texture/TextureStage.h"

class OpenGLRenderer;
class GLTexture2D;

class GLTextureStage : public TextureStage {
	
public:
	
	GLTextureStage(OpenGLRenderer * renderer, unsigned textureStage);
	~GLTextureStage();
	
	void SetTexture(Texture * pTexture);
	void ResetTexture();
	
	void SetColorOp(TextureOp textureOp, TextureArg arg0, TextureArg arg1);
	void SetColorOp(TextureOp textureOp);
	void SetAlphaOp(TextureOp textureOp, TextureArg arg0, TextureArg arg1);
	void SetAlphaOp(TextureOp textureOp);
	
	void SetWrapMode(WrapMode wrapMode);
	
	void SetMinFilter(FilterMode filterMode);
	void SetMagFilter(FilterMode filterMode);
	void SetMipFilter(FilterMode filterMode);
	
	void SetMipMapLODBias(float bias);
	void SetTextureCoordIndex(int texCoordIdx);
	
	void apply();
	
private:
	
	OpenGLRenderer * renderer;
	
	enum OpType {
		Color,
		Alpha
	};
	
	enum Arg {
		Arg0,
		Arg1
	};
	
	TextureArg args[2][2];
	
	void setArg(OpType alpha, Arg idx, TextureArg arg);
	
	void setOp(OpType alpha, GLenum op, GLint scale);
	void setOp(OpType alpha, TextureOp op);
	void setOp(OpType alpha, TextureOp op, TextureArg arg0, TextureArg arg1);
	
	GLTexture2D * tex;
	GLTexture2D * current;
	
	WrapMode wrapMode;
	FilterMode minFilter;
	FilterMode magFilter;
	FilterMode mipFilter;
	
	friend class GLTexture2D;
};

#endif // ARX_GRAPHICS_OPENGL_GLTEXTURESTAGE_H
