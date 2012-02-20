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
/* Based on:
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
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "window/D3D9Window.h"

#include <algorithm>

#include "core/Config.h"
#include "graphics/d3d9/D3D9Renderer.h"
#include "io/log/Logger.h"
#include "math/Rectangle.h"
#include "platform/Thread.h"

using std::string;
using std::vector;

LPDIRECT3DDEVICE9 GD3D9Device = NULL;

D3D9Window::D3D9Window() : deviceInfo(NULL), d3d(NULL) { }

D3D9Window::~D3D9Window() {
	destroyObjects();

	// Do a safe check for releasing the D3DDEVICE. RefCount must be zero.
	if(GD3D9Device) {

		if(0 < GD3D9Device->Release()) {
			LogWarning << "D3DDevice object is still referenced";
		}
		GD3D9Device = NULL;
	}

	if(d3d) {
		d3d->Release(), d3d = NULL;
	}
}

bool D3D9Window::initFramework() {
	
	arx_assert(devices.empty() && displayModes.empty());
	
	devices.clear();

	if( NULL == ( d3d = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return false;

	UINT adapterCount = d3d->GetAdapterCount();
	devices.resize(adapterCount);
	std::vector<DeviceInfo>::iterator it;
	UINT adapterIdx = 0;
	for(it = devices.begin(); it != devices.end(); ++it, ++adapterIdx)
	{
		d3d->GetAdapterIdentifier(adapterIdx, 0, &it->adapterIdentifier);
		it->adapterIdx = adapterIdx;
	}
	
	displayModes.clear();
	
	for(vector<DeviceInfo>::const_iterator it = devices.begin(); it != devices.end(); ++it) {

		// 16 bits
		{
			UINT adapterModeCount = d3d->GetAdapterModeCount(it->adapterIdx, D3DFMT_R5G6B5);
			for(UINT adapterModeIdx = 0; adapterModeIdx < adapterModeCount; ++adapterModeIdx) {
				D3DDISPLAYMODE m;
				d3d->EnumAdapterModes(it->adapterIdx, D3DFMT_R5G6B5, adapterModeIdx, &m);

				DisplayMode mode(Vec2i(m.Width, m.Height), 16);
				if(std::find(displayModes.begin(), displayModes.end(), mode) == displayModes.end()) {
					displayModes.push_back(mode);
				}
			}
		}

		// 32 bits
		{
			UINT adapterModeCount = d3d->GetAdapterModeCount(it->adapterIdx, D3DFMT_X8R8G8B8);
			for(UINT adapterModeIdx = 0; adapterModeIdx < adapterModeCount; ++adapterModeIdx) {
				D3DDISPLAYMODE m;
				d3d->EnumAdapterModes(it->adapterIdx, D3DFMT_X8R8G8B8, adapterModeIdx, &m);

				DisplayMode mode(Vec2i(m.Width, m.Height), 32);
				if(std::find(displayModes.begin(), displayModes.end(), mode) == displayModes.end()) {
					displayModes.push_back(mode);
				}
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

bool D3D9Window::init(const std::string & title, Vec2i size, bool fullscreen, unsigned depth) {
	
	arx_assert(deviceInfo == NULL);
	arx_assert(!devices.empty() && !displayModes.empty());
	
	if(!Win32Window::init(title, size, fullscreen, depth))
		return false;

    DisplayMode displayMode(size, depth);

    if(!updatePresentParams(displayMode))
        return false;
	
	bool succeeded = false;
	if(initialize()) {
		LogInfo << "Using Win32 windowing";
		succeeded = true;
	} else {
		deviceInfo = NULL;
		LogError << "Could not initialize any D3D device!";
	}

	return succeeded;
}

void D3D9Window::tick() {
    HRESULT hr = GD3D9Device->TestCooperativeLevel();
    if(hr == D3DERR_DEVICELOST){
        LogWarning << "D3DERR_DEVICELOST!";
        Thread::sleep(1000);
	} else if(hr == D3DERR_DEVICENOTRESET) {
        LogWarning << "D3DERR_DEVICENOTRESET! - Start";
        destroyObjects();
        d3d9Renderer->reset(d3dpp);
        restoreObjects();
        LogWarning << "D3DERR_DEVICENOTRESET! - End";
    }

    Win32Window::tick();
}

bool D3D9Window::showFrame() {
	GD3D9Device->Present(NULL, NULL, NULL, NULL);
	return true;
}

bool D3D9Window::updatePresentParams(DisplayMode mode) {
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferCount = 1;
    d3dpp.hDeviceWindow = (HWND)GetHandle();
	
	// VSync
	d3dpp.PresentationInterval = config.video.vsync ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE;
	
	// Backbuffer format
	if(IsFullScreen()) {
		d3dpp.Windowed = FALSE;
		d3dpp.BackBufferWidth = m_Size.x;
		d3dpp.BackBufferHeight = m_Size.y;
		d3dpp.BackBufferFormat = mode.depth == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8;
	} else {
		D3DDISPLAYMODE displayMode;
		d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);

		d3dpp.Windowed = TRUE;
		d3dpp.BackBufferFormat = displayMode.Format;
	}

	// Multisampling - Part 1
	// Check for availability in the mode we're going to use
	if(config.video.antialiasing) {
		DWORD QualityLevels;
		if(SUCCEEDED(d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, d3dpp.Windowed, D3DMULTISAMPLE_4_SAMPLES, &QualityLevels))) {
			d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
			d3dpp.MultiSampleQuality = QualityLevels - 1;
		}
	}

	// Depth buffer precision - prefer better precision...
	D3DFORMAT D32Formats[] = { D3DFMT_D32, D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D16, D3DFMT_D15S1, (D3DFORMAT)0 };
	D3DFORMAT* pFormatList = D32Formats;
	while (*pFormatList) {
		if(SUCCEEDED(d3d->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, *pFormatList))) {
			if (SUCCEEDED(d3d->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.BackBufferFormat, d3dpp.BackBufferFormat, *pFormatList))) {
				break;
			}
		}

		pFormatList++;
	};
	
	if( *pFormatList != 0 ) {
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = *pFormatList;
	} else {
		LogError << "Could not find any matching zbuffer format!";
		return false;
	}

    return true;
}

// Internal functions for the framework class
bool D3D9Window::initialize() {
	
	arx_assert(GetHandle() != NULL);
	arx_assert(renderer == NULL);
	
	// Set default settings
	UINT AdapterToUse = D3DADAPTER_DEFAULT;
	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;

	///////////////////////////////////
	// Support for NVIDIA PerfHUD
	// Look for 'NVIDIA PerfHUD' adapter
	// If it is present, override default settings
	for (UINT Adapter=0; Adapter < d3d->GetAdapterCount(); Adapter++) 
	{
		D3DADAPTER_IDENTIFIER9 Identifier;
		d3d->GetAdapterIdentifier(Adapter, 0, &Identifier);
		if (strstr(Identifier.Description,"PerfHUD") != 0)
		{
			AdapterToUse = Adapter;
			DeviceType = D3DDEVTYPE_REF;
			break;
		}
	}
	///////////////////////////////////

	// Create the D3D9 devices
	HRESULT ret = d3d->CreateDevice(AdapterToUse, DeviceType, (HWND)GetHandle(), D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &GD3D9Device);
	if( FAILED(ret) ) {
		LogError << "CreateDevice failed: " << DXGetErrorStringA(ret) << " - " << DXGetErrorDescriptionA(ret);
		return false;
	}

	deviceInfo = &devices[0];
	    
	d3d9Renderer = new D3D9Renderer(this);
    renderer = d3d9Renderer;
	renderer->Initialize();

	// Multisampling - Part 2
	// Set the multisampling render state
	renderer->SetAntialiasing(d3dpp.MultiSampleType != D3DMULTISAMPLE_NONE);
	
	// Finally, set the viewport for the newly created device
	renderer->SetViewport(Rect(GetSize().x, GetSize().y));

    onRendererInit();

	return true;
}

void D3D9Window::changeDisplay(Vec2i resolution, unsigned _depth) {
   
    updatePresentParams(DisplayMode(resolution, _depth));
    destroyObjects();
    d3d9Renderer->reset(d3dpp);
	restoreObjects();
}

void D3D9Window::setFullscreenMode(Vec2i resolution, unsigned _depth) {
	
	if(m_IsFullscreen && m_Size == resolution && depth == _depth) {
		return;
	}
		
	Win32Window::setFullscreenMode(resolution, _depth);
	
    changeDisplay(resolution, _depth);
}

void D3D9Window::setWindowSize(Vec2i size) {
	
	if(!m_IsFullscreen && size == GetSize()) {
		return;
	}
	
	Win32Window::setWindowSize(size);

    changeDisplay(size, 0);
}

void D3D9Window::restoreObjects() {
	
    arx_assert(renderer != NULL);
    arx_assert(deviceInfo != NULL);
	
    // Reload all textures
    onRendererInit();
		
	// Set the multisampling render state
	renderer->SetAntialiasing(d3dpp.MultiSampleType != D3DMULTISAMPLE_NONE);

	// Finally, set the viewport for the newly created device
	renderer->SetViewport(Rect(m_Size.x, m_Size.y));
}

void D3D9Window::destroyObjects() {

    arx_assert(renderer != NULL);
	
	onRendererShutdown();
}
