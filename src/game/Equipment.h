/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/Item.h"
#include "math/Types.h"
#include "util/Flags.h"

class Entity;

enum WeaponType {
	WEAPON_BARE = 0,
	WEAPON_DAGGER = 1,
	WEAPON_1H = 2,
	WEAPON_2H = 3,
	WEAPON_BOW = 4
};

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
void ARX_EQUIPMENT_Remove_All_Special(Entity * io);
void ARX_EQUIPMENT_SetEquip(Entity * io, bool special,
                            const std::string & modifierName, float val,
                            EquipmentModifierFlags flags);

//! Sets/unsets an object type flag
bool ARX_EQUIPMENT_SetObjectType(Entity & io, const std::string & temp, bool set);

ItemType ARX_EQUIPMENT_GetObjectTypeFlag(const std::string & temp);
void ARX_EQUIPMENT_Equip(Entity * target, Entity * toequip);
void ARX_EQUIPMENT_UnEquip(Entity * target, Entity * tounequip, long flags = 0);
void ARX_EQUIPMENT_ReleaseAll(Entity * io);

void ARX_EQUIPMENT_AttachPlayerWeaponToHand();
void ARX_EQUIPMENT_AttachPlayerWeaponToBack();
void ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
 
void ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon();
WeaponType ARX_EQUIPMENT_GetPlayerWeaponType();

/*!
 * Get a base modifier (of a specific type) for all items equipped by the player.
 *
 * \param modifier The modifier type to calculate the value for.
 * \param relative \c true to sum up relative modifiers,
 *                 \c false to sum up absolute modifiers.
 *
 * \return an absolute modifier value to add to the base stat (for absolute modifiers),
 *         or a relative modifier value to be moltiplied by the base and then added to
 *         the base (for relative modifiers).
 */
float getEquipmentBaseModifier(EquipmentModifierType modifier, bool relative = false);

/*!
 * Get the total modifier (of a specific type) for all items equipped by the player.
 *
 * This includes both absolute and relative modifiers.
 *
 * \param modifier The modifier type to calculate the value for.
 * \param baseval  The base value to apply to relative modifiers.
 *
 * \return an absolute modifier value to add to the base stat.
 */
float getEquipmentModifier(EquipmentModifierType modifier, float baseval);

bool ARX_EQUIPMENT_Strike_Check(Entity * io_source, Entity * io_weapon, float ratioaim, long flags, EntityHandle targ = EntityHandle());
void ARX_EQUIPMENT_RecreatePlayerMesh();
float ARX_EQUIPMENT_ComputeDamages(Entity * io_source, Entity * io_target, float ratioaim, Vec3f * pos = NULL);
 
void ARX_EQUIPMENT_IdentifyAll();

void ARX_EQUIPMENT_UnEquipAllPlayer();
float GetHitValue(const std::string & name);
void ARX_EQUIPMENT_UnEquipPlayerWeapon();
bool ARX_EQUIPMENT_IsPlayerEquip(Entity * _pIO);

#endif // ARX_GAME_EQUIPMENT_H
