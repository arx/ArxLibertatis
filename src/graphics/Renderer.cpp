
#include "core/Core.h" // TODO remove once init has been moved here
#include "graphics/VertexBuffer.h"
#include "graphics/Renderer.h"
#include "graphics/GraphicsUtility.h"
#include "graphics/GraphicsEnum.h"
#include "graphics/Math.h"
#include "io/Logger.h"

#include <list>

// TEMP: needed until all D3D code is isolated...
extern LPDIRECT3DDEVICE7 GDevice;


///////////////////////////////////////////////////////////////////////////////
// ARXToDX7 mapping tables - MOVE THAT SOMEWHERE ELSE!
///////////////////////////////////////////////////////////////////////////////
const D3DTEXTUREOP ARXToDX7TextureOp[] = {
						D3DTOP_DISABLE,						// OpDisable
						D3DTOP_SELECTARG1,					// OpSelectArg1,
						D3DTOP_SELECTARG2,					// OpSelectArg2,
						D3DTOP_MODULATE,					// OpModulate,
						D3DTOP_MODULATE2X,					// OpModulate2X,
						D3DTOP_MODULATE4X,					// OpModulate4X,
						D3DTOP_ADDSIGNED					// OpAddSigned
									};

const DWORD ARXToDX7TextureArg[] = {
						D3DTA_DIFFUSE,						// TexArgDiffuse,
						D3DTA_CURRENT,						// TexArgCurrent,						
						D3DTA_TEXTURE						// TexArgTexture,
									};

const D3DTEXTUREADDRESS ARXToDX7WrapMode[] = {
						D3DTADDRESS_WRAP,					// WrapRepeat,
						D3DTADDRESS_MIRROR,					// WrapMirror,
						D3DTADDRESS_CLAMP					// WrapClamp,
									};

D3DTEXTUREMAGFILTER ARXToDX7MagFilter[] = {
						D3DTFG_POINT,						// FilterNone - Invalid for magnification
						D3DTFG_POINT,						// FilterNearest,
						D3DTFG_LINEAR						// FilterLinear
									};

D3DTEXTUREMINFILTER ARXToDX7MinFilter[] = {
						D3DTFN_POINT,						// FilterNone - Invalid for minification
						D3DTFN_POINT,						// FilterNearest,
						D3DTFN_LINEAR,						// FilterLinear
									};

D3DTEXTUREMIPFILTER ARXToDX7MipFilter[] = {
						D3DTFP_NONE,						// FilterNone
						D3DTFP_POINT,						// FilterNearest,
						D3DTFP_LINEAR						// FilterLinear
									};


const D3DRENDERSTATETYPE ARXToDXRenderState[] = {
						D3DRENDERSTATE_ALPHABLENDENABLE,	// AlphaBlending,
						D3DRENDERSTATE_COLORKEYENABLE,		// ColorKey,
						D3DRENDERSTATE_ZENABLE,				// DepthTest,
						D3DRENDERSTATE_ZWRITEENABLE,		// DepthWrite,
						D3DRENDERSTATE_FOGENABLE,			// Fog,
						D3DRENDERSTATE_LIGHTING,            // Lighting,
						D3DRENDERSTATE_ZBIAS				// ZBias
										};
				   
const D3DCMPFUNC ARXToDXPixelCompareFunc[] = {
						D3DCMP_NEVER,                       // CmpNever,
						D3DCMP_LESS,                        // CmpLess,
						D3DCMP_EQUAL,                       // CmpEqual,
						D3DCMP_LESSEQUAL,                   // CmpLessEqual,
						D3DCMP_GREATER,                     // CmpGreater,
						D3DCMP_NOTEQUAL,                    // CmpNotEqual,
						D3DCMP_GREATEREQUAL,                // CmpGreaterEqual,
						D3DCMP_ALWAYS                       // CmpAlways
										};

const D3DBLEND ARXToDXPixelBlendingFactor[] =  {
						D3DBLEND_ZERO,                      // BlendZero,
						D3DBLEND_ONE,                       // BlendOne,
						D3DBLEND_SRCCOLOR,                  // BlendSrcColor,
						D3DBLEND_SRCALPHA,                  // BlendSrcAlpha,
						D3DBLEND_INVSRCCOLOR,               // BlendInvSrcColor,
						D3DBLEND_INVSRCALPHA,               // BlendInvSrcAlpha,
						D3DBLEND_SRCALPHASAT,               // BlendSrcAlphaSaturate,
						D3DBLEND_DESTCOLOR,                 // BlendDstColor,
						D3DBLEND_DESTALPHA,                 // BlendDstAlpha,
						D3DBLEND_INVDESTCOLOR,              // BlendInvDstColor,
						D3DBLEND_INVDESTALPHA               // BlendInvDstAlpha    
											};

const D3DCULL ARXToDXCullMode[] = {
						D3DCULL_NONE,						// CullNone,
						D3DCULL_CW,							// CullCW,
						D3DCULL_CCW							// CullCCW,
										};

const D3DFILLMODE ARXToDXFillMode[] = {
						D3DFILL_POINT,						// FillPoint,
						D3DFILL_WIREFRAME,					// FillWireframe,
						D3DFILL_SOLID						// FillSolid,
										};

const D3DFOGMODE ARXToDXFogMode[] = {
						D3DFOG_NONE,						// FogNone,
						D3DFOG_EXP,							// FogExp,
						D3DFOG_EXP2,						// FogExp2,
						D3DFOG_LINEAR						// FogLinear
									};

static const D3DPRIMITIVETYPE ARXToDXPrimitiveType[] = {
	D3DPT_TRIANGLELIST,  // TriangleList
	D3DPT_TRIANGLESTRIP, // TriangleStrip
	D3DPT_TRIANGLEFAN,   // TriangleFan
	D3DPT_LINELIST,      // LineList
	D3DPT_LINESTRIP      // LineStrip
};

static const DWORD ARXToDXBufferFlags[] = {
	0, // none
	DDLOCK_DISCARDCONTENTS,                     // DiscardContents
	DDLOCK_NOOVERWRITE,                         // NoOverwrite
	DDLOCK_DISCARDCONTENTS | DDLOCK_NOOVERWRITE // DiscardContents | NoOverwrite
};

///////////////////////////////////////////////////////////////////////////////
// DX7Texture2D - MOVE THAT SOMEWHERE ELSE!
///////////////////////////////////////////////////////////////////////////////
class DX7Texture2D : public Texture2D
{
	friend class Renderer;

public:
	LPDIRECTDRAWSURFACE7 GetTextureID() const
	{
		return m_pddsSurface;
	}

private:
	DX7Texture2D();
	virtual ~DX7Texture2D();

	virtual bool Create();
	virtual void Upload();
	virtual void Destroy();

	static HRESULT CALLBACK TextureSearchCallback( DDPIXELFORMAT* pddpf, VOID* param );

private:
	void CopyNextMipLevel(LPDIRECTDRAWSURFACE7 pddsDst, LPDIRECTDRAWSURFACE7 pddsSrc);

	LPDIRECTDRAWSURFACE7 m_pddsSurface;
};

// TODO-DX7: This should really be an intrusive list!
std::list<DX7Texture2D*> g_Textures2D;

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
	
	DWORD flags = DDPF_RGB | ((ddsd->dwFlags & DDSD_CKSRCBLT) ? 0 : DDPF_ALPHAPIXELS);
	
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
	arx_assert(mWidth != 0);
	arx_assert(mHeight != 0);

	HRESULT hr;

	// Get the device caps so we can check if the device has any constraints
	// when using textures
	D3DDEVICEDESC7 ddDesc;
	if( FAILED( GDevice->GetCaps( &ddDesc ) ) )
		return false;

	// Setup the new surface desc for the texture. Note how we are using the
	// texture manage attribute, so Direct3D does alot of dirty work for us
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
	ddsd.dwSize          = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
	ddsd.dwWidth         = mWidth;
	ddsd.dwHeight        = mHeight;

	// Specify black as our color key
	if(!mImage.HasAlpha()) {
		ddsd.dwFlags |= DDSD_CKSRCBLT;
		ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = RGB(0, 0, 0);
		ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = RGB(0, 0, 0);
	}

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
	GDevice->EnumTextureFormats( TextureSearchCallback, &ddsd);
	if( 0L == ddsd.ddpfPixelFormat.dwRGBBitCount )
		return false;

	// Get the device's render target, so we can then use the render target to
	// get a ptr to a DDraw object. We need the DirectDraw interface for
	// creating surfaces.
	LPDIRECTDRAWSURFACE7 pddsRender;
	LPDIRECTDRAW7        pDD;
	GDevice->GetRenderTarget( &pddsRender );
	pddsRender->GetDDInterface( (VOID**)&pDD );
	pddsRender->Release();

	// Create a new surface for the texture
	if( FAILED( hr = pDD->CreateSurface( &ddsd, &m_pddsSurface, NULL ) ) )
	{
		pDD->Release();
		return false;
	}

	// Store texture size
	mWidth = ddsd.dwWidth;
	mHeight = ddsd.dwHeight;

	// Done with DDraw
	pDD->Release();

	// Set color key
	if( m_pddsSurface )
		m_pddsSurface->SetColorKey(DDCKEY_SRCBLT, &ddsd.ddckCKSrcBlt);

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

	arx_assert_msg(descDst.dwWidth == (descSrc.dwWidth >> 1), "src width = %d, dst width = %d (%s)", descSrc.dwWidth, descDst.dwWidth, mFileName.c_str());
	arx_assert_msg(descDst.dwHeight == (descSrc.dwHeight >> 1), "src height = %d, dst height = %d (%s)", descSrc.dwHeight, descDst.dwHeight, mFileName.c_str());

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


///////////////////////////////////////////////////////////////////////////////
// DX7TextureStage - MOVE THAT SOMEWHERE ELSE!
///////////////////////////////////////////////////////////////////////////////
class DX7TextureStage : public TextureStage
{
public:
	DX7TextureStage(unsigned int textureStage);

	void SetTexture( Texture* pTexture );
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
};

DX7TextureStage::DX7TextureStage(unsigned int textureStage)
	: TextureStage(textureStage)
{
}

void DX7TextureStage::SetColorOp(TextureOp textureOp)
{
	// TODO-DX7: Cache states
	DWORD colorOp = ARXToDX7TextureOp[textureOp];
	GDevice->SetTextureStageState(mStage, D3DTSS_COLOROP, colorOp);
}

void DX7TextureStage::SetColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2)
{
	// TODO-DX7: Cache states
	DWORD colorOp = ARXToDX7TextureOp[textureOp];
	GDevice->SetTextureStageState(mStage, D3DTSS_COLOROP, colorOp);

	if(textureOp != TextureStage::OpDisable)
	{
		if(textureOp != TextureStage::OpSelectArg2)
		{
			DWORD colorArg1 = ARXToDX7TextureArg[texArg1 & TextureStage::ArgMask];
			colorArg1 |= (texArg1 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GDevice->SetTextureStageState(mStage, D3DTSS_COLORARG1, colorArg1);
		}

		if(textureOp != TextureStage::OpSelectArg1)
		{
			DWORD colorArg2 = ARXToDX7TextureArg[texArg2 & TextureStage::ArgMask];
			colorArg2 |= (texArg2 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GDevice->SetTextureStageState(mStage, D3DTSS_COLORARG2, colorArg2);
		}
	}
}

void DX7TextureStage::SetAlphaOp(TextureOp textureOp)
{
	// TODO-DX7: Cache states
	DWORD colorOp = ARXToDX7TextureOp[textureOp];
	GDevice->SetTextureStageState(mStage, D3DTSS_ALPHAOP, colorOp);
}

void DX7TextureStage::SetAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2)
{
	// TODO-DX7: Cache states
	DWORD alphaOp = ARXToDX7TextureOp[textureOp];
	GDevice->SetTextureStageState(mStage, D3DTSS_ALPHAOP, alphaOp);

	if(textureOp != TextureStage::OpDisable)
	{
		if(textureOp != TextureStage::OpSelectArg2)
		{
			DWORD alphaArg1 = ARXToDX7TextureArg[texArg1 & TextureStage::ArgMask];
			alphaArg1 |= (texArg1 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GDevice->SetTextureStageState(mStage, D3DTSS_ALPHAARG1, alphaArg1);
		}

		if(textureOp != TextureStage::OpSelectArg1)
		{
			DWORD alphaArg2 = ARXToDX7TextureArg[texArg2 & TextureStage::ArgMask];
			alphaArg2 |= (texArg2 & TextureStage::ArgComplement) ? D3DTA_COMPLEMENT : 0;
			GDevice->SetTextureStageState(mStage, D3DTSS_ALPHAARG2, alphaArg2);
		}
	}
}

void DX7TextureStage::SetTexture( Texture* pTexture )
{
	// TODO-DX7: Support multiple texture types
	DX7Texture2D* tex = (DX7Texture2D*)pTexture;

	GDevice->SetTexture(mStage, tex->GetTextureID());
}

void DX7TextureStage::ResetTexture()
{
	GDevice->SetTexture(mStage, 0);
}

void DX7TextureStage::SetWrapMode(TextureStage::WrapMode wrapMode)
{
	GDevice->SetTextureStageState(mStage, D3DTSS_ADDRESS, ARXToDX7WrapMode[wrapMode]);
}

void DX7TextureStage::SetMinFilter(FilterMode filterMode)
{
	arx_assert_msg(filterMode != TextureStage::FilterNone, "Invalid minification filter");
	GDevice->SetTextureStageState(mStage, D3DTSS_MINFILTER, ARXToDX7MinFilter[filterMode]);
}

void DX7TextureStage::SetMagFilter(FilterMode filterMode)
{
	arx_assert_msg(filterMode != TextureStage::FilterNone, "Invalid magnification filter");
	GDevice->SetTextureStageState(mStage, D3DTSS_MAGFILTER, ARXToDX7MagFilter[filterMode]);
}

void DX7TextureStage::SetMipFilter(FilterMode filterMode)
{
	D3DTEXTUREMIPFILTER mipFilter = ARXToDX7MipFilter[filterMode];
	GDevice->SetTextureStageState(mStage, D3DTSS_MIPFILTER, mipFilter);
}

void DX7TextureStage::SetMipMapLODBias(float bias) {
	if(GetKeyState(VK_F12) != 0) { // TODO what kind of hack is this?
		float val = 0;
		GDevice->SetTextureStageState(mStage, D3DTSS_MIPMAPLODBIAS, *((LPDWORD)(&val)));
	} else {
		GDevice->SetTextureStageState(mStage, D3DTSS_MIPMAPLODBIAS, *((LPDWORD)(&bias)));
	}
}

void DX7TextureStage::SetTextureCoordIndex(int texCoordIdx)
{
	GDevice->SetTextureStageState(mStage, D3DTSS_TEXCOORDINDEX, texCoordIdx);
}

///////////////////////////////////////////////////////////////////////////////
// Renderer - DX7 implementation
///////////////////////////////////////////////////////////////////////////////

Renderer* GRenderer = 0;
Renderer  RendererInstance;

Renderer::Renderer()
{
	GRenderer = this;
}

void Renderer::Initialize()
{
	D3DDEVICEDESC7 devicedesc;
	GDevice->GetCaps(&devicedesc);

	///////////////////////////////////////////////////////////////////////////
	// Initialize filtering options depending on the card capabilities...

	// Minification
	if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
		ARXToDX7MinFilter[TextureStage::FilterLinear] = D3DTFN_ANISOTROPIC;
	else if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
		ARXToDX7MinFilter[TextureStage::FilterLinear] = D3DTFN_LINEAR;
	else
		ARXToDX7MinFilter[TextureStage::FilterLinear] = D3DTFN_POINT;
		
	// Magnification
	if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
		ARXToDX7MagFilter[TextureStage::FilterLinear] = D3DTFG_ANISOTROPIC;
	else if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
		ARXToDX7MagFilter[TextureStage::FilterLinear] = D3DTFG_LINEAR;
	else
		ARXToDX7MagFilter[TextureStage::FilterLinear] = D3DTFG_POINT;

	// Mipmapping
	if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR)
		ARXToDX7MipFilter[TextureStage::FilterLinear] = D3DTFP_LINEAR;
	else if (devicedesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT)
		ARXToDX7MipFilter[TextureStage::FilterLinear] = D3DTFP_POINT;
	else
		ARXToDX7MipFilter[TextureStage::FilterLinear] = D3DTFP_NONE;

	///////////////////////////////////////////////////////////////////////////
	// Texture stages...

	m_TextureStages.resize(devicedesc.wMaxTextureBlendStages, 0);

	DWORD maxAnisotropy = 1;
	if( devicedesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANISOTROPY )
        maxAnisotropy = devicedesc.dwMaxAnisotropy;

	for(size_t i = 0; i < m_TextureStages.size(); ++i)
	{
		m_TextureStages[i] = new DX7TextureStage(i);
		GDevice->SetTextureStageState(i, D3DTSS_MAXANISOTROPY, maxAnisotropy);

        // Set default state
		m_TextureStages[i]->SetWrapMode(TextureStage::WrapRepeat);
		m_TextureStages[i]->SetMinFilter(TextureStage::FilterLinear);
		m_TextureStages[i]->SetMagFilter(TextureStage::FilterLinear);
		m_TextureStages[i]->SetMipFilter(TextureStage::FilterLinear);
	}

	SetRenderState(Renderer::ColorKey, true);

	// Clear screen
	Clear(Renderer::ColorBuffer | Renderer::DepthBuffer);
}

bool Renderer::BeginScene()
{
	return GDevice->BeginScene() == D3D_OK;
}

bool Renderer::EndScene()
{
	return GDevice->EndScene() == D3D_OK;
}

void Renderer::SetViewMatrix(const EERIEMATRIX& matView)
{
	GDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX*)&matView);
}

void Renderer::SetViewMatrix(const Vec3f & vPosition, const Vec3f & vDir, const Vec3f & vUp) {
	
	D3DMATRIX matView;
	D3DVECTOR pos(vPosition.x, vPosition.y, vPosition.z);
	D3DVECTOR at(vDir.x, vDir.y, vDir.z);
	D3DVECTOR up(vUp.x, vUp.y, vUp.z);
	
	D3DUtil_SetViewMatrix(matView, pos, at, up);
	GDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &matView);
}

void Renderer::GetViewMatrix(EERIEMATRIX& matView) const
{
	GDevice->GetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX*)&matView);
}

void Renderer::SetProjectionMatrix(const EERIEMATRIX& matProj)
{
	GDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX*)&matProj);
}

void Renderer::GetProjectionMatrix(EERIEMATRIX& matProj) const
{
	GDevice->GetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX*)&matProj);
}

Renderer::~Renderer()
{
	for(size_t i = 0; i < m_TextureStages.size(); ++i)
	{
		delete m_TextureStages[i];
	}

	GRenderer = 0;
}

void Renderer::ReleaseAllTextures()
{
	std::list<DX7Texture2D*>::iterator it;
	for(it = g_Textures2D.begin(); it != g_Textures2D.end(); ++it)
	{
		(*it)->Destroy();
	}
}

void Renderer::RestoreAllTextures()
{
	std::list<DX7Texture2D*>::iterator it;
	for(it = g_Textures2D.begin(); it != g_Textures2D.end(); ++it)
	{
		(*it)->Restore();
	}
}

Texture2D* Renderer::CreateTexture2D()
{
	return new DX7Texture2D();
}

void Renderer::SetRenderState(Renderer::RenderState renderState, bool enable)
{
	GDevice->SetRenderState(ARXToDXRenderState[renderState], enable ? TRUE : FALSE);
}

void Renderer::SetAlphaFunc(Renderer::PixelCompareFunc func, float fef)
{
	GDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)fef*255);
	GDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, ARXToDXPixelCompareFunc[func]);
}

void Renderer::SetBlendFunc(Renderer::PixelBlendingFactor srcFactor, Renderer::PixelBlendingFactor dstFactor)
{
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  ARXToDXPixelBlendingFactor[srcFactor]);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, ARXToDXPixelBlendingFactor[dstFactor]);
}

void Renderer::SetViewport(const Rect & viewport) {
	D3DVIEWPORT7 tmpViewport = { viewport.left, viewport.top, viewport.width(), viewport.height(), 0.f, 1.f };
	GDevice->SetViewport(&tmpViewport);
}

Rect Renderer::GetViewport() {
	
	D3DVIEWPORT7 tmpViewport;
	GDevice->GetViewport(&tmpViewport);
	
	return Rect(Vec2i(tmpViewport.dwX, tmpViewport.dwY), tmpViewport.dwWidth, tmpViewport.dwHeight);
}

void Renderer::SetCulling(Renderer::CullingMode mode)
{
	GDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, ARXToDXCullMode[mode]);
}

void Renderer::SetDepthBias(int depthBias)
{
	GDevice->SetRenderState(D3DRENDERSTATE_ZBIAS, depthBias);
}

void Renderer::SetFillMode(Renderer::FillMode mode)
{
	GDevice->SetRenderState(D3DRENDERSTATE_FILLMODE, ARXToDXFillMode[mode]);
}

void DX7MatrixIdentity(D3DMATRIX *pout)
{
	pout->_11 = 1;
	pout->_12 = 0;
	pout->_13 = 0;
	pout->_14 = 0;
	pout->_21 = 0;
	pout->_22 = 1;
	pout->_23 = 0;
	pout->_24 = 0;
	pout->_31 = 0;
	pout->_32 = 0;
	pout->_33 = 1;
	pout->_34 = 0;
	pout->_41 = 0;
	pout->_42 = 0;
	pout->_43 = 0;
	pout->_44 = 1;
}

D3DMATRIX* DX7MatrixOrthoOffCenterLH(D3DMATRIX *pout, float l, float r, float b, float t, float zn, float zf)
{
	DX7MatrixIdentity(pout);
	pout->_11 = 2.0f / (r - l);
	pout->_22 = 2.0f / (t - b);
	pout->_33 = 1.0f / (zf -zn);
	pout->_41 = -1.0f -2.0f *l / (r - l);
	pout->_42 = 1.0f + 2.0f * t / (b - t);
	pout->_43 = zn / (zn -zf);
	return pout;
}

D3DMATRIX g_MatProj;
D3DMATRIX g_MatWorld;
D3DMATRIX g_MatView;

void Renderer::Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	GDevice->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &g_MatProj);
	GDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &g_MatWorld);
	GDevice->GetTransform(D3DTRANSFORMSTATE_VIEW, &g_MatView);
	
	D3DMATRIX matOrtho;
	DX7MatrixOrthoOffCenterLH(&matOrtho, left, right, bottom, top, zNear, zFar);

	D3DMATRIX matIdentity;
	DX7MatrixIdentity(&matIdentity);

	GDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matOrtho);
	GDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matIdentity);
	GDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &matIdentity);
}

void Renderer::End2DProjection()
{
	GDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &g_MatProj);
	GDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &g_MatWorld);
	GDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &g_MatView);
}

void Renderer::Clear(BufferFlags bufferFlags, Color clearColor, float clearDepth, size_t nrects, Rect * rects) {
	
	DWORD clearTargets = 0;
	clearTargets |= (bufferFlags & ColorBuffer) ? D3DCLEAR_TARGET : 0;
	clearTargets |= (bufferFlags & DepthBuffer) ?  D3DCLEAR_ZBUFFER : 0;
	clearTargets |= (bufferFlags & StencilBuffer) ? D3DCLEAR_STENCIL : 0;
	
	// TODO check that struct layout is the same using static assert and just cast?
	static D3DRECT d3drects[2];
	arx_assert(nrects <= 2);
	for(size_t i = 0; i < nrects; i++) {
		d3drects[i].x1 = rects[i].left;
		d3drects[i].y1 = rects[i].top;
		d3drects[i].x2 = rects[i].right;
		d3drects[i].y2 = rects[i].bottom;
	}
	
	GDevice->Clear(DWORD(nrects), d3drects, clearTargets, clearColor.toBGRA(), clearDepth, 0);
}

void Renderer::SetFogColor(Color color)
{
	GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, color.toBGRA());
}

void Renderer::SetFogParams(Renderer::FogMode fogMode, float fogStart, float fogEnd, float fogDensity)
{
	GDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, ARXToDXFogMode[fogMode]);
	GDevice->SetRenderState(D3DRENDERSTATE_FOGSTART,	*((LPDWORD) (&fogStart)));
	GDevice->SetRenderState(D3DRENDERSTATE_FOGEND,		*((LPDWORD) (&fogEnd)));
	GDevice->SetRenderState(D3DRENDERSTATE_FOGDENSITY,  *((LPDWORD) (&fogDensity)));
}

void Renderer::SetAntialiasing(bool enable)
{
	if(enable)
	{
		D3DDEVICEDESC7 ddDesc;
		GDevice->GetCaps(&ddDesc);

		if(ddDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
			GDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT);
		else if(ddDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT)
			GDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTDEPENDENT);
	}
	else
	{
		GDevice->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE);
	}
}

unsigned int Renderer::GetTextureStageCount() const
{
	return m_TextureStages.size();
}

TextureStage* Renderer::GetTextureStage(unsigned int textureStage)
{
	if(textureStage < m_TextureStages.size())
		return m_TextureStages[textureStage];

	return 0;
}

void Renderer::ResetTexture(unsigned int textureStage)
{
	GetTextureStage(textureStage)->ResetTexture();
}

void Renderer::SetTexture(unsigned int textureStage, Texture* pTexture)
{
	GetTextureStage(textureStage)->SetTexture(pTexture);
}

void Renderer::SetTexture(unsigned int textureStage, TextureContainer* pTextureContainer)
{
	if(pTextureContainer && pTextureContainer->m_pTexture)
		GetTextureStage(textureStage)->SetTexture(pTextureContainer->m_pTexture);
	else
		GetTextureStage(textureStage)->ResetTexture();
}

float Renderer::GetMaxAnisotropy() const
{
	return 8.0f; // TODO-DX7
}

void Renderer::DrawTexturedRect( float pX, float pY, float pW, float pH, float pUStart, float pVStart, float pUEnd, float pVEnd, Color pCol )
{
	D3DLVERTEX rect[4];

	pX -= 0.5f;
	pY -= 0.5f;

	const D3DCOLOR DIFFUSE  = pCol.toBGR();
	const D3DCOLOR SPECULAR = 0;

	rect[0].x = pX;
	rect[0].y = pY;
	rect[0].z = 0;
	rect[0].tu = pUStart;
	rect[0].tv = pVStart;
	rect[0].color = DIFFUSE;
	rect[0].specular = SPECULAR;

	rect[1].x = pX+pW;
	rect[1].y = pY;
	rect[1].z = 0;
	rect[1].tu = pUEnd;
	rect[1].tv = pVStart;
	rect[1].color = DIFFUSE;
	rect[1].specular = SPECULAR;

	rect[2].x = pX;
	rect[2].y = pY+pH;
	rect[2].z = 0;
	rect[2].tu = pUStart;
	rect[2].tv = pVEnd;
	rect[2].color = DIFFUSE;
	rect[2].specular = SPECULAR;

	rect[3].x = pX+pW;
	rect[3].y = pY+pH;
	rect[3].z = 0;
	rect[3].tu = pUEnd;
	rect[3].tv = pVEnd;
	rect[3].color = DIFFUSE;
	rect[3].specular = SPECULAR;
	
	GDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_LVERTEX, &rect, 4, 0);
}

template <class Vertex>
class DX7VertexBuffer : public VertexBuffer<Vertex> {
	
public:
	
	DX7VertexBuffer(DWORD format, size_t capacity) : VertexBuffer<Vertex>(capacity) {
		
		D3DVERTEXBUFFERDESC d3dvbufferdesc;
		d3dvbufferdesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
		
		d3dvbufferdesc.dwCaps = D3DVBCAPS_WRITEONLY;
		if(!(danaeApp.m_pDeviceInfo->ddDeviceDesc.dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)) {
			d3dvbufferdesc.dwCaps |= D3DVBCAPS_SYSTEMMEMORY;
		}
		
		d3dvbufferdesc.dwFVF = format;
		d3dvbufferdesc.dwNumVertices = capacity;
		
		HRESULT hr = danaeApp.m_pD3D->CreateVertexBuffer(&d3dvbufferdesc, &vb, 0);
		arx_assert_msg(SUCCEEDED(hr), "error creating vertex buffer: %08x", hr);
		ARX_UNUSED(hr);
		
	}
	
	void setData(const Vertex * vertices, size_t count, size_t offset = 0, BufferFlags flags = 0) {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		Vertex * dest;
		
		HRESULT hr = vb->Lock(DDLOCK_WRITEONLY | ARXToDXBufferFlags[flags], (LPVOID*)&dest, NULL);
		arx_assert_msg(SUCCEEDED(hr), "error locking vertex buffer: %08x", hr);
		ARX_UNUSED(hr);
		
		memcpy(dest + offset, vertices, count * sizeof(Vertex));
		
		vb->Unlock();
	}
	
	Vertex * lock(BufferFlags flags = 0) {
		
		Vertex * dest;
		
		HRESULT hr = vb->Lock(DDLOCK_WRITEONLY | ARXToDXBufferFlags[flags], (LPVOID*)&dest, NULL);
		arx_assert_msg(SUCCEEDED(hr), "error locking vertex buffer: %08x", hr);
		ARX_UNUSED(hr);
		
		return dest;
	}
	
	void unlock() {
		vb->Unlock();
	}
	
	void draw(Renderer::Primitive primitive, size_t count, size_t offset = 0) const {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		
		D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];
		HRESULT hr = GDevice->DrawPrimitiveVB(type, vb, offset, count, 0);
		arx_assert_msg(SUCCEEDED(hr), "DrawPrimitiveVB failed: %08x", hr);
		ARX_UNUSED(hr);
	}
	
	
	void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		arx_assert(indices != NULL);
		
		D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];
		HRESULT hr = GDevice->DrawIndexedPrimitiveVB(type, vb, offset, count, indices, nbindices, 0);
		arx_assert_msg(SUCCEEDED(hr), "DrawIndexedPrimitiveVB failed: %08x", hr);
		ARX_UNUSED(hr);
	}
	
	~DX7VertexBuffer() {
		vb->Release();
	};
	
private:
	
	LPDIRECT3DVERTEXBUFFER7 vb;
	
};

VertexBuffer<D3DTLVERTEX> * Renderer::createVertexBufferTL(size_t capacity, BufferUsage usage) {
	ARX_UNUSED(usage);
	return new DX7VertexBuffer<D3DTLVERTEX>(D3DFVF_TLVERTEX, capacity);
}

VertexBuffer<SMY_D3DVERTEX> * Renderer::createVertexBuffer(size_t capacity, BufferUsage usage) {
	ARX_UNUSED(usage);
	const DWORD format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXTUREFORMAT2;
	return new DX7VertexBuffer<SMY_D3DVERTEX>(format, capacity);
}

VertexBuffer<SMY_D3DVERTEX3> * Renderer::createVertexBuffer3(size_t capacity, BufferUsage usage) {
	ARX_UNUSED(usage);
	const DWORD format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX3 | D3DFVF_TEXTUREFORMAT2;
	return new DX7VertexBuffer<SMY_D3DVERTEX3>(format, capacity);
}

void Renderer::drawIndexed(Primitive primitive, const D3DTLVERTEX * vertices, size_t nvertices, unsigned short * indices, size_t nindices) {
	
	arx_assert(vertices != NULL && indices != NULL);
	
	D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];
	HRESULT hr = GDevice->DrawIndexedPrimitive(type, D3DFVF_TLVERTEX, (LPVOID)vertices, (DWORD)nvertices, (LPWORD)indices, (DWORD)nindices, 0);
	arx_assert_msg(SUCCEEDED(hr), "DrawIndexedPrimitive failed: %08x", hr);
	ARX_UNUSED(hr);
	
}

static bool downloadSurface(LPDIRECTDRAWSURFACE7 surface, Image & image) {
	
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

bool Renderer::getSnapshot(Image & image) {
	
	LPDIRECTDRAWSURFACE7 pddsRender;
	HRESULT hr = GDevice->GetRenderTarget(&pddsRender);
	if(FAILED(hr)) {
		LogError << "GetRenderTarget failed: " << hr;
		return false;
	}
	
	bool ret = downloadSurface(pddsRender, image);
	
	pddsRender->Release();
	
	return ret;
}

bool Renderer::getSnapshot(Image& image, size_t width, size_t height) {
	
	LPDIRECTDRAWSURFACE7 pddsRender;
	HRESULT hr = GDevice->GetRenderTarget(&pddsRender);
	if(FAILED(hr)) {
		LogError << "GetRenderTarget failed: " << hr;
		return false;
	}
	
	DDSURFACEDESC2 desc;
	memset((void *)&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);
	pddsRender->GetSurfaceDesc(&desc);
	
	LPDIRECTDRAWSURFACE7 tempSurface = NULL;
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	desc.dwWidth = width;
	desc.dwHeight = height;
	desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	
	LPDIRECTDRAW7 pDD;
	pddsRender->GetDDInterface((VOID **)&pDD);
	hr = pDD->CreateSurface(&desc, &tempSurface, NULL);
	pDD->Release();
	if(FAILED(hr)) {
		pddsRender->Release();
		return false;
	}
	
	tempSurface->Blt(NULL, pddsRender, NULL, DDBLT_WAIT, NULL);
	pddsRender->Release();
	
	bool ret = downloadSurface(tempSurface, image);
	
	tempSurface->Release();
	
	return ret;
}

