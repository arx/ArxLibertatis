/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_GUI_INTERFACE_H
#define ARX_GUI_INTERFACE_H

#include <string>
#include <map>

#include "game/Spells.h"
#include "math/Types.h"

#include "Configure.h"

#include "graphics/Color.h"
#include "graphics/texture/TextureStage.h"
#include "gui/book/Book.h"
#include "gui/CinematicBorder.h"
#include "gui/Note.h"

class TextureContainer;
class Entity;

extern Entity * FlyingOverIO;

//-----------------------------------------------------------------------------
class INTERFACE_TC
{
public:
	INTERFACE_TC()
		: playerbook(NULL)
		, ic_casting(NULL)
		, ic_close_combat(NULL)
		, ic_constitution(NULL)
		, ic_defense(NULL)
		, ic_dexterity(NULL)
		, ic_etheral_link(NULL)
		, ic_mind(NULL)
		, ic_intuition(NULL)
		, ic_mecanism(NULL)
		, ic_object_knowledge(NULL)
		, ic_projectile(NULL)
		, ic_stealth(NULL)
		, ic_strength(NULL)
		
		, questbook(NULL)
		, ptexspellbook(NULL)
		, bookmark_char(NULL)
		, bookmark_magic(NULL)
		, bookmark_map(NULL)
		, bookmark_quest(NULL)
	{
		std::fill_n(accessibleTab, ARRAY_SIZE(accessibleTab), static_cast<TextureContainer *>(NULL));
		std::fill_n(currentTab, ARRAY_SIZE(currentTab), static_cast<TextureContainer *>(NULL));
	}
	
	void init();

public:
	TextureContainer * playerbook;
	TextureContainer * ic_casting;
	TextureContainer * ic_close_combat;
	TextureContainer * ic_constitution;
	TextureContainer * ic_defense;
	TextureContainer * ic_dexterity;
	TextureContainer * ic_etheral_link;
	TextureContainer * ic_mind;
	TextureContainer * ic_intuition;
	TextureContainer * ic_mecanism;
	TextureContainer * ic_object_knowledge;
	TextureContainer * ic_projectile;
	TextureContainer * ic_stealth;
	TextureContainer * ic_strength;
	
	TextureContainer * questbook;
	TextureContainer * ptexspellbook;
	TextureContainer * bookmark_char;
	TextureContainer * bookmark_magic;
	TextureContainer * bookmark_map;
	TextureContainer * bookmark_quest;
	
	TextureContainer * accessibleTab[10];
	TextureContainer * currentTab[10];
	
	std::string        Level;
	std::string        Xp;
};

enum E_ARX_STATE_MOUSE
{
	MOUSE_IN_WORLD,
	MOUSE_IN_TORCH_ICON,
	MOUSE_IN_BOOK,
	MOUSE_IN_INVENTORY_ICON,
	MOUSE_IN_STEAL_ICON,
	MOUSE_IN_INVENTORY,
	MOUSE_IN_NOTE
};

//-----------------------------------------------------------------------------

enum ARX_INTERFACE_MOVE_MODE
{
	MOVE_UNDEFINED,
	MOVE_WAIT,
	MOVE_WALK,
	MOVE_RUN
};

enum ARX_INTERFACE_COMBAT_MODE {
	COMBAT_MODE_OFF,
	COMBAT_MODE_ON,
	COMBAT_MODE_DRAW_WEAPON
};

//-----------------------------------------------------------------------------
extern INTERFACE_TC g_bookResouces;
extern Vec2s MemoMouse;

extern float lSLID_VALUE;
extern E_ARX_STATE_MOUSE eMouseState;
extern bool bInverseInventory;
extern bool lOldTruePlayerMouseLook;
extern bool TRUE_PLAYER_MOUSELOOK_ON;
extern bool bForceEscapeFreeLook;
extern bool COMBINEGOLD;
extern bool DRAGGING;
extern bool PLAYER_MOUSELOOK_ON;
extern bool bRenderInCursorMode;
extern bool MAGICMODE;

extern Note g_note;

extern EntityHandle LastSelectedIONum;

void ARX_INTERFACE_setCombatMode(ARX_INTERFACE_COMBAT_MODE i);

bool ARX_INTERFACE_MouseInBook();

void ARX_INTERFACE_Reset();

void ARX_INTERFACE_NoteManage();

void ARX_INTERFACE_NoteOpen(Note::Type type, const std::string & tex);
void ARX_INTERFACE_NoteClose();
void ARX_INTERFACE_NoteClear();

bool NeedHalo(Entity * io);

void ARX_INTERFACE_HALO_Render(Color3f color, long _lHaloType, TextureContainer * haloTexture, Vec2f pos, Vec2f ratio);
void ResetPlayerInterface();
void Set_DragInter(Entity * io);

void ARX_INTERFACE_DrawNumberInit();
void ARX_INTERFACE_DrawNumber(const Vec2f & pos, long num, Color color, float scale);

// 0 switch 1 forceopen 2 forceclose
void InventoryOpenClose(unsigned long t);

float getInterfaceScale(float scaleFactor, bool roundToInteger);
TextureStage::FilterMode getInterfaceTextureFilter();

extern bool g_cursorOverBook;

#endif // ARX_GUI_INTERFACE_H
