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
// EERIEEnum
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Functions to enumerate DDraw/D3D drivers, devices, and modes.
//
// Updates: (date) (person) (update)
//
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "graphics/GraphicsEnum.h"

#include <cstdio>

#include <windowsx.h>

#include "io/Logger.h"

#define IDD_CHANGEDEVICE       144 // "Change Device" dialog box

#define IDC_DEVICE_COMBO      1000 // Device combobox for "Change Device" dlg
#define IDC_MODE_COMBO        1001 // Mode combobox for "Change Device" dlg
#define IDC_WINDOWED_CHECKBOX 1012 // Checkbox for windowed-mode
#define IDC_STEREO_CHECKBOX   1013 // Checkbox for stereo modes
#define IDC_FULLSCREEN_TEXT   1014 // Group box text label

//************************************************************************************
// Global data
//************************************************************************************

static D3DEnum_DeviceInfo g_pDeviceList[20];
static DWORD g_dwNumDevicesEnumerated = 0L;
static DWORD g_dwNumDevices           = 0L;

//************************************************************************************
// SortModesCallback()
// Callback function for sorting display modes.
//************************************************************************************
int SortModesCallback(const VOID * arg1, const VOID * arg2)
{
	DDSURFACEDESC2 * p1 = (DDSURFACEDESC2 *)arg1;
	DDSURFACEDESC2 * p2 = (DDSURFACEDESC2 *)arg2;

	if (p1->dwWidth < p2->dwWidth)        return -1;

	if (p1->dwWidth > p2->dwWidth)        return +1;

	if (p1->dwHeight < p2->dwHeight)        return -1;

	if (p1->dwHeight > p2->dwHeight)        return +1;

	if (p1->ddpfPixelFormat.dwRGBBitCount < p2->ddpfPixelFormat.dwRGBBitCount)
		return -1;

	if (p1->ddpfPixelFormat.dwRGBBitCount > p2->ddpfPixelFormat.dwRGBBitCount)
		return +1;

	return 0;
}


//************************************************************************************
// ModeEnumCallback()
// Callback function for enumerating display modes.
//************************************************************************************
static HRESULT WINAPI ModeEnumCallback(DDSURFACEDESC2 * pddsd,
                                       VOID * pParentInfo)
{
	D3DEnum_DeviceInfo * pDevice = (D3DEnum_DeviceInfo *)pParentInfo;

	// Reallocate storage for the modes
	DDSURFACEDESC2 * pddsdNewModes = new DDSURFACEDESC2[pDevice->dwNumModes+1];
	memcpy(pddsdNewModes, pDevice->pddsdModes,
	       pDevice->dwNumModes * sizeof(DDSURFACEDESC2));
	delete[] pDevice->pddsdModes;
	pDevice->pddsdModes = pddsdNewModes;

	// Add the new mode
	pDevice->pddsdModes[pDevice->dwNumModes++] = (*pddsd);

	return DDENUMRET_OK;
}




//************************************************************************************
// DeviceEnumCallback()
// Callback function for enumerating devices
//************************************************************************************
static HRESULT WINAPI DeviceEnumCallback( char*	strDesc,
                                          char* strName,
                                          D3DDEVICEDESC7* pDesc,
                                          VOID* pParentInfo)
{
	
	(void)strDesc;
	
	// Keep track of # of devices that were enumerated
	g_dwNumDevicesEnumerated++;

	D3DEnum_DeviceInfo * pDriverInfo = (D3DEnum_DeviceInfo *)pParentInfo;
	D3DEnum_DeviceInfo * pDeviceInfo = &g_pDeviceList[g_dwNumDevices];
	memset(pDeviceInfo, 0, sizeof(D3DEnum_DeviceInfo));

	//info sur le devices
	pDeviceInfo->wNbBlendStage			= pDesc->wMaxTextureBlendStages;
	pDeviceInfo->wNbTextureSimultaneous	= pDesc->wMaxSimultaneousTextures;
	pDeviceInfo->dwTextureOpCaps		= pDesc->dwTextureOpCaps;

	// Select either the HAL or HEL device desc:
	pDeviceInfo->bHardware = (pDesc->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) == D3DDEVCAPS_HWRASTERIZATION;
	memcpy(&pDeviceInfo->ddDeviceDesc, pDesc, sizeof(D3DDEVICEDESC7));

	// Set up device info for this device
	pDeviceInfo->bDesktopCompatible = pDriverInfo->bDesktopCompatible;
	pDeviceInfo->ddDriverCaps       = pDriverInfo->ddDriverCaps;
	pDeviceInfo->ddHELCaps          = pDriverInfo->ddHELCaps;
	pDeviceInfo->guidDevice         = pDesc->deviceGUID;
	pDeviceInfo->pDeviceGUID        = &pDeviceInfo->guidDevice;
	pDeviceInfo->pddsdModes         = new DDSURFACEDESC2[pDriverInfo->dwNumModes];

	// Copy the driver GUID and description for the device
	if (pDriverInfo->pDriverGUID)
	{
		pDeviceInfo->guidDriver  = pDriverInfo->guidDriver;
		pDeviceInfo->pDriverGUID = &pDeviceInfo->guidDriver;
		lstrcpyn(pDeviceInfo->strDesc, pDriverInfo->strDesc, 39);
	}
	else
	{
		pDeviceInfo->pDriverGUID = NULL;
		lstrcpyn(pDeviceInfo->strDesc, strName, 39);
	}

	// Avoid duplicates: only enum HW devices for secondary DDraw drivers.
	if (NULL != pDeviceInfo->pDriverGUID && false == pDeviceInfo->bHardware)
	{
		return D3DENUMRET_OK;
	}

	// Build list of supported modes for the device
	for (DWORD i = 0 ; i < pDriverInfo->dwNumModes ; i++)
	{
		DDSURFACEDESC2 ddsdMode = pDriverInfo->pddsdModes[i];
		DWORD dwRenderDepths    = pDeviceInfo->ddDeviceDesc.dwDeviceRenderBitDepth;
		DWORD dwDepth           = ddsdMode.ddpfPixelFormat.dwRGBBitCount;

		// Accept modes that are compatable with the device
		if (((dwDepth == 32) && (dwRenderDepths & DDBD_32)) ||
		        ((dwDepth == 24) && (dwRenderDepths & DDBD_24)) ||
		        ((dwDepth == 16) && (dwRenderDepths & DDBD_16)))
		{
			// Copy compatible modes to the list of device-supported modes
			pDeviceInfo->pddsdModes[pDeviceInfo->dwNumModes++] = ddsdMode;

			// Record whether the device has any stereo modes
			if (ddsdMode.ddsCaps.dwCaps2 & DDSCAPS2_STEREOSURFACELEFT)
				pDeviceInfo->bStereoCompatible = true;
		}
	}

	// Bail if the device has no supported modes
	if (0 == pDeviceInfo->dwNumModes)
	{
		return D3DENUMRET_OK;
	}

	// Find a 640x480x16 mode for the default fullscreen mode
	for (UINT i = 0 ; i < pDeviceInfo->dwNumModes ; i++)
	{
		if ((pDeviceInfo->pddsdModes[i].dwWidth  == 640) &&
		        (pDeviceInfo->pddsdModes[i].dwHeight == 480) &&
		        (pDeviceInfo->pddsdModes[i].ddpfPixelFormat.dwRGBBitCount == 16))
		{
			pDeviceInfo->ddsdFullscreenMode = pDeviceInfo->pddsdModes[i];
			pDeviceInfo->dwCurrentMode      = i;
		}
	}

	// Select whether the device is initially windowed
	pDeviceInfo->bWindowed = pDeviceInfo->bDesktopCompatible;

	// Accept the device and return
	g_dwNumDevices++;

	return D3DENUMRET_OK;
}


//************************************************************************************
// DriverEnumCallback()
// Callback function for enumerating drivers.
//************************************************************************************
static BOOL WINAPI DriverEnumCallback(GUID * pGUID, char* strDesc,
                                      char* strName, VOID *, HMONITOR)
{
	
	(void)strName;
	
	D3DEnum_DeviceInfo d3dDeviceInfo;
	LPDIRECTDRAW7      pDD;
	LPDIRECT3D7        pD3D;
	HRESULT            hr;

	// Use the GUID to create the DirectDraw object
	hr = DirectDrawCreateEx(pGUID, (VOID **)&pDD, IID_IDirectDraw7, NULL);

	if (FAILED(hr))
	{
		LogError << "Can't create DDraw during enumeration!";
		return D3DENUMRET_OK;
	}

	// Create a D3D object, to enumerate the d3d devices
	hr = pDD->QueryInterface(IID_IDirect3D7, (VOID **)&pD3D);

	if (FAILED(hr))
	{
		pDD->Release();
		LogError << "Can't query IDirect3D7 during enumeration!";
		return D3DENUMRET_OK;
	}

	// Copy data to a device info structure
	memset(&d3dDeviceInfo, 0, sizeof(d3dDeviceInfo));
	lstrcpyn(d3dDeviceInfo.strDesc, strDesc, 39);
	d3dDeviceInfo.ddDriverCaps.dwSize = sizeof(DDCAPS);
	d3dDeviceInfo.ddHELCaps.dwSize    = sizeof(DDCAPS);
	pDD->GetCaps(&d3dDeviceInfo.ddDriverCaps, &d3dDeviceInfo.ddHELCaps);

	if (pGUID)
	{
		d3dDeviceInfo.guidDriver = (*pGUID);
		d3dDeviceInfo.pDriverGUID = &d3dDeviceInfo.guidDriver;
	}

	// Record whether the device can render into a desktop window
	if (d3dDeviceInfo.ddDriverCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
		if (NULL == d3dDeviceInfo.pDriverGUID)
			d3dDeviceInfo.bDesktopCompatible = true;

	// Enumerate the fullscreen display modes.
	pDD->EnumDisplayModes(0, NULL, &d3dDeviceInfo, ModeEnumCallback);

	// Sort list of display modes
	qsort(d3dDeviceInfo.pddsdModes, d3dDeviceInfo.dwNumModes,
	      sizeof(DDSURFACEDESC2), SortModesCallback);

	// Now, enumerate all the 3D devices
	pD3D->EnumDevices(DeviceEnumCallback, &d3dDeviceInfo);

	// Clean up and return
	if(d3dDeviceInfo.pddsdModes) {
		delete[] d3dDeviceInfo.pddsdModes;
		d3dDeviceInfo.pddsdModes = NULL;
	}
	pD3D->Release();
	pDD->Release();

	return DDENUMRET_OK;
}

//************************************************************************************
// D3DEnum_EnumerateDevices()
// Enumerates all drivers, devices, and modes. The callback function is
// called each device, to confirm that the device supports the feature
// set required by the app.
//************************************************************************************
HRESULT D3DEnum_EnumerateDevices()
{

	// Enumerate drivers, devices, and modes
	DirectDrawEnumerateEx(DriverEnumCallback, NULL,
	                      DDENUM_ATTACHEDSECONDARYDEVICES |
	                      DDENUM_DETACHEDSECONDARYDEVICES |
	                      DDENUM_NONDISPLAYDEVICES);

	// Make sure devices were actually enumerated
	if (0 == g_dwNumDevicesEnumerated)
	{
		LogError << "No devices and/or modes were enumerated!";
		return D3DENUMERR_ENUMERATIONFAILED;
	}

	if (0 == g_dwNumDevices)
	{
		LogError << "No enumerated devices were accepted!";
		LogError << "Try enabling the D3D Reference Rasterizer.";
		return D3DENUMERR_SUGGESTREFRAST;
	}

	return S_OK;
}

//************************************************************************************
// D3DEnum_FreeResources()
// Cleans up any memory allocated during device enumeration
//************************************************************************************
void D3DEnum_FreeResources() {
	for(DWORD i = 0; i < g_dwNumDevices; i++) {
		delete[] g_pDeviceList[i].pddsdModes, g_pDeviceList[i].pddsdModes = NULL;
	}
}

//************************************************************************************
// D3DEnum_GetDevices()
// Returns a ptr to the array of D3DEnum_DeviceInfo structures.
//************************************************************************************
static void D3DEnum_GetDevices(D3DEnum_DeviceInfo ** ppDevices, DWORD * pdwCount)
{
	if (ppDevices)
		(*ppDevices) = g_pDeviceList;

	if (pdwCount)
		(*pdwCount)  = g_dwNumDevices;
}

//************************************************************************************
// D3DEnum_SelectDefaultDevice()
// Pick a default device, preferably hardware and desktop compatible.
//************************************************************************************
HRESULT D3DEnum_SelectDefaultDevice(D3DEnum_DeviceInfo ** ppDevice,
                                    DWORD dwFlags)
{
	// Check arguments
	if (NULL == ppDevice)
		return E_INVALIDARG;

	// Get access to the enumerated device list
	D3DEnum_DeviceInfo * pDeviceList;
	DWORD               dwNumDevices;
	D3DEnum_GetDevices(&pDeviceList, &dwNumDevices);

	// Look for windowable software, hardware, and hardware TnL devices
	D3DEnum_DeviceInfo * pRefRastDevice     = NULL;
	D3DEnum_DeviceInfo * pSoftwareDevice    = NULL;
	D3DEnum_DeviceInfo * pHardwareDevice    = NULL;
	D3DEnum_DeviceInfo * pHardwareTnLDevice = NULL;

	for (DWORD i = 0; i < dwNumDevices; i++)
	{
		if (pDeviceList[i].bDesktopCompatible)
		{
			if (pDeviceList[i].bHardware)
			{
#ifdef START_NO_TNL

				if ((*pDeviceList[i].pDeviceGUID) == IID_IDirect3DHALDevice)
					pHardwareDevice = &pDeviceList[i];
				else
#endif
					if ((*pDeviceList[i].pDeviceGUID) == IID_IDirect3DTnLHalDevice)
					{
						pHardwareTnLDevice = &pDeviceList[i];
						break;
					}
					else
						pHardwareDevice = &pDeviceList[i];
			}
			else
			{
				if ((*pDeviceList[i].pDeviceGUID) == IID_IDirect3DRefDevice)
					pRefRastDevice = &pDeviceList[i];
				else
					pSoftwareDevice = &pDeviceList[i];
			}
		}
	}

#ifdef START_NO_TNL
	// Prefer a hardware NON-TnL device first, then a non-TnL hardware device, and
	// finally, a software device.

	if (0 == (dwFlags & D3DENUM_SOFTWAREONLY) && pHardwareDevice)
		(*ppDevice) = pHardwareDevice;
	else if (0 == (dwFlags & D3DENUM_SOFTWAREONLY) && pHardwareTnLDevice)
		(*ppDevice) = pHardwareTnLDevice;
	else if (pSoftwareDevice)
		(*ppDevice) = pSoftwareDevice;
	else if (pRefRastDevice)
		(*ppDevice) = pRefRastDevice;
	else
		return D3DENUMERR_NOCOMPATIBLEDEVICES;

#else
	// Prefer a hardware TnL device first, then a non-TnL hardware device, and
	// finally, a software device.

	if (0 == (dwFlags & D3DENUM_SOFTWAREONLY) && pHardwareTnLDevice)
		(*ppDevice) = pHardwareTnLDevice;
	else if (0 == (dwFlags & D3DENUM_SOFTWAREONLY) && pHardwareDevice)
		(*ppDevice) = pHardwareDevice;
	else if (pSoftwareDevice)
		(*ppDevice) = pSoftwareDevice;
	else if (pRefRastDevice)
		(*ppDevice) = pRefRastDevice;
	else
		return D3DENUMERR_NOCOMPATIBLEDEVICES;

#endif

	// Enforce support of at least 3 simultaneous textures (only cards released in the previous millenium supported less than that!)
	if((*ppDevice)->wNbTextureSimultaneous < 3)
	{
		(*ppDevice) = NULL;
		return D3DENUMERR_NOCOMPATIBLEDEVICES;
	}

	// Enforce support of the ADDSIGNED texture op...
	if(!((*ppDevice)->dwTextureOpCaps & D3DTEXOPCAPS_ADDSIGNED))
	{
		(*ppDevice) = NULL;
		return D3DENUMERR_NOCOMPATIBLEDEVICES;
	}

	// Set the windowed state of the newly selected device
	(*ppDevice)->bWindowed = true;

	return S_OK;
}
