
#include "graphics/d3d7/D3D7TextureStage.h"

#include "graphics/Math.h"
#include "graphics/d3d7/D3D7Texture2D.h"

extern LPDIRECT3DDEVICE7 GD3D7Device;

const D3DTEXTUREOP ARXToDX7TextureOp[] = {
	D3DTOP_DISABLE,    // OpDisable
	D3DTOP_SELECTARG1, // OpSelectArg1,
	D3DTOP_SELECTARG2, // OpSelectArg2,
	D3DTOP_MODULATE,   // OpModulate,
	D3DTOP_MODULATE2X, // OpModulate2X,
	D3DTOP_MODULATE4X, // OpModulate4X,
	D3DTOP_ADDSIGNED   // OpAddSigned
};

const DWORD ARXToDX7TextureArg[] = {
	D3DTA_DIFFUSE, // TexArgDiffuse,
	D3DTA_CURRENT, // TexArgCurrent,
	D3DTA_TEXTURE  // TexArgTexture,
};

D3DTEXTUREMAGFILTER ARXToDX7MagFilter[] = {
	D3DTFG_POINT, // FilterNone - Invalid for magnification
	D3DTFG_POINT, // FilterNearest,
	D3DTFG_LINEAR // FilterLinear
};

D3DTEXTUREMINFILTER ARXToDX7MinFilter[] = {
	D3DTFN_POINT,  // FilterNone - Invalid for minification
	D3DTFN_POINT,  // FilterNearest,
	D3DTFN_LINEAR, // FilterLinear
};

D3DTEXTUREMIPFILTER ARXToDX7MipFilter[] = {
	D3DTFP_NONE,  // FilterNone
	D3DTFP_POINT, // FilterNearest,
	D3DTFP_LINEAR // FilterLinear
};

const D3DTEXTUREADDRESS ARXToDX7WrapMode[] = {
	D3DTADDRESS_WRAP,   // WrapRepeat,
	D3DTADDRESS_MIRROR, // WrapMirror,
	D3DTADDRESS_CLAMP   // WrapClamp,
};

D3D7TextureStage::D3D7TextureStage(unsigned int textureStage) : TextureStage(textureStage) {
}

void D3D7TextureStage::SetColorOp(TextureOp textureOp)
{
	// TODO-DX7: Cache states
	DWORD colorOp = ARXToDX7TextureOp[textureOp];
	GD3D7Device->SetTextureStageState(mStage, D3DTSS_COLOROP, colorOp);
}

void D3D7TextureStage::SetColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2)
{
	// TODO-DX7: Cache states
	DWORD colorOp = ARXToDX7TextureOp[textureOp];
	GD3D7Device->SetTextureStageState(mStage, D3DTSS_COLOROP, colorOp);

	if(textureOp != TextureStage::OpDisable)
	{
		if(textureOp != TextureStage::OpSelectArg2)
		{
			DWORD colorArg1 = ARXToDX7TextureArg[texArg1 & TextureStage::ArgMask];
			colorArg1 |= (texArg1 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GD3D7Device->SetTextureStageState(mStage, D3DTSS_COLORARG1, colorArg1);
		}

		if(textureOp != TextureStage::OpSelectArg1)
		{
			DWORD colorArg2 = ARXToDX7TextureArg[texArg2 & TextureStage::ArgMask];
			colorArg2 |= (texArg2 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GD3D7Device->SetTextureStageState(mStage, D3DTSS_COLORARG2, colorArg2);
		}
	}
}

void D3D7TextureStage::SetAlphaOp(TextureOp textureOp)
{
	// TODO-DX7: Cache states
	DWORD colorOp = ARXToDX7TextureOp[textureOp];
	GD3D7Device->SetTextureStageState(mStage, D3DTSS_ALPHAOP, colorOp);
}

void D3D7TextureStage::SetAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2)
{
	// TODO-DX7: Cache states
	DWORD alphaOp = ARXToDX7TextureOp[textureOp];
	GD3D7Device->SetTextureStageState(mStage, D3DTSS_ALPHAOP, alphaOp);

	if(textureOp != TextureStage::OpDisable)
	{
		if(textureOp != TextureStage::OpSelectArg2)
		{
			DWORD alphaArg1 = ARXToDX7TextureArg[texArg1 & TextureStage::ArgMask];
			alphaArg1 |= (texArg1 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GD3D7Device->SetTextureStageState(mStage, D3DTSS_ALPHAARG1, alphaArg1);
		}

		if(textureOp != TextureStage::OpSelectArg1)
		{
			DWORD alphaArg2 = ARXToDX7TextureArg[texArg2 & TextureStage::ArgMask];
			alphaArg2 |= (texArg2 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GD3D7Device->SetTextureStageState(mStage, D3DTSS_ALPHAARG2, alphaArg2);
		}
	}
}

void D3D7TextureStage::SetTexture( Texture* pTexture )
{
	// TODO-DX7: Support multiple texture types
	DX7Texture2D* tex = (DX7Texture2D*)pTexture;

	GD3D7Device->SetTexture(mStage, tex->GetTextureID());
}

void D3D7TextureStage::ResetTexture()
{
	GD3D7Device->SetTexture(mStage, 0);
}

void D3D7TextureStage::SetWrapMode(TextureStage::WrapMode wrapMode)
{
	GD3D7Device->SetTextureStageState(mStage, D3DTSS_ADDRESS, ARXToDX7WrapMode[wrapMode]);
}

void D3D7TextureStage::SetMinFilter(FilterMode filterMode)
{
	arx_assert_msg(filterMode != TextureStage::FilterNone, "Invalid minification filter");
	GD3D7Device->SetTextureStageState(mStage, D3DTSS_MINFILTER, ARXToDX7MinFilter[filterMode]);
}

void D3D7TextureStage::SetMagFilter(FilterMode filterMode)
{
	arx_assert_msg(filterMode != TextureStage::FilterNone, "Invalid magnification filter");
	GD3D7Device->SetTextureStageState(mStage, D3DTSS_MAGFILTER, ARXToDX7MagFilter[filterMode]);
}

void D3D7TextureStage::SetMipFilter(FilterMode filterMode)
{
	D3DTEXTUREMIPFILTER mipFilter = ARXToDX7MipFilter[filterMode];
	GD3D7Device->SetTextureStageState(mStage, D3DTSS_MIPFILTER, mipFilter);
}

void D3D7TextureStage::SetMipMapLODBias(float bias) {
	if(GetKeyState(VK_F12) != 0) { // TODO what kind of hack is this?
		float val = 0;
		GD3D7Device->SetTextureStageState(mStage, D3DTSS_MIPMAPLODBIAS, reinterpret<DWORD, f32>(val));
	} else {
		GD3D7Device->SetTextureStageState(mStage, D3DTSS_MIPMAPLODBIAS, reinterpret<DWORD, f32>(bias));
	}
}

void D3D7TextureStage::SetTextureCoordIndex(int texCoordIdx)
{
	GD3D7Device->SetTextureStageState(mStage, D3DTSS_TEXCOORDINDEX, texCoordIdx);
}
