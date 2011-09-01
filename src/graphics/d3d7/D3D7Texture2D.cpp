
#include "graphics/d3d7/D3D7Texture2D.h"

#include "graphics/Math.h"

std::list<DX7Texture2D*> g_Textures2D;

extern LPDIRECT3DDEVICE7 GD3D7Device;

DX7Texture2D::DX7Texture2D()
	: m_pddsSurface(0)
{
	g_Textures2D.push_back(this);
}

DX7Texture2D::~DX7Texture2D()
{
	g_Textures2D.remove(this);
	Destroy();
}

HRESULT CALLBACK DX7Texture2D::TextureSearchCallback(DDPIXELFORMAT* pddpf, VOID * param) {
	
	if(NULL == pddpf || NULL == param) {
		return DDENUMRET_OK;
	}
	
	DDSURFACEDESC2 * ddsd = (DDSURFACEDESC2*)param;
	
	DWORD flags = DDPF_RGB | DDPF_ALPHAPIXELS;
	
	// Since we'll get rid of this DX7 renderer as soon as possible, I won't invest any more time figuring out the optimal texture matches
	// Always use 32 bit RGBA (8-8-8-8)... with no support for DXT (arx doesn't use them yet anyway!
	if((pddpf->dwFlags & (DDPF_RGB|DDPF_ALPHAPIXELS)) == flags && pddpf->dwRGBBitCount >= 24) {
		
		// We found a good match. Copy the current pixel format to our output parameter
		memcpy(&ddsd->ddpfPixelFormat, pddpf, sizeof(DDPIXELFORMAT));
		
		// Return with DDENUMRET_CANCEL to end enumeration.
		return DDENUMRET_CANCEL;
	}
	
	return DDENUMRET_OK;
}

bool DX7Texture2D::Create()
{
	
	arx_assert_msg(m_pddsSurface == 0, "Texture already created!");
	arx_assert(size.x != 0);
	arx_assert(size.y != 0);

	HRESULT hr;

	// Get the device caps so we can check if the device has any constraints
	// when using textures
	D3DDEVICEDESC7 ddDesc;
	if( FAILED( GD3D7Device->GetCaps( &ddDesc ) ) )
		return false;

	// Setup the new surface desc for the texture. Note how we are using the
	// texture manage attribute, so Direct3D does alot of dirty work for us
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize          = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
	ddsd.dwWidth         = size.x;
	ddsd.dwHeight        = size.y;

	// Turn on texture management for hardware devices
	if( ddDesc.deviceGUID == IID_IDirect3DHALDevice )
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	else if( ddDesc.deviceGUID == IID_IDirect3DTnLHalDevice )
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

	// Adjust width and height, if the driver requires it
	if( mHasMipmaps || ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2 )
	{
		ddsd.dwWidth = GetNextPowerOf2(ddsd.dwWidth);
		ddsd.dwHeight = GetNextPowerOf2(ddsd.dwHeight);
	}
	if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
	{
		if( ddsd.dwWidth > ddsd.dwHeight ) 
			ddsd.dwHeight = ddsd.dwWidth;
		else
			ddsd.dwWidth  = ddsd.dwHeight;
	}

	// Limit max texture sizes, if the driver can't handle large textures
	DWORD dwMaxWidth  = ddDesc.dwMaxTextureWidth;
	DWORD dwMaxHeight = ddDesc.dwMaxTextureHeight;

	if (ddsd.dwWidth > dwMaxWidth)
		ddsd.dwWidth = dwMaxWidth;

	if (ddsd.dwHeight > dwMaxHeight)
		ddsd.dwHeight = dwMaxHeight;

	if(mHasMipmaps)
	{
		arx_assert_msg(IsPowerOf2(ddsd.dwWidth), "Texture size must be a power of two to support mipmapping");

		// Count how many mip-map levels we need
		int mipLevels = 0;
		int mipWidth = ddsd.dwWidth; 
		int mipHeight = ddsd.dwHeight; 

		// Smallest mip-map we want is 2 x 2
		while ((mipWidth > 2) && (mipHeight > 2)) 
		{ 
			mipLevels++; 
			mipWidth  >>= 1; 
			mipHeight >>= 1; 
		} 

		// Set appropriate flags
		if (mipLevels > 1) 
		{ 
			ddsd.dwFlags  |= DDSD_MIPMAPCOUNT; 
			ddsd.ddsCaps.dwCaps  |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
			ddsd.dwMipMapCount = mipLevels;
		}
	}
	

	// Enumerate the texture formats, and find the closest device-supported
	// texture pixel format.
	GD3D7Device->EnumTextureFormats( TextureSearchCallback, &ddsd);
	if( 0L == ddsd.ddpfPixelFormat.dwRGBBitCount )
		return false;

	// Get the device's render target, so we can then use the render target to
	// get a ptr to a DDraw object. We need the DirectDraw interface for
	// creating surfaces.
	LPDIRECTDRAWSURFACE7 pddsRender;
	LPDIRECTDRAW7        pDD;
	GD3D7Device->GetRenderTarget( &pddsRender );
	pddsRender->GetDDInterface( (VOID**)&pDD );
	pddsRender->Release();

	// Create a new surface for the texture
	if( FAILED( hr = pDD->CreateSurface( &ddsd, &m_pddsSurface, NULL ) ) )
	{
		pDD->Release();
		return false;
	}

	// Store texture size
	storedSize = Vec2i(ddsd.dwWidth, ddsd.dwHeight);

	// Done with DDraw
	pDD->Release();

	return m_pddsSurface != NULL;
}

void DX7Texture2D::Upload()
{
	arx_assert_msg(m_pddsSurface != NULL, "Texture was never initialized!");

	// Get a DDraw object to create a temporary surface
	LPDIRECTDRAW7 pDD;
	m_pddsSurface->GetDDInterface((VOID **)&pDD);

	// Setup the new surface desc
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	m_pddsSurface->GetSurfaceDesc(&ddsd);

	ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd.ddsCaps.dwCaps2 = 0L;
	
	// Create a new surface for the texture
	LPDIRECTDRAWSURFACE7 pddsTempSurface;
	HRESULT hr;

	hr = pDD->CreateSurface(&ddsd, &pddsTempSurface, NULL);
	if (hr != S_OK)
	{
		pDD->Release();
		return;
	}

	hr = pddsTempSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, 0);
	arx_assert(hr == S_OK);

	DWORD * pDst = (DWORD *)ddsd.lpSurface;

	DWORD dwRMask = ddsd.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = ddsd.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = ddsd.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftR = 0;
	DWORD dwGShiftR = 0;
	DWORD dwBShiftR = 0;
	DWORD dwAShiftR = 0;

	DWORD dwMask;
	for (dwMask = dwRMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwRShiftR++;
	for (dwMask = dwGMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwGShiftR++;
	for (dwMask = dwBMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwBShiftR++;
	for (dwMask = dwAMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwAShiftR++;

	BYTE* pSrc = (BYTE*)mImage.GetData();
	DWORD r, g, b, a;

	DWORD imageWidth = mImage.GetWidth();
	DWORD imageHeight = mImage.GetHeight();

	arx_assert(imageWidth <= ddsd.dwWidth);
	arx_assert(imageHeight <= ddsd.dwHeight);
	
	DWORD pitchIncrement = (ddsd.lPitch >> 2) - imageWidth;

	if(mFormat == Image::Format_L8)
	{
		for (DWORD y = 0; y < imageHeight; y++)
		{
			for (DWORD x = 0; x < imageWidth; x++, pDst++)
			{
				r = g = b = *pSrc;
				a = 0xFF;
				pSrc++;

				DWORD dr = (r << dwRShiftR)&dwRMask;
				DWORD dg = (g << dwGShiftR)&dwGMask;
				DWORD db = (b << dwBShiftR)&dwBMask;
				DWORD da = (a << dwAShiftR)&dwAMask;
				*pDst = (DWORD)(dr + dg + db + da);
			}
			pDst += pitchIncrement;
		}
	}
	else if(mFormat == Image::Format_A8)
	{
		for (DWORD y = 0; y < imageHeight; y++)
		{
			for (DWORD x = 0; x < imageWidth; x++, pDst++)
			{
				r = g = b = 0xFF;
				a = *pSrc;
				pSrc++;

				DWORD dr = (r << dwRShiftR)&dwRMask;
				DWORD dg = (g << dwGShiftR)&dwGMask;
				DWORD db = (b << dwBShiftR)&dwBMask;
				DWORD da = (a << dwAShiftR)&dwAMask;
				*pDst = (DWORD)(dr + dg + db + da);
			}
			pDst += pitchIncrement;
		}
	}
	else if(mFormat == Image::Format_L8A8)
	{
		for (DWORD y = 0; y < imageHeight; y++)
		{
			for (DWORD x = 0; x < imageWidth; x++, pDst++)
			{
				r = g = b = pSrc[0];
				a = pSrc[1];
				pSrc += 2;

				DWORD dr = (r << dwRShiftR)&dwRMask;
				DWORD dg = (g << dwGShiftR)&dwGMask;
				DWORD db = (b << dwBShiftR)&dwBMask;
				DWORD da = (a << dwAShiftR)&dwAMask;
				*pDst = (DWORD)(dr + dg + db + da);
			}
			pDst += pitchIncrement;
		}
	}
	else if(mFormat == Image::Format_R8G8B8)
	{
		for (DWORD y = 0; y < imageHeight; y++)
		{
			for (DWORD x = 0; x < imageWidth; x++, pDst++)
			{
				r = pSrc[0];
				g = pSrc[1];
				b = pSrc[2];
				a = r + g + b == 0 ? 0x00 : 0xFF;
				pSrc += 3;

				DWORD dr = (r << dwRShiftR)&dwRMask;
				DWORD dg = (g << dwGShiftR)&dwGMask;
				DWORD db = (b << dwBShiftR)&dwBMask;
				DWORD da = (a << dwAShiftR)&dwAMask;
				*pDst = (DWORD)(dr + dg + db + da);
			}
			pDst += pitchIncrement;
		}
	}
	else if(mFormat == Image::Format_B8G8R8)
	{
		for (DWORD y = 0; y < imageHeight; y++)
		{
			for (DWORD x = 0; x < imageWidth; x++, pDst++)
			{
				b = pSrc[0];
				g = pSrc[1];
				r = pSrc[2];
				a = r + g + b == 0 ? 0x00 : 0xFF;
				pSrc += 3;

				DWORD dr = (r << dwRShiftR)&dwRMask;
				DWORD dg = (g << dwGShiftR)&dwGMask;
				DWORD db = (b << dwBShiftR)&dwBMask;
				DWORD da = (a << dwAShiftR)&dwAMask;
				*pDst = (DWORD)(dr + dg + db + da);
			}
			pDst += pitchIncrement;
		}
	}
	else if(mFormat == Image::Format_R8G8B8A8)
	{
		for (DWORD y = 0; y < imageHeight; y++)
		{
			for (DWORD x = 0; x < imageWidth; x++, pDst++)
			{
				r = pSrc[0];
				g = pSrc[1];
				b = pSrc[2];
				a = pSrc[3];
				pSrc += 4;

				DWORD dr = (r << dwRShiftR)&dwRMask;
				DWORD dg = (g << dwGShiftR)&dwGMask;
				DWORD db = (b << dwBShiftR)&dwBMask;
				DWORD da = (a << dwAShiftR)&dwAMask;
				*pDst = (DWORD)(dr + dg + db + da);
			}
			pDst += pitchIncrement;
		}
	}
	else if(mFormat == Image::Format_B8G8R8A8)
	{
		for (DWORD y = 0; y < imageHeight; y++)
		{
			for (DWORD x = 0; x < imageWidth; x++, pDst++)
			{
				b = pSrc[0];
				g = pSrc[1];
				r = pSrc[2];
				a = pSrc[3];
				pSrc += 4;

				DWORD dr = (r << dwRShiftR)&dwRMask;
				DWORD dg = (g << dwGShiftR)&dwGMask;
				DWORD db = (b << dwBShiftR)&dwBMask;
				DWORD da = (a << dwAShiftR)&dwAMask;
				*pDst = (DWORD)(dr + dg + db + da);
			}
			pDst += pitchIncrement;
		}
	}
	else
	{
		arx_assert_msg(false, "Unsupported image format");
	}
	
	pddsTempSurface->Unlock(0);

	m_pddsSurface->Blt(NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL);

	// Done with the temp objects
	pddsTempSurface->Release();
	pDD->Release();
	
	if (ddsd.dwMipMapCount > 0)
	{
		DDSCAPS2 ddsCaps;
		ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
		ddsCaps.dwCaps2 = 0;
		ddsCaps.dwCaps3 = 0;
		ddsCaps.dwCaps4 = 0;
		LPDIRECTDRAWSURFACE7 pddsSrc = m_pddsSurface;
		pddsSrc->AddRef();
		
		for (unsigned int i = 0; i < ddsd.dwMipMapCount; i++)
		{
			LPDIRECTDRAWSURFACE7 pddsDst = 0;
			if (FAILED(hr = pddsSrc->GetAttachedSurface(&ddsCaps, &pddsDst)))
			{
				pddsSrc->Release();
				break;
			}

			CopyNextMipLevel(pddsDst, pddsSrc);

			pddsSrc->Release();
			pddsSrc = pddsDst;
		}
	}
	
}

void DX7Texture2D::CopyNextMipLevel(LPDIRECTDRAWSURFACE7 pddsDst, LPDIRECTDRAWSURFACE7 pddsSrc)
{
	DDSURFACEDESC2 descSrc;
	DDSURFACEDESC2 descDst;
	
	descSrc.dwSize = descDst.dwSize = sizeof(DDSURFACEDESC2);
	
	HRESULT res;
	
	res = pddsSrc->Lock(NULL, &descSrc, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
	arx_assert_msg(res == S_OK, "res=%08x", res);
	ARX_UNUSED(res);

	res = pddsDst->Lock(NULL, &descDst, DDLOCK_WAIT, NULL);
	arx_assert_msg(res == S_OK, "res=%08x", res);
	ARX_UNUSED(res);

	arx_assert_msg(descDst.dwWidth == (descSrc.dwWidth >> 1), "src width = %d, dst width = %d (%s)", descSrc.dwWidth, descDst.dwWidth, mFileName.string().c_str());
	arx_assert_msg(descDst.dwHeight == (descSrc.dwHeight >> 1), "src height = %d, dst height = %d (%s)", descSrc.dwHeight, descDst.dwHeight, mFileName.string().c_str());

	DWORD pitchIncrementSrc = (descSrc.lPitch >> 2) - descSrc.dwWidth;
	DWORD pitchIncrementDst = (descDst.lPitch >> 2) - descDst.dwWidth;

	const DWORD * pSrc[4];
	
	pSrc[0] = (DWORD *)descSrc.lpSurface;		// Top left pixel
	pSrc[1] = pSrc[0] + 1;						// Top right pixel
	pSrc[2] = pSrc[0] + (descSrc.lPitch >> 2);	// Bottom left pixel
	pSrc[3] = pSrc[2] + 1;						// Bottom right pixel

	DWORD * pDst = (DWORD *)descDst.lpSurface;

	DWORD dwRMask = descDst.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = descDst.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = descDst.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = descDst.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwRShiftR++;
	for (; dwMask ; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwGShiftR++;
	for (; dwMask ; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwBShiftR++;
	for (; dwMask ; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask ; dwMask && !(dwMask & 0x1) ; dwMask >>= 1) dwAShiftR++;
	for (; dwMask ; dwMask >>= 1) dwAShiftL--;

	for (DWORD y = 0; y < descDst.dwHeight; y++)
	{
		for (DWORD x = 0; x < descDst.dwWidth; x++, pDst++ )
		{
			DWORD r = 0;
			DWORD g = 0;
			DWORD b = 0;
			DWORD a = 0;

			for(int i = 0; i < 4; i++)
			{
				r += (((*pSrc[i] & dwRMask) >> dwRShiftR) << dwRShiftL);
				g += (((*pSrc[i] & dwGMask) >> dwGShiftR) << dwGShiftL);      
				b += (((*pSrc[i] & dwBMask) >> dwBShiftR) << dwBShiftL);     
				a += (((*pSrc[i] & dwAMask) >> dwAShiftR) << dwAShiftL);

				pSrc[i] += 2;
			}

			r >>= 2;
			g >>= 2;
			b >>= 2;
			a >>= 2;

			DWORD dr, dg, db, da;
			dr = ((r >> (dwRShiftL)) << dwRShiftR) & dwRMask;
			dg = ((g >> (dwGShiftL)) << dwGShiftR) & dwGMask;
			db = ((b >> (dwBShiftL)) << dwBShiftR) & dwBMask;
			da = ((a >> (dwAShiftL)) << dwAShiftR) & dwAMask;
			
			*pDst = (DWORD)(dr + dg + db + da);
		}

		pDst += pitchIncrementDst;

		for(int i = 0; i < 4; i++)
			pSrc[i] += pitchIncrementSrc + (descSrc.lPitch >> 2);
	}

	res = pddsDst->Unlock(0);
	arx_assert(res == S_OK);

	res = pddsSrc->Unlock(0);
	arx_assert(res == S_OK);
}

void DX7Texture2D::Destroy()
{
	if(m_pddsSurface)
	{ 
		m_pddsSurface->DeleteAttachedSurface(0, NULL);
		m_pddsSurface->Release();
		m_pddsSurface = 0;
	}
}

bool downloadSurface(LPDIRECTDRAWSURFACE7 surface, Image & image) {
	
	DDSURFACEDESC2 desc;
	memset((void *)&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);
	
	if(surface->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_READONLY, 0) != DD_OK) {
		return false;
	}
	
	Image::Format format;
	if(desc.ddpfPixelFormat.dwRBitMask || desc.ddpfPixelFormat.dwGBitMask || desc.ddpfPixelFormat.dwBBitMask) {
		format = (desc.ddpfPixelFormat.dwRGBAlphaBitMask) ? Image::Format_R8G8B8A8 : Image::Format_R8G8B8;
	} else if(desc.ddpfPixelFormat.dwLuminanceBitMask || desc.ddpfPixelFormat.dwZBitMask) {
		format = (desc.ddpfPixelFormat.dwLuminanceAlphaBitMask) ? Image::Format_L8A8 : Image::Format_L8;
	} else if(desc.ddpfPixelFormat.dwRGBAlphaBitMask) {
		format = Image::Format_A8;
	} else {
		surface->Unlock(NULL);
		return false;
	}
	
	image.Create(desc.dwWidth, desc.dwHeight, format);
	
	DWORD dwRMask = desc.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = desc.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = desc.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = desc.ddpfPixelFormat.dwRGBAlphaBitMask | desc.ddpfPixelFormat.dwLuminanceAlphaBitMask;
	DWORD dwLMask = desc.ddpfPixelFormat.dwLuminanceBitMask | desc.ddpfPixelFormat.dwZBitMask;
	
	DWORD dwRShiftR = 0;
	DWORD dwGShiftR = 0;
	DWORD dwBShiftR = 0;
	DWORD dwAShiftR = 0;
	DWORD dwLShiftR = 0;
	
	for(DWORD dwMask = dwRMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwRShiftR++;
	for(DWORD dwMask = dwGMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwGShiftR++;
	for(DWORD dwMask = dwBMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwBShiftR++;
	for(DWORD dwMask = dwAMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwAShiftR++;
	for(DWORD dwMask = dwLMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwLShiftR++;
	
	unsigned char * data = image.GetData();
	
	size_t offset = desc.lPitch * (desc.dwHeight - 1);
	
	if(format == Image::Format_L8) {
		for(DWORD y = 0; y < desc.dwHeight; y++, offset -= desc.lPitch) {
			DWORD * src = (DWORD *)((BYTE*)desc.lpSurface + offset);
			for(DWORD x = 0; x < desc.dwWidth; x++, src++) {
				*data++ = (unsigned char)(((*src)&dwLMask) >> dwLShiftR);
			}
		}
	} else if(format == Image::Format_A8) {
		for(DWORD y = 0; y < desc.dwHeight; y++, offset -= desc.lPitch) {
			DWORD * src = (DWORD *)((BYTE*)desc.lpSurface + offset);
			for(DWORD x = 0; x < desc.dwWidth; x++, src++) {
				*data++ = (unsigned char)(((*src)&dwAMask) >> dwAShiftR);
			}
		}
	} else if(format == Image::Format_L8A8) {
		for(DWORD y = 0; y < desc.dwHeight; y++, offset -= desc.lPitch) {
			DWORD * src = (DWORD *)((BYTE*)desc.lpSurface + offset);
			for(DWORD x = 0; x < desc.dwWidth; x++, src++) {
				*data++ = (unsigned char)(((*src)&dwLMask) >> dwLShiftR);
				*data++ = (unsigned char)(((*src)&dwAMask) >> dwAShiftR);
			}
		}
	} else if(format == Image::Format_R8G8B8) {
		for(DWORD y = 0; y < desc.dwHeight; y++, offset -= desc.lPitch) {
			DWORD * src = (DWORD *)((BYTE*)desc.lpSurface + offset);
			for(DWORD x = 0; x < desc.dwWidth; x++, src++) {
				*data++ = (unsigned char)(((*src)&dwRMask) >> dwRShiftR);
				*data++ = (unsigned char)(((*src)&dwGMask) >> dwGShiftR);
				*data++ = (unsigned char)(((*src)&dwBMask) >> dwBShiftR);
			}
		}
	} else if(format == Image::Format_R8G8B8A8) {
		for(DWORD y = 0; y < desc.dwHeight; y++, offset -= desc.lPitch) {
			DWORD * src = (DWORD *)((BYTE*)desc.lpSurface + offset);
			for(DWORD x = 0; x < desc.dwWidth; x++, src++) {
				*data++ = (unsigned char)(((*src)&dwRMask) >> dwRShiftR);
				*data++ = (unsigned char)(((*src)&dwGMask) >> dwGShiftR);
				*data++ = (unsigned char)(((*src)&dwBMask) >> dwBShiftR);
				*data++ = (unsigned char)(((*src)&dwAMask) >> dwAShiftR);
			}
		}
	} else {
		arx_assert(false);
	}
	
	surface->Unlock(NULL);
	
	return true;
}

