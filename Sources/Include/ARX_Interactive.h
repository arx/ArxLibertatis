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
// ARX_Interactive
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Interactive Objects Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_INTERACTIVE_H
#define ARX_INTERACTIVE_H

//#include "Danae.h"
#include "EERIEpoly.h"
#include <tchar.h>

//-----------------------------------------------------------------------------
#define MAX_LINKS 12

typedef struct
{
	long			init;
	long			nbmax;
	INTERACTIVE_OBJ ** iobj;
	char		*	lock;
} INTERACTIVE_OBJECTS;

typedef struct
{
	INTERACTIVE_OBJ * io;
	long			show;
} INVENTORY_SLOT;

typedef struct
{
	INTERACTIVE_OBJ * io;
	long			sizex;
	long			sizey;
	INVENTORY_SLOT	slot[20][20];
} INVENTORY_DATA;

typedef struct
{
	short		exist;
	short		selected;
	_TCHAR		UName[64];
	char		name[64];
	long		link[MAX_LINKS];
	char		lnames[MAX_LINKS][64];
	EERIE_3D	pos;
	EERIE_S2D	bboxmin;
	EERIE_S2D	bboxmax;
} ARX_NODE;

typedef struct
{
	long		init;
	long		nbmax;
	ARX_NODE	* nodes;
} ARX_NODES;

//-----------------------------------------------------------------------------
#define INVENTORY_X			16
#define INVENTORY_Y			3
#define EQUIP_RIGHTHAND		1
#define EQUIP_LEFTHAND		2
#define EQUIP_SECONDARY		3
#define EQUIP_SHIELD		4
#define TARGET_PATH			-3
#define TARGET_NONE			-2
#define TARGET_PLAYER		0 //-1
#define TARGET_NODE			50000
#define NO_IDENT			1
#define NO_MESH				2
#define NO_ON_LOAD			4
#define IO_IMMEDIATELOAD	8
#define RENDER_INTER_FLAG_DUMMY_DRAW 1
#define FLAG_NOCONFIRM		1
#define FLAG_DONTKILLDIR	2

//-----------------------------------------------------------------------------
extern ARX_NODES nodes;
extern INVENTORY_DATA * SecondaryInventory;
extern INVENTORY_DATA * TSecondaryInventory;
extern INTERACTIVE_OBJECTS inter;
extern INTERACTIVE_OBJ * DRAGINTER;
extern INTERACTIVE_OBJ * CURRENTINTER;

extern INTERACTIVE_OBJ * ioSteal;
extern INVENTORY_SLOT inventory[3][INVENTORY_X][INVENTORY_Y];
extern long InventoryY;
extern long NbIOSelected;

//-----------------------------------------------------------------------------
void ARX_INTERACTIVE_UnfreezeAll();
void ARX_INTERACTIVE_TWEAK_Icon(INTERACTIVE_OBJ * io, char * s1);
void ARX_INTERACTIVE_DestroyDynamicInfo(INTERACTIVE_OBJ * io);
void ARX_INTERACTIVE_HideGore(INTERACTIVE_OBJ * io, long flag = 0);
void ARX_INTERACTIVE_DeleteByIndex(long i, long flag = 0);
BOOL ARX_INTERACTIVE_Attach(long n_source, long n_target, char * ap_source, char * ap_target);
void ARX_INTERACTIVE_Detach(long n_source, long n_target);
void ARX_INTERACTIVE_Show_Hide_1st(INTERACTIVE_OBJ * io, long state);
 
void ARX_INTERACTIVE_RemoveGoreOnIO(INTERACTIVE_OBJ * io);
bool ARX_INTERACTIVE_ConvertToValidPosForIO(INTERACTIVE_OBJ * io, EERIE_3D * target);
void ARX_INTERACTIVE_TeleportBehindTarget(INTERACTIVE_OBJ * io);
BOOL ARX_INTERACTIVE_CheckCollision(EERIE_3DOBJ * obj, long kk, long source = -1);
void ARX_INTERACTIVE_DestroyIO(INTERACTIVE_OBJ * ioo);
void ARX_INTERACTIVE_MEMO_TWEAK(INTERACTIVE_OBJ * io, long type, char * param1, char * param2);
void ARX_INTERACTIVE_MEMO_TWEAK_CLEAR(INTERACTIVE_OBJ * io);
void ARX_INTERACTIVE_APPLY_TWEAK_INFO(INTERACTIVE_OBJ * io);
void ARX_INTERACTIVE_USEMESH(INTERACTIVE_OBJ * io, char * temp);
void ARX_INTERACTIVE_Teleport(INTERACTIVE_OBJ * io, EERIE_3D * target, long flags = 0);

bool IsEquipedByPlayer(INTERACTIVE_OBJ * io);
void CleanScriptLoadedIO();
void PrepareIOTreatZone(long flag = 0);
 
void LinkObjToMe(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * io2, char * attach);
 
void PutInFrontOfPlayer(INTERACTIVE_OBJ * io, long flag = 0);
BOOL CanBePutInInventory(INTERACTIVE_OBJ * io);
void SetShield(char * temp);
void MakeTemporaryIOIdent(INTERACTIVE_OBJ * io);
INTERACTIVE_OBJ * GetInventoryObj();
long ValidIONum(long num);
long ValidIOAddress(INTERACTIVE_OBJ * io);
BOOL GetItemWorldPosition(INTERACTIVE_OBJ * io, EERIE_3D * pos);
BOOL GetItemWorldPositionSound(INTERACTIVE_OBJ * io, EERIE_3D * pos);
long GetTargetByNameTarget(char * name);
void RestoreInitialIOStatusOfIO(INTERACTIVE_OBJ * io);
 
void SetWeapon_Back(INTERACTIVE_OBJ * io);
 
void ReloadAllScripts();
BOOL ForceNPC_Above_Ground(INTERACTIVE_OBJ * io);

// BEGIN NODES DATA ****************************************************
void InitNodes(long nb);
void ClearNode(long i, long spec);
void ClearNodes();
long GetFreeNode();
void UnselectAllNodes();
void SelectNode(long i);
void MakeNodeName(long i);
void EditNodeName(long i);
void TranslateSelectedNodes(EERIE_3D * trans);
void ClearSelectedNodes();
BOOL ExistNodeName(char * name);
void LinkNodeToNode(long i, long j);
void UnLinkNodeFromNode(long i, long j);
long CountNodes();
void RestoreNodeNumbers();
long GetNumNodeByName(char * name);
void ReleaseNode();
// END NODES DATA ******************************************************
void RestoreInitialIOStatus();
long GetInterNum(INTERACTIVE_OBJ * io);
// io from 0 TO 10000
INTERACTIVE_OBJ * GetInventoryObj_INVENTORYUSE(EERIE_S2D * pos);
void CheckForInventoryReplaceMe(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * old);
 
void SelectIO(INTERACTIVE_OBJ * io);
void UnSelectIO(INTERACTIVE_OBJ * io);
void RotateSelectedIO(EERIE_3D * op);
void TranslateSelectedIO(EERIE_3D * op);
void GroundSnapSelectedIO();
void DeleteSelectedIO();
void ResetSelectedIORot();
 
long GetNumberInterWithOutScriptLoadForLevel(long level);
long IsCollidingAnyFIXInter(float x, float y, float z, EERIE_3D * size);
BOOL InSecondaryInventoryPos(EERIE_S2D * pos);
BOOL InPlayerInventoryPos(EERIE_S2D * pos);
BOOL CanBePutInSecondaryInventory(INVENTORY_DATA * id, INTERACTIVE_OBJ * io, long * xx, long * yy);
void FreeAllInter();
 
void SetRightHand(char * temp);
void SetLeftHand(char * temp);
void UnlinkAllLinkedObjects();
void LinkObjectToObject(EERIE_3DOBJ * inter, EERIE_3DOBJ * tolink, char * actiontext, char * actiontext2);
void UnLinkObjectFromObject(EERIE_3DOBJ * inter, EERIE_3DOBJ * tolink);
BOOL IsCollidingInter(INTERACTIVE_OBJ * io, EERIE_3D * pos);
long IsCollidingAnyInter(float x, float y, float z, EERIE_3D * size);
 
INTERACTIVE_OBJ * AddInteractive(LPDIRECT3DDEVICE7 pd3dDevice, char * file, long id, long flags = 0);
INTERACTIVE_OBJ * AddFix(LPDIRECT3DDEVICE7 pd3dDevice, char * file, long flags = 0);
INTERACTIVE_OBJ * AddNPC(LPDIRECT3DDEVICE7 pd3dDevice, char * file, long flags = 0);
INTERACTIVE_OBJ  * AddItem(LPDIRECT3DDEVICE7 pd3dDevice, char * file, long flags = 0);
INTERACTIVE_OBJ * AddCamera(LPDIRECT3DDEVICE7 pd3dDevice, char * file);
INTERACTIVE_OBJ * AddMarker(LPDIRECT3DDEVICE7 pd3dDevice, char * file);

void InitInter(long nb);
INTERACTIVE_OBJ * CreateFreeInter(long num = -1);
void ReleaseInter(INTERACTIVE_OBJ * io);
void ExecuteObjectAction(INTERACTIVE_OBJ * io);
void AddRandomObj(LPDIRECT3DDEVICE7 pd3dDevice);
void UpdateCameras();
 
void PlayObjectSound(INTERACTIVE_OBJ * io);
INTERACTIVE_OBJ * InterClick(EERIE_S2D * pos, long flag = 0);
 
void RenderInter(LPDIRECT3DDEVICE7 pd3dDevice, float from, float to, long flags = 0);
INTERACTIVE_OBJ * FlyingOverObject(EERIE_S2D * pos, long flag = 0);
void MakeIOIdent(INTERACTIVE_OBJ * io);
void SelectIO(INTERACTIVE_OBJ * io);
void ForcePlayerLookAtIO(INTERACTIVE_OBJ * io);
void SetWeapon_On(INTERACTIVE_OBJ * io);
 
void Prepare_SetWeapon(INTERACTIVE_OBJ * io, char * temp);
void ComputeVVPos(INTERACTIVE_OBJ * io);
void SetYlsideDeath(INTERACTIVE_OBJ * io);
void GetMaterialString(char * origin, char * dest);
INTERACTIVE_OBJ * CloneIOItem(INTERACTIVE_OBJ * src);

void CleanInventory();
void SendInventoryObjectCommand(char * _lpszText, long _lCommand);
BOOL PutInInventory();
char * GetInventoryName();
BOOL TakeFromInventory(EERIE_S2D * pos);
INTERACTIVE_OBJ * GetFromInventory(EERIE_S2D * pos);
BOOL IsFlyingOverInventory(EERIE_S2D * pos);
void ForcePlayerInventoryObjectLevel(long level);
BOOL IsInPlayerInventory(INTERACTIVE_OBJ * io);
BOOL IsInSecondaryInventory(INTERACTIVE_OBJ * io);
BOOL InInventoryPos(EERIE_S2D * pos);
void ReplaceInAllInventories(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo);
void RemoveFromAllInventories(INTERACTIVE_OBJ * io);
INTERACTIVE_OBJ * ARX_INVENTORY_GetTorchLowestDurability();
void ARX_INVENTORY_IdentifyAll();
void ARX_INVENTORY_OpenClose(INTERACTIVE_OBJ *);
void ARX_INVENTORY_TakeAllFromSecondaryInventory();

//***********************************************************************************************
// Retreives IO Number with its address
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/16)
//***********************************************************************************************
__inline long GetInterNum(INTERACTIVE_OBJ * io)
{
	if (io == NULL) return -1;

	for (long i = 0; i < inter.nbmax; i++)
		if (inter.iobj[i] == io) return i;

	return -1;
}
float ARX_INTERACTIVE_GetArmorClass(INTERACTIVE_OBJ * io);
float ARX_INTERACTIVE_fGetPrice(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * shop);
long  ARX_INTERACTIVE_GetPrice(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * shop);
void IO_UnlinkAllLinkedObjects(INTERACTIVE_OBJ * io);
void IO_Drop_Item(INTERACTIVE_OBJ * io_src, INTERACTIVE_OBJ * io);

typedef struct
{
	long				num;
	INTERACTIVE_OBJ *	io;
	long				ioflags;
	long				show;
} TREATZONE_IO;
extern TREATZONE_IO * treatio;
extern long TREATZONE_CUR;
extern long TREATZONE_MAX;
void TREATZONE_Clear();
void TREATZONE_Release();
void TREATZONE_AddIO(INTERACTIVE_OBJ * io, long num, long flag = 0);
void TREATZONE_RemoveIO(INTERACTIVE_OBJ * io);
BOOL IsSameObject(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo);
void ARX_INTERACTIVE_ClearAllDynData();
BOOL HaveCommonGroup(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo);
void ShowIOPath(INTERACTIVE_OBJ * io);
void UpdateIOInvisibility(INTERACTIVE_OBJ * io);
void CheckSetAnimOutOfTreatZone(INTERACTIVE_OBJ * io, long num);
void RestoreIOInitPos(INTERACTIVE_OBJ * io);
void RestoreAllIOInitPos();
void ARX_HALO_SetToNative(INTERACTIVE_OBJ * io);
void ARX_INTERACTIVE_ForceIOLeaveZone(INTERACTIVE_OBJ * io, long flags = 0);
void ARX_INTERACTIVE_ActivatePhysics(long t);
void ResetVVPos(INTERACTIVE_OBJ * io);
 
#endif
