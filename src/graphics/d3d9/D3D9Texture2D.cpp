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

#include "graphics/d3d9/D3D9Texture2D.h"

// TODO(broken-wine) wine's d3dx9math.h requires min and max but doesn't include them!
#include <algorithm>
using std::max;
using std::min;

#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9.h>
#include <d3dx9tex.h>

#include "graphics/Math.h"
#include "io/log/Logger.h"

std::list<DX9Texture2D*> g_Textures2D;

extern LPDIRECT3DDEVICE9 GD3D9Device;

D3DFORMAT ARXToDX9InternalFormat[] = {
	D3DFMT_L8,			// Format_L8,
	D3DFMT_A8,			// Format_A8,
	D3DFMT_A8L8,		// Format_L8A8,
	D3DFMT_X8B8G8R8,	// Format_R8G8B8,
	D3DFMT_X8B8G8R8,	// Format_B8G8R8,
	D3DFMT_A8B8G8R8,	// Format_R8G8B8A8,
	D3DFMT_A8B8G8R8,	// Format_B8G8R8A8,
	D3DFMT_DXT1,		// Format_DXT1,
	D3DFMT_DXT3,		// Format_DXT3,
	D3DFMT_DXT5,		// Format_DXT5,
	D3DFMT_UNKNOWN,		// Format_Unknown,
};

DX9Texture2D::DX9Texture2D()
	: m_pTexture(0)
{
	g_Textures2D.push_back(this);
}

DX9Texture2D::~DX9Texture2D()
{
	g_Textures2D.remove(this);
	Destroy();
}

bool DX9Texture2D::Create()
{	
	arx_assert_msg(m_pTexture == 0, "Texture already created!");
	arx_assert(size.x != 0);
	arx_assert(size.y != 0);

	HRESULT hr;
	hr = D3DXCreateTexture(GD3D9Device, size.x, size.y, hasMipmaps() ? D3DX_DEFAULT : 1, 0, ARXToDX9InternalFormat[mFormat], D3DPOOL_MANAGED, &m_pTexture);
	if( FAILED(hr) )
		return false;

	D3DSURFACE_DESC surfaceDesc;
	hr = m_pTexture->GetLevelDesc(0, &surfaceDesc);
	if( FAILED(hr) ) {
		m_pTexture->Release();
		m_pTexture = 0;
		return false;
	}

	// Store texture size
	storedSize = Vec2i(surfaceDesc.Width, surfaceDesc.Height);

	return true;
}

void DX9Texture2D::Copy_L8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch)
{
	ARX_UNUSED(dstPixelSize);
	ARX_UNUSED(srcPixelSize);

	DWORD srcPitchIncrement = srcPitch - imageWidth;
	DWORD dstPitchIncrement = dstPitch - imageWidth;
	
	if(d3dFormat == D3DFMT_L8) {
		if(srcPitchIncrement != 0 || dstPitchIncrement != 0) {
			for (DWORD y = 0; y < imageHeight; y++) {
				// D3DFMT_L8 <- Format_L8
				memcpy(pDst, pSrc, imageWidth);
		
				pSrc += imageWidth;
				pDst += imageWidth;

				pSrc += srcPitchIncrement;
				pDst += dstPitchIncrement;
			}
		} else {
			memcpy(pDst, pSrc, imageWidth*imageHeight);
		}
	} else {
		LogError << "No conversion possible from Image::Format_L8 to D3D format #" << d3dFormat;
	}
}

void DX9Texture2D::Copy_A8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch)
{
	ARX_UNUSED(dstPixelSize);
	ARX_UNUSED(srcPixelSize);

	DWORD srcPitchIncrement = srcPitch - imageWidth;
	DWORD dstPitchIncrement = dstPitch - imageWidth;
	
	if(d3dFormat == D3DFMT_A8) {
		if(srcPitchIncrement != 0 || dstPitchIncrement != 0) {
			for (DWORD y = 0; y < imageHeight; y++) {
				// D3DFMT_A8 <- Format_A8
				memcpy(pDst, pSrc, imageWidth);
		
				pSrc += imageWidth;
				pSrc += srcPitchIncrement;

				pDst += imageWidth;
				pDst += dstPitchIncrement;				
			}
		} else {
			memcpy(pDst, pSrc, imageWidth*imageHeight);
		}
	} else {
		LogError << "No conversion possible from Image::Format_A8 to D3D format #" << d3dFormat;
	}
}

void DX9Texture2D::Copy_L8A8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch)
{
	DWORD srcPitchIncrement = srcPitch - imageWidth * srcPixelSize;
	DWORD dstPitchIncrement = dstPitch - imageWidth * dstPixelSize;	

	if(d3dFormat == D3DFMT_A8L8) {
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_A8L8 <- Format_L8A8
				pDst[0] = pSrc[1];
				pDst[1] = pSrc[0];

				pSrc += srcPixelSize;
				pDst += dstPixelSize;
			}
			pSrc += srcPitchIncrement;
			pDst += dstPitchIncrement;
		}
	} else {
		LogError << "No conversion possible from Image::Format_L8A8 to D3D format #" << d3dFormat;
	}
}

void DX9Texture2D::Copy_R8G8B8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch)
{
	DWORD srcPitchIncrement = srcPitch - imageWidth * srcPixelSize;
	DWORD dstPitchIncrement = dstPitch - imageWidth * dstPixelSize;
	
	switch(d3dFormat) {
	case D3DFMT_X8B8G8R8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_X8B8G8R8 <- Format_R8G8B8
				pDst[0] = pSrc[0];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[2];

				pSrc += srcPixelSize;
				pDst += dstPixelSize;
			}
			pSrc += srcPitchIncrement;
			pDst += dstPitchIncrement;
		}
		break;
	
	case D3DFMT_X8R8G8B8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_X8R8G8B8 <- Format_R8G8B8
				pDst[0] = pSrc[2];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[0];

				pSrc += srcPixelSize;
				pDst += dstPixelSize;
			}
			pSrc += srcPitchIncrement;
			pDst += dstPitchIncrement;
		}
		break;

	default:
		LogError << "No conversion possible from Image::Format_R8G8B8 to D3D format #" << d3dFormat;
	}
}

void DX9Texture2D::Copy_B8G8R8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch)
{
	DWORD srcPitchIncrement = srcPitch - imageWidth * srcPixelSize;
	DWORD dstPitchIncrement = dstPitch - imageWidth * dstPixelSize;
	
	switch(d3dFormat) {
	case D3DFMT_X8B8G8R8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_X8B8G8R8 <- Format_B8G8R8
				pDst[0] = pSrc[2];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[0];

				pSrc += srcPixelSize;
				pDst += dstPixelSize;
			}
			pSrc += srcPitchIncrement;
			pDst += dstPitchIncrement;			
		}
		break;

	case D3DFMT_X8R8G8B8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_X8R8G8B8 <- Format_B8G8R8
				pDst[0] = pSrc[0];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[2];

				pSrc += srcPixelSize;
				pDst += dstPixelSize;
			}
			pSrc += srcPitchIncrement;
			pDst += dstPitchIncrement;			
		}
		break;

	default:
		LogError << "No conversion possible from Image::Format_B8G8R8 to D3D format #" << d3dFormat;
	}
}

void DX9Texture2D::Copy_R8G8B8A8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch)
{
	DWORD srcPitchIncrement = srcPitch - imageWidth * srcPixelSize;
	DWORD dstPitchIncrement = dstPitch - imageWidth * dstPixelSize;
	
	switch(d3dFormat) {
	case D3DFMT_A8B8G8R8:
		// D3DFMT_A8B8G8R8 <- Format_R8G8B8A8
		if(srcPitchIncrement != 0 || dstPitchIncrement != 0) {
			for (DWORD y = 0; y < imageHeight; y++) {
				memcpy(pDst, pSrc, imageWidth*dstPixelSize);
		
				pSrc += imageWidth*srcPixelSize;
				pSrc += srcPitchIncrement;

				pDst += imageWidth*dstPixelSize;
				pDst += dstPitchIncrement;				
			}
		} else {
			memcpy(pDst, pSrc, imageWidth*imageHeight*dstPixelSize);
		}
		break;

	case D3DFMT_A8R8G8B8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_A8R8G8B8 <- Format_R8G8B8A8
				pDst[0] = pSrc[2];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[0];
				pDst[3] = pSrc[3];

				pSrc += srcPixelSize;
				pDst += dstPixelSize;
			}
			pSrc += srcPitchIncrement;
			pDst += dstPitchIncrement;			
		}
		break;

	default:
		LogError << "No conversion possible from Image::Format_R8G8B8A8 to D3D format #" << d3dFormat;
	}
}

void DX9Texture2D::Copy_B8G8R8A8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch)
{
	DWORD srcPitchIncrement = srcPitch - imageWidth * srcPixelSize;
	DWORD dstPitchIncrement = dstPitch - imageWidth * dstPixelSize;
	
	switch(d3dFormat) {
	case D3DFMT_A8B8G8R8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_A8B8G8R8 <- Format_B8G8R8A8
				pDst[0] = pSrc[2];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[0];
				pDst[3] = pSrc[3];

				pSrc += srcPixelSize;
				pDst += dstPixelSize;
			}
			pSrc += srcPitchIncrement;
			pDst += dstPitchIncrement;			
		}
		break;

	case D3DFMT_A8R8G8B8:
		// D3DFMT_A8R8G8B8 <- Format_B8G8R8A8
		if(srcPitchIncrement != 0 || dstPitchIncrement != 0) {
			for (DWORD y = 0; y < imageHeight; y++) {
				memcpy(pDst, pSrc, imageWidth*dstPixelSize);
		
				pSrc += imageWidth*srcPixelSize;
				pSrc += srcPitchIncrement;

				pDst += imageWidth*dstPixelSize;
				pDst += dstPitchIncrement;				
			}
		} else {
			memcpy(pDst, pSrc, imageWidth*imageHeight*dstPixelSize);
		}
		break;

	default:
		LogError << "No conversion possible from Image::Format_B8G8R8A8 to D3D format #" << d3dFormat;
	}
}

void DX9Texture2D::Upload() 
{
	arx_assert(m_pTexture != 0);

	HRESULT hr;

	LPDIRECT3DSURFACE9 pSurface;
	hr = m_pTexture->GetSurfaceLevel(0, &pSurface);
	if( FAILED(hr) ) {
		LogError << "IDirect3DSurface9::GetSurfaceLevel() failed: " << hr;
		return;
	}

	D3DLOCKED_RECT lockedRect;
	hr = pSurface->LockRect(&lockedRect, 0, 0);
	if( SUCCEEDED(hr) ) {

		D3DSURFACE_DESC surfaceDesc;
		pSurface->GetDesc(&surfaceDesc);

		BYTE* pSrc = (BYTE*)mImage.GetData();
		
		DWORD imageWidth = mImage.GetWidth();
		DWORD imageHeight = mImage.GetHeight();

		arx_assert(imageWidth == surfaceDesc.Width);
		arx_assert(imageHeight == surfaceDesc.Height);

		switch(mFormat)
		{
		case Image::Format_L8:
			Copy_L8((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, 1, 1, lockedRect.Pitch, imageWidth);
			break;

		case Image::Format_A8:
			Copy_A8((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, 1, 1, lockedRect.Pitch, imageWidth);
			break;

		case Image::Format_L8A8:
			Copy_L8A8((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, 2, 2, lockedRect.Pitch, imageWidth * 2);
			break;

		case Image::Format_R8G8B8:
			Copy_R8G8B8((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, 4, 3, lockedRect.Pitch, imageWidth * 3);
			break;

		case Image::Format_B8G8R8:
			Copy_B8G8R8((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, 4, 3, lockedRect.Pitch, imageWidth * 3);
			break;

		case Image::Format_R8G8B8A8:
			Copy_R8G8B8A8((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, 4, 4, lockedRect.Pitch, imageWidth * 4);
			break;

		case Image::Format_B8G8R8A8:
			Copy_B8G8R8A8((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, 4, 4, lockedRect.Pitch, imageWidth * 4);
			break;

		default:
			LogError << "CopyData: unsupported source format" << mFormat;
		}
			
		pSurface->UnlockRect();	
	} else {
		LogError << "IDirect3DSurface9::LockRect() failed: " << hr;
		pSurface->Release();
		return;
	}

	if(hasMipmaps()) {
		hr = D3DXFilterTexture(m_pTexture, 0, 0, D3DX_DEFAULT);	
		if( FAILED(hr) )
			LogError << "D3DXFilterTexture() failed: " << hr;
	}

	pSurface->Release();
}

void DX9Texture2D::Destroy()
{
	if(m_pTexture)
	{ 
		m_pTexture->Release();
		m_pTexture = 0;
	}
}
