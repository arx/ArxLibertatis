
#ifndef ARX_GRAPHICS_DIRECT3D_DX7TEXTURESTAGE_H
#define ARX_GRAPHICS_DIRECT3D_DX7TEXTURESTAGE_H

#include <d3d.h>

#include "graphics/texture/TextureStage.h"

class DX7TextureStage : public TextureStage {
	
public:
	
	DX7TextureStage(unsigned int textureStage);
	
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

extern D3DTEXTUREMAGFILTER ARXToDX7MagFilter[];
extern D3DTEXTUREMINFILTER ARXToDX7MinFilter[];
extern D3DTEXTUREMIPFILTER ARXToDX7MipFilter[];

#endif // ARX_GRAPHICS_DIRECT3D_DX7TEXTURESTAGE_H
