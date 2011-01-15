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
///////////////////////////////////////////////////////////////////////////////
// EERIEEnum
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////

// Desc: Functions to enumerate DDraw/D3D drivers, devices, and modes.
#ifndef D3DENUM_H
#define D3DENUM_H
#include <d3d.h>
#include <ARX_Common.h>

//-----------------------------------------------------------------------------
// Flag and error definitions
//-----------------------------------------------------------------------------
#define D3DENUM_SOFTWAREONLY           0x00000001 // Software-devices only flag

#define D3DENUMERR_NODIRECTDRAW        0x81000001 // Could not create DDraw
#define D3DENUMERR_ENUMERATIONFAILED   0x81000002 // Enumeration failed
#define D3DENUMERR_SUGGESTREFRAST      0x81000003 // Suggest using the RefRast
#define D3DENUMERR_NOCOMPATIBLEDEVICES 0x81000004 // No devices were found that
// meet the app's desired
// capabilities

//-----------------------------------------------------------------------------
// Name: struct D3DEnum_DeviceInfo
// Desc: Structure to hold info about the enumerated Direct3D devices.
//-----------------------------------------------------------------------------
struct D3DEnum_DeviceInfo
{
	// D3D Device info
	TCHAR          strDesc[40];
	GUID     *     pDeviceGUID;
	D3DDEVICEDESC7 ddDeviceDesc;
	BOOL           bHardware;
	WORD			wNbBlendStage;
	WORD			wNbTextureSimultaneous;
	DWORD			dwTextureOpCaps;

	// DDraw Driver info
	GUID     *     pDriverGUID;
	DDCAPS         ddDriverCaps;
	DDCAPS         ddHELCaps;

	// DDraw Mode Info
	DDSURFACEDESC2 ddsdFullscreenMode;
	BOOL           bWindowed;
	BOOL           bStereo;

	// For internal use (apps should not need to use these)
	GUID            guidDevice;
	GUID            guidDriver;
	DDSURFACEDESC2 * pddsdModes;
	DWORD           dwNumModes;
	DWORD           dwCurrentMode;
	BOOL            bDesktopCompatible;
	BOOL            bStereoCompatible;
};

//-----------------------------------------------------------------------------
// Name: D3DEnum_EnumerateDevices()
// Desc: Enumerates all drivers, devices, and modes. The callback function is
//       called each device, to confirm that the device supports the feature
//       set required by the app.
//-----------------------------------------------------------------------------
HRESULT D3DEnum_EnumerateDevices(HRESULT(*fn)(DDCAPS *, D3DDEVICEDESC7 *));

//-----------------------------------------------------------------------------
// Name: D3DEnum_FreeResources()
// Desc: Cleans up any memory allocated during device enumeration
//-----------------------------------------------------------------------------
VOID D3DEnum_FreeResources();

//-----------------------------------------------------------------------------
// Name: D3DEnum_GetDevices()
// Desc: Returns a ptr to the array of enumerated D3DDEVICEINFO structures.
//-----------------------------------------------------------------------------
VOID D3DEnum_GetDevices(D3DEnum_DeviceInfo ** ppDevices, DWORD * pdwCount);

//-----------------------------------------------------------------------------
// Name: D3DEnum_SelectDefaultDevice()
// Desc: Picks a driver based on a set of passed in criteria. The
//       D3DENUM_SOFTWAREONLY flag can be used to pick a software device.
//-----------------------------------------------------------------------------
HRESULT D3DEnum_SelectDefaultDevice(D3DEnum_DeviceInfo ** pDevice,
                                    DWORD dwFlags = 0L);

//-----------------------------------------------------------------------------
// Name: D3DEnum_UserChangeDevice()
// Desc: Pops up a dialog which allows the user to select a new device.
//-----------------------------------------------------------------------------
HRESULT D3DEnum_UserChangeDevice(D3DEnum_DeviceInfo ** ppDevice);

#endif // D3DENUM_H



