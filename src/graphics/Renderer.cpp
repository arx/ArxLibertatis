#include "graphics/Renderer.h"

#include <list>
#include <d3d.h>


// TEMP: needed until all D3D code is isolated...
extern LPDIRECT3DDEVICE7 GDevice;


///////////////////////////////////////////////////////////////////////////////
// ARXToDX7 mapping tables - MOVE THAT SOMEWHERE ELSE!
///////////////////////////////////////////////////////////////////////////////
const D3DTEXTUREOP ARXToDX7TextureOp[] = {
						D3DTOP_SELECTARG1,					// TexOpSelectArg1,
						D3DTOP_SELECTARG2,					// TexOpSelectArg2,
						D3DTOP_MODULATE						// TexOpModulate
									};

const DWORD ARXToDX7TextureArg[] = {
						D3DTA_CURRENT,						// TexArgCurrent,
						D3DTA_DIFFUSE,						// TexArgDiffuse,
						D3DTA_TEXTURE,						// TexArgTexture
									};

const D3DTEXTUREADDRESS ARXToDX7WrapMode[] = {
                        D3DTADDRESS_CLAMP,					// Wrap_Clamp,
                        D3DTADDRESS_WRAP					// Wrap_Repeat
                                    };

const D3DTEXTUREMAGFILTER ARXToDX7MagFilter[] = {
                        D3DTFG_POINT,						// MagFilter_Nearest,
                        D3DTFG_LINEAR						// MagFilter_Linear
                                    };

const D3DTEXTUREMINFILTER ARXToDX7MinFilter[] = {
                        D3DTFN_POINT,						// MinFilter_Nearest,
                        D3DTFN_LINEAR,						// MinFilter_Linear
						D3DTFN_POINT,						// MinFilter_NearestMipmapNearest,
						D3DTFN_LINEAR,						// MinFilter_LinearMipmapNearest,
						D3DTFN_POINT,						// MinFilter_NearestMipmapLinear,
						D3DTFN_LINEAR						// MinFilter_LinearMipmapLinear,
                                    };

const D3DTEXTUREMIPFILTER ARXToDX7MipFilter[] = {
						D3DTFP_NONE,						// MinFilter_Nearest,
						D3DTFP_NONE,						// MinFilter_Linear,
						D3DTFP_POINT,						// MinFilter_NearestMipmapNearest,
						D3DTFP_POINT,						// MinFilter_LinearMipmapNearest,
						D3DTFP_LINEAR,						// MinFilter_NearestMipmapLinear,
						D3DTFP_LINEAR						// MinFilter_LinearMipmapLinear,
									};


const D3DRENDERSTATETYPE ARXToDXRenderState[] = {
						D3DRENDERSTATE_ALPHABLENDENABLE,	// Blend,
						D3DRENDERSTATE_ZENABLE,				// DepthTest
						D3DRENDERSTATE_ZWRITEENABLE,		// DepthMask
						D3DRENDERSTATE_LIGHTING,            // Lighting,
                                        };
                   
const unsigned int ARXToDXPixelCompareFunc[] = {
						D3DCMP_NEVER,                       // CmpNever,
						D3DCMP_LESS,                        // CmpLess,
						D3DCMP_EQUAL,                       // CmpEqual,
						D3DCMP_LESSEQUAL,                   // CmpLessEqual,
						D3DCMP_GREATER,                     // CmpGreater,
						D3DCMP_NOTEQUAL,                    // CmpNotEqual,
						D3DCMP_GREATEREQUAL,                // CmpGreaterEqual,
						D3DCMP_ALWAYS                       // CmpAlways
                                        };

const unsigned int ARXToDXPixelBlendingFactor[] =  {
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

const unsigned int ARXToDXCullMode[] = {
						D3DCULL_NONE,						// CullNone,
						D3DCULL_CW,							// CullCW,
						D3DCULL_CCW							// CullCCW,
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

    virtual void Init();
    virtual void Kill();

	LPDIRECTDRAWSURFACE7 CreateTexture();
	void LoadTextureFromImage();

	static HRESULT CALLBACK TextureSearchCallback( DDPIXELFORMAT* pddpf, VOID* param );

private:
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
	Kill();
}

void DX7Texture2D::Init()
{
	if(m_pddsSurface == 0)
	{
		m_pddsSurface = CreateTexture();
		
		if(m_pddsSurface)
			LoadTextureFromImage();
	}
}

void DX7Texture2D::Kill()
{
	if(m_pddsSurface)
	{ 
		m_pddsSurface->Release();
		m_pddsSurface = 0;
	}
}

HRESULT CALLBACK DX7Texture2D::TextureSearchCallback( DDPIXELFORMAT* pddpf, VOID* param )
{
    if (NULL == pddpf || NULL == param)
		return DDENUMRET_OK;

	// Since we'll get rid of this DX7 renderer as soon as possible, I won't invest any more time figuring out the optimal texture matches
	// Always use 32 bit RGBA (8-8-8-8)... with no support for DXT (arx doesn't use them yet anyway!
	if(((pddpf->dwFlags & (DDPF_RGB|DDPF_ALPHAPIXELS)) == (DDPF_RGB|DDPF_ALPHAPIXELS)) && (pddpf->dwRGBBitCount == 32))
	{
		DDPIXELFORMAT * destDDPF = (DDPIXELFORMAT*)param;

		// We found a good match. Copy the current pixel format to our output parameter
		memcpy( destDDPF, pddpf, sizeof(DDPIXELFORMAT) );

		// Return with DDENUMRET_CANCEL to end enumeration.
		return DDENUMRET_CANCEL;
	}	

	return DDENUMRET_OK;
}

LPDIRECTDRAWSURFACE7 DX7Texture2D::CreateTexture()
{
    LPDIRECTDRAWSURFACE7 pddsTexture;
    HRESULT hr;

    // Get the device caps so we can check if the device has any constraints
    // when using textures
    D3DDEVICEDESC7 ddDesc;
    if( FAILED( GDevice->GetCaps( &ddDesc ) ) )
        return NULL;

    // Setup the new surface desc for the texture. Note how we are using the
    // texture manage attribute, so Direct3D does alot of dirty work for us
    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
    ddsd.dwSize          = sizeof(DDSURFACEDESC2);
    ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
    ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
    ddsd.dwWidth         = mWidth;
    ddsd.dwHeight        = mHeight;

	if(mHasMipmaps)
	{
		ddsd.dwFlags        |= DDSD_MIPMAPCOUNT;
		ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	}

    // Turn on texture management for hardware devices
    if( ddDesc.deviceGUID == IID_IDirect3DHALDevice )
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    else if( ddDesc.deviceGUID == IID_IDirect3DTnLHalDevice )
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
    else
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

    // Adjust width and height, if the driver requires it
    if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2 )
    {
        for( ddsd.dwWidth=1;  mWidth>ddsd.dwWidth;   ddsd.dwWidth<<=1 );
        for( ddsd.dwHeight=1; mHeight>ddsd.dwHeight; ddsd.dwHeight<<=1 );
    }
    if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
    {
        if( ddsd.dwWidth > ddsd.dwHeight ) ddsd.dwHeight = ddsd.dwWidth;
        else                               ddsd.dwWidth  = ddsd.dwHeight;
    }

	// Limit max texture sizes, if the driver can't handle large textures
	DWORD dwMaxWidth  = ddDesc.dwMaxTextureWidth;
	DWORD dwMaxHeight = ddDesc.dwMaxTextureHeight;

	if (ddsd.dwWidth > dwMaxWidth)
		ddsd.dwWidth = dwMaxWidth;

	if (ddsd.dwHeight > dwMaxHeight)
		ddsd.dwHeight = dwMaxHeight;

    // Enumerate the texture formats, and find the closest device-supported
    // texture pixel format.
    GDevice->EnumTextureFormats( TextureSearchCallback, &ddsd.ddpfPixelFormat );
    if( 0L == ddsd.ddpfPixelFormat.dwRGBBitCount )
        return NULL;

    // Get the device's render target, so we can then use the render target to
    // get a ptr to a DDraw object. We need the DirectDraw interface for
    // creating surfaces.
    LPDIRECTDRAWSURFACE7 pddsRender;
    LPDIRECTDRAW7        pDD;
    GDevice->GetRenderTarget( &pddsRender );
    pddsRender->GetDDInterface( (VOID**)&pDD );
    pddsRender->Release();

    // Create a new surface for the texture
    if( FAILED( hr = pDD->CreateSurface( &ddsd, &pddsTexture, NULL ) ) )
    {
        pDD->Release();
        return NULL;
    }

    // Done with DDraw
    pDD->Release();

	return pddsTexture;
}

void DX7Texture2D::LoadTextureFromImage()
{
	// Get a DDraw object to create a temporary surface
	LPDIRECTDRAW7 pDD;
	m_pddsSurface->GetDDInterface((VOID **)&pDD);

	// Setup the new surface desc
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	m_pddsSurface->GetSurfaceDesc(&ddsd);
	ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd.ddsCaps.dwCaps2 = 0L;

	// Create a new surface for the texture
	LPDIRECTDRAWSURFACE7 pddsTempSurface;
	HRESULT hr;

	if (FAILED(hr = pDD->CreateSurface(&ddsd, &pddsTempSurface, NULL)))
	{
		pDD->Release();
		return;
	}

	while (pddsTempSurface->Lock(NULL, &ddsd, 0, 0) == DDERR_WASSTILLDRAWING);

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
	BYTE r, g, b, a;
	DWORD pitchIncrement = (ddsd.lPitch >> 2) - ddsd.dwWidth;

	if(mFormat == Image::Format_L8)
	{
		for (DWORD y = 0; y < ddsd.dwHeight; y++)
		{
			for (DWORD x = 0; x < ddsd.dwWidth; x++, pDst++)
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
		for (DWORD y = 0; y < ddsd.dwHeight; y++)
		{
			for (DWORD x = 0; x < ddsd.dwWidth; x++, pDst++)
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
		for (DWORD y = 0; y < ddsd.dwHeight; y++)
		{
			for (DWORD x = 0; x < ddsd.dwWidth; x++, pDst++)
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
		for (DWORD y = 0; y < ddsd.dwHeight; y++)
		{
			for (DWORD x = 0; x < ddsd.dwWidth; x++, pDst++)
			{
				r = pSrc[0];
				g = pSrc[1];
				b = pSrc[2];
				a = 0xFF;
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
		for (DWORD y = 0; y < ddsd.dwHeight; y++)
		{
			for (DWORD x = 0; x < ddsd.dwWidth; x++, pDst++)
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
	else
	{
		arx_assert_msg(false, "Unsupported image format");
	}
	
	pddsTempSurface->Unlock(0);

	m_pddsSurface->Blt(NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL);

	// Done with the temp objects
	pddsTempSurface->Release();
	pDD->Release();
}

///////////////////////////////////////////////////////////////////////////////
// DX7TextureStage - MOVE THAT SOMEWHERE ELSE!
///////////////////////////////////////////////////////////////////////////////
class DX7TextureStage : public TextureStage
{
public:
	DX7TextureStage(unsigned int textureStage);

	void SetTexture( Texture& pTexture );
	void ResetTexture();

	void SetColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2);
	void SetAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2);
};

DX7TextureStage::DX7TextureStage(unsigned int textureStage)
	: TextureStage(textureStage)
{
}

void DX7TextureStage::SetColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2)
{
	// TODO-DX7: Cache states
	DWORD colorOp = ARXToDX7TextureOp[textureOp];
	DWORD colorArg1 = ARXToDX7TextureArg[texArg1];
	DWORD colorArg2 = ARXToDX7TextureArg[texArg2];

	GDevice->SetTextureStageState(mStage, D3DTSS_COLOROP,   colorOp);
	GDevice->SetTextureStageState(mStage, D3DTSS_COLORARG1, colorArg1);
	GDevice->SetTextureStageState(mStage, D3DTSS_COLORARG2, colorArg2);
}

void DX7TextureStage::SetAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2)
{
	// TODO-DX7: Cache states
	DWORD alphaOp = ARXToDX7TextureOp[textureOp];
	DWORD alphaArg1 = ARXToDX7TextureArg[texArg1];
	DWORD alphaArg2 = ARXToDX7TextureArg[texArg2];

	GDevice->SetTextureStageState(mStage, D3DTSS_ALPHAOP,   alphaOp);
	GDevice->SetTextureStageState(mStage, D3DTSS_ALPHAARG1, alphaArg1);
	GDevice->SetTextureStageState(mStage, D3DTSS_ALPHAARG2, alphaArg2);
}

void DX7TextureStage::SetTexture( Texture& pTexture )
{
	// TODO-DX7: Support multiple texture types
	DX7Texture2D* tex = (DX7Texture2D*)&pTexture;

	// TODO-DX7: Cache states
	D3DTEXTUREADDRESS addressU = ARXToDX7WrapMode[tex->GetWrapMode(Texture::Wrap_S)];
	D3DTEXTUREADDRESS addressV = ARXToDX7WrapMode[tex->GetWrapMode(Texture::Wrap_T)];
	D3DTEXTUREMAGFILTER magFilter = ARXToDX7MagFilter[tex->GetMagFilter()];
	D3DTEXTUREMINFILTER minFilter = ARXToDX7MinFilter[tex->GetMinFilter()];	
	D3DTEXTUREMIPFILTER mipFilter = ARXToDX7MipFilter[tex->GetMinFilter()];	

	GDevice->SetTextureStageState(mStage, D3DTSS_ADDRESSU, addressU);
	GDevice->SetTextureStageState(mStage, D3DTSS_ADDRESSV, addressV);
	GDevice->SetTextureStageState(mStage, D3DTSS_MAGFILTER, magFilter);
    GDevice->SetTextureStageState(mStage, D3DTSS_MINFILTER, minFilter);
	GDevice->SetTextureStageState(mStage, D3DTSS_MIPFILTER, mipFilter);

	// TODO-DX7: Handle anisotropy...
	//GDevice->SetTextureStageState(mStage, D3DTSS_MAXANISOTROPY, tex->GetAnisotropy() );

	GDevice->SetTexture(mStage, tex->GetTextureID());
}

void DX7TextureStage::ResetTexture()
{
	GDevice->SetTexture(mStage, 0);
}


///////////////////////////////////////////////////////////////////////////////
// Renderer - DX7 implementation
///////////////////////////////////////////////////////////////////////////////

Renderer* GRenderer = 0;
Renderer  RendererInstance;

Renderer::Renderer()
{
	GRenderer = this;

	// TODO-DX7: Hardcoded... number of textures stages available for DX7
	const unsigned int DX7_TEXTURE_STAGE_COUNT = 8;
	m_TextureStages.resize(DX7_TEXTURE_STAGE_COUNT, 0);

	for(size_t i = 0; i < m_TextureStages.size(); ++i)
	{
		m_TextureStages[i] = new DX7TextureStage(i);
	}
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
		(*it)->Kill();
	}
}

void Renderer::RestoreAllTextures()
{
	std::list<DX7Texture2D*>::iterator it;
	for(it = g_Textures2D.begin(); it != g_Textures2D.end(); ++it)
	{
		(*it)->Update();
	}
}

Texture1D* Renderer::CreateTexture1D()
{
	return 0;
}

Texture2D* Renderer::CreateTexture2D()
{
	return new DX7Texture2D();
}

Texture3D* Renderer::CreateTexture3D()
{
	return 0;
}

Cubemap* Renderer::CreateCubemap()
{
	return 0;
}

void Renderer::SetRenderState(RenderState renderState, bool enable)
{
	GDevice->SetRenderState(ARXToDXRenderState[renderState], enable ? TRUE : FALSE);
}

void Renderer::SetAlphaFunc(PixelCompareFunc func, float fef)
{
	GDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)fef*255);
	GDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, ARXToDXPixelCompareFunc[func]);
}

void Renderer::SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor)
{
	GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  ARXToDXPixelBlendingFactor[srcFactor]);
	GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, ARXToDXPixelBlendingFactor[dstFactor]);
}

void Renderer::SetViewport(int x, int y, int width, int height)
{
	D3DVIEWPORT7 viewport = { x, y, width, height, 0.f, 1.f };
	GDevice->SetViewport(&viewport);
}

void Renderer::GetViewport(int viewport[4])
{
	D3DVIEWPORT7 tmpViewport;
	GDevice->GetViewport(&tmpViewport);
	
	viewport[0] = tmpViewport.dwX;
	viewport[1] = tmpViewport.dwY;
	viewport[2] = tmpViewport.dwWidth;
	viewport[3] = tmpViewport.dwHeight;	
}

void Renderer::SetCulling(CullingMode mode)
{
	GDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, ARXToDXCullMode[mode]);
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

D3DMATRIX matProj;
D3DMATRIX matWorld;
D3DMATRIX matView;
void Renderer::Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	GDevice->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);
	GDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);
	GDevice->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView);
	
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
	GDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);
	GDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);
	GDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &matView);
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

void Renderer::SetTexture(unsigned int textureStage, Texture& pTexture)
{
	GetTextureStage(textureStage)->SetTexture(pTexture);
}

float Renderer::GetMaxAnisotropy() const
{
	return 8.0f; // TODO-DX7
}

void Renderer::DrawTexturedRect( float pX, float pY, float pW, float pH, float pUStart, float pVStart, float pUEnd, float pVEnd, COLORREF pCol )
{
	D3DLVERTEX rect[4];

	pX -= 0.5f;
	pY -= 0.5f;

	const D3DCOLOR DIFFUSE  = RGBA_MAKE(GetRValue(pCol), GetGValue(pCol), GetBValue(pCol), 0xFF);
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
