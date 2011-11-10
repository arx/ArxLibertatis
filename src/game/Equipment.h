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
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#ifndef ARX_GAME_EQUIPMENT_H
#define ARX_GAME_EQUIPMENT_H

#include <stddef.h>
#include <string>

#include "math/MathFwd.h"
#include "platform/Flags.h"

struct INTERACTIVE_OBJ;

enum WeaponType {
	WEAPON_BARE = 0,
	WEAPON_DAGGER = 1,
	WEAPON_1H = 2,
	WEAPON_2H = 3,
	WEAPON_BOW = 4
};

enum ItemTypeFlag {
	OBJECT_TYPE_WEAPON   = (1<<0),
	OBJECT_TYPE_DAGGER   = (1<<1),
	OBJECT_TYPE_1H       = (1<<2),
	OBJECT_TYPE_2H       = (1<<3),
	OBJECT_TYPE_BOW      = (1<<4),
	OBJECT_TYPE_SHIELD   = (1<<5),
	OBJECT_TYPE_FOOD     = (1<<6),
	OBJECT_TYPE_GOLD     = (1<<7),
	OBJECT_TYPE_ARMOR    = (1<<8),
	OBJECT_TYPE_HELMET   = (1<<9),
	OBJECT_TYPE_RING     = (1<<10),
	OBJECT_TYPE_LEGGINGS = (1<<11)
};
DECLARE_FLAGS(ItemTypeFlag, ItemType)
DECLARE_FLAGS_OPERATORS(ItemType)

enum EquipmentSlot {
	EQUIP_SLOT_RING_LEFT = 0,
	EQUIP_SLOT_RING_RIGHT = 1,
	EQUIP_SLOT_WEAPON = 2,
	EQUIP_SLOT_SHIELD = 3,
	EQUIP_SLOT_TORCH = 4,
	EQUIP_SLOT_ARMOR = 5,
	EQUIP_SLOT_HELMET = 6,
	EQUIP_SLOT_LEGGINGS = 7
};

void ARX_EQUIPMENT_Init();
void ARX_EQUIPMENT_Remove_All_Special(INTERACTIVE_OBJ * io);
void ARX_EQUIPMENT_SetEquip(INTERACTIVE_OBJ * io, const std::string& param1, const std::string& param2, float val, short flags);

//! Sets/unsets an object type flag
bool ARX_EQUIPMENT_SetObjectType(INTERACTIVE_OBJ & io, const std::string & temp, bool set);

ItemType ARX_EQUIPMENT_GetObjectTypeFlag(const std::string& temp);
void ARX_EQUIPMENT_Equip(INTERACTIVE_OBJ * target, INTERACTIVE_OBJ * toequip);
void ARX_EQUIPMENT_UnEquip(INTERACTIVE_OBJ * target, INTERACTIVE_OBJ * toequip, long flags = 0);
void ARX_EQUIPMENT_ReleaseAll(INTERACTIVE_OBJ * io);

void ARX_EQUIPMENT_AttachPlayerWeaponToHand();
void ARX_EQUIPMENT_AttachPlayerWeaponToBack();
void ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
 
void ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
long ARX_EQUIPMENT_GetPlayerWeaponType();
float ARX_EQUIPMENT_Apply(INTERACTIVE_OBJ * io, long ident, float trueval);
bool ARX_EQUIPMENT_Strike_Check(INTERACTIVE_OBJ * io_source, INTERACTIVE_OBJ * io_weapon, float percentaim, long flags, long targ = -1);
void ARX_EQUIPMENT_RecreatePlayerMesh();
float ARX_EQUIPMENT_ComputeDamages(INTERACTIVE_OBJ * io_source, INTERACTIVE_OBJ * io_target, float ratioaim, Vec3f * pos = NULL);
 
void ARX_EQUIPMENT_IdentifyAll();

void ARX_EQUIPMENT_UnEquipAllPlayer();
float GetHitValue(const std::string & name);
void ARX_EQUIPMENT_UnEquipPlayerWeapon();
bool ARX_EQUIPMENT_IsPlayerEquip(INTERACTIVE_OBJ * _pIO);

#endif // ARX_GAME_EQUIPMENT_H
