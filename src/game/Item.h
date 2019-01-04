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

#ifndef ARX_GAME_ITEM_H
#define ARX_GAME_ITEM_H

#include "game/Entity.h"

enum EquipmentModifierType {
	// Attributes
	IO_EQUIPITEM_ELEMENT_STRENGTH         = 0,
	IO_EQUIPITEM_ELEMENT_DEXTERITY        = 1,
	IO_EQUIPITEM_ELEMENT_CONSTITUTION     = 2,
	IO_EQUIPITEM_ELEMENT_MIND             = 3,
	// Skills
	IO_EQUIPITEM_ELEMENT_Stealth          = 4,
	IO_EQUIPITEM_ELEMENT_Mecanism         = 5,
	IO_EQUIPITEM_ELEMENT_Intuition        = 6,
	IO_EQUIPITEM_ELEMENT_Etheral_Link     = 7,
	IO_EQUIPITEM_ELEMENT_Object_Knowledge = 8,
	IO_EQUIPITEM_ELEMENT_Casting          = 9,
	IO_EQUIPITEM_ELEMENT_Projectile       = 10,
	IO_EQUIPITEM_ELEMENT_Close_Combat     = 11,
	IO_EQUIPITEM_ELEMENT_Defense          = 12,
	// Other stats
	IO_EQUIPITEM_ELEMENT_Armor_Class      = 13,
	IO_EQUIPITEM_ELEMENT_Resist_Magic     = 14,
	IO_EQUIPITEM_ELEMENT_Resist_Poison    = 15,
	IO_EQUIPITEM_ELEMENT_Critical_Hit     = 16,
	IO_EQUIPITEM_ELEMENT_Damages          = 17,
	IO_EQUIPITEM_ELEMENT_Duration         = 18,
	IO_EQUIPITEM_ELEMENT_AimTime          = 19,
	IO_EQUIPITEM_ELEMENT_Identify_Value   = 20,
	IO_EQUIPITEM_ELEMENT_Life             = 21,
	IO_EQUIPITEM_ELEMENT_Mana             = 22,
	IO_EQUIPITEM_ELEMENT_MaxLife          = 23,
	IO_EQUIPITEM_ELEMENT_MaxMana          = 24,
	IO_EQUIPITEM_ELEMENT_SPECIAL_1        = 25,
	IO_EQUIPITEM_ELEMENT_SPECIAL_2        = 26,
	IO_EQUIPITEM_ELEMENT_SPECIAL_3        = 27,
	IO_EQUIPITEM_ELEMENT_SPECIAL_4        = 28,
	IO_EQUIPITEM_ELEMENT_Number
};

enum EquipmentModifierFlag {
	IO_ELEMENT_FLAG_PERCENT = 1 << 0
};
DECLARE_FLAGS(EquipmentModifierFlag, EquipmentModifierFlags)
DECLARE_FLAGS_OPERATORS(EquipmentModifierFlags)

enum EquipmentModifiedSpecialType {
	IO_SPECIAL_ELEM_NONE       = 0,
	IO_SPECIAL_ELEM_PARALYZE   = 1,
	IO_SPECIAL_ELEM_DRAIN_LIFE = 2
};

struct IO_EQUIPITEM_ELEMENT {
	
	float value;
	EquipmentModifierFlags flags;
	EquipmentModifiedSpecialType special;
	
	IO_EQUIPITEM_ELEMENT()
		: value(0.f)
		, flags(0)
		, special(IO_SPECIAL_ELEM_NONE)
	{ }
	
};

struct IO_EQUIPITEM {
	IO_EQUIPITEM_ELEMENT elements[IO_EQUIPITEM_ELEMENT_Number];
};

struct IO_ITEMDATA {
	
	IO_EQUIPITEM * equipitem; // Equipitem Datas
	long price;
	short maxcount; // max number cumulable
	short count; // current number
	char food_value;
	char stealvalue;
	short playerstacksize;
	short LightValue;
	
	IO_ITEMDATA()
		: equipitem(0)
		, price(0)
		, maxcount(0)
		, count(0)
		, food_value(0)
		, stealvalue(0)
		, playerstacksize(0)
		, LightValue(0)
	{ }
	
};

#endif // ARX_GAME_ITEM_H
