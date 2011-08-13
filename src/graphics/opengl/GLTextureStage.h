
#ifndef ARX_GRAPHICS_OPENGL_GLTEXTURESTAGE_H
#define ARX_GRAPHICS_OPENGL_GLTEXTURESTAGE_H

#include "graphics/texture/TextureStage.h"

class GLTextureStage : public TextureStage {
	
public:
	
	GLTextureStage(unsigned textureStage);
	
	void SetTexture(Texture * pTexture);
	void ResetTexture();
	
	void SetColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2);
	void SetColorOp(TextureOp textureOp);
	void SetAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2);
	void SetAlphaOp(TextureOp textureOp);
	
	void SetWrapMode(WrapMode wrapMode);
	
	void SetMinFilter(FilterMode filterMode);
	void SetMagFilter(FilterMode filterMode);
	void SetMipFilter(FilterMode filterMode);
	
	void SetMipMapLODBias(float bias);
	void SetTextureCoordIndex(int texCoordIdx);
	
};

#endif // ARX_GRAPHICS_OPENGL_GLTEXTURESTAGE_H
