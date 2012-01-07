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

using std::string;
using std::vector;

LPDIRECT3DDEVICE9 GD3D9Device = NULL;

D3D9Window::D3D9Window() : deviceInfo(NULL), d3d(NULL) { }

D3D9Window::~D3D9Window() {
	destroyObjects();

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
	
	if(!Win32Window::init(title, size, fullscreen, depth)) {
		return false;
	}
	
	bool succeeded = false;
	if(initialize(DisplayMode(size, depth))) {
		LogInfo << "Using Win32 windowing";
		succeeded = true;
	} else {
		destroyObjects();
	}
	
	if(!succeeded) {
		deviceInfo = NULL;
		LogError << "Could not initialize any D3D device!";
	}

	return succeeded;
}

bool D3D9Window::showFrame() {
	GD3D9Device->Present(NULL, NULL, NULL, NULL);
	return true;
}

void D3D9Window::restoreSurfaces() {
}

void D3D9Window::destroyObjects() {
	
	if(renderer) {
		onRendererShutdown();
		delete renderer, renderer = NULL;
	}
	
	// Do a safe check for releasing the D3DDEVICE. RefCount must be zero.
	if(GD3D9Device) {

		// Restore initial gamma ramp - Needed for wine
		GD3D9Device->SetGammaRamp(0, D3DSGR_NO_CALIBRATION, &initialGammaRamp);

		if(0 < GD3D9Device->Release()) {
			LogWarning << "D3DDevice object is still referenced";
		}
		GD3D9Device = NULL;
	}
}

// Internal functions for the framework class
bool D3D9Window::initialize(DisplayMode mode) {
	
	arx_assert(GetHandle() != NULL);
	arx_assert(renderer == NULL);
	
	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	
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

	renderer = new D3D9Renderer(this);
	renderer->Initialize();

	// Multisampling - Part 2
	// Set the multisampling render state
	renderer->SetAntialiasing(d3dpp.MultiSampleType != D3DMULTISAMPLE_NONE);
	
	// Finally, set the viewport for the newly created device
	renderer->SetViewport(Rect(GetSize().x, GetSize().y));

	// Store initial gamma settings to restore them later - Needed for wine
	GD3D9Device->GetGammaRamp(0, &initialGammaRamp);
		
	onRendererInit();
	
	return true;
}

void D3D9Window::setFullscreenMode(Vec2i resolution, unsigned _depth) {
	
	if(m_IsFullscreen && m_Size == resolution && depth == _depth) {
		return;
	}
	
	destroyObjects();
	
	Win32Window::setFullscreenMode(resolution, _depth);
	
	restoreObjects();
}

void D3D9Window::setWindowSize(Vec2i size) {
	
	if(!m_IsFullscreen && size == GetSize()) {
		return;
	}
	
	destroyObjects();

	Win32Window::setWindowSize(size);

	restoreObjects();
}

void D3D9Window::restoreObjects() {
	
	if(!deviceInfo) {
		// not initialized yet!
		return;
	}
	
	if(!renderer) {
		
		if(m_IsFullscreen) {
			initialize(DisplayMode(m_Size, depth));
		} else {
			initialize(DisplayMode(m_Size, 0));
		}
		
	}
	
	// Finally, set the viewport for the newly created device
	renderer->SetViewport(Rect(m_Size.x, m_Size.y));
}

void D3D9Window::setGammaRamp(const u16 * red, const u16 * green, const u16 * blue) {

	D3DGAMMARAMP ramp;
	
	if(!red || !green || !blue) {
		GD3D9Device->GetGammaRamp(0, &ramp);
	}
	
	if(red) {
		std::copy(red, red + 256, ramp.red);
	}
	
	if(green) {
		std::copy(green, green + 256, ramp.green);
	}
	
	if(blue) {
		std::copy(blue, blue + 256, ramp.blue);
	}

	GD3D9Device->SetGammaRamp(0, D3DSGR_NO_CALIBRATION, &ramp);
}

void D3D9Window::evictManagedTextures() {
	// Nothing to do here
}
