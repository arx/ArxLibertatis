
#ifndef ARX_GRAPHICS_D3D9_D3D9TEXTURESTAGE_H
#define ARX_GRAPHICS_D3D9_D3D9TEXTURESTAGE_H

#include <d3d9.h>

#include "graphics/texture/TextureStage.h"

class D3D9TextureStage : public TextureStage {
	
public:
	
	D3D9TextureStage(unsigned int textureStage);
	
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
};

extern D3DTEXTUREFILTERTYPE ARXToDX9MagFilter[];
extern D3DTEXTUREFILTERTYPE ARXToDX9MinFilter[];
extern D3DTEXTUREFILTERTYPE ARXToDX9MipFilter[];

#endif // ARX_GRAPHICS_D3D9_D3D9TEXTURESTAGE_H
