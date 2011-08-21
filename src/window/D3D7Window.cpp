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

#include "window/D3D7Window.h"

#include <algorithm>

#include "graphics/d3d7/D3D7Renderer.h"
#include "io/Logger.h"
#include "math/Rectangle.h"

using std::string;
using std::vector;

D3D7Window::D3D7Window() : dd(NULL), d3d(NULL), backBuffer(NULL), frontBuffer(NULL), device(NULL), deviceInfo(NULL) { }

D3D7Window::~D3D7Window() {
	destroyObjects();
}

// Callback function for enumerating display modes.
HRESULT WINAPI D3D7Window::modeEnumCallback(DDSURFACEDESC2 * pddsd, VOID * devinfo) {
	
	DeviceInfo * dev = reinterpret_cast<DeviceInfo *>(devinfo);
	
	dev->modes.push_back(*pddsd);
	
	return DDENUMRET_OK;
}

// Callback function for sorting display modes.
static bool modesOrdering(const DDSURFACEDESC2 & p1, const DDSURFACEDESC2 & p2) {
	
	if(p1.dwWidth != p2.dwWidth) {
		return (p1.dwWidth < p2.dwWidth);
	} else if(p1.dwHeight != p2.dwHeight) {
		return (p1.dwHeight < p2.dwHeight);
	} else {
		return (p1.ddpfPixelFormat.dwRGBBitCount < p2.ddpfPixelFormat.dwRGBBitCount);
	}
}

// Callback function for enumerating devices
HRESULT WINAPI D3D7Window::deviceEnumCallback(char * desc, char * name, D3DDEVICEDESC7 * pDesc, VOID * window) {
	
	if((pDesc->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) != D3DDEVCAPS_HWRASTERIZATION) {
		LogDebug << "Skipping software D3D device: " << name;
		return D3DENUMRET_OK;
	}
	
	if(pDesc->wMaxSimultaneousTextures < 3) {
		LogWarning << "Skipping D3D device: " << name << ", supported texture texture stages " << pDesc->wMaxSimultaneousTextures << " < 3";
		return D3DENUMRET_OK;
	}
	
	if(!(pDesc->dwTextureOpCaps & D3DTEXOPCAPS_ADDSIGNED)) {
		LogWarning << "Skipping D3D device: " << name << ", missing the ADDSIGNED texture op";
		return D3DENUMRET_OK;
	}
	
	D3D7Window * win = reinterpret_cast<D3D7Window *>(window);
	
	arx_assert(!win->devices.empty());
	win->devices.push_back(win->devices.back());
	DeviceInfo * dev = &*(win->devices.rbegin() + 1);
	
	dev->name = name;
	dev->desc = desc;
	dev->device = *pDesc;
	
	DWORD dwRenderDepths = pDesc->dwDeviceRenderBitDepth;
	
	// Build list of supported modes for the device
	for(size_t i = 0 ; i < dev->modes.size(); ) {
		
		DDSURFACEDESC2 & mode = dev->modes[i];
		DWORD depth = mode.ddpfPixelFormat.dwRGBBitCount;
		
		// Accept modes that are compatable with the device
		if(((depth == 32) && (dwRenderDepths & DDBD_32)) ||
		   ((depth == 24) && (dwRenderDepths & DDBD_24)) ||
		   ((depth == 16) && (dwRenderDepths & DDBD_16))) {
			// Accept the mode;
			i++;
		} else {
			dev->modes.erase(dev->modes.begin() + i);
		}
	}
	
	// Bail if the device has no supported modes
	if(dev->modes.empty()) {
		LogWarning << "Skipping D3D device: " << name << ", no compatible display modes.";
		win->devices.erase(win->devices.end() - 2);
		return D3DENUMRET_OK;
	}
	
	return D3DENUMRET_OK;
}

// Callback function for enumerating drivers.
BOOL WINAPI D3D7Window::driverEnumCallback(GUID * pGUID, char * desc, char * name,
                                           VOID * window, HMONITOR){
	
	HRESULT hr;
	
	// Use the GUID to create the DirectDraw object
	LPDIRECTDRAW7 pDD;
	if(FAILED(hr = DirectDrawCreateEx(pGUID, (VOID **)&pDD, IID_IDirectDraw7, NULL))) {
		LogError << "Can't create DDraw for driver " << name << " during enumeration!";
		return D3DENUMRET_OK;
	}
	
	DDCAPS ddDriverCaps;
	memset(&ddDriverCaps, 0, sizeof(ddDriverCaps));
	ddDriverCaps.dwSize = sizeof(DDCAPS);
	if(FAILED(pDD->GetCaps(&ddDriverCaps, NULL))) {
		pDD->Release();
		LogError << "Can't get driver caps for " << name << " during enumeration!";
		return D3DENUMRET_OK;
	}
	
	if(!(ddDriverCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)) {
		pDD->Release();
		LogInfo << "Skipping " << name << " during enumeration! (does not support windowed mode)";
		return D3DENUMRET_OK;
	}
	
	// Create a D3D object, to enumerate the d3d devices
	LPDIRECT3D7 pD3D;
	if(FAILED(hr = pDD->QueryInterface(IID_IDirect3D7, (VOID **)&pD3D))) {
		pDD->Release();
		LogError << "Can't query IDirect3D7 for driver " << name << " during enumeration!";
		return D3DENUMRET_OK;
	}
	
	D3D7Window * win = reinterpret_cast<D3D7Window *>(window);
	
	win->devices.resize(win->devices.size() + 1);
	DeviceInfo & dev = win->devices.back();
	
	dev.driver = name;
	dev.driverDesc = desc;
	if(pGUID) {
		dev.driverGUID = *pGUID;
	} else {
		memset(&dev.driverGUID, 0, sizeof(dev.driverGUID));
	}
	
	// Enumerate the fullscreen display modes.
	pDD->EnumDisplayModes(0, NULL, &dev, modeEnumCallback);
	
	// Sort list of display modes
	std::sort(dev.modes.begin(), dev.modes.end(), modesOrdering);
	
	// Now, enumerate all the 3D devices
	pD3D->EnumDevices(deviceEnumCallback, window);
	
	win->devices.pop_back();
	
	pD3D->Release();
	pDD->Release();
	
	return DDENUMRET_OK;
}

bool D3D7Window::initFramework() {
	
	arx_assert(devices.empty() && displayModes.empty());
	
	// Enumerate drivers, devices, and modes
	DirectDrawEnumerateEx(driverEnumCallback, this,
	                      DDENUM_ATTACHEDSECONDARYDEVICES |
	                      DDENUM_DETACHEDSECONDARYDEVICES |
	                      DDENUM_NONDISPLAYDEVICES);
	
	displayModes.clear();
	
	for(vector<DeviceInfo>::const_iterator dev = devices.begin(); dev != devices.end(); ++dev) {
		for(vector<DDSURFACEDESC2>::const_iterator m = dev->modes.begin(); m != dev->modes.end(); ++m) {
			DisplayMode mode(Vec2i(m->dwWidth, m->dwHeight), m->ddpfPixelFormat.dwRGBBitCount);
			if(std::find(displayModes.begin(), displayModes.end(), mode) == displayModes.end()) {
				displayModes.push_back(mode);
			}
		}
	}
	
	if(devices.empty() || displayModes.empty()) {
		LogError << "No compatible D3D devices found!";
		return false;
	}
	
	LogInfo << "Using Win32 windowing";
	
	std::sort(displayModes.begin(), displayModes.end());
	
	return true;
}

bool D3D7Window::init(const std::string & title, Vec2i size, bool fullscreen, unsigned depth) {
	
	arx_assert(deviceInfo == NULL);
	arx_assert(!devices.empty() && !displayModes.empty());
	
	if(!Win32Window::init(title, size, fullscreen, depth)) {
		return false;
	}
	
	for(vector<DeviceInfo>::iterator dev = devices.begin(); dev != devices.end(); ++dev) {
		
		if(IsEqualIID(dev->device.deviceGUID, IID_IDirect3DTnLHalDevice)) {
			continue;
		}
		
		deviceInfo = &*dev;
		if(initialize(DisplayMode(size, depth))) {
			displayModes.clear();
			for(vector<DDSURFACEDESC2>::iterator m = dev->modes.begin(); m != dev->modes.end(); ++m) {
				DisplayMode mode(Vec2i(m->dwWidth, m->dwHeight), m->ddpfPixelFormat.dwRGBBitCount);
				displayModes.push_back(mode);
			}
			
			LogInfo << "Using Win32 windowing";
			
			return true;
		} else {
			LogWarning << "Failed to initialize " << dev->name << " (" << dev->desc << ") using " << dev->driver;
			destroyObjects();
		}
		
	}
	
	deviceInfo = NULL;
	
	LogError << "Could not initialize any D3D device!";
	
	return false;
}

bool D3D7Window::showFrame() {
	
	if(!frontBuffer) {
		return false;
	}
	
	HRESULT ret;
	
	if(IsFullScreen()) {
		
		if(isVSync()) {
			ret = frontBuffer->Flip(NULL, DDFLIP_WAIT); 
		} else {
			RECT rect = { 0, 0, GetSize().x, GetSize().y };
			ret = frontBuffer->Blt(&rect, backBuffer, NULL, DDBLT_WAIT, NULL);
		}
		
	} else {
		
		// We are in windowed mode, so perform a blit.
		RECT rect;
		GetClientRect((HWND)GetHandle(), &rect);
		ClientToScreen((HWND)GetHandle(), (LPPOINT)&rect.left); // TODO violates strict aliasing?
		ClientToScreen((HWND)GetHandle(), (LPPOINT)&rect.right);
		
		ret = frontBuffer->Blt(&rect, backBuffer, NULL, DDBLT_WAIT, NULL);
	
	if(FAILED(ret)) {
		LogWarning << "Show frame failed: " << ret << " " << (ret&0xFFFF) <<" <- look for this in ddraw.h";
		return true;
	}
	}
	
	if(ret == DDERR_SURFACELOST) {
		restoreSurfaces();
		return true;
	}
	
	if(FAILED(ret)) {
		LogWarning << "Show frame failed: " << ret << " " << (ret&0xFFFF) <<" <- look for this in ddraw.h";
		return false;
	}
	
	return true;
}

void D3D7Window::restoreSurfaces() {
	dd->RestoreAllSurfaces();
}

void D3D7Window::destroyObjects() {
	
	if(renderer) {
		onRendererShutdown();
		delete renderer, renderer = NULL;
	}

	if(dd) {
		dd->SetCooperativeLevel((HWND)GetHandle(), DDSCL_NORMAL);
	}
	
	// Do a safe check for releasing the D3DDEVICE. RefCount must be zero.
	if(device) {
		if(0 < device->Release()) {
			LogWarning << "D3DDevice object is still referenced";
		}
		device = NULL;
	}
	
	if(backBuffer) {
		backBuffer->Release(), backBuffer = NULL;
	}
	
	if(frontBuffer) {
		frontBuffer->Release(), frontBuffer = NULL;
	}

	if(d3d) {
		d3d->Release(), d3d = NULL;
	}
	
	if(dd) {
		// Do a safe check for releasing DDRAW. RefCount must be zero.
		if(0 < dd->Release()) {
			LogWarning << "DDraw object is still referenced";
		}
		dd = NULL;
	}
}

// Internal functions for the framework class
bool D3D7Window::initialize(DisplayMode mode) {
	
	arx_assert(GetHandle() != NULL);
	arx_assert(renderer == NULL);
	
	// Create the DDraw object
	if(!createDirectDraw(&deviceInfo->driverGUID)) {
		return false;
	}
	
	// Create the front and back buffers, and attach a clipper
	if(IsFullScreen()) {
		if(!createFullscreenBuffers(mode)) {
			return false;
		}
	} else {
		if(!createWindowedBuffers()) {
			return false;
		}
	}
	
	// Create the Direct3D object and the Direct3DDevice object
	if(!createDirect3D(&deviceInfo->device.deviceGUID)) {
		return false;
	}
	
	// Create and attach the zbuffer
	if(!createZBuffer(&deviceInfo->device.deviceGUID)) {
		delete renderer, renderer = NULL;
		return false;
	}
	
	renderer = new D3D7Renderer(this);
	
	renderer->Initialize();
	
	// Finally, set the viewport for the newly created device
	renderer->SetViewport(Rect(GetSize().x, GetSize().y));
	
	onRendererInit();
	
	return true;
}

bool D3D7Window::createDirectDraw(GUID * driver) {
	
	arx_assert(dd == NULL);
	
	// Create the DirectDraw interface, and query for the DD7 interface
	if(FAILED(DirectDrawCreateEx(driver, (VOID **)&dd, IID_IDirectDraw7, NULL))) {
		LogError << "Could not create DirectDraw";
		return false;
	}
	
	// Set the Windows cooperative level
	DWORD dwCoopFlags = DDSCL_FPUSETUP;
	if(IsFullScreen()) {
		dwCoopFlags |= DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN;
	} else {
		dwCoopFlags |= DDSCL_NORMAL;
	}
	
	if(FAILED(dd->SetCooperativeLevel((HWND)GetHandle(), dwCoopFlags))) {
		LogError << "Couldn't set cooperative level";
		return false;
	}
	
	// Check that we are NOT in a palettized display. That case will fail,
	// since the framework doesn't use palettes.
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	dd->GetDisplayMode(&ddsd);
	
	if(ddsd.ddpfPixelFormat.dwRGBBitCount <= 8) {
		LogError << "Palettized display not supported";
		return false;
	}
	
	return true;
}

bool D3D7Window::createFullscreenBuffers(DisplayMode req) {
	
	arx_assert(frontBuffer == NULL);
	arx_assert(backBuffer == NULL);
	
	// Find the best matching display mode.
	DDSURFACEDESC2 * mode = NULL;
	std::vector<DDSURFACEDESC2>::iterator i;
	for(i = deviceInfo->modes.begin(); i != deviceInfo->modes.end(); ++i) {
		
		if(i->dwWidth != DWORD(req.resolution.x) || i->dwHeight != DWORD(req.resolution.y)) {
			continue;
		}
		
		if(req.depth) {
			if(i->ddpfPixelFormat.dwRGBBitCount != req.depth) {
				continue;
			}
		}
		
		if(mode && mode->dwRefreshRate == 60) {
			continue;
		}
		
		mode = &*i;
	}
	if(!mode) {
		return false;
	}
	
	// Set the display mode to the requested dimensions. Check for
	// 640x480x16 modes, and set flag to avoid using ModeX
	DWORD dwModeFlags = 0;
	if((640 == mode->dwWidth) && (480 == mode->dwHeight) &&
	   (16 == mode->ddpfPixelFormat.dwRGBBitCount)) {
		dwModeFlags |= DDSDM_STANDARDVGAMODE;
	}
	
	if(FAILED(dd->SetDisplayMode(mode->dwWidth, mode->dwHeight,
	                             mode->ddpfPixelFormat.dwRGBBitCount,
	                             mode->dwRefreshRate, dwModeFlags))) {
		LogError << "Unable to set display mode";
		return false;
	}
	
	m_Size = Vec2i(mode->dwWidth, mode->dwHeight);
	depth = mode->ddpfPixelFormat.dwRGBBitCount;
	
	// Setup to create the primary surface w/backbuffer
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize            = sizeof(ddsd);
	ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE
	                         | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTANTIALIASING;
	ddsd.dwBackBufferCount = 1;
	
	// Create the primary surface
	if(FAILED(dd->CreateSurface(&ddsd, &frontBuffer, NULL))) {
		LogWarning << "Can't create primary surface";
		return false;
	}
	
	// Get the backbuffer, which was created along with the primary.
	DDSCAPS2 ddscaps = { DDSCAPS_BACKBUFFER, 0, 0, 0 };
	if(FAILED(frontBuffer->GetAttachedSurface(&ddscaps, &backBuffer))) {
		LogError << "Can't get the backbuffer";
		return false;
	}
	
	// Increment the backbuffer count (for consistency with windowed mode)
	backBuffer->AddRef();
	
	return true;
}

bool D3D7Window::createWindowedBuffers() {
	
	arx_assert(frontBuffer == NULL);
	arx_assert(backBuffer == NULL);
	
	// Create the primary surface
	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize         = sizeof(ddsd);
	ddsd.dwFlags        = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTANTIALIASING;
	
	if(FAILED(dd->CreateSurface(&ddsd, &frontBuffer, NULL))) {
		LogWarning << "Unable to create primary surface";
		return false;
	}
	
	// If in windowed-mode, create a clipper object
	LPDIRECTDRAWCLIPPER pcClipper;
	if(FAILED(dd->CreateClipper(0, &pcClipper, NULL))) {
		LogWarning << "Unable to create clipper";
		return false;
	}
	
	// Associate the clipper with the window
	if(FAILED(pcClipper->SetHWnd(0, (HWND)GetHandle()))) {
		LogWarning << "Unable to link clipper to window";
		return false;
	}
	
	if(frontBuffer->SetClipper(pcClipper)) {
		LogWarning << "Unable set clipper";
		return false;
	}
	
	pcClipper->Release(), pcClipper = NULL;

	// Create a backbuffer
	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS ;
	ddsd.dwWidth        = GetSize().x;
	ddsd.dwHeight       = GetSize().y;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
	ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTANTIALIASING;
	
	if(FAILED(dd->CreateSurface(&ddsd, &backBuffer, NULL))) {
		LogError << "Unable to create the backbuffer";
		return false;
	}
	
	return true;
}

bool D3D7Window::createDirect3D(GUID * deviceGUID) {
	
	arx_assert(d3d == NULL);
	
	// Query DirectDraw for access to Direct3D
	if(FAILED(dd->QueryInterface(IID_IDirect3D7, (VOID **)&d3d))) {
		LogWarning << "Unable to Access Direct3D interface";
		return false;
	}

	// Create the device
	if(FAILED(d3d->CreateDevice(*deviceGUID, backBuffer, &device))) {
		LogWarning << "Unable to create D3DDevice";
		return false;
	}
	
	return true;
	
}

static HRESULT WINAPI enumZBufferFormatsCallback(DDPIXELFORMAT * pddpf, VOID * pContext) {
	
	std::vector<DDPIXELFORMAT> * formats = reinterpret_cast<std::vector<DDPIXELFORMAT> *>(pContext);
	
	if(pddpf->dwFlags == DDPF_ZBUFFER) {
		formats->push_back(*pddpf);
	}
	
	return D3DENUMRET_OK;
}

bool compareZBuffer(const DDPIXELFORMAT & p1, const DDPIXELFORMAT & p2) {
	return (p1.dwZBufferBitDepth < p2.dwZBufferBitDepth);
}

bool D3D7Window::createZBuffer(GUID * deviceGUID) {
	
	// Select the default memory type, for whether the device is HW or SW
	DWORD m_dwDeviceMemType;
	if(IsEqualIID(*deviceGUID, IID_IDirect3DHALDevice)) {
		m_dwDeviceMemType = DDSCAPS_VIDEOMEMORY;
	} else if(IsEqualIID(*deviceGUID, IID_IDirect3DTnLHalDevice)) {
		m_dwDeviceMemType = DDSCAPS_VIDEOMEMORY;
	} else {
		m_dwDeviceMemType = DDSCAPS_SYSTEMMEMORY;
	}
	
	// Check if the device supports z-bufferless hidden surface removal. If so,
	// we don't really need a z-buffer
	
	// Get z-buffer dimensions from the render target
	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	backBuffer->GetSurfaceDesc(&ddsd);
	
	// Setup the surface desc for the z-buffer.
	ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT ;
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | m_dwDeviceMemType;
	ddsd.ddpfPixelFormat.dwSize = 0;  // Tag the pixel format as unitialized
	ddsd.ddpfPixelFormat.dwRGBBitCount = 16; // Force 16 bits
	
	std::vector<DDPIXELFORMAT> formats;
	
	// Get an appropiate pixel format from enumeration of the formats. On the
	// first pass, we look for a zbuffer dpeth which is equal to the frame
	// buffer depth (as some cards unfornately require this).
	
	d3d->EnumZBufferFormats(*deviceGUID, enumZBufferFormatsCallback, (VOID *)&formats);
	
	std::sort(formats.begin(), formats.end(), compareZBuffer);
	
	for(std::vector<DDPIXELFORMAT>::reverse_iterator i = formats.rbegin(); i != formats.rend(); ++i) {
		
		ddsd.ddpfPixelFormat = *i;
		
		// Create and attach a z-buffer
		LPDIRECTDRAWSURFACE7 m_pddsZBuffer; // The zbuffer surface
		if(FAILED(dd->CreateSurface(&ddsd, &m_pddsZBuffer, NULL))) {
			continue;
		}
		
		if(FAILED(backBuffer->AddAttachedSurface(m_pddsZBuffer))) {
			m_pddsZBuffer->Release(), m_pddsZBuffer = NULL;
			continue;
		}
		
		m_pddsZBuffer->Release(), m_pddsZBuffer = NULL;
		
		// Finally, this call rebuilds internal structures
		if(SUCCEEDED(device->SetRenderTarget(backBuffer, 0L))) {
			return true;
		}
	}
	
	LogWarning << "SetRenderTarget() failed after attaching zbuffer!";
	return false;
}

void D3D7Window::evictManagedTextures() {
	
	if(d3d) {
		d3d->EvictManagedTextures();
	}
}

void D3D7Window::setFullscreenMode(Vec2i resolution, unsigned _depth) {
	
	if(m_IsFullscreen && m_Size == resolution && depth == _depth) {
		return;
	}
	
	bool oldFullscreen = m_IsFullscreen;
	DisplayMode oldMode(m_Size, depth);
	
	// Release all scene objects that will be re-created for the new device
	// Release framework objects, so a new device can be created
	destroyObjects();
	
	// Check if going from fullscreen to windowed mode, or vice versa.
	Win32Window::setFullscreenMode(resolution, _depth);
	
	if(!initialize(DisplayMode(resolution, depth))) {
		
		// try to restore the previous resolution
		
		destroyObjects();
		
		if(oldFullscreen) {
			Win32Window::setFullscreenMode(oldMode.resolution, oldMode.depth);
		} else {
			Win32Window::setWindowSize(oldMode.resolution);
		}
		
		if(!initialize(oldMode)) {
			LogError << "failed to restore display mode!";
		}
		
	}
	
}

void D3D7Window::setWindowSize(Vec2i size) {
	
	if(!m_IsFullscreen && size == GetSize()) {
		return;
	}
	
	bool oldFullscreen = m_IsFullscreen;
	DisplayMode oldMode(m_Size, depth);
	
	if(oldFullscreen) {
		destroyObjects();
	}
	
	Win32Window::setWindowSize(size);
	
	if(oldFullscreen) {
		
		if(!initialize(DisplayMode(size, 0))) {
			
			// try to restore the previous resolution
			
			destroyObjects();
			
			if(oldFullscreen) {
				Win32Window::setFullscreenMode(oldMode.resolution, oldMode.depth);
			} else {
				Win32Window::setWindowSize(oldMode.resolution);
			}
			
			if(!initialize(oldMode)) {
				LogError << "failed to restore display mode!";
			}
			
		}
		
	}
	
}
