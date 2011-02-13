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
#ifndef ARX_CARTE_H
#define ARX_CARTE_H

#include "EERIEPoly.h"
#include "EERIEApp.h"


class C_ARX_Carte
{
	private:
		LPDIRECT3DDEVICE7	device;
		EERIE_BACKGROUND	* background;
		int					width;
		int					height;
		int					nbpixels;
		int					widthrender;
		int					heightrender;
		float				posx;
		float				posz;
		float				minx, minz;
		float				maxx, maxz;
		float				ecx, ecz;

		LPDIRECTDRAWSURFACE7	surfacetemp;

	public:
		C_ARX_Carte(LPDIRECT3DDEVICE7 d = NULL, EERIE_BACKGROUND * bkg = NULL, int nbpixels = 10, int wrender = 640, int hrender = 480);
		~C_ARX_Carte() {};
		BOOL Render(void);
		void MoveMap(float newposx, float newposy);
		void IncMoveMap(float incx, float incz);
 
		BOOL CreateSurfaceTemp(CD3DFramework7 * framework);
		BOOL BltOnSurfTemp(CD3DFramework7 * framework, int x, int y, int dw, int dh, int largs, int largh);
		BOOL BuildMap(CD3DFramework7 * framework, char * name);
 
};

//-----------------------------------------------------------------------------
extern C_ARX_Carte * ARXCarte;
extern int iCreateMap;

//-----------------------------------------------------------------------------
BOOL NeedMapCreation();
void DANAE_Manage_CreateMap();

#endif ARX_CARTE_H
