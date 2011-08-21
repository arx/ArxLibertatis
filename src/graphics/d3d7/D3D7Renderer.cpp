
#include "graphics/d3d7/D3D7Renderer.h"

#include <list>

#include "graphics/GraphicsUtility.h"
#include "graphics/Math.h"
#include "graphics/d3d7/D3D7VertexBuffer.h"
#include "graphics/d3d7/D3D7TextureStage.h"
#include "graphics/d3d7/D3D7Texture2D.h"
#include "io/Logger.h"

///////////////////////////////////////////////////////////////////////////////
// ARXToDX7 mapping tables - MOVE THAT SOMEWHERE ELSE!
///////////////////////////////////////////////////////////////////////////////

const D3DRENDERSTATETYPE ARXToDXRenderState[] = {
						D3DRENDERSTATE_ALPHABLENDENABLE,	// AlphaBlending,
						D3DRENDERSTATE_ALPHATESTENABLE,		// AlphaTest,
						D3DRENDERSTATE_COLORKEYENABLE,		// ColorKey,
						D3DRENDERSTATE_ZENABLE,			// DepthTest,
						D3DRENDERSTATE_ZWRITEENABLE,		// DepthWrite,
						D3DRENDERSTATE_FOGENABLE,		// Fog,
						D3DRENDERSTATE_LIGHTING,		// Lighting,
						D3DRENDERSTATE_ZBIAS			// ZBias
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

const D3DPRIMITIVETYPE ARXToDXPrimitiveType[] = {
	D3DPT_TRIANGLELIST,  // TriangleList
	D3DPT_TRIANGLESTRIP, // TriangleStrip
	D3DPT_TRIANGLEFAN,   // TriangleFan
	D3DPT_LINELIST,      // LineList
	D3DPT_LINESTRIP      // LineStrip
};

const DWORD ARXToDXBufferFlags[] = {
	0, // none
	DDLOCK_DISCARDCONTENTS,                     // DiscardContents
	DDLOCK_NOOVERWRITE,                         // NoOverwrite
	DDLOCK_DISCARDCONTENTS | DDLOCK_NOOVERWRITE // DiscardContents | NoOverwrite
};

///////////////////////////////////////////////////////////////////////////////
// Renderer - DX7 implementation
///////////////////////////////////////////////////////////////////////////////

D3D7Renderer::D3D7Renderer(D3D7Window * _window) : device(NULL), window(_window), gammaControl(NULL) { }

void D3D7Renderer::Initialize() {
	
	arx_assert(device == NULL);
	device = window->getDevice();
	arx_assert(device != NULL);
	
	D3DDEVICEDESC7 devicedesc;
	device->GetCaps(&devicedesc);

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
		m_TextureStages[i] = new D3D7TextureStage(device, i);
		device->SetTextureStageState(i, D3DTSS_MAXANISOTROPY, maxAnisotropy);

        // Set default state
		m_TextureStages[i]->SetWrapMode(TextureStage::WrapRepeat);
		m_TextureStages[i]->SetMinFilter(TextureStage::FilterLinear);
		m_TextureStages[i]->SetMagFilter(TextureStage::FilterLinear);
		m_TextureStages[i]->SetMipFilter(TextureStage::FilterLinear);
	}

	SetRenderState(ColorKey, true);

	// Clear screen
	Clear(ColorBuffer | DepthBuffer);
	
	// Backup original gamma values
	
	LPVOID _gammaControl;
	window->getFront()->QueryInterface(IID_IDirectDrawGammaControl, &_gammaControl);
	gammaControl = reinterpret_cast<LPDIRECTDRAWGAMMACONTROL>(_gammaControl);
	if(gammaControl) {
		gammaControl->GetGammaRamp(0, &oldGamma);
	}
	
	LogInfo << "Using Direct3D 7";
	LogInfo << "Device: " << window->getInfo().name << " (" << window->getInfo().desc << ')';
	LogInfo << "Driver: " << window->getInfo().driver << " (" << window->getInfo().driverDesc << ')';
}

bool D3D7Renderer::BeginScene() {
	return device->BeginScene() == D3D_OK;
}

bool D3D7Renderer::EndScene() {
	return device->EndScene() == D3D_OK;
}

void D3D7Renderer::SetViewMatrix(const EERIEMATRIX & matView) {
	device->SetTransform(D3DTRANSFORMSTATE_VIEW, (LPD3DMATRIX)&matView);
}

void D3D7Renderer::GetViewMatrix(EERIEMATRIX & matView) const {
	device->GetTransform(D3DTRANSFORMSTATE_VIEW, (LPD3DMATRIX)&matView);
}

void D3D7Renderer::SetProjectionMatrix(const EERIEMATRIX & matProj) {
	device->SetTransform(D3DTRANSFORMSTATE_PROJECTION, (LPD3DMATRIX)&matProj);
}

void D3D7Renderer::GetProjectionMatrix(EERIEMATRIX & matProj) const {
	device->GetTransform(D3DTRANSFORMSTATE_PROJECTION, (LPD3DMATRIX)&matProj);
}

D3D7Renderer::~D3D7Renderer() {
	
	if(gammaControl) {
		gammaControl->SetGammaRamp(0, &oldGamma);
		gammaControl->Release(), gammaControl = NULL;
	}
}

void D3D7Renderer::ReleaseAllTextures() {
	std::list<DX7Texture2D*>::iterator it;
	for(it = g_Textures2D.begin(); it != g_Textures2D.end(); ++it) {
		(*it)->Destroy();
	}
}

void D3D7Renderer::RestoreAllTextures() {
	std::list<DX7Texture2D*>::iterator it;
	for(it = g_Textures2D.begin(); it != g_Textures2D.end(); ++it) {
		(*it)->Restore();
	}
}

Texture2D* D3D7Renderer::CreateTexture2D() {
	return new DX7Texture2D(device);
}

void D3D7Renderer::SetRenderState(RenderState renderState, bool enable) {
	if(renderState == ColorKey)
	{
		SetRenderState(AlphaTest, enable);
		if(enable)
			SetAlphaFunc(CmpNotEqual, 0.0f);
	}
	else
	{
		device->SetRenderState(ARXToDXRenderState[renderState], enable ? TRUE : FALSE);
	}
}

void D3D7Renderer::SetAlphaFunc(PixelCompareFunc func, float fef) {
	device->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)fef*255);
	device->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, ARXToDXPixelCompareFunc[func]);
}

void D3D7Renderer::SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor) {
	device->SetRenderState(D3DRENDERSTATE_SRCBLEND,  ARXToDXPixelBlendingFactor[srcFactor]);
	device->SetRenderState(D3DRENDERSTATE_DESTBLEND, ARXToDXPixelBlendingFactor[dstFactor]);
}

void D3D7Renderer::SetViewport(const Rect & viewport) {
	D3DVIEWPORT7 tmpViewport = { viewport.left, viewport.top, viewport.width(), viewport.height(), 0.f, 1.f };
	device->SetViewport(&tmpViewport);
}

Rect D3D7Renderer::GetViewport() {
	
	D3DVIEWPORT7 tmpViewport;
	device->GetViewport(&tmpViewport);
	
	return Rect(Vec2i(tmpViewport.dwX, tmpViewport.dwY), tmpViewport.dwWidth, tmpViewport.dwHeight);
}

void D3D7Renderer::SetCulling(CullingMode mode) {
	device->SetRenderState(D3DRENDERSTATE_CULLMODE, ARXToDXCullMode[mode]);
}

void D3D7Renderer::SetDepthBias(int depthBias) {
	device->SetRenderState(D3DRENDERSTATE_ZBIAS, depthBias);
}

void D3D7Renderer::SetFillMode(FillMode mode) {
	device->SetRenderState(D3DRENDERSTATE_FILLMODE, ARXToDXFillMode[mode]);
}

static void DX7MatrixIdentity(D3DMATRIX *pout) {
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

static D3DMATRIX * DX7MatrixOrthoOffCenterLH(D3DMATRIX * pout, float l, float r, float b, float t, float zn, float zf) {
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

void D3D7Renderer::Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar) {
	
	device->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &g_MatProj);
	device->GetTransform(D3DTRANSFORMSTATE_WORLD, &g_MatWorld);
	device->GetTransform(D3DTRANSFORMSTATE_VIEW, &g_MatView);
	
	D3DMATRIX matOrtho;
	DX7MatrixOrthoOffCenterLH(&matOrtho, left, right, bottom, top, zNear, zFar);

	D3DMATRIX matIdentity;
	DX7MatrixIdentity(&matIdentity);

	device->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matOrtho);
	device->SetTransform(D3DTRANSFORMSTATE_WORLD, &matIdentity);
	device->SetTransform(D3DTRANSFORMSTATE_VIEW, &matIdentity);
}

void D3D7Renderer::End2DProjection() {
	device->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &g_MatProj);
	device->SetTransform(D3DTRANSFORMSTATE_WORLD, &g_MatWorld);
	device->SetTransform(D3DTRANSFORMSTATE_VIEW, &g_MatView);
}

void D3D7Renderer::Clear(BufferFlags bufferFlags, Color clearColor, float clearDepth, size_t nrects, Rect * rects) {
	
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
	
	device->Clear(DWORD(nrects), nrects != 0 ? d3drects : 0, clearTargets, clearColor.toBGRA(), clearDepth, 0);
}

void D3D7Renderer::SetFogColor(Color color) {
	device->SetRenderState(D3DRENDERSTATE_FOGCOLOR, color.toBGRA());
}

void D3D7Renderer::SetFogParams(FogMode fogMode, float fogStart, float fogEnd, float fogDensity) {
	device->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, ARXToDXFogMode[fogMode]);
	device->SetRenderState(D3DRENDERSTATE_FOGSTART, reinterpret<DWORD, f32>(fogStart));
	device->SetRenderState(D3DRENDERSTATE_FOGEND, reinterpret<DWORD, f32>(fogEnd));
	device->SetRenderState(D3DRENDERSTATE_FOGDENSITY, reinterpret<DWORD, f32>(fogDensity));
}

void D3D7Renderer::SetAntialiasing(bool enable) {
	
	if(enable)
	{
		D3DDEVICEDESC7 ddDesc;
		device->GetCaps(&ddDesc);

		if(ddDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)
			device->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTINDEPENDENT);
		else if(ddDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT)
			device->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_SORTDEPENDENT);
	}
	else
	{
		device->SetRenderState(D3DRENDERSTATE_ANTIALIAS, D3DANTIALIAS_NONE);
	}
}

float D3D7Renderer::GetMaxAnisotropy() const {
	return 8.0f; // TODO-DX7
}

void D3D7Renderer::DrawTexturedRect( float pX, float pY, float pW, float pH, float pUStart, float pVStart, float pUEnd, float pVEnd, Color pCol ) {
	
	D3DLVERTEX rect[4];

	pX -= 0.5f;
	pY -= 0.5f;

	const D3DCOLOR diffuse  = pCol.toBGR();
	const D3DCOLOR specular = 0;

	rect[0].x = pX;
	rect[0].y = pY;
	rect[0].z = 0;
	rect[0].tu = pUStart;
	rect[0].tv = pVStart;
	rect[0].color = diffuse;
	rect[0].specular = specular;

	rect[1].x = pX+pW;
	rect[1].y = pY;
	rect[1].z = 0;
	rect[1].tu = pUEnd;
	rect[1].tv = pVStart;
	rect[1].color = diffuse;
	rect[1].specular = specular;

	rect[2].x = pX;
	rect[2].y = pY+pH;
	rect[2].z = 0;
	rect[2].tu = pUStart;
	rect[2].tv = pVEnd;
	rect[2].color = diffuse;
	rect[2].specular = specular;

	rect[3].x = pX+pW;
	rect[3].y = pY+pH;
	rect[3].z = 0;
	rect[3].tu = pUEnd;
	rect[3].tv = pVEnd;
	rect[3].color = diffuse;
	rect[3].specular = specular;
	
	device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_LVERTEX, &rect, 4, 0);
}

VertexBuffer<TexturedVertex> * D3D7Renderer::createVertexBufferTL(size_t capacity, BufferUsage usage) {
	ARX_UNUSED(usage);
	return new D3D7VertexBuffer<TexturedVertex>(window, D3DFVF_TLVERTEX, capacity);
}

VertexBuffer<SMY_VERTEX> * D3D7Renderer::createVertexBuffer(size_t capacity, BufferUsage usage) {
	ARX_UNUSED(usage);
	const DWORD format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXTUREFORMAT2;
	return new D3D7VertexBuffer<SMY_VERTEX>(window, format, capacity);
}

VertexBuffer<SMY_VERTEX3> * D3D7Renderer::createVertexBuffer3(size_t capacity, BufferUsage usage) {
	ARX_UNUSED(usage);
	const DWORD format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX3 | D3DFVF_TEXTUREFORMAT2;
	return new D3D7VertexBuffer<SMY_VERTEX3>(window, format, capacity);
}

void D3D7Renderer::drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices) {
	
	arx_assert(vertices != NULL && indices != NULL);
	
	D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];
	HRESULT hr = device->DrawIndexedPrimitive(type, D3DFVF_TLVERTEX, (LPVOID)vertices, (DWORD)nvertices, (LPWORD)indices, (DWORD)nindices, 0);
	arx_assert_msg(SUCCEEDED(hr), "DrawIndexedPrimitive failed: %08x", hr);
	ARX_UNUSED(hr);
	
}

bool D3D7Renderer::getSnapshot(Image & image) {
	
	LPDIRECTDRAWSURFACE7 pddsRender;
	HRESULT hr = device->GetRenderTarget(&pddsRender);
	if(FAILED(hr)) {
		LogError << "GetRenderTarget failed: " << hr;
		return false;
	}
	
	bool ret = downloadSurface(pddsRender, image);
	
	pddsRender->Release();
	
	return ret;
}

bool D3D7Renderer::getSnapshot(Image& image, size_t width, size_t height) {
	
	LPDIRECTDRAWSURFACE7 pddsRender;
	HRESULT hr = device->GetRenderTarget(&pddsRender);
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
	
	hr = window->getDD()->CreateSurface(&desc, &tempSurface, NULL);
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

void D3D7Renderer::setGamma(float brightness, float contrast, float gamma) {
	
	if(!gammaControl) {
		return;
	}
	
	float fGammaMax = (1.f / 6.f);
	float fGammaMin = 2.f;
	float fGamma = ((fGammaMax - fGammaMin) / 11.f) * (gamma + 1.f) + fGammaMin;
	
	float fLuminosityMin = -.2f;
	float fLuminosityMax = .2f;
	float fLuminosity = ((fLuminosityMax - fLuminosityMin) / 11.f) * (brightness + 1.f) + fLuminosityMin;
	
	float fContrastMax = -.3f;
	float fContrastMin = .3f;
	float fContrast = ((fContrastMax - fContrastMin) / 11.f) * (contrast + 1.f) + fContrastMin;
	
	float fRangeMin = 0.f + fContrast;
	float fRangeMax = 1.f - fContrast;
	float fdVal = (fRangeMax - fRangeMin) / 256.f;
	float fVal = 0.f;
	
	DDGAMMARAMP ramp;
	
	for(int iI = 0; iI < 256; iI++) {
		
		int iColor = clamp((int)(65536.f * (fLuminosity + pow(fVal, fGamma))), 0, 65535);
		
		WORD wColor = checked_range_cast<WORD>(iColor);
		
		ramp.red[iI] = wColor;
		ramp.green[iI] = wColor;
		ramp.blue[iI] = wColor;
		
		fVal += fdVal;
	}
	
	gammaControl->SetGammaRamp(0, &ramp);
	
}
