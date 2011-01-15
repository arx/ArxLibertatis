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
// EERIEAnchors
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Anchors funcs
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////
#ifndef  EERIEANCHORS_H
#define  EERIEANCHORS_H
#include <EERIETypes.h>
#include "EERIEPoly.h"
#include "EERIEObject.h"

//-----------------------------------------------------------------------------
void AnchorData_ClearAll(EERIE_BACKGROUND * eb);
bool CylinderAboveInvalidZone(EERIE_CYLINDER * cyl);

#define MUST_BE_BIG 1

bool DirectAddAnchor(EERIE_BACKGROUND * eb, EERIE_BKG_INFO * eg, EERIE_3D * pos, long flags);
bool AddAnchor(EERIE_BACKGROUND * eb, EERIE_BKG_INFO * eg, EERIE_3D * pos, long flags);
void AddAnchorLink(EERIE_BACKGROUND * eb, long anchor, long linked);
void AnchorData_Create_Links(EERIE_BACKGROUND * eb);
void TOKEEP_AnchorData_Create_Phase_II(EERIE_BACKGROUND * eb);
void AnchorData_Create_Phase_II(EERIE_BACKGROUND * eb);
void AnchorData_Create(EERIE_BACKGROUND * eb);

long AnchorData_GetNearest(EERIE_3D * pos, EERIE_CYLINDER * cyl);
long AnchorData_GetNearest_2(float beta, EERIE_3D * pos, EERIE_CYLINDER * cyl);
long AnchorData_GetNearest_Except(EERIE_3D * pos, EERIE_CYLINDER * cyl, long except);
 
#endif
