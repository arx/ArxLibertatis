
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
#include "io/Logger.h"

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
	hr = D3DXCreateTexture(GD3D9Device, size.x, size.y, mHasMipmaps ? D3DX_DEFAULT : 1, 0, ARXToDX9InternalFormat[mFormat], D3DPOOL_MANAGED, &m_pTexture);
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

template <Image::Format fmt>
void CopyData(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT dstFormat, DWORD dstPitch)
{
	LogError << "CopyData: unsupported source format" << fmt;
}

template <>
void CopyData<Image::Format_L8>(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT dstFormat, DWORD dstPitch)
{
	DWORD pitchIncrement = dstPitch - imageWidth;

	if(dstFormat == D3DFMT_L8) {
		if(pitchIncrement != 0)	{
			for (DWORD y = 0; y < imageHeight; y++) {
				// D3DFMT_L8 <- Format_L8
				memcpy(pDst, pSrc, imageWidth);
		
				pSrc += imageWidth;
				pDst += imageWidth;
				pDst += pitchIncrement;
			}
		} else {
			memcpy(pDst, pSrc, imageWidth*imageHeight);
		}
	} else {
		LogError << "No conversion possible from Image::Format_L8 to D3D format #" << dstFormat;
	}
}

template <>
void CopyData<Image::Format_A8>(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT dstFormat, DWORD dstPitch)
{
	DWORD pitchIncrement = dstPitch - imageWidth;

	if(dstFormat == D3DFMT_A8) {
		if(pitchIncrement != 0)	{
			for (DWORD y = 0; y < imageHeight; y++) {
				// D3DFMT_A8 <- Format_A8
				memcpy(pDst, pSrc, imageWidth);
		
				pSrc += imageWidth;
				pDst += imageWidth;
				pDst += pitchIncrement;
			}
		} else {
			memcpy(pDst, pSrc, imageWidth*imageHeight);
		}
	} else {
		LogError << "No conversion possible from Image::Format_A8 to D3D format #" << dstFormat;
	}
}

template <>
void CopyData<Image::Format_L8A8>(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT dstFormat, DWORD dstPitch)
{
	DWORD pitchIncrement = dstPitch - imageWidth * 2;

	if(dstFormat == D3DFMT_A8L8) {
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_A8L8 <- Format_L8A8
				pDst[0] = pSrc[1];
				pDst[1] = pSrc[0];

				pSrc += 2;
				pDst += 2;
			}
			pDst += pitchIncrement;
		}
	} else {
		LogError << "No conversion possible from Image::Format_L8A8 to D3D format #" << dstFormat;
	}
}

template <>
void CopyData<Image::Format_R8G8B8>(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT dstFormat, DWORD dstPitch)
{
	DWORD pitchIncrement = dstPitch - imageWidth * 4;

	switch(dstFormat) {
	case D3DFMT_X8B8G8R8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_X8B8G8R8 <- Format_R8G8B8
				pDst[0] = pSrc[0];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[2];

				pSrc += 3;
				pDst += 4;
			}
			pDst += pitchIncrement;
		}
		break;
	
	case D3DFMT_X8R8G8B8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_X8R8G8B8 <- Format_R8G8B8
				pDst[0] = pSrc[2];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[0];

				pSrc += 3;
				pDst += 4;
			}
			pDst += pitchIncrement;
		}
		break;

	default:
		LogError << "No conversion possible from Image::Format_R8G8B8 to D3D format #" << dstFormat;
	}
}

template <>
void CopyData<Image::Format_B8G8R8>(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT dstFormat, DWORD dstPitch)
{
	DWORD pitchIncrement = dstPitch - imageWidth * 4;

	switch(dstFormat) {
	case D3DFMT_X8B8G8R8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_X8B8G8R8 <- Format_B8G8R8
				pDst[0] = pSrc[2];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[0];

				pSrc += 3;
				pDst += 4;
			}
			pDst += pitchIncrement;
		}
		break;

	case D3DFMT_X8R8G8B8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_X8R8G8B8 <- Format_B8G8R8
				pDst[0] = pSrc[0];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[2];

				pSrc += 3;
				pDst += 4;
			}
			pDst += pitchIncrement;
		}
		break;

	default:
		LogError << "No conversion possible from Image::Format_B8G8R8 to D3D format #" << dstFormat;
	}
}

template <>
void CopyData<Image::Format_R8G8B8A8>(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT dstFormat, DWORD dstPitch)
{
	DWORD pitchIncrement = dstPitch - imageWidth * 4;

	switch(dstFormat) {
	case D3DFMT_A8B8G8R8:
		// D3DFMT_A8B8G8R8 <- Format_R8G8B8A8
		if(pitchIncrement != 0)	{
			for (DWORD y = 0; y < imageHeight; y++) {
				memcpy(pDst, pSrc, imageWidth*4);
		
				pSrc += imageWidth*4;
				pDst += imageWidth*4;
				pDst += pitchIncrement;
			}
		} else {
			memcpy(pDst, pSrc, imageWidth*imageHeight*4);
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

				pSrc += 4;
				pDst += 4;
			}
			pDst += pitchIncrement;
		}
		break;

	default:
		LogError << "No conversion possible from Image::Format_R8G8B8A8 to D3D format #" << dstFormat;
	}
}

template <>
void CopyData<Image::Format_B8G8R8A8>(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT dstFormat, DWORD dstPitch)
{
	DWORD pitchIncrement = dstPitch - imageWidth * 4;

	switch(dstFormat) {
	case D3DFMT_A8B8G8R8:
		for (DWORD y = 0; y < imageHeight; y++) {
			for (DWORD x = 0; x < imageWidth; x++) {
				// D3DFMT_A8B8G8R8 <- Format_B8G8R8A8
				pDst[0] = pSrc[2];
				pDst[1] = pSrc[1];
				pDst[2] = pSrc[0];
				pDst[3] = pSrc[3];

				pSrc += 4;
				pDst += 4;
			}
			pDst += pitchIncrement;
		}
		break;

	case D3DFMT_A8R8G8B8:
		// D3DFMT_A8R8G8B8 <- Format_B8G8R8A8
		if(pitchIncrement != 0)	{
			for (DWORD y = 0; y < imageHeight; y++) {
				memcpy(pDst, pSrc, imageWidth*4);
		
				pSrc += imageWidth*4;
				pDst += imageWidth*4;
				pDst += pitchIncrement;
			}
		} else {
			memcpy(pDst, pSrc, imageWidth*imageHeight*4);
		}
		break;

	default:
		LogError << "No conversion possible from Image::Format_B8G8R8A8 to D3D format #" << dstFormat;
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
			CopyData<Image::Format_L8>((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, lockedRect.Pitch);
			break;

		case Image::Format_A8:
			CopyData<Image::Format_A8>((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, lockedRect.Pitch);
			break;

		case Image::Format_L8A8:
			CopyData<Image::Format_L8A8>((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, lockedRect.Pitch);
			break;

		case Image::Format_R8G8B8:
			CopyData<Image::Format_R8G8B8>((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, lockedRect.Pitch);
			break;

		case Image::Format_B8G8R8:
			CopyData<Image::Format_B8G8R8>((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, lockedRect.Pitch);
			break;

		case Image::Format_R8G8B8A8:
			CopyData<Image::Format_R8G8B8A8>((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, lockedRect.Pitch);
			break;

		case Image::Format_B8G8R8A8:
			CopyData<Image::Format_B8G8R8A8>((BYTE*)lockedRect.pBits, pSrc, imageWidth, imageHeight, surfaceDesc.Format, lockedRect.Pitch);
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

	if(mHasMipmaps) {
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
