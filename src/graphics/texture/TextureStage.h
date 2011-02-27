#ifndef _TEXTURE_STAGE_H_
#define _TEXTURE_STAGE_H_


#include "graphics/texture/Texture.h"


class TextureStage
{
public:
	enum TextureOp
	{
		TexOpSelectArg1,
		TexOpSelectArg2,
		TexOpModulate
	};

	enum TextureArg
	{
		TexArgCurrent,
		TexArgDiffuse,
		TexArgTexture
	};
	
	TextureStage(unsigned int stage);
	
	virtual void SetTexture( Texture& pTexture ) = 0;
	virtual void ResetTexture() = 0;

	virtual void SetColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2) = 0;
	virtual void SetAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2) = 0;
	
protected:
	unsigned int mStage;
};


#endif // _TEXTURE_STAGE_H_
