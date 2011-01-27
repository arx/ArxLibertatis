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
#include "ARX_GlobalMods.h"
#include "EERIEMath.h"
#include "arx_menu2.h"
#include "arx_time.h"

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>


bool bUSE_D3DFOG_INTER;
float fZFogStartWorld;
float fZFogEndWorld;

GLOBAL_MODS current;
GLOBAL_MODS desired;
GLOBAL_MODS stacked;
extern long USE_D3DFOG;
extern long EDITMODE;

// changement du clipping Z valeur max & min
#define DEFAULT_ZCLIP		6400.f 
#define DEFAULT_MINZCLIP	1200.f 

extern CMenuConfig * pMenuConfig;
extern float fZFogEnd;
extern float fZFogStart;
extern bool bZBUFFER;
extern D3DMATRIX ProjectionMatrix;

extern unsigned long ulBKGColor = 0;

void ARX_GLOBALMODS_Reset()
{
	memset(&desired, 0, sizeof(GLOBAL_MODS));	
	memset(&current, 0, sizeof(GLOBAL_MODS));
	current.zclip = DEFAULT_ZCLIP;
	memset(&stacked, 0, sizeof(GLOBAL_MODS));
	stacked.zclip = DEFAULT_ZCLIP;

	desired.zclip = DEFAULT_ZCLIP;
	desired.depthcolor.r = 0.f;
	desired.depthcolor.g = 0.f;
	desired.depthcolor.b = 0.f;
}
float Approach(float current, float desired, float increment)
{
	if (desired > current)
	{
		current += increment;

		if (desired < current)
			current = desired;
	}
	else if (desired < current)
	{
		current -= increment;

		if (desired > current)
			current = desired;
	}

	return current;
}
void ARX_GLOBALMODS_Stack()
{
	memcpy(&stacked, &desired, sizeof(GLOBAL_MODS));

	desired.depthcolor.r = 0.f;
	desired.depthcolor.g = 0.f;
	desired.depthcolor.b = 0.f;
	desired.zclip = DEFAULT_ZCLIP;
}
void ARX_GLOBALMODS_UnStack()
{
	memcpy(&desired, &stacked, sizeof(GLOBAL_MODS));
}

void ARX_GLOBALMODS_Apply()
{
	if (EDITMODE) return;

	float baseinc = _framedelay;
	float incdiv1000 = _framedelay * DIV1000;

	if (desired.flags & GMOD_ZCLIP)
	{
		current.zclip = Approach(current.zclip, desired.zclip, baseinc * 2);
	}
	else // return to default...
	{
		desired.zclip = current.zclip = Approach(current.zclip, DEFAULT_ZCLIP, baseinc * 2);
	}

	// Now goes for RGB mods
	if (desired.flags & GMOD_DCOLOR)
	{
		current.depthcolor.r = Approach(current.depthcolor.r, desired.depthcolor.r, incdiv1000);
		current.depthcolor.g = Approach(current.depthcolor.g, desired.depthcolor.g, incdiv1000);
		current.depthcolor.b = Approach(current.depthcolor.b, desired.depthcolor.b, incdiv1000);
	}
	else
	{
		current.depthcolor.r = Approach(current.depthcolor.r, 0, incdiv1000);
		current.depthcolor.g = Approach(current.depthcolor.g, 0, incdiv1000);
		current.depthcolor.b = Approach(current.depthcolor.b, 0, incdiv1000);
	}

	ModeLight &= ~MODE_DEPTHCUEING;

	if (pMenuConfig)
	{
		float fZclipp = ((((float)pMenuConfig->iFogDistance) * 1.2f) * (DEFAULT_ZCLIP - DEFAULT_MINZCLIP) / 10.f) + DEFAULT_MINZCLIP;
		fZclipp += (ACTIVECAM->focal - 310.f) * 5.f;
		SetCameraDepth(__min(current.zclip, fZclipp));
	}
	else
	{
		SetCameraDepth(current.zclip);
	}

	if (USE_D3DFOG)
	{
		D3DDEVICEDESC7 d3dDeviceDesc;
		GDevice->GetCaps(&d3dDeviceDesc);
#define PERCENT_FOG (.6f)

		ulBKGColor = D3DRGB(current.depthcolor.r, current.depthcolor.g, current.depthcolor.b);
		
		//pour compatibilité ATI, etc...
		GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, ulBKGColor);
		float zval;

		zval = fZFogEnd;
		float zvalstart = fZFogStart;

		if ((!pMenuConfig->bATI) &&
		        (d3dDeviceDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE))
		{
			GDevice->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE,  D3DFOG_NONE);
			GDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR);
			bUSE_D3DFOG_INTER = true;

			//WORLD COORDINATE
			if (d3dDeviceDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_WFOG)
			{
				zval *= ACTIVECAM->cdepth;
				zvalstart *= ACTIVECAM->cdepth;
			}
		}
		else
		{
			zval *= ACTIVECAM->cdepth;
			zvalstart *= ACTIVECAM->cdepth;

			fZFogStartWorld = zvalstart;
			fZFogEndWorld = zval; 

			GDevice->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE,  D3DFOG_LINEAR);
			GDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);
			bUSE_D3DFOG_INTER = false;
		}

		GDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEEND,    *((LPDWORD)(&zval)));
		GDevice->SetRenderState(D3DRENDERSTATE_FOGTABLESTART,  *((LPDWORD)(&zvalstart)));
	}
	else
	{
		GDevice->SetRenderState(D3DRENDERSTATE_FOGCOLOR, D3DRGB(current.depthcolor.r, current.depthcolor.g, current.depthcolor.b));
		GDevice->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE,  D3DFOG_NONE);
		GDevice->SetRenderState(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);
	}
}
