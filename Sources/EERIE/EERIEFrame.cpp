/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// EERIEFrame
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		EERIE Framework funcs
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#define STRICT
#include <windows.h>
#include <tchar.h>

#include "arx_time.h"

#include "EerieApp.h"
#include "EerieFrame.h"
#include "EerieUtil.h"

#include "HERMESmain.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
// Name: CD3DFramework7()
// Desc: The constructor. Clears static variables
//-----------------------------------------------------------------------------
CD3DFramework7::CD3DFramework7()
{
	m_hWnd           = NULL;
	m_bIsFullscreen  = FALSE;
	m_bIsStereo      = FALSE;
	m_dwRenderWidth  = 0L;
	m_dwRenderHeight = 0L;
	b_dlg = FALSE;
	m_pddsFrontBuffer    = NULL;
	m_pddsBackBuffer     = NULL;
	m_pddsBackBufferLeft = NULL;

	m_pddsZBuffer     = NULL;
	m_pd3dDevice      = NULL;
	m_pDD             = NULL;
	m_pD3D            = NULL;
	m_dwDeviceMemType = NULL;
	Ystart = 0;
	Xstart = 0;
	ClipWin.top = 0;
	ClipWin.bottom = 0;
	ClipWin.right = 0;
	ClipWin.left = 0;

	usBeginEndSceneCount = 0;
}

//-----------------------------------------------------------------------------
// Name: ~CD3DFramework7()
// Desc: The destructor. Deletes all objects
//-----------------------------------------------------------------------------
CD3DFramework7::~CD3DFramework7()
{
	DestroyObjects();
}


//-----------------------------------------------------------------------------
// Name: DestroyObjects()
// Desc: Cleans everything up upon deletion. This code returns an error
//       if any of the objects have remaining reference counts.
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::DestroyObjects()
{
	LONG nDD  = 0L; // Number of outstanding DDraw references
	LONG nD3D = 0L; // Number of outstanding D3DDevice references

	if (m_pDD)
	{
		m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL);
	}

	// Do a safe check for releasing the D3DDEVICE. RefCount must be zero.
	if (m_pd3dDevice)
		if (0 < (nD3D = m_pd3dDevice->Release()))
			DEBUG_MSG(_T("Error: D3DDevice object is still referenced!"));

	m_pd3dDevice = NULL;

	SAFE_RELEASE(m_pddsBackBuffer);
	SAFE_RELEASE(m_pddsBackBufferLeft);
	SAFE_RELEASE(m_pddsZBuffer);
	SAFE_RELEASE(m_pddsFrontBuffer);
	SAFE_RELEASE(m_pD3D);

	if (m_pDD)
	{
		// Do a safe check for releasing DDRAW. RefCount must be zero.
		if (0 < (nDD = m_pDD->Release()))
			DEBUG_MSG(_T("Error: DDraw object is still referenced!"));
	}

	m_pDD = NULL;

	return (nDD == 0 && nD3D == 0) ? S_OK : D3DFWERR_NONZEROREFCOUNT;
}


//-----------------------------------------------------------------------------
// Name: Initialize()
// Desc: Creates the internal objects for the framework
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::Initialize(HWND hWnd, GUID * pDriverGUID,
                                   GUID * pDeviceGUID, DDSURFACEDESC2 * pMode,
                                   DWORD dwFlags)
{
	HRESULT hr;

	// Check params. Note: A NULL mode is valid for windowed modes only.
	if ((NULL == hWnd) || (NULL == pDeviceGUID) ||
	        (NULL == pMode && (dwFlags & D3DFW_FULLSCREEN)))
		return E_INVALIDARG;

	// Setup state for windowed/fullscreen mode
	m_hWnd          = hWnd;
	m_bIsStereo     = FALSE;
	m_bIsFullscreen = (dwFlags & D3DFW_FULLSCREEN) ? TRUE : FALSE;

	// Support stereoscopic viewing for fullscreen modes which support it
	if ((dwFlags & D3DFW_STEREO) && (dwFlags & D3DFW_FULLSCREEN))
		if (pMode->ddsCaps.dwCaps2 & DDSCAPS2_STEREOSURFACELEFT)
			m_bIsStereo = TRUE;

	// Create the D3D rendering environment (surfaces, device, viewport, etc.)
	if (FAILED(hr = CreateEnvironment(pDriverGUID, pDeviceGUID, pMode,
	                                  dwFlags)))
	{
		DestroyObjects();
		return hr;
	}

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CreateEnvironment()
// Desc: Creates the internal objects for the framework
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::CreateEnvironment(GUID * pDriverGUID, GUID * pDeviceGUID,
        DDSURFACEDESC2 * pMode, DWORD dwFlags)
{
	HRESULT hr;

	// Select the default memory type, for whether the device is HW or SW
	if (IsEqualIID(*pDeviceGUID, IID_IDirect3DHALDevice))
		m_dwDeviceMemType = DDSCAPS_VIDEOMEMORY;
	else if (IsEqualIID(*pDeviceGUID, IID_IDirect3DTnLHalDevice))
		m_dwDeviceMemType = DDSCAPS_VIDEOMEMORY;
	else
		m_dwDeviceMemType = DDSCAPS_SYSTEMMEMORY;

	// Create the DDraw object
	hr = CreateDirectDraw(pDriverGUID, dwFlags);

	if (FAILED(hr))
		return hr;

	// Create the front and back buffers, and attach a clipper
	if (dwFlags & D3DFW_FULLSCREEN)
		hr = CreateFullscreenBuffers(pMode);
	else
		hr = CreateWindowedBuffers();

	if (FAILED(hr))
		return hr;

	// Create the Direct3D object and the Direct3DDevice object
	hr = CreateDirect3D(pDeviceGUID);

	if (FAILED(hr))
		return hr;

	// Create and attach the zbuffer
	if (dwFlags & D3DFW_ZBUFFER)
		hr = CreateZBuffer(pDeviceGUID);

	if (FAILED(hr))
		return hr;

	this->m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0L);

	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: EnumZBufferFormatsCallback()
// Desc: Simply returns the first matching enumerated z-buffer format
//-----------------------------------------------------------------------------

typedef struct
{
	int				iNbZBufferInfo;
	DDPIXELFORMAT	* pddpfPixelFormat;
} ZBUFFER_INFO;

ZBUFFER_INFO zbiZBufferInfo;

static HRESULT WINAPI EnumZBufferFormatsCallback(DDPIXELFORMAT * pddpf,
        VOID * pContext)
{

	if (pddpf->dwFlags == DDPF_ZBUFFER)
	{
		zbiZBufferInfo.iNbZBufferInfo++;
		zbiZBufferInfo.pddpfPixelFormat = (DDPIXELFORMAT *)realloc(zbiZBufferInfo.pddpfPixelFormat, zbiZBufferInfo.iNbZBufferInfo * sizeof(DDPIXELFORMAT));
		*(zbiZBufferInfo.pddpfPixelFormat + zbiZBufferInfo.iNbZBufferInfo - 1) = *pddpf;
	}

	return D3DENUMRET_OK;
}




//-----------------------------------------------------------------------------
// CreateDirectDraw()
//  Create the DirectDraw interface
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::CreateDirectDraw(GUID * pDriverGUID, DWORD dwFlags)
{
	// Create the DirectDraw interface, and query for the DD7 interface
	if (FAILED(DirectDrawCreateEx(pDriverGUID, (VOID **)&m_pDD,
	                              IID_IDirectDraw7, NULL)))
	{
		DEBUG_MSG(_T("Could not create DirectDraw"));
		return D3DFWERR_NODIRECTDRAW;
	}

	// Set the Windows cooperative level
	DWORD dwCoopFlags = DDSCL_NORMAL;

	if (m_bIsFullscreen)
		dwCoopFlags = DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;

	// By default, set the flag to allow D3D to optimize floating point calcs
	if (0L == (dwFlags & D3DFW_NO_FPUSETUP))
		dwCoopFlags |= DDSCL_FPUSETUP;

	if (FAILED(m_pDD->SetCooperativeLevel(m_hWnd, dwCoopFlags)))
	{
		DEBUG_MSG(_T("Couldn't set cooperative level"));
		return D3DFWERR_COULDNTSETCOOPLEVEL;
	}

	// Check that we are NOT in a palettized display. That case will fail,
	// since the framework doesn't use palettes.
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	m_pDD->GetDisplayMode(&ddsd);

	if (ddsd.ddpfPixelFormat.dwRGBBitCount <= 8)
		return D3DFWERR_INVALIDMODE;

	return S_OK;
}




//-----------------------------------------------------------------------------
// CreateFullscreenBuffers()
//   Creates the primary buffer and optionally the backbuffer for rendering.
//   Windowed/fullscreen mode are handled differently.
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::CreateFullscreenBuffers(DDSURFACEDESC2 * pddsd)
{
	HRESULT hr;

	// Get the dimensions of the screen bounds
	// Store the rectangle which contains the renderer
	SetRect(&m_rcScreenRect, 0, 0, pddsd->dwWidth, pddsd->dwHeight); 
	m_dwRenderWidth  = m_rcScreenRect.right  - m_rcScreenRect.left;
	m_dwRenderHeight = m_rcScreenRect.bottom - m_rcScreenRect.top;

	// Set the display mode to the requested dimensions. Check for
	// 640x480x16 modes, and set flag to avoid using ModeX
	DWORD dwModeFlags = 0;

	if ((640 == m_dwRenderWidth) && (480 == m_dwRenderHeight) &&
	        (16 == pddsd->ddpfPixelFormat.dwRGBBitCount))
		dwModeFlags |= DDSDM_STANDARDVGAMODE;

	if (FAILED(m_pDD->SetDisplayMode(m_dwRenderWidth, m_dwRenderHeight,
	                                 pddsd->ddpfPixelFormat.dwRGBBitCount,
	                                 pddsd->dwRefreshRate, dwModeFlags)))
	{
		DEBUG_MSG(_T("Unable to set display mode"));
		return D3DFWERR_BADDISPLAYMODE;
	}

	// Setup to create the primary surface w/backbuffer
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize            = sizeof(ddsd);
	ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE |
	                         DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTANTIALIASING;
	ddsd.dwBackBufferCount = 1;

	// Support for stereoscopic viewing
	if (m_bIsStereo)
	{
		ddsd.ddsCaps.dwCaps  |= DDSCAPS_VIDEOMEMORY;
		ddsd.ddsCaps.dwCaps2 |= DDSCAPS2_STEREOSURFACELEFT;
	}

	// Create the primary surface
	if (FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsFrontBuffer, NULL)))
	{
		DEBUG_MSG(_T("Error: Can't create primary surface"));

		if (hr != DDERR_OUTOFVIDEOMEMORY)
			return D3DFWERR_NOPRIMARY;

		DEBUG_MSG(_T("Error: Out of video memory"));
		return DDERR_OUTOFVIDEOMEMORY;
	}

	// Get the backbuffer, which was created along with the primary.
	DDSCAPS2 ddscaps = { DDSCAPS_BACKBUFFER, 0, 0, 0 };

	if (FAILED(hr = m_pddsFrontBuffer->GetAttachedSurface(&ddscaps,
	                &m_pddsBackBuffer)))
	{
		DEBUG_ERR(hr, _T("Error: Can't get the backbuffer"));
		return D3DFWERR_NOBACKBUFFER;
	}

	// Increment the backbuffer count (for consistency with windowed mode)
	m_pddsBackBuffer->AddRef();

	// Support for stereoscopic viewing
	if (m_bIsStereo)
	{
		// Get the left backbuffer, which was created along with the primary.
		DDSCAPS2 ddscaps = { 0, DDSCAPS2_STEREOSURFACELEFT, 0, 0 };

		if (FAILED(hr = m_pddsBackBuffer->GetAttachedSurface(&ddscaps,
		                &m_pddsBackBufferLeft)))
		{
			DEBUG_ERR(hr, _T("Error: Can't get the left backbuffer"));
			return D3DFWERR_NOBACKBUFFER;
		}

		m_pddsBackBufferLeft->AddRef();
	}

	bitdepth = pddsd->ddpfPixelFormat.dwRGBBitCount;
	return S_OK;
}




//-----------------------------------------------------------------------------
// CreateWindowedBuffers()
//  Creates the primary buffer and optionally the backbuffer for rendering.
//  Windowed/fullscreen mode are handled differently.
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::CreateWindowedBuffers()
{
	HRESULT hr;

	// Get the dimensions of the viewport and screen bounds
	GetClientRect(m_hWnd, &m_rcScreenRect);

	ClientToScreen(m_hWnd, (POINT *)&m_rcScreenRect.left);
	ClientToScreen(m_hWnd, (POINT *)&m_rcScreenRect.right);
	m_dwRenderWidth  = m_rcScreenRect.right  - m_rcScreenRect.left;
	m_dwRenderHeight = m_rcScreenRect.bottom - m_rcScreenRect.top;

	// Create the primary surface
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize         = sizeof(ddsd);
	ddsd.dwFlags        = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTANTIALIASING;

	if (FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsFrontBuffer, NULL)))
	{
		DEBUG_MSG(_T("Error: Unable to create primary surface"));

		if (hr != DDERR_OUTOFVIDEOMEMORY)
			return D3DFWERR_NOPRIMARY;

		DEBUG_MSG(_T("Error: Out of video memory"));
		return DDERR_OUTOFVIDEOMEMORY;
	}

	// If in windowed-mode, create a clipper object
	LPDIRECTDRAWCLIPPER pcClipper;

	if (FAILED(hr = m_pDD->CreateClipper(0, &pcClipper, NULL)))
	{
		DEBUG_MSG(_T("Error: Unable to create clipper"));
		return D3DFWERR_NOCLIPPER;
	}

	// Associate the clipper with the window
	pcClipper->SetHWnd(0, m_hWnd);
	m_pddsFrontBuffer->SetClipper(pcClipper);
	SAFE_RELEASE(pcClipper);

	// Create a backbuffer
	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS ;
	ddsd.dwWidth        = m_dwRenderWidth;
	ddsd.dwHeight       = m_dwRenderHeight;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTANTIALIASING;


	if (FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsBackBuffer, NULL)))
	{
		DEBUG_ERR(hr, _T("Error: Unable to create the backbuffer"));

		if (hr != DDERR_OUTOFVIDEOMEMORY)
			return D3DFWERR_NOBACKBUFFER;

		DEBUG_MSG(_T("Error: Out of video memory"));
		return DDERR_OUTOFVIDEOMEMORY;
	}

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	m_pddsFrontBuffer->GetSurfaceDesc(&ddsd);
	bitdepth = ddsd.ddpfPixelFormat.dwRGBBitCount;
	return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CreateDirect3D()
// Desc: Create the Direct3D interface
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::CreateDirect3D(GUID * pDeviceGUID)
{
	// Query DirectDraw for access to Direct3D
	if (FAILED(m_pDD->QueryInterface(IID_IDirect3D7, (VOID **)&m_pD3D)))
	{
		DEBUG_MSG(_T("Unable to Access Direct3D interface"));
		return D3DFWERR_NODIRECT3D;
	}

	// Create the device
	if (FAILED(m_pD3D->CreateDevice(*pDeviceGUID, m_pddsBackBuffer,
	                                &m_pd3dDevice)))
	{
		DEBUG_MSG(_T("Unable to create D3DDevice"));
		return D3DFWERR_NO3DDEVICE;
	}

	// Finally, set the viewport for the newly created device
	D3DVIEWPORT7 vp = { 0, 0, m_dwRenderWidth, m_dwRenderHeight, 0.f, 1.f };

	if (FAILED(m_pd3dDevice->SetViewport(&vp)))
	{
		DEBUG_MSG(_T("Unable to set current viewport to device"));
		return D3DFWERR_NOVIEWPORT;
	}

	return S_OK;
}



int compareZBuffer(const void * arg1, const void * arg2)
{
	DDPIXELFORMAT	* p1 = (DDPIXELFORMAT *)arg1;
	DDPIXELFORMAT	* p2 = (DDPIXELFORMAT *)arg2;

	if (p1->dwZBufferBitDepth < p2->dwZBufferBitDepth) return -1;
	else if (p1->dwZBufferBitDepth > p2->dwZBufferBitDepth) return 1;
	else return 0;
}


//-----------------------------------------------------------------------------
// Name: CreateZBuffer()
// Desc: Internal function called by Create() to make and attach a zbuffer
//       to the renderer
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::CreateZBuffer(GUID * pDeviceGUID)
{
	HRESULT hr;

	// Check if the device supports z-bufferless hidden surface removal. If so,
	// we don't really need a z-buffer

	// Get z-buffer dimensions from the render target
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	m_pddsBackBuffer->GetSurfaceDesc(&ddsd);

	// Setup the surface desc for the z-buffer.
	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT ;
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | m_dwDeviceMemType;
	ddsd.ddpfPixelFormat.dwSize = 0;  // Tag the pixel format as unitialized
	ddsd.ddpfPixelFormat.dwRGBBitCount = 16; // Force 16 bits

	// Get an appropiate pixel format from enumeration of the formats. On the
	// first pass, we look for a zbuffer dpeth which is equal to the frame
	// buffer depth (as some cards unfornately require this).
	zbiZBufferInfo.iNbZBufferInfo = 0;
	zbiZBufferInfo.pddpfPixelFormat = NULL;

	m_pD3D->EnumZBufferFormats(*pDeviceGUID, EnumZBufferFormatsCallback,
	                           (VOID *)&ddsd.ddpfPixelFormat);
	qsort(zbiZBufferInfo.pddpfPixelFormat, zbiZBufferInfo.iNbZBufferInfo, sizeof(ddsd.ddpfPixelFormat), compareZBuffer);
	
	if (IsEqualIID(*pDeviceGUID, IID_IDirect3DHALDevice) ||
	        IsEqualIID(*pDeviceGUID, IID_IDirect3DTnLHalDevice))
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	else
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

	while (zbiZBufferInfo.iNbZBufferInfo--)
	{
		ddsd.ddpfPixelFormat = zbiZBufferInfo.pddpfPixelFormat[zbiZBufferInfo.iNbZBufferInfo];

		// Create and attach a z-buffer
		if (FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsZBuffer, NULL)))
		{
			continue;
		}

		if (FAILED(m_pddsBackBuffer->AddAttachedSurface(m_pddsZBuffer)))
		{
			SAFE_RELEASE(m_pddsZBuffer);
			continue;
		}

		// For stereoscopic viewing, attach zbuffer to left surface as well
		if (m_bIsStereo)
		{
			if (FAILED(m_pddsBackBufferLeft->AddAttachedSurface(m_pddsZBuffer)))
			{
				SAFE_RELEASE(m_pddsZBuffer);
				continue;
			}
		}

		// Finally, this call rebuilds internal structures
		if (SUCCEEDED(m_pd3dDevice->SetRenderTarget(m_pddsBackBuffer, 0L)))
		{
			free((void *)zbiZBufferInfo.pddpfPixelFormat);
			return S_OK;
		}

		SAFE_RELEASE(m_pddsZBuffer);
	}

	free((void *)zbiZBufferInfo.pddpfPixelFormat);
	DEBUG_MSG(_T("Error: SetRenderTarget() failed after attaching zbuffer!"));
	return D3DFWERR_NOZBUFFER;
}

//-----------------------------------------------------------------------------
// Name: RestoreSurfaces()
// Desc: Checks for lost surfaces and restores them if lost. Note: Don't
//       restore render surface, since it's just a duplicate ptr.
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::RestoreSurfaces()
{
	// Restore all surfaces (including video memory vertex buffers)
	m_pDD->RestoreAllSurfaces();

	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Move()
// Desc: Moves the screen rect for windowed renderers
//-----------------------------------------------------------------------------
VOID CD3DFramework7::Move(INT x, INT y)
{
	if (TRUE == m_bIsFullscreen)
		return;

	if (TRUE == b_dlg)
	{
		POINT pt;
		m_bHasMoved = TRUE;
		GetClientRect(m_hWnd, &m_rcScreenRect);
		pt.x = 0;
		pt.y = 0;
		ClientToScreen(m_hWnd, &pt);
		m_rcScreenRect.top += pt.y;
		m_rcScreenRect.left += pt.x;
		m_rcScreenRect.bottom += pt.y;
		m_rcScreenRect.right += pt.x;
		ClipWindow(0, 0, m_dwRenderWidth, m_dwRenderHeight);
	}
	else
	{
		m_bHasMoved = TRUE;
		SetRect(&m_rcScreenRect, x, y, x + m_dwRenderWidth, y + m_dwRenderHeight);
		ClipWindow(0, 0, m_dwRenderWidth, m_dwRenderHeight);
	}
}

 
VOID CD3DFramework7::ClipWindow(long x0, long y0, long x1, long y1)
{
	long height, width;
	height = m_rcScreenRect.bottom - m_rcScreenRect.top;
	width = m_rcScreenRect.right - m_rcScreenRect.left;

	this->ClipWin.top = y0;
	this->ClipWin.bottom = y1;
	this->ClipWin.left = x0;
	this->ClipWin.right = x1;

	if (this->ClipWin.top < 0) this->ClipWin.top = 0;
	else if (this->ClipWin.top > height) this->ClipWin.top = height;

	if (this->ClipWin.bottom < 0) this->ClipWin.bottom = 0;
	else if (this->ClipWin.bottom > height) this->ClipWin.bottom = height - 1;

	if (this->ClipWin.left < 0) this->ClipWin.left = 0;
	else if (this->ClipWin.left > width) this->ClipWin.left = width - 1;

	if (this->ClipWin.right < 0) this->ClipWin.right = 0;
	else if (this->ClipWin.right > width) this->ClipWin.right = width - 1;

	if (this->ClipWin.top > this->ClipWin.bottom) this->ClipWin.top = this->ClipWin.bottom;

	if (this->ClipWin.left > this->ClipWin.right) this->ClipWin.left = this->ClipWin.right;
}

//-----------------------------------------------------------------------------
// Name: FlipToGDISurface()
// Desc: Puts the GDI surface in front of the primary, so that dialog
//       boxes and other windows drawing funcs may happen.
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::FlipToGDISurface(BOOL bDrawFrame)
{
	if (m_pDD && m_bIsFullscreen)
	{
		m_pDD->FlipToGDISurface();

		if (bDrawFrame)
		{
			DrawMenuBar(m_hWnd);
			RedrawWindow(m_hWnd, NULL, NULL, RDW_FRAME);
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
bool CD3DFramework7::RenderError()
{
	return true;
}

//-----------------------------------------------------------------------------

bool CD3DFramework7::StartRender()
{
	if (FAILED(m_pd3dDevice->BeginScene()))
	{
		return false;
	}

	usBeginEndSceneCount++;
	return true;
}

//-----------------------------------------------------------------------------
bool CD3DFramework7::EndRender()
{
	if (FAILED(m_pd3dDevice->EndScene()))
	{
		return false;
	}

	usBeginEndSceneCount--;
	return true;
}
DWORD RenderStartTicks = 0;
extern bool bForceGDI;

//-----------------------------------------------------------------------------
// Name: ShowFrame()
// Desc: Show the frame on the primary surface, via a blt or a flip.
//-----------------------------------------------------------------------------
HRESULT CD3DFramework7::ShowFrame()
{
	RECT rc, screct;

	if (NULL == m_pddsFrontBuffer)
		return D3DFWERR_NOTINITIALIZED;

	RenderError();

	if (m_bIsFullscreen)
	{
		if (Project.vsync)
		{
			return m_pddsFrontBuffer->Flip(NULL, DDFLIP_WAIT); 
		}

		rc.top = 0;
		rc.left = 0;
		rc.right = m_rcScreenRect.right - m_rcScreenRect.left;
		rc.bottom = m_rcScreenRect.bottom - m_rcScreenRect.top;
		return m_pddsFrontBuffer->Blt(&m_rcScreenRect, m_pddsBackBuffer,
		                              &rc, DDBLT_WAIT   , NULL);
	}
	else
	{
		// We are in windowed mode, so perform a blit.
		rc.top = 0;
		rc.left = 0;
		rc.right = m_rcScreenRect.right - m_rcScreenRect.left - this->Xstart;
		rc.bottom = m_rcScreenRect.bottom - m_rcScreenRect.top - this->Ystart;
		screct.left = m_rcScreenRect.left + this->Xstart;
		screct.right = m_rcScreenRect.right;
		screct.top = m_rcScreenRect.top + this->Ystart;
		screct.bottom = m_rcScreenRect.bottom;
		return m_pddsFrontBuffer->Blt(&screct , m_pddsBackBuffer, &rc, DDBLT_WAIT, NULL);
	}
}
