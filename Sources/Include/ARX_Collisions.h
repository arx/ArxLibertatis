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
// ARX_Collisions
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Collisions Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_COLLISIONS_H
#define ARX_COLLISIONS_H

#include <windows.h>
#include "EERIEPoly.h"

//-----------------------------------------------------------------------------
#define MAX_IN_SPHERE 20

//-----------------------------------------------------------------------------
extern long MAX_IN_SPHERE_Pos;
extern short EVERYTHING_IN_SPHERE[MAX_IN_SPHERE+1];
extern long EXCEPTIONS_LIST_Pos;
extern short EXCEPTIONS_LIST[MAX_IN_SPHERE+1];
extern BOOL DIRECT_PATH;

//-----------------------------------------------------------------------------
BOOL ARX_COLLISION_Move_Cylinder(IO_PHYSICS * ip,INTERACTIVE_OBJ * io,float MOVE_CYLINDER_STEP,long flags=0);
float CheckAnythingInCylinder(EERIE_CYLINDER * cyl,INTERACTIVE_OBJ * ioo,long flags=0);
// CheckIOInSphere Flags START -------------------------------------------
#define CAS_NO_NPC_COL			1
#define CAS_NO_SAME_GROUP		2
#define CAS_NO_BACKGROUND_COL	4
#define CAS_NO_ITEM_COL			8
#define CAS_NO_FIX_COL			16
#define CAS_NO_DEAD_COL			32
// CheckIOInSphere Flags END ---------------------------------------------
BOOL CheckAnythingInSphere(EERIE_SPHERE * sphere,long source,long flags=0,long *num=NULL); //except source...
BOOL CheckEverythingInSphere(EERIE_SPHERE * sphere,long source,long targ=-1); //except source...
EERIEPOLY * CheckBackgroundInSphere(EERIE_SPHERE * sphere); //except source...
 
BOOL IsCollidingIO(INTERACTIVE_OBJ * io,INTERACTIVE_OBJ * ioo);
// CheckIOInSphere Flags START -------------------------------------------
#define IIS_NO_NOCOL	1
// CheckIOInSphere Flags END ---------------------------------------------
BOOL CheckIOInSphere(EERIE_SPHERE * sphere,long target,long flags=0);
BOOL AttemptValidCylinderPos(EERIE_CYLINDER * cyl,INTERACTIVE_OBJ * io,long flags);
BOOL IO_Visible(EERIE_3D * orgn, EERIE_3D * dest,EERIEPOLY * epp,EERIE_3D * hit);

void ANCHOR_BLOCK_By_IO(INTERACTIVE_OBJ * io,long status);
void ANCHOR_BLOCK_Clear();
float CylinderPlatformCollide(EERIE_CYLINDER * cyl,INTERACTIVE_OBJ * io);
bool IsAnyNPCInPlatform(INTERACTIVE_OBJ * pfrm);
void PushIO_ON_Top(INTERACTIVE_OBJ * ioo,float ydec);

#endif
