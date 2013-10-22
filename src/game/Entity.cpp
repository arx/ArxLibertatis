/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "scene/GameSound.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "scene/LoadLevel.h"

extern Entity * pIOChangeWeapon;

Entity::Entity(const res::path & classPath)
	: m_index(size_t(-1)),
	  m_classPath(classPath) {
	
	m_index = entities.add(this);
	
	ioflags = 0;
	lastpos = Vec3f_ZERO;
	pos = Vec3f_ZERO;
	move = Vec3f_ZERO;
	lastmove = Vec3f_ZERO;
	forcedmove = Vec3f_ZERO;
	
	angle = Anglef::ZERO;
	std::memset(&physics, 0, sizeof(IO_PHYSICS)); // TODO use constructor
	room = -1;
	room_flags = 1;
	original_height = 0.f;
	original_radius = 0.f;
	inv = NULL;
	obj = NULL;
	std::fill_n(anims, MAX_ANIMS, (ANIM_HANDLE *)NULL);
	std::memset(animlayer, 0, sizeof(ANIM_USE) * MAX_ANIM_LAYERS); // TODO use constructor

	animBlend.nb_lastanimvertex = 0;
	animBlend.lastanimtime = 0;
	
	std::memset(&bbox3D, 0, sizeof(EERIE_3D_BBOX)); // TODO use constructor
	
	bbox2D.min = Vec2f(-1.f, -1.f);
	bbox2D.max = Vec2f(-1.f, -1.f);

	tweaky = NULL;
	sound = audio::INVALID_ID;
	type_flags = 0;
	scriptload = 0;
	target = Vec3f_ZERO;
	targetinfo = TARGET_NONE;
	
	_itemdata = NULL, _fixdata = NULL, _npcdata = NULL, _camdata = NULL;
	
	inventory = NULL;
	show = SHOW_FLAG_IN_SCENE;
	collision = 0;
	infracolor = Color3f::blue;
	changeanim = -1;
	
	ident = 0;
	weight = 1.f;
	gameFlags = GFLAG_NEEDINIT | GFLAG_INTERACTIVITY;
	velocity = Vec3f_ZERO;
	fall = 0.f;
	
	stopped = 1;
	initpos = Vec3f_ZERO;
	initangle = Anglef::ZERO;
	scale = 1.f;
	
	usepath = NULL;
	symboldraw = NULL;
	dynlight = -1;
	lastspeechflag = 2;
	inzone = NULL;
	std::memset(&halo, 0, sizeof(IO_HALO)); // TODO use constructor
	std::memset(&halo_native, 0, sizeof(IO_HALO)); // TODO use constructor
	halo_native.color = Color3f(0.2f, 0.5f, 1.f);
	halo_native.radius = 45.f;
	halo_native.flags = 0;
	halo_native.dynlight = -1;
	ARX_HALO_SetToNative(this);
	halo.dynlight = -1;
	
	std::memset(&script, 0, sizeof(EERIE_SCRIPT)); // TODO use constructor
	std::memset(&over_script, 0, sizeof(EERIE_SCRIPT)); // TODO use constructor
	stat_count = 0;
	stat_sent = 0;
	tweakerinfo = NULL;
	material = MATERIAL_NONE;
	
	sizex = 1;
	sizey = 1;
	soundtime = 0;
	soundcount = 0;
	
	sfx_time = 0;
	collide_door_time = 0;
	ouch_time = 0;
	dmg_sum = 0.f;
	
	std::memset(&spellcast_data, 0, sizeof(IO_SPELLCAST_DATA));
	flarecount = 0;
	no_collide = -1;
	invisibility = 0.f;
	basespeed = 1.f;
	
	speed_modif = 0.f;
	spells_on = NULL;
	nb_spells_on = 0;
	damagedata = -1;
	
	rubber = BASE_RUBBER;
	max_durability = durability = 100.f;
	poisonous = 0;
	poisonous_count = 0;
	
	ignition = 0.f;
	ignit_light = -1;
	ignit_sound = audio::INVALID_ID;
	head_rot = 0.f;
	
	damager_damages = 0;
	damager_type = 0;
	
	sfx_flag = 0;
	secretvalue = -1;
	
	shop_multiply = 1.f;
	isHit = false;
	inzone_show = 0;
	summoner = 0;
	spark_n_blood = 0;

	special_color = Color3f::white;
	highlightColor = Color3f::black;
	
	ARX_SCRIPT_SetMainEvent(this, "main");
	
}

Entity::~Entity() {
	
	cleanReferences();
	
	if(!FAST_RELEASE) {
		TREATZONE_RemoveIO(this);
	}
	
	if(ignit_light > -1) {
		DynLight[ignit_light].exist = 0, ignit_light = -1;
	}
	
	if(ignit_sound != audio::INVALID_ID) {
		ARX_SOUND_Stop(ignit_sound), ignit_sound = audio::INVALID_ID;
	}
	
	if(FlyingOverIO == this) {
		FlyingOverIO = NULL;
	}
	
	if((MasterCamera.exist & 1) && MasterCamera.io == this) {
		MasterCamera.exist = 0;
	}
	
	if((MasterCamera.exist & 2) && MasterCamera.want_io == this) {
		MasterCamera.exist = 0;
	}
	
	ARX_INTERACTIVE_DestroyDynamicInfo(this);
	IO_UnlinkAllLinkedObjects(this);
	
	// Releases ToBeDrawn Transparent Polys linked to this object !
	tweaks.clear();
	ARX_SCRIPT_Timer_Clear_For_IO(this);
	
	if(obj && !(ioflags & IO_CAMERA) && !(ioflags & IO_MARKER) && !(ioflags & IO_GOLD)) {
		delete obj, obj = NULL;
	}
	
	ARX_SPELLS_RemoveAllSpellsOn(this);
	
	delete tweakerinfo;
	delete tweaky, tweaky = NULL;
	
	RemoveFromAllInventories(this);
	
	ReleaseScript(&script);
	ReleaseScript(&over_script);
	
	for(long n = 0; n < MAX_ANIMS; n++) {
		if(anims[n]) {
			EERIE_ANIMMANAGER_ReleaseHandle(anims[n]);
			anims[n] = NULL;
		}
	}
	
	if(damagedata >= 0) {
		damages[damagedata].exist = 0;
	}
	
	if(ValidDynLight(dynlight)) {
		DynLight[dynlight].exist = 0, dynlight = -1;
	}
	
	if(ValidDynLight(halo.dynlight)) {
		DynLight[halo.dynlight].exist = 0, halo.dynlight = -1;
	}
	
	free(usepath);
	free(symboldraw), symboldraw = NULL;
	
	if(ioflags & IO_NPC) {
		delete _npcdata;
		
	} else if(ioflags & IO_ITEM) {
		free(_itemdata->equipitem);
		free(_itemdata);
		
	} else if(ioflags & IO_FIX) {
		free(_fixdata);
		
	} else if(ioflags & IO_CAMERA && _camdata) {
		if(ACTIVECAM == &_camdata->cam) {
			ACTIVECAM = &subj;
		}
		free(_camdata);
	}
	
	if(TSecondaryInventory && TSecondaryInventory->io == this) {
		TSecondaryInventory = NULL;
	}
	
	free(inventory);
	
	if(m_index != size_t(-1)) {
		entities.remove(m_index);
	}
	
	if(pIOChangeWeapon == this) {
		pIOChangeWeapon = NULL; // TODO we really need a proper weak_ptr
	}
}

std::string Entity::short_name() const {
	return m_classPath.filename();
}

std::string Entity::long_name() const {
	std::stringstream ss;
	ss << short_name() << '_' << std::setw(4) << std::setfill('0') << ident;
	return ss.str();
}

res::path Entity::full_name() const {
	return m_classPath.parent() / long_name();
}

void Entity::cleanReferences() {
	
	if(DRAGINTER == this) {
		Set_DragInter(NULL);
	}
	
}

void Entity::destroy() {
	
	if(scriptload) {
		delete this;
	} else {
		show = SHOW_FLAG_KILLED;
		cleanReferences();
	}
	
}

