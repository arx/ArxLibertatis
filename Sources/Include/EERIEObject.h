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
///////////////////////////////////////////////////////////////////////////////
// EERIEObject
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//	Provides funcs for 3D Object Handling
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
///////////////////////////////////////////////////////////////////////////////
#ifndef EERIEOBJECT_H
#define EERIEOBJECT_H

#include <d3d.h>
#include "EERIEApp.h"
#include "EERIELight.h"
#include "EERIETexture.h"

//-----------------------------------------------------------------------------
typedef struct
{
	EERIE_LIGHT	light;
	EERIE_3D	pos;
	long		dl;
	short		type;
	short		exist;
} ACTIONSTRUCT;

//-----------------------------------------------------------------------------
#define ACT_FIRE			1
#define ACT_FIREOFF			2
#define ACT_FIRE2			3
#define ACT_FIRE2OFF		4

#define MAX_ACTIONS			100

#define TEA_NO_SAMPLES		0
#define TEA_NPC_SAMPLES		1
#define TEA_FIX_SAMPLES		2
#define TEA_PLAYER_SAMPLES	3

#define TTE_NOPOPUP			1
#define TTE_SLOWLOAD		2
#define TTE_NO_RESTORE		4
#define TTE_NO_NDATA		8
#define TTE_NO_PDATA		16
#define TTE_NO_PHYSICS_BOX	32
#define TTE_NPC				64

//-----------------------------------------------------------------------------
extern long FASTLOADS;
extern D3DTLVERTEX	vert_list[4];
extern ACTIONSTRUCT actions[MAX_ACTIONS];

//-----------------------------------------------------------------------------
EERIE_MULTI3DSCENE	* MultiSceneToEerie(char * dir);
EERIE_MULTI3DSCENE	* PAK_MultiSceneToEerie(char * dir);

void ReleaseMultiScene(EERIE_MULTI3DSCENE * ms);
void ReleaseScene(EERIE_3DSCENE	*	scene);
void MakeUserFlag(TextureContainer	* tc);
long EERIE_OBJECT_GetGroup(EERIE_3DOBJ * obj, char * groupname);
long EERIE_OBJECT_GetSelection(EERIE_3DOBJ * obj, char * selname);
 
void GlobalInitLight();
void MoveAllLights(EERIE_3D * trans);
void ReCreateUVs(EERIE_3DOBJ * eerie, long flag = 0);
long GetGroupOriginByName(EERIE_3DOBJ * eobj, char * text);
long GetActionPointIdx(EERIE_3DOBJ * eobj, char * text);
long GetActionPointGroup(EERIE_3DOBJ * eobj, long idx);
void XRotatePoint(EERIE_3D * in, EERIE_3D * out, float c, float s);
void YRotatePoint(EERIE_3D * in, EERIE_3D * out, float c, float s);
void ZRotatePoint(EERIE_3D * in, EERIE_3D * out, float c, float s);

EERIE_3DOBJ * TheoToEerie(unsigned char * adr,long size,char * texpath,char * fic,long flag,LPDIRECT3DDEVICE7 pd3dDevice=NULL,long flag2=0);
void _THEObjLoad(EERIE_3DOBJ *eerie,unsigned char * adr,long * poss,long version,long flag=0,long flag2=0);
EERIE_3DOBJ * TheoToEerie_Fast(char * texpath,char * fic,long flag,LPDIRECT3DDEVICE7 pd3dDevice=NULL);
EERIE_ANIM * TheaToEerie(unsigned char * adr,long size,char * fic,long flags);
EERIE_ANIM * OldTheaToEerie(unsigned char * adr,long size,char * fic);

EERIE_3DSCENE * ScnToEerie(unsigned char * adr,  long		  size, char   *  fic,	long flags = 0);

void Clear3DObj(EERIE_3DOBJ	* eerie);
void Clear3DScene(EERIE_3DSCENE	* eerie);
void DrawEERIEObj(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3DOBJ * eobj, float Xrot, float Yrot, float Zrot, int Xoffs, int Yoffs, float Zoffs, IDirectDrawSurface7 * envir);
void DrawEERIEAnim(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3DOBJ * eobj, EERIE_ANIM * eanim, float Xrot, float Yrot, float Zrot, int Xoffs, int Yoffs, float Zoffs, long fr, float pour, long flag);
float DrawEERIEAnimT(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3DOBJ * eobj, EERIE_ANIM * eanim, float Xrot, float Yrot, float Zrot, int Xoffs, int Yoffs, float Zoffs, long tim, long flag);
float DrawEERIEAnimTQuat(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3DOBJ * eobj, EERIE_ANIM * eanim, float Xrot, float Yrot, float Zrot, int Xoffs, int Yoffs, float Zoffs, long tim, long flag);

void DrawEERIEAnimQuat(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3DOBJ * eobj, EERIE_ANIM * eanim, float Xrot, float Yrot, float Zrot, int Xoffs, int Yoffs, float Zoffs, long fr, float pour, long flag);
void SceneAddObj(EERIE_3DOBJ * eobj);
void SceneAddScn(EERIE_3DSCENE * escn);
void ReleaseEERIE3DObj(EERIE_3DOBJ * eerie);
void ReleaseEERIE3DObjFromScene(EERIE_3DOBJ * eerie);
void ReleaseAnim(EERIE_ANIM * ea);

EERIE_3DOBJ * Eerie_Copy(EERIE_3DOBJ * obj);
void EERIE_Object_Precompute_Fast_Access(EERIE_3DOBJ * obj);
void EERIE_3DOBJ_RestoreTextures(EERIE_3DOBJ * eobj);
void EERIE_OBJECT_CenterObjectCoordinates(EERIE_3DOBJ * ret);
void EERIE_CreateCedricData(EERIE_3DOBJ * eobj);
void EERIEOBJECT_CreatePFaces(EERIE_3DOBJ * eobj);
void RemoveAllBackgroundActions();

#endif
