/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphics/d3d9/D3D9TextureStage.h"

#include "graphics/Math.h"
#include "graphics/d3d9/D3D9Texture2D.h"

extern LPDIRECT3DDEVICE9 GD3D9Device;

const D3DTEXTUREOP ARXToDX9TextureOp[] = {
	D3DTOP_DISABLE,		// OpDisable,
	D3DTOP_SELECTARG1,	// OpSelectArg1,
	D3DTOP_SELECTARG2,	// OpSelectArg2,
	D3DTOP_MODULATE,	// OpModulate,
	D3DTOP_MODULATE2X,	// OpModulate2X,
	D3DTOP_MODULATE4X,	// OpModulate4X,
	D3DTOP_ADDSIGNED	// OpAddSigned
};

const DWORD ARXToDX9TextureArg[] = {
	D3DTA_DIFFUSE,		// TexArgDiffuse,
	D3DTA_CURRENT,		// TexArgCurrent,
	D3DTA_TEXTURE		// TexArgTexture,
};

D3DTEXTUREFILTERTYPE ARXToDX9MagFilter[] = {
	D3DTEXF_POINT,		// FilterNone - Invalid for magnification,
	D3DTEXF_POINT,		// FilterNearest,
	D3DTEXF_LINEAR		// FilterLinear
};

D3DTEXTUREFILTERTYPE ARXToDX9MinFilter[] = {
	D3DTEXF_POINT,		// FilterNone - Invalid for minification,
	D3DTEXF_POINT,		// FilterNearest,
	D3DTEXF_LINEAR,		// FilterLinear
};

D3DTEXTUREFILTERTYPE ARXToDX9MipFilter[] = {
	D3DTEXF_NONE,		// FilterNone,
	D3DTEXF_POINT,		// FilterNearest,
	D3DTEXF_LINEAR		// FilterLinear
};

const D3DTEXTUREADDRESS ARXToDX9WrapMode[] = {
	D3DTADDRESS_WRAP,	// WrapRepeat,
	D3DTADDRESS_MIRROR,	// WrapMirror,
	D3DTADDRESS_CLAMP	// WrapClamp,
};

D3D9TextureStage::D3D9TextureStage(unsigned int textureStage) : TextureStage(textureStage) {
	GD3D9Device->SetTextureStageState(textureStage, D3DTSS_TEXCOORDINDEX, textureStage);
}

void D3D9TextureStage::SetColorOp(TextureOp textureOp)
{
	// TODO-DX9: Cache states
	DWORD colorOp = ARXToDX9TextureOp[textureOp];
	GD3D9Device->SetTextureStageState(mStage, D3DTSS_COLOROP, colorOp);
}

void D3D9TextureStage::SetColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2)
{
	// TODO-DX9: Cache states
	DWORD colorOp = ARXToDX9TextureOp[textureOp];
	GD3D9Device->SetTextureStageState(mStage, D3DTSS_COLOROP, colorOp);

	if(textureOp != TextureStage::OpDisable)
	{
		if(textureOp != TextureStage::OpSelectArg2)
		{
			DWORD colorArg1 = ARXToDX9TextureArg[texArg1 & TextureStage::ArgMask];
			colorArg1 |= (texArg1 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GD3D9Device->SetTextureStageState(mStage, D3DTSS_COLORARG1, colorArg1);
		}

		if(textureOp != TextureStage::OpSelectArg1)
		{
			DWORD colorArg2 = ARXToDX9TextureArg[texArg2 & TextureStage::ArgMask];
			colorArg2 |= (texArg2 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GD3D9Device->SetTextureStageState(mStage, D3DTSS_COLORARG2, colorArg2);
		}
	}
}

void D3D9TextureStage::SetAlphaOp(TextureOp textureOp)
{
	// TODO-DX9: Cache states
	DWORD colorOp = ARXToDX9TextureOp[textureOp];
	GD3D9Device->SetTextureStageState(mStage, D3DTSS_ALPHAOP, colorOp);
}

void D3D9TextureStage::SetAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2)
{
	// TODO-DX9: Cache states
	DWORD alphaOp = ARXToDX9TextureOp[textureOp];
	GD3D9Device->SetTextureStageState(mStage, D3DTSS_ALPHAOP, alphaOp);

	if(textureOp != TextureStage::OpDisable)
	{
		if(textureOp != TextureStage::OpSelectArg2)
		{
			DWORD alphaArg1 = ARXToDX9TextureArg[texArg1 & TextureStage::ArgMask];
			alphaArg1 |= (texArg1 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GD3D9Device->SetTextureStageState(mStage, D3DTSS_ALPHAARG1, alphaArg1);
		}

		if(textureOp != TextureStage::OpSelectArg1)
		{
			DWORD alphaArg2 = ARXToDX9TextureArg[texArg2 & TextureStage::ArgMask];
			alphaArg2 |= (texArg2 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GD3D9Device->SetTextureStageState(mStage, D3DTSS_ALPHAARG2, alphaArg2);
		}
	}
}

void D3D9TextureStage::SetTexture( Texture* pTexture )
{
	// TODO-DX9: Support multiple texture types
	DX9Texture2D* tex = (DX9Texture2D*)pTexture;

	GD3D9Device->SetTexture(mStage, tex->GetTextureID());
}

void D3D9TextureStage::ResetTexture()
{
	GD3D9Device->SetTexture(mStage, 0);
}

void D3D9TextureStage::SetWrapMode(TextureStage::WrapMode wrapMode)
{
	GD3D9Device->SetSamplerState(mStage, D3DSAMP_ADDRESSU, ARXToDX9WrapMode[wrapMode]);
	GD3D9Device->SetSamplerState(mStage, D3DSAMP_ADDRESSV, ARXToDX9WrapMode[wrapMode]);
	GD3D9Device->SetSamplerState(mStage, D3DSAMP_ADDRESSW, ARXToDX9WrapMode[wrapMode]);
}

void D3D9TextureStage::SetMinFilter(FilterMode filterMode)
{
	arx_assert_msg(filterMode != TextureStage::FilterNone, "Invalid minification filter");
	GD3D9Device->SetSamplerState(mStage, D3DSAMP_MINFILTER, ARXToDX9MinFilter[filterMode]);
}

void D3D9TextureStage::SetMagFilter(FilterMode filterMode)
{
	arx_assert_msg(filterMode != TextureStage::FilterNone, "Invalid magnification filter");
	GD3D9Device->SetSamplerState(mStage, D3DSAMP_MAGFILTER, ARXToDX9MagFilter[filterMode]);
}

void D3D9TextureStage::SetMipFilter(FilterMode filterMode)
{
	D3DTEXTUREFILTERTYPE mipFilter = ARXToDX9MipFilter[filterMode];
	GD3D9Device->SetSamplerState(mStage, D3DSAMP_MIPFILTER, mipFilter);
}

void D3D9TextureStage::SetMipMapLODBias(float bias) {
	GD3D9Device->SetSamplerState(mStage, D3DSAMP_MIPMAPLODBIAS, reinterpret<DWORD, f32>(bias));
}
