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

#include "game/Entity.h"

#include <sstream>
#include <iomanip>
#include <cstring>

#include "animation/Animation.h"
#include "ai/Paths.h"

#include "core/Core.h"

#include "game/Camera.h"
#include "game/EntityManager.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/Levels.h"
#include "game/NPC.h"

#include "graphics/data/Mesh.h"

#include "gui/Interface.h"

#include "io/log/Logger.h"

#include "scene/ChangeLevel.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/LinkedObject.h"
#include "scene/LoadLevel.h"

extern Entity * pIOChangeWeapon;

Entity::Entity(const res::path & classPath, EntityInstance instance)
	: ioflags(0)
	, lastpos(0.f)
	, pos(0.f)
	, move(0.f)
	, lastmove(0.f)
	, forcedmove(0.f)
	, room(-1)
	, requestRoomUpdate(true)
	, original_height(0.f)
	, original_radius(0.f)
	, m_icon(NULL)
	, obj(NULL)
	, tweaky(NULL)
	, type_flags(0)
	, scriptload(0)
	, target(0.f)
	, targetinfo(TARGET_NONE)
	, inventory(NULL)
	, show(SHOW_FLAG_IN_SCENE)
	, collision(0)
	, mainevent(SM_MAIN)
	, infracolor(Color3f::blue)
	, weight(1.f)
	, gameFlags(GFLAG_NEEDINIT | GFLAG_INTERACTIVITY)
	, fall(0.f)
	, initpos(0.f)
	, scale(1.f)
	, usepath(NULL)
	, symboldraw(NULL)
	, lastspeechflag(2)
	, inzone(NULL)
	, m_disabledEvents(0)
	, stat_count(0)
	, stat_sent(0)
	, tweakerinfo(NULL)
	, material(MATERIAL_NONE)
	, m_inventorySize(1)
	, soundtime(0)
	, soundcount(0)
	, sfx_time(0)
	, collide_door_time(0)
	, ouch_time(0)
	, dmg_sum(0.f)
	, flarecount(0)
	, invisibility(0.f)
	, basespeed(1.f)
	, speed_modif(0.f)
	, rubber(BASE_RUBBER)
	, max_durability(100.f)
	, durability(max_durability)
	, poisonous(0)
	, poisonous_count(0)
	, ignition(0.f)
	, head_rot(0.f)
	, damager_damages(0)
	, damager_type(0)
	, sfx_flag(0)
	, secretvalue(-1)
	, shop_multiply(1.f)
	, isHit(false)
	, inzone_show(0)
	, spark_n_blood(0)
	, special_color(Color3f::white)
	, highlightColor(Color3f::black)
	, m_index(size_t(-1))
	, m_id(classPath, instance)
	, m_classPath(classPath)
{
	
	m_index = entities.add(this);
	
	std::fill_n(anims, MAX_ANIMS, static_cast<ANIM_HANDLE *>(NULL));
	
	for(size_t l = 0; l < MAX_ANIM_LAYERS; l++) {
		animlayer[l] = AnimLayer();
	}
	
	animBlend.m_active = false;
	animBlend.lastanimtime = 0;
	
	bbox3D = EERIE_3D_BBOX(Vec3f(0.f), Vec3f(0.f));
	
	bbox2D.min = Vec2f(-1.f, -1.f);
	bbox2D.max = Vec2f(-1.f, -1.f);
	
	_itemdata = NULL;
	_fixdata = NULL;
	_npcdata = NULL;
	_camdata = NULL;
	
	halo_native.color = Color3f(0.2f, 0.5f, 1.f);
	halo_native.radius = 45.f;
	halo_native.flags = 0;
	ARX_HALO_SetToNative(this);
	
	for(size_t j = 0; j < MAX_SCRIPTTIMERS; j++) {
		m_scriptTimers[j] = 0;
	}
	
}

Entity::~Entity() {
	
	cleanReferences();
	
	if(g_cameraEntity == this) {
		g_cameraEntity = NULL;
	}
	
	// Releases ToBeDrawn Transparent Polys linked to this object !
	tweaks.clear();
	
	if(obj && !(ioflags & IO_CAMERA) && !(ioflags & IO_MARKER) && !(ioflags & IO_GOLD)) {
		delete obj, obj = NULL;
	}
	
	spells.removeTarget(this);
	
	delete tweakerinfo;
	delete tweaky, tweaky = NULL;
	
	ReleaseScript(&script);
	ReleaseScript(&over_script);
	
	for(size_t n = 0; n < MAX_ANIMS; n++) {
		if(anims[n]) {
			EERIE_ANIMMANAGER_ReleaseHandle(anims[n]);
			anims[n] = NULL;
		}
	}
	
	lightHandleDestroy(dynlight);
	
	delete usepath;
	
	delete symboldraw;
	symboldraw = NULL;
	
	if(ioflags & IO_NPC) {
		delete _npcdata;
	} else if(ioflags & IO_ITEM) {
		delete _itemdata->equipitem;
		delete _itemdata;
	} else if(ioflags & IO_FIX) {
		delete _fixdata;
	} else if(ioflags & IO_CAMERA && _camdata) {
		if(g_camera == &_camdata->cam) {
			SetActiveCamera(&g_playerCamera);
		}
		delete _camdata;
	}
	
	if(SecondaryInventory && SecondaryInventory->io == this) {
		SecondaryInventory = NULL;
	}
	
	if(TSecondaryInventory && TSecondaryInventory->io == this) {
		TSecondaryInventory = NULL;
	}
	
	delete inventory;
	
	if(m_index != size_t(-1)) {
		entities.remove(m_index);
	}
	
}

res::path Entity::instancePath() const {
	return m_classPath.parent() / idString();
}

void Entity::cleanReferences() {
	
	ARX_INTERACTIVE_DestroyIOdelayedRemove(this);
	
	if(DRAGINTER == this) {
		Set_DragInter(NULL);
	}
	
	if(FlyingOverIO == this) {
		FlyingOverIO = NULL;
	}
	
	if(COMBINE == this) {
		COMBINE = NULL;
	}
	
	if(pIOChangeWeapon == this) {
		pIOChangeWeapon = NULL; // TODO we really need a proper weak_ptr
	}
	
	if(ioSteal == this) {
		ioSteal = NULL;
	}
	
	if(!FAST_RELEASE) {
		TREATZONE_RemoveIO(this);
	}
	gameFlags &= ~GFLAG_ISINTREATZONE;
	
	ARX_INTERACTIVE_DestroyDynamicInfo(this);
	
	RemoveFromAllInventories(this);
	
	ARX_SCRIPT_Timer_Clear_For_IO(this);
	
	spells.endByCaster(index());
	
	lightHandleDestroy(ignit_light);
	
	ARX_SOUND_Stop(ignit_sound);
	ignit_sound = audio::SourcedSample();
}

void Entity::destroy() {
	
	LogDebug("destroying entity " << idString());
	
	if(instance() > 0 && !(ioflags & IO_NOSAVE)) {
		if(scriptload) {
			// In case we previously saved this entity...
			currentSavedGameRemoveEntity(idString());
		} else {
			currentSavedGameStoreEntityDeletion(idString());
		}
	}
	
	if(obj) {
		while(!obj->linked.empty()) {
			if(obj->linked[0].lgroup != ObjVertGroup() && obj->linked[0].obj) {
				Entity * linked = obj->linked[0].io;
				if(linked && ValidIOAddress(linked)) {
					EERIE_LINKEDOBJ_UnLinkObjectFromObject(obj, linked->obj);
					linked->destroy();
				}
			}
		}
	}
	
	delete this;
	
}

void Entity::destroyOne() {
	
	if((ioflags & IO_ITEM) && _itemdata->count > 1) {
		_itemdata->count--;
	} else {
		destroy();
	}
	
}
