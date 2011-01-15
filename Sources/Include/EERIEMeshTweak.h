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

#ifndef EERIE_MESH_TWEAK_H
#define EERIE_MESH_TWEAK_H

#include "EERIETypes.h"
#include "EERIEPoly.h"

#define	TWEAK_ERROR	0
#define TWEAK_HEAD	2
#define TWEAK_TORSO 4
#define	TWEAK_LEGS	8
#define	TWEAK_ALL	TWEAK_HEAD | TWEAK_TORSO | TWEAK_LEGS
#define	TWEAK_UPPER	TWEAK_HEAD | TWEAK_TORSO
#define TWEAK_LOWER TWEAK_TORSO | TWEAK_LEGS
#define	TWEAK_UP_LO	TWEAK_HEAD | TWEAK_LEGS
#define TWEAK_REMOVE 1
#define	TWEAK_TYPE_SKIN	16
#define TWEAK_TYPE_ICON	32
#define TWEAK_TYPE_MESH	64

void EERIE_MESH_TWEAK_Do(INTERACTIVE_OBJ * io, long tw, char * path);
void TweakMesh(INTERACTIVE_OBJ * io, long tw, char * temp);
long IsInSelection(EERIE_3DOBJ * obj, long vert, long tw);
void AddVertexIdxToGroup(EERIE_3DOBJ * obj, long group, long val);
void EERIE_MESH_TWEAK_Skin(EERIE_3DOBJ * obj, char * skintochange, char * skinname);
void MESH_CACHE_Init();
long ObjectAddMap(EERIE_3DOBJ * obj, TextureContainer * tc);
long GetEquivalentVertex(EERIE_3DOBJ * obj, EERIE_VERTEX * vert);
void EERIE_MESH_ReleaseTransPolys(EERIE_3DOBJ * obj);

#endif
