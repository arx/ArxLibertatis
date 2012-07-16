/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

// MinGW's d3dx9math.h requires min and max but doesn't include them!
#include <algorithm>
using std::max;
using std::min;

#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9.h>
#include <d3dx9tex.h>

#include "graphics/Math.h"
#include "io/log/Logger.h"

extern LPDIRECT3DDEVICE9 GD3D9Device;

D3DFORMAT ARXToDX9InternalFormat[] = {
	D3DFMT_L8,          // Format_L8,
	D3DFMT_A8,          // Format_A8,
	D3DFMT_A8L8,        // Format_L8A8,
	D3DFMT_R8G8B8,      // Format_R8G8B8,
	D3DFMT_R8G8B8,      // Format_B8G8R8,
	D3DFMT_A8B8G8R8,    // Format_R8G8B8A8,
	D3DFMT_A8R8G8B8,    // Format_B8G8R8A8,
	D3DFMT_DXT1,        // Format_DXT1,
	D3DFMT_DXT3,        // Format_DXT3,
	D3DFMT_DXT5,        // Format_DXT5,
	D3DFMT_UNKNOWN,     // Format_Unknown,
};

DX9Texture2D::DX9Texture2D()
	: m_pTexture(0)
{
}

DX9Texture2D::~DX9Texture2D()
{
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

void DX9Texture2D::Upload() 
{
	arx_assert(m_pTexture != 0);

	HRESULT hr;

	if(mFormat == Image::Format_R8G8B8) {
		bool converted = mImage.ConvertTo(Image::Format_B8G8R8);
		if(!converted) {
			LogError << "Upload failed because of invalid source format.";
			return;
		} else {
			mFormat = mImage.GetFormat();
		}
	}

	LPDIRECT3DSURFACE9 pSurface;
	hr = m_pTexture->GetSurfaceLevel(0, &pSurface);
	if( FAILED(hr) ) {
		LogError << "IDirect3DSurface9::GetSurfaceLevel() failed: " << hr;
		return;
	}

	BYTE* pSrc = (BYTE*)mImage.GetData();
	RECT srcRect;
	SetRect(&srcRect, 0, 0, mImage.GetWidth(), mImage.GetHeight());
	hr = D3DXLoadSurfaceFromMemory(pSurface, 0, 0, pSrc, ARXToDX9InternalFormat[mFormat], mImage.GetWidth() * Image::GetSize(mFormat), 0, &srcRect, D3DX_FILTER_NONE, 0);
	if(FAILED(hr)) {
		LogError << "D3DXLoadSurfaceFromMemory() failed." << hr;
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
