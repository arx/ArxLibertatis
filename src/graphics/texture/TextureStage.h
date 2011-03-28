#ifndef _TEXTURE_STAGE_H_
#define _TEXTURE_STAGE_H_


#include "graphics/texture/Texture.h"


class TextureStage
{
public:
	// Texture blending operations
	enum TextureOp
	{
		OpDisable,		// Disables output from this texture stage and all stages with a higher index.
		OpSelectArg1,	// Use this texture stage's first color or alpha argument, unmodified, as the output.
		OpSelectArg2,	// Use this texture stage's second color or alpha argument, unmodified, as the output.
		OpModulate,		// Multiply the components of the arguments together.
		OpModulate2X,	// Multiply the components of the arguments, and shift the products to the left 1 bit.
		OpModulate4X,	// Multiply the components of the arguments, and shift the products to the left 2 bits.
		OpAddSigned		// Add args with -0.5 bias
	};

	// Texture blending arguments
	enum TextureArg
	{
		ArgDiffuse		= 0x00000,
		ArgCurrent		= 0x00001,
		ArgTexture		= 0x00002,
		ArgMask			= 0x0000F,
		ArgComplement   = 0x00010		
	};

	enum WrapMode
    {
        WrapRepeat,
        WrapMirror,
		WrapClamp
	};
	
	TextureStage(unsigned int stage);
	
	virtual void SetTexture( Texture& pTexture ) = 0;
	virtual void ResetTexture() = 0;

	virtual void SetColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2) = 0;
	virtual void SetColorOp(TextureOp textureOp) = 0;
	inline void SetColorOp(TextureArg texArg);
	inline void DisableColor();

	virtual void SetAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2) = 0;
	virtual void SetAlphaOp(TextureOp textureOp) = 0;
	inline void SetAlphaOp(TextureArg texArg);
	inline void DisableAlpha();

	virtual void SetWrapMode(WrapMode wrapMode) = 0;
	
protected:
	unsigned int mStage;
};


inline void TextureStage::SetColorOp(TextureArg texArg)
{
	SetColorOp(OpSelectArg1, texArg, ArgCurrent);
}

inline void TextureStage::DisableColor()
{
	SetColorOp(OpDisable, ArgCurrent, ArgCurrent);
}

inline void TextureStage::SetAlphaOp(TextureArg texArg)
{
	SetAlphaOp(OpSelectArg1, texArg, ArgCurrent);
}

inline void TextureStage::DisableAlpha()
{
	SetAlphaOp(OpDisable, ArgCurrent, ArgCurrent);
}


#endif // _TEXTURE_STAGE_H_
