
#ifndef ARX_GRAPHICS_D3D7_D3D7TEXTURESTAGE_H
#define ARX_GRAPHICS_D3D7_D3D7TEXTURESTAGE_H

#include <d3d.h>

#include "graphics/texture/TextureStage.h"

class D3D7TextureStage : public TextureStage {
	
public:
	
	D3D7TextureStage(LPDIRECT3DDEVICE7 device, unsigned int textureStage);
	
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
	
private:
	
	LPDIRECT3DDEVICE7 device;
	
};

extern D3DTEXTUREMAGFILTER ARXToDX7MagFilter[];
extern D3DTEXTUREMINFILTER ARXToDX7MinFilter[];
extern D3DTEXTUREMIPFILTER ARXToDX7MipFilter[];

#endif // ARX_GRAPHICS_D3D7_D3D7TEXTURESTAGE_H
