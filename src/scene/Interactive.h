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

#ifndef ARX_SCENE_INTERACTIVE_H
#define ARX_SCENE_INTERACTIVE_H

#include <stddef.h>
#include <string>

#include "graphics/data/MeshManipulation.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "platform/Flags.h"

#include "Configure.h"

struct INTERACTIVE_OBJ;
struct EERIE_3DOBJ;

namespace res { class path; }

struct INTERACTIVE_OBJECTS {
	
	long init;
	long nbmax;
	
	INTERACTIVE_OBJ ** iobj;
	
	long getById(const std::string & name);
	INTERACTIVE_OBJ * getById(const std::string & name, INTERACTIVE_OBJ * self);
	
};

const size_t MAX_LINKS = 12;

struct ARX_NODE {
	short exist;
	short selected;
	std::string UName;
	char name[64];
	long link[MAX_LINKS];
	char lnames[MAX_LINKS][64];
	Vec3f pos;
	Vec2s bboxmin;
	Vec2s bboxmax;
};

struct ARX_NODES {
	long init;
	long nbmax;
	ARX_NODE * nodes;
};

enum TargetInfo {
	TARGET_PATH = -3,
	TARGET_NONE = -2,
	TARGET_PLAYER = 0, //-1
	TARGET_NODE = 50000
};

enum AddInteractiveFlag {
	NO_IDENT = 1,
	NO_MESH = 2,
	NO_ON_LOAD = 4,
	IO_IMMEDIATELOAD = 8
};
DECLARE_FLAGS(AddInteractiveFlag, AddInteractiveFlags)
DECLARE_FLAGS_OPERATORS(AddInteractiveFlags)

enum DeleteByIndexFlag {
	FLAG_NOCONFIRM = 1,
	FLAG_DONTKILLDIR = 2
};
DECLARE_FLAGS(DeleteByIndexFlag, DeleteByIndexFlags)
DECLARE_FLAGS_OPERATORS(DeleteByIndexFlags)


extern ARX_NODES nodes;
extern INTERACTIVE_OBJECTS inter;
extern INTERACTIVE_OBJ * CURRENTINTER;

#ifdef BUILD_EDITOR
extern long NbIOSelected;
#endif

void ARX_INTERACTIVE_UnfreezeAll();
void ARX_INTERACTIVE_TWEAK_Icon(INTERACTIVE_OBJ * io, const res::path & s1);
void ARX_INTERACTIVE_DestroyDynamicInfo(INTERACTIVE_OBJ * io);
void ARX_INTERACTIVE_HideGore(INTERACTIVE_OBJ * io, long flag = 0);
void ARX_INTERACTIVE_DeleteByIndex(long i, DeleteByIndexFlags flag = 0);
bool ARX_INTERACTIVE_Attach(long n_source, long n_target, const std::string & ap_source, const std::string & ap_target);
void ARX_INTERACTIVE_Detach(long n_source, long n_target);
void ARX_INTERACTIVE_Show_Hide_1st(INTERACTIVE_OBJ * io, long state);

void ARX_INTERACTIVE_RemoveGoreOnIO(INTERACTIVE_OBJ * io);
bool ARX_INTERACTIVE_ConvertToValidPosForIO(INTERACTIVE_OBJ * io, Vec3f * target);
void ARX_INTERACTIVE_TeleportBehindTarget(INTERACTIVE_OBJ * io);
bool ARX_INTERACTIVE_CheckCollision(EERIE_3DOBJ * obj, long kk, long source = -1);
void ARX_INTERACTIVE_DestroyIO(INTERACTIVE_OBJ * ioo);
void ARX_INTERACTIVE_MEMO_TWEAK(INTERACTIVE_OBJ * io, TweakType type, const res::path & param1, const res::path & param2);
void ARX_INTERACTIVE_APPLY_TWEAK_INFO(INTERACTIVE_OBJ * io);
bool ARX_INTERACTIVE_USEMESH(INTERACTIVE_OBJ * io, const res::path & temp);
void ARX_INTERACTIVE_Teleport(INTERACTIVE_OBJ * io, Vec3f * target, long flags = 0);

bool IsEquipedByPlayer(const INTERACTIVE_OBJ * io);
void CleanScriptLoadedIO();
void PrepareIOTreatZone(long flag = 0);

void LinkObjToMe(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * io2, const std::string & attach);

void MakeTemporaryIOIdent(INTERACTIVE_OBJ * io);
long ValidIONum(long num);
long ValidIOAddress(const INTERACTIVE_OBJ * io);
void RestoreInitialIOStatusOfIO(INTERACTIVE_OBJ * io);

void SetWeapon_Back(INTERACTIVE_OBJ * io);

void ReloadAllScripts();
bool ForceNPC_Above_Ground(INTERACTIVE_OBJ * io);

void InitNodes(long nb);
void ClearNode(long i, long spec);
void ClearNodes();
long GetFreeNode();
void UnselectAllNodes();
void SelectNode(long i);
void MakeNodeName(long i);
void TranslateSelectedNodes(Vec3f * trans);
void ClearSelectedNodes();
bool ExistNodeName(char * name);
void LinkNodeToNode(long i, long j);
void UnLinkNodeFromNode(long i, long j);
long CountNodes();
void RestoreNodeNumbers();
long GetNumNodeByName(char * name);
void ReleaseNode();
void RestoreInitialIOStatus();
long GetInterNum(const INTERACTIVE_OBJ * io);

#ifdef BUILD_EDITOR
void SelectIO(INTERACTIVE_OBJ * io);
void UnSelectIO(INTERACTIVE_OBJ * io);
void RotateSelectedIO(Anglef * op);
void TranslateSelectedIO(Vec3f * op);
void GroundSnapSelectedIO();
void DeleteSelectedIO();
void ResetSelectedIORot();
#endif

#ifdef BUILD_EDIT_LOADSAVE
void MakeIOIdent(INTERACTIVE_OBJ * io);
#endif

long GetNumberInterWithOutScriptLoadForLevel(long level);
void FreeAllInter();

void UnlinkAllLinkedObjects();
long IsCollidingAnyInter(float x, float y, float z, Vec3f * size);
INTERACTIVE_OBJ * GetFirstInterAtPos(Vec2s * pos, long flag = 0, Vec3f * _pRef = NULL, INTERACTIVE_OBJ ** _pTable = NULL, int * _pnNbInTable = NULL);

/*!
 * Adds an Interactive Object to the Scene
 * Calls appropriate func depending on object Type (ITEM, NPC or FIX)
 * Creates an IO Ident for added object if necessary
 * @param flags can be IO_IMMEDIATELOAD (1) to FORCE loading
 */
INTERACTIVE_OBJ * AddInteractive(const res::path & file, long id, AddInteractiveFlags flags = 0);
INTERACTIVE_OBJ * AddFix(const res::path & file, AddInteractiveFlags flags = 0);
INTERACTIVE_OBJ * AddNPC(const res::path & file, AddInteractiveFlags flags = 0);
INTERACTIVE_OBJ * AddItem(const res::path & file, AddInteractiveFlags flags = 0);

void InitInter(long nb);
INTERACTIVE_OBJ * CreateFreeInter(long num = -1);
void ReleaseInter(INTERACTIVE_OBJ * io);
void UpdateCameras();

INTERACTIVE_OBJ * InterClick(Vec2s * pos);
 
void RenderInter(float from, float to);
void SetWeapon_On(INTERACTIVE_OBJ * io);
 
void Prepare_SetWeapon(INTERACTIVE_OBJ * io, const res::path & temp);
void ComputeVVPos(INTERACTIVE_OBJ * io);
void SetYlsideDeath(INTERACTIVE_OBJ * io);
std::string GetMaterialString(const res::path & origin );
INTERACTIVE_OBJ * CloneIOItem(INTERACTIVE_OBJ * src);

float ARX_INTERACTIVE_GetArmorClass(INTERACTIVE_OBJ * io);
long  ARX_INTERACTIVE_GetPrice(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * shop);
void IO_UnlinkAllLinkedObjects(INTERACTIVE_OBJ * io);

struct TREATZONE_IO {
	long num;
	INTERACTIVE_OBJ * io;
	long ioflags;
	long show;
};

extern TREATZONE_IO * treatio;
extern long TREATZONE_CUR;

void TREATZONE_Clear();
void TREATZONE_Release();
void TREATZONE_AddIO(INTERACTIVE_OBJ * io, long num, long flag = 0);
void TREATZONE_RemoveIO(INTERACTIVE_OBJ * io);
bool IsSameObject(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo);
void ARX_INTERACTIVE_ClearAllDynData();
bool HaveCommonGroup(INTERACTIVE_OBJ * io, INTERACTIVE_OBJ * ioo);
void ShowIOPath(INTERACTIVE_OBJ * io);
void UpdateIOInvisibility(INTERACTIVE_OBJ * io);
void CheckSetAnimOutOfTreatZone(INTERACTIVE_OBJ * io, long num);
void RestoreAllIOInitPos();
void ARX_HALO_SetToNative(INTERACTIVE_OBJ * io);
void ARX_INTERACTIVE_ActivatePhysics(long t);
void ResetVVPos(INTERACTIVE_OBJ * io);

#endif // ARX_SCENE_INTERACTIVE_H
