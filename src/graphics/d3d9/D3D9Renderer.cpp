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

#include "graphics/d3d9/D3D9Renderer.h"

#include <list>

// TODO(broken-wine) wine's d3dx9math.h requires min and max but doesn't include them!
#include <algorithm>
using std::max;
using std::min;

#include <d3d9types.h>
#include <d3dx9.h>
#include <d3dx9tex.h>

#include "graphics/GraphicsUtility.h"
#include "graphics/Math.h"
#include "graphics/d3d9/D3D9VertexBuffer.h"
#include "graphics/d3d9/D3D9TextureStage.h"
#include "graphics/d3d9/D3D9Texture2D.h"
#include "io/log/Logger.h"

///////////////////////////////////////////////////////////////////////////////
// ARXToDX9 mapping tables
///////////////////////////////////////////////////////////////////////////////

const D3DRENDERSTATETYPE ARXToDXRenderState[] = {
						D3DRS_ALPHABLENDENABLE,				// AlphaBlending,
						D3DRS_ALPHATESTENABLE,				// AlphaTest,
						(D3DRENDERSTATETYPE)0,				// ColorKey,
						D3DRS_ZENABLE,						// DepthTest,
						D3DRS_ZWRITEENABLE,					// DepthWrite,
						D3DRS_FOGENABLE,					// Fog,
						D3DRS_LIGHTING,						// Lighting,
						D3DRS_DEPTHBIAS						// ZBias
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
						D3DPT_TRIANGLELIST,					// TriangleList
						D3DPT_TRIANGLESTRIP,				// TriangleStrip
						D3DPT_TRIANGLEFAN,					// TriangleFan
						D3DPT_LINELIST,						// LineList
						D3DPT_LINESTRIP						// LineStrip
					};

const DWORD ARXToDXBufferFlags[] = {
	0,                                     // None
	D3DLOCK_DISCARD,                       // DiscardBuffer
	D3DLOCK_DISCARD,                       // DiscardRange
	D3DLOCK_DISCARD,                       // DiscardBuffer | DiscardRange
	D3DLOCK_NOOVERWRITE,                   // NoOverwrite
	D3DLOCK_NOOVERWRITE | D3DLOCK_DISCARD, // NoOverwrite | DiscardBuffer
	D3DLOCK_NOOVERWRITE | D3DLOCK_DISCARD, // NoOverwrite | DiscardRange
	D3DLOCK_NOOVERWRITE | D3DLOCK_DISCARD  // NoOverwrite | DiscardBuffer | DiscardRange
};

UINT GetNumberOfPrimitives(Renderer::Primitive primitive, UINT nindices) {
	UINT numPrimitives;
	switch(primitive)
	{
	case Renderer::TriangleList:
		arx_assert(nindices % 3 == 0);
		numPrimitives = nindices / 3;
		break;

	case Renderer::TriangleStrip:
		numPrimitives = nindices - 2;
		break;

	case Renderer::TriangleFan:
		numPrimitives = nindices - 2;
		break;

	case Renderer::LineList:
		arx_assert(nindices % 2 == 0);
		numPrimitives = nindices / 2;
		break;

	case Renderer::LineStrip:
		numPrimitives = nindices - 1;
		break;
	}

	return numPrimitives;
}


// DX7 hardcoded vertex types are not provided in newer version of DX...

#define D3DFVF_LVERTEX		D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1
#define D3DFVF_TLVERTEX		D3DFVF_XYZRHW | D3DFVF_DIFFUSE  |D3DFVF_SPECULAR | D3DFVF_TEX1

typedef struct _D3DLVERTEX {
    union {
       float x;
       float dvX;
    };
    union {
       float y;
        float dvY;
    };
    union {
        float z;
        float dvZ;
    };
    union {
        D3DCOLOR color;
        D3DCOLOR dcColor;
    };
    union {
        D3DCOLOR specular;
        D3DCOLOR dcSpecular;
    };
    union {
        float tu;
        float dvTU;
    };
    union {
        float tv;
        float dvTV;
    };
    _D3DLVERTEX() { }
    _D3DLVERTEX(const D3DVECTOR& v,D3DCOLOR col,D3DCOLOR spec,float _tu, float _tv)
        { x = v.x; y = v.y; z = v.z; 
          color = col; specular = spec;
          tu = _tu; tv = _tv;
    }
} D3DLVERTEX, *LPD3DLVERTEX;



///////////////////////////////////////////////////////////////////////////////
// Renderer - DX9 implementation
///////////////////////////////////////////////////////////////////////////////

D3D9Renderer::D3D9Renderer(D3D9Window * _window) : window(_window) { }

void D3D9Renderer::Initialize() {

	arx_assert(GD3D9Device != NULL);
	
	D3DCAPS9 deviceCaps;
	GD3D9Device->GetDeviceCaps(&deviceCaps);

	///////////////////////////////////////////////////////////////////////////
	// Initialize filtering options depending on the card capabilities...

	// Minification
	if (deviceCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
		ARXToDX9MinFilter[TextureStage::FilterLinear] = D3DTEXF_ANISOTROPIC;
	else if (deviceCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
		ARXToDX9MinFilter[TextureStage::FilterLinear] = D3DTEXF_LINEAR;
	else
		ARXToDX9MinFilter[TextureStage::FilterLinear] = D3DTEXF_POINT;
		
	// Magnification
	if (deviceCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
		ARXToDX9MagFilter[TextureStage::FilterLinear] = D3DTEXF_ANISOTROPIC;
	else if (deviceCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
		ARXToDX9MagFilter[TextureStage::FilterLinear] = D3DTEXF_LINEAR;
	else
		ARXToDX9MagFilter[TextureStage::FilterLinear] = D3DTEXF_POINT;

	// Mipmapping
	if (deviceCaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR)
		ARXToDX9MipFilter[TextureStage::FilterLinear] = D3DTEXF_LINEAR;
	else if (deviceCaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFPOINT)
		ARXToDX9MipFilter[TextureStage::FilterLinear] = D3DTEXF_POINT;
	else
		ARXToDX9MipFilter[TextureStage::FilterLinear] = D3DTEXF_NONE;

	///////////////////////////////////////////////////////////////////////////
	// Texture stages...

	m_TextureStages.resize(deviceCaps.MaxTextureBlendStages, 0);

	DWORD maxAnisotropy = 1;
	if( deviceCaps.RasterCaps & D3DPRASTERCAPS_ANISOTROPY )
        maxAnisotropy = deviceCaps.MaxAnisotropy;

	for(size_t i = 0; i < m_TextureStages.size(); ++i)
	{
		m_TextureStages[i] = new D3D9TextureStage(i);
		GD3D9Device->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, maxAnisotropy);

        // Set default state
		m_TextureStages[i]->SetWrapMode(TextureStage::WrapRepeat);
		m_TextureStages[i]->SetMinFilter(TextureStage::FilterLinear);
		m_TextureStages[i]->SetMagFilter(TextureStage::FilterLinear);
		m_TextureStages[i]->SetMipFilter(TextureStage::FilterLinear);
	}

	SetRenderState(ColorKey, true);

	// Clear screen
	Clear(ColorBuffer | DepthBuffer);
	
	LogInfo << "Using Direct3D 9";
	LogInfo << "Device: " << window->getInfo().adapterIdentifier.Description;
	LogInfo << "Driver: " << window->getInfo().adapterIdentifier.Driver;
	LogInfo << "Version: " << HIWORD(window->getInfo().adapterIdentifier.DriverVersion.HighPart) 
						   << "." << LOWORD(window->getInfo().adapterIdentifier.DriverVersion.HighPart) 
						   << "." << HIWORD(window->getInfo().adapterIdentifier.DriverVersion.LowPart) 
						   << " build " << LOWORD(window->getInfo().adapterIdentifier.DriverVersion.LowPart);
}

bool D3D9Renderer::BeginScene() {
	return GD3D9Device->BeginScene() == D3D_OK;
}

bool D3D9Renderer::EndScene() {
	return GD3D9Device->EndScene() == D3D_OK;
}

void D3D9Renderer::SetViewMatrix(const EERIEMATRIX & matView) {
	GD3D9Device->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&matView);
}

void D3D9Renderer::GetViewMatrix(EERIEMATRIX & matView) const {
	GD3D9Device->GetTransform(D3DTS_VIEW, (D3DMATRIX*)&matView);
}

void D3D9Renderer::SetProjectionMatrix(const EERIEMATRIX & matProj) {
	GD3D9Device->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&matProj);
}

void D3D9Renderer::GetProjectionMatrix(EERIEMATRIX & matProj) const {
	GD3D9Device->GetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&matProj);
}

D3D9Renderer::~D3D9Renderer() { }

void D3D9Renderer::ReleaseAllTextures() {
	std::list<DX9Texture2D*>::iterator it;
	for(it = g_Textures2D.begin(); it != g_Textures2D.end(); ++it) {
		(*it)->Destroy();
	}
}

void D3D9Renderer::RestoreAllTextures() {
	std::list<DX9Texture2D*>::iterator it;
	for(it = g_Textures2D.begin(); it != g_Textures2D.end(); ++it) {
		(*it)->Restore();
	}
}

Texture2D* D3D9Renderer::CreateTexture2D() {
	return new DX9Texture2D();
}

void D3D9Renderer::SetRenderState(RenderState renderState, bool enable) {
	if(renderState == ColorKey)
	{
		SetRenderState(AlphaTest, enable);
		if(enable)
			SetAlphaFunc(CmpNotEqual, 0.0f);
	}
	else
	{
		GD3D9Device->SetRenderState(ARXToDXRenderState[renderState], enable ? TRUE : FALSE);
	}
}

void D3D9Renderer::SetAlphaFunc(PixelCompareFunc func, float fef) {
	GD3D9Device->SetRenderState(D3DRS_ALPHAREF, (DWORD)fef*255);
	GD3D9Device->SetRenderState(D3DRS_ALPHAFUNC, ARXToDXPixelCompareFunc[func]);
}

void D3D9Renderer::SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor) {
	GD3D9Device->SetRenderState(D3DRS_SRCBLEND,  ARXToDXPixelBlendingFactor[srcFactor]);
	GD3D9Device->SetRenderState(D3DRS_DESTBLEND, ARXToDXPixelBlendingFactor[dstFactor]);
}

void D3D9Renderer::SetViewport(const Rect & viewport) {
	D3DVIEWPORT9 tmpViewport = { viewport.left, viewport.top, viewport.width(), viewport.height(), 0.f, 1.f };
	GD3D9Device->SetViewport(&tmpViewport);
}

Rect D3D9Renderer::GetViewport() {
	
	D3DVIEWPORT9 tmpViewport;
	GD3D9Device->GetViewport(&tmpViewport);
	
	return Rect(Vec2i(tmpViewport.X, tmpViewport.Y), tmpViewport.Width, tmpViewport.Height);
}

void D3D9Renderer::SetCulling(CullingMode mode) {
	GD3D9Device->SetRenderState(D3DRS_CULLMODE, ARXToDXCullMode[mode]);
}

void D3D9Renderer::SetDepthBias(int depthBias) {
	float bias = -(float)depthBias;
	bias /= 65536.0f;
	u32 val = reinterpret<u32, f32>(bias);
	GD3D9Device->SetRenderState(D3DRS_DEPTHBIAS, val);
}

void D3D9Renderer::SetFillMode(FillMode mode) {
	GD3D9Device->SetRenderState(D3DRS_FILLMODE, ARXToDXFillMode[mode]);
}

static void DX9MatrixIdentity(D3DMATRIX *pout) {
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

static D3DMATRIX * DX9MatrixOrthoOffCenterLH(D3DMATRIX * pout, float l, float r, float b, float t, float zn, float zf) {
	DX9MatrixIdentity(pout);
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

void D3D9Renderer::Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar) {
	
	GD3D9Device->GetTransform(D3DTS_PROJECTION, &g_MatProj);
	GD3D9Device->GetTransform(D3DTS_WORLD, &g_MatWorld);
	GD3D9Device->GetTransform(D3DTS_VIEW, &g_MatView);
	
	D3DMATRIX matOrtho;
	DX9MatrixOrthoOffCenterLH(&matOrtho, left, right, bottom, top, zNear, zFar);

	D3DMATRIX matIdentity;
	DX9MatrixIdentity(&matIdentity);

	GD3D9Device->SetTransform(D3DTS_PROJECTION, &matOrtho);
	GD3D9Device->SetTransform(D3DTS_WORLD, &matIdentity);
	GD3D9Device->SetTransform(D3DTS_VIEW, &matIdentity);
}

void D3D9Renderer::End2DProjection() {
	GD3D9Device->SetTransform(D3DTS_PROJECTION, &g_MatProj);
	GD3D9Device->SetTransform(D3DTS_WORLD, &g_MatWorld);
	GD3D9Device->SetTransform(D3DTS_VIEW, &g_MatView);
}

void D3D9Renderer::Clear(BufferFlags bufferFlags, Color clearColor, float clearDepth, size_t nrects, Rect * rects) {
	
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
	
	GD3D9Device->Clear(DWORD(nrects), nrects != 0 ? d3drects : 0, clearTargets, clearColor.toBGRA(), clearDepth, 0);
}

void D3D9Renderer::SetFogColor(Color color) {
	GD3D9Device->SetRenderState(D3DRS_FOGCOLOR, color.toBGRA());
}

void D3D9Renderer::SetFogParams(FogMode fogMode, float fogStart, float fogEnd, float fogDensity) {
	GD3D9Device->SetRenderState(D3DRS_FOGTABLEMODE, ARXToDXFogMode[fogMode]);
	GD3D9Device->SetRenderState(D3DRS_FOGSTART, reinterpret<DWORD, f32>(fogStart));
	GD3D9Device->SetRenderState(D3DRS_FOGEND, reinterpret<DWORD, f32>(fogEnd));
	GD3D9Device->SetRenderState(D3DRS_FOGDENSITY, reinterpret<DWORD, f32>(fogDensity));
}

void D3D9Renderer::SetAntialiasing(bool enable) {
	// This won't have much effect if the device was initially created 
	// with present parameter MultiSampleType == D3DMULTISAMPLE_NONE
	GD3D9Device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, enable);
}

float D3D9Renderer::GetMaxAnisotropy() const {
	D3DCAPS9 deviceCaps;
	GD3D9Device->GetDeviceCaps(&deviceCaps);
	return deviceCaps.MaxAnisotropy;
}

void D3D9Renderer::DrawTexturedRect( float pX, float pY, float pW, float pH, float pUStart, float pVStart, float pUEnd, float pVEnd, Color pCol ) {
	
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
	
	GD3D9Device->SetFVF(D3DFVF_LVERTEX);
	GD3D9Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &rect, sizeof(D3DLVERTEX));
}

VertexBuffer<TexturedVertex> * D3D9Renderer::createVertexBufferTL(size_t capacity, BufferUsage usage) {
	ARX_UNUSED(usage);
	return new D3D9VertexBuffer<TexturedVertex>(D3DFVF_TLVERTEX, capacity, usage);
}

VertexBuffer<SMY_VERTEX> * D3D9Renderer::createVertexBuffer(size_t capacity, BufferUsage usage) {
	ARX_UNUSED(usage);
	const DWORD format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXTUREFORMAT2;
	return new D3D9VertexBuffer<SMY_VERTEX>(format, capacity, usage);
}

VertexBuffer<SMY_VERTEX3> * D3D9Renderer::createVertexBuffer3(size_t capacity, BufferUsage usage) {
	ARX_UNUSED(usage);
	const DWORD format = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX3 | D3DFVF_TEXTUREFORMAT2;
	return new D3D9VertexBuffer<SMY_VERTEX3>(format, capacity, usage);
}

void D3D9Renderer::drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices) {
	
	arx_assert(vertices != NULL && indices != NULL);

	UINT numPrimitives = GetNumberOfPrimitives(primitive, nindices);
	
	D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];

	GD3D9Device->SetFVF(D3DFVF_TLVERTEX);
	HRESULT hr = GD3D9Device->DrawIndexedPrimitiveUP(type, 0, nvertices, numPrimitives, indices, D3DFMT_INDEX16, vertices, sizeof(TexturedVertex));
	arx_assert_msg(SUCCEEDED(hr), "DrawIndexedPrimitiveUP failed: %08x", hr);
	ARX_UNUSED(hr);
	
}

bool D3D9Renderer::getSnapshot(Image & image) {
	// Use the current backbuffer size
	return getSnapshot(image, (size_t)-1, (size_t)-1);
}

bool D3D9Renderer::getSnapshot(Image& image, size_t width, size_t height) {
	HRESULT hr;
	LPDIRECT3DSURFACE9 pRenderTarget;
	hr = GD3D9Device->GetRenderTarget( 0, &pRenderTarget );
	if( !pRenderTarget || FAILED(hr) )
		return false;

	D3DSURFACE_DESC rtDesc;
	pRenderTarget->GetDesc( &rtDesc );

	// If -1, use real backbuffer size
	if(width == (size_t)-1)
		width = rtDesc.Width;
	if(height == (size_t)-1)
		height = rtDesc.Height;

	// If multisampling is enabled
	//	OR
	// The size of the snapshot doesn't match the backbuffer size
	//	OR 
	// The backbuffer format isn't D3DFMT_X8R8G8B8
	//	THEN
	// We need to create a render surface with the right parameters and copy our backbuffer into it.
	// Will support snapshots from 16-bit backbuffers too.
	if( rtDesc.MultiSampleType != D3DMULTISAMPLE_NONE || rtDesc.Width != width || rtDesc.Height != height || rtDesc.Format != D3DFMT_X8R8G8B8) {
		LPDIRECT3DSURFACE9 pResolvedSurface;
		hr = GD3D9Device->CreateRenderTarget( width, height, D3DFMT_X8R8G8B8, D3DMULTISAMPLE_NONE, 0, FALSE, &pResolvedSurface, NULL );
		if( FAILED(hr) ) {
			pRenderTarget->Release();
			return false;
		}

		// Copy will automatically adjust the size for us
		hr = GD3D9Device->StretchRect( pRenderTarget, NULL, pResolvedSurface, NULL, D3DTEXF_NONE );
		if( FAILED(hr) ) {
			pResolvedSurface->Release();
			pRenderTarget->Release();
			return false;
		}
		
		pRenderTarget->Release();

		pRenderTarget = pResolvedSurface;
		pRenderTarget->GetDesc( &rtDesc );
    }

	// Create a surface in system memory
	LPDIRECT3DSURFACE9 pSurface;
	hr = GD3D9Device->CreateOffscreenPlainSurface( rtDesc.Width, rtDesc.Height, rtDesc.Format, D3DPOOL_SYSTEMMEM, &pSurface, NULL );
	if( FAILED(hr) ) {
		pRenderTarget->Release();
		return false;
	}

	// Obtain the image from the render target
	hr = GD3D9Device->GetRenderTargetData( pRenderTarget, pSurface );
	bool ok = SUCCEEDED(hr);
	if( ok ) {
		D3DLOCKED_RECT lockedRect;
		hr = pSurface->LockRect(&lockedRect, 0, 0);

		if( SUCCEEDED(hr) ) {
			D3DSURFACE_DESC surfaceDesc;
			pSurface->GetDesc(&surfaceDesc);

			image.Create(surfaceDesc.Width, surfaceDesc.Height, Image::Format_R8G8B8);
			BYTE* pDst = (BYTE*)image.GetData();

			DX9Texture2D::Copy_R8G8B8(pDst, (BYTE*)lockedRect.pBits, surfaceDesc.Width, surfaceDesc.Height, surfaceDesc.Format, 3, 4, surfaceDesc.Width * 3, lockedRect.Pitch);
		}
		
		pSurface->UnlockRect();
	}

	// Release references to the offscreen surface and render target.
	pRenderTarget->Release();
	pSurface->Release();

	return ok;
}

bool D3D9Renderer::isFogInEyeCoordinates() {
	D3DCAPS9 deviceCaps;
	GD3D9Device->GetDeviceCaps(&deviceCaps);
	return (deviceCaps.RasterCaps & D3DPRASTERCAPS_WFOG) == D3DPRASTERCAPS_WFOG;
}
