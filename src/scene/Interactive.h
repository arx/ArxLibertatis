/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/Entity.h"
#include "game/EntityId.h"
#include "graphics/data/MeshManipulation.h"
#include "math/Vector.h"
#include "platform/Flags.h"

#include "Configure.h"

struct EERIE_3DOBJ;

namespace res { class path; }

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
	NO_MESH          = (1<<0),
	NO_ON_LOAD       = (1<<1),
	IO_IMMEDIATELOAD = (1<<2)
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

#ifdef BUILD_EDITOR
extern long NbIOSelected;
#endif

void ARX_INTERACTIVE_UnfreezeAll();
void ARX_INTERACTIVE_TWEAK_Icon(Entity * io, const res::path & s1);
void ARX_INTERACTIVE_DestroyDynamicInfo(Entity * io);
void ARX_INTERACTIVE_HideGore(Entity * io, long flag = 0);
void ARX_INTERACTIVE_DeleteByIndex(long i, DeleteByIndexFlags flag = 0);
bool ARX_INTERACTIVE_Attach(long n_source, long n_target, const std::string & ap_source, const std::string & ap_target);
void ARX_INTERACTIVE_Detach(long n_source, long n_target);
void ARX_INTERACTIVE_Show_Hide_1st(Entity * io, long state);

void ARX_INTERACTIVE_RemoveGoreOnIO(Entity * io);
bool ARX_INTERACTIVE_ConvertToValidPosForIO(Entity * io, Vec3f * target);
void ARX_INTERACTIVE_TeleportBehindTarget(Entity * io);
bool ARX_INTERACTIVE_CheckCollision(EERIE_3DOBJ * obj, long kk, long source = -1);
void ARX_INTERACTIVE_DestroyIO(Entity * ioo);
void ARX_INTERACTIVE_MEMO_TWEAK(Entity * io, TweakType type, const res::path & param1, const res::path & param2);
void ARX_INTERACTIVE_APPLY_TWEAK_INFO(Entity * io);
bool ARX_INTERACTIVE_USEMESH(Entity * io, const res::path & temp);
void ARX_INTERACTIVE_Teleport(Entity * io, Vec3f * target, bool flag = false);

bool IsEquipedByPlayer(const Entity * io);
void CleanScriptLoadedIO();
void PrepareIOTreatZone(long flag = 0);

void LinkObjToMe(Entity * io, Entity * io2, const std::string & attach);

void ARX_INTERACTIVE_DestroyIOdelayed(Entity * entity);
void ARX_INTERACTIVE_DestroyIOdelayedExecute();

/* TODO remove
 * ValidIONum and ValidIOAddress are fundamentally flawed and vulnerable to
 * index / address aliasing as both indices and memory addresses can be reused.
 *
 * We should instead use a proper weak pointer!
 */
bool ValidIONum(long num);
long ValidIOAddress(const Entity * io);

void RestoreInitialIOStatusOfIO(Entity * io);

void SetWeapon_Back(Entity * io);

void ReloadAllScripts();
bool ForceNPC_Above_Ground(Entity * io);

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

#ifdef BUILD_EDITOR
void SelectIO(Entity * io);
void UnSelectIO(Entity * io);
void RotateSelectedIO(Anglef * op);
void TranslateSelectedIO(Vec3f * op);
void GroundSnapSelectedIO();
void DeleteSelectedIO();
void ResetSelectedIORot();
#endif

#ifdef BUILD_EDIT_LOADSAVE
void MakeIOIdent(Entity * io);
#endif

long GetNumberInterWithOutScriptLoad();

void UnlinkAllLinkedObjects();
long IsCollidingAnyInter(float x, float y, float z, Vec3f * size);
Entity * GetFirstInterAtPos(Vec2s * pos, long flag = 0, Vec3f * _pRef = NULL, Entity ** _pTable = NULL, int * _pnNbInTable = NULL);

/*!
 * Adds an Interactive Object to the Scene
 * Calls appropriate func depending on object Type (ITEM, NPC or FIX)
 * Creates an IO Ident for added object if necessary
 * @param flags can be IO_IMMEDIATELOAD (1) to FORCE loading
 */
Entity * AddInteractive(const res::path & classPath, EntityInstance instance = -1, AddInteractiveFlags flags = 0);
Entity * AddItem(const res::path & classPath, EntityInstance instance = -1, AddInteractiveFlags flags = 0);
Entity * AddNPC(const res::path & classPath, EntityInstance instance = -1, AddInteractiveFlags flags = 0);
Entity * AddFix(const res::path & classPath, EntityInstance instance = -1, AddInteractiveFlags flags = 0);

void UpdateCameras();

Entity * InterClick(Vec2s * pos);
 
void UpdateInter();
void RenderInter();
void SetWeapon_On(Entity * io);
 
void Prepare_SetWeapon(Entity * io, const res::path & temp);
void ComputeVVPos(Entity * io);
void SetYlsideDeath(Entity * io);
std::string GetMaterialString(const res::path & origin );
Entity * CloneIOItem(Entity * src);

float ARX_INTERACTIVE_GetArmorClass(Entity * io);
long  ARX_INTERACTIVE_GetPrice(Entity * io, Entity * shop);
void IO_UnlinkAllLinkedObjects(Entity * io);

struct TREATZONE_IO {
	long num;
	Entity * io;
	EntityFlags ioflags;
	long show;
};

extern TREATZONE_IO * treatio;
extern long TREATZONE_CUR;

void TREATZONE_Clear();
void TREATZONE_Release();
void TREATZONE_AddIO(Entity * io, long flag = 0);
void TREATZONE_RemoveIO(Entity * io);
bool IsSameObject(Entity * io, Entity * ioo);
void ARX_INTERACTIVE_ClearAllDynData();
bool HaveCommonGroup(Entity * io, Entity * ioo);
void ShowIOPath(Entity * io);
void UpdateIOInvisibility(Entity * io);
void CheckSetAnimOutOfTreatZone(Entity * io, long num);
void RestoreAllIOInitPos();
void ARX_HALO_SetToNative(Entity * io);
void ARX_INTERACTIVE_ActivatePhysics(long t);
void ResetVVPos(Entity * io);

void UpdateGoldObject(Entity * io);

#endif // ARX_SCENE_INTERACTIVE_H
