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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include "animation/Animation.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "gui/Interface.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/MeshManipulation.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/resource/ResourcePath.h"

#include "math/Random.h"
#include "math/Vector.h"

#include "physics/Collisions.h"

#include "platform/Platform.h"

#include "scene/Object.h"
#include "scene/LinkedObject.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

#include "script/Script.h"

using std::min;
using std::max;
using std::string;

struct EQUIP_INFO
{
	char name[64];
};

#define SP_SPARKING 1
#define SP_BLOODY 2

extern Vec3f PUSH_PLAYER_FORCE;
extern long HERO_SHOW_1ST;
extern bool EXTERNALVIEW;

extern EERIE_3DOBJ * arrowobj;

EQUIP_INFO equipinfo[IO_EQUIPITEM_ELEMENT_Number];

//! \brief Returns the object type flag corresponding to a string
ItemType ARX_EQUIPMENT_GetObjectTypeFlag(const string & temp) {
	
	if(temp.empty()) {
		return 0;
	}
	
	char c = temp[0];
	
	arx_assert(std::tolower(c) == c);
	
	switch(c) {
		case 'w':
			return OBJECT_TYPE_WEAPON;
		case 'd':
			return OBJECT_TYPE_DAGGER;
		case '1':
			return OBJECT_TYPE_1H;
		case '2':
			return OBJECT_TYPE_2H;
		case 'b':
			return OBJECT_TYPE_BOW;
		case 's':
			return OBJECT_TYPE_SHIELD;
		case 'f':
			return OBJECT_TYPE_FOOD;
		case 'g':
			return OBJECT_TYPE_GOLD;
		case 'r':
			return OBJECT_TYPE_RING;
		case 'a':
			return OBJECT_TYPE_ARMOR;
		case 'h':
			return OBJECT_TYPE_HELMET;
		case 'l':
			return OBJECT_TYPE_LEGGINGS;
	}
	
	return 0;
}

//! \brief Releases Equiped Id from player
void ARX_EQUIPMENT_Release(long id) {
	if(id) {
		for(long i = 0; i < MAX_EQUIPED; i++) {
			if(player.equiped[i] == id) {
				player.equiped[i] = 0;
			}
		}
	}
}

//! \brief Releases Equipment Structure
void ARX_EQUIPMENT_ReleaseAll(Entity * io) {
	if(!io || !(io->ioflags & IO_ITEM)) {
		return;
	}
	
	free(io->_itemdata->equipitem);
	io->_itemdata->equipitem = NULL;
}

extern long EXITING;

//! \brief Recreates player mesh from scratch
static void applyTweak(EquipmentSlot equip, TweakType tw, const string & selection) {
	
	if(!player.equiped[equip] || !ValidIONum(player.equiped[equip])) {
		return;
	}
	
	Entity * io = entities.player();
	
	arx_assert(entities[player.equiped[equip]]->tweakerinfo != NULL);
	
	const IO_TWEAKER_INFO & tweak = *entities[player.equiped[equip]]->tweakerinfo;
	
	if(!tweak.filename.empty()) {
		res::path mesh = "graph/obj3d/interactive/npc/human_base/tweaks" / tweak.filename;
		EERIE_MESH_TWEAK_Do(io, tw, mesh);
	}
	
	if(tweak.skintochange.empty() || tweak.skinchangeto.empty()) {
		return;
	}
	
	res::path file = "graph/obj3d/textures" / tweak.skinchangeto;
	TextureContainer * temp = TextureContainer::Load(file, TextureContainer::Level);
	
	long mapidx = ObjectAddMap(io->obj, temp);
	
	long sel = -1;
	for(size_t i = 0; i < io->obj->selections.size(); i++) {
		if(io->obj->selections[i].name == selection) {
			sel = i;
			break;
		}
	}
	if(sel == -1) {
		return;
	}
	
	long textochange = -1;
	for(size_t i = 0; i < io->obj->texturecontainer.size(); i++) {
		if(tweak.skintochange == io->obj->texturecontainer[i]->m_texName.filename()) {
			textochange = i;
		}
	}
	if(textochange == -1) {
		return;
	}
	
	for(size_t i = 0; i < io->obj->facelist.size(); i++) {
		if(IsInSelection(io->obj, io->obj->facelist[i].vid[0], sel) != -1
		   && IsInSelection(io->obj, io->obj->facelist[i].vid[1], sel) != -1
		   && IsInSelection(io->obj, io->obj->facelist[i].vid[2], sel) != -1) {
			if(io->obj->facelist[i].texid == textochange) {
				io->obj->facelist[i].texid = (short)mapidx;
			}
		}
	}
	
}

void ARX_EQUIPMENT_RecreatePlayerMesh() {
	
	if(EXITING)
		return;
	
	Entity * io = entities.player();
	if(!io)
		return;
	
	if(io->obj != hero)
		delete io->obj;

	io->obj = loadObject("graph/obj3d/interactive/npc/human_base/human_base.teo", false);
	
	applyTweak(EQUIP_SLOT_HELMET, TWEAK_HEAD, "head");
	applyTweak(EQUIP_SLOT_ARMOR, TWEAK_TORSO, "chest");
	applyTweak(EQUIP_SLOT_LEGGINGS, TWEAK_LEGS, "leggings");
	
	Entity * target = entities.player();
	if(!target)
		return;

	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] && ValidIONum(player.equiped[i])) {
			Entity *toequip = entities[player.equiped[i]];

			if(toequip) {
				if(toequip->type_flags & (OBJECT_TYPE_DAGGER | OBJECT_TYPE_1H | OBJECT_TYPE_2H | OBJECT_TYPE_BOW)) {
					if(player.Interface & INTER_COMBATMODE) {
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
					} else {
						EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "weapon_attach", "primary_attach", toequip);
					}
				} else if(toequip->type_flags & OBJECT_TYPE_SHIELD) {
					if(player.equiped[EQUIP_SLOT_SHIELD] != 0) {
						EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "shield_attach", "shield_attach", toequip);
					}
				}
			}
		}
	}

	ARX_PLAYER_Restore_Skin();
	HERO_SHOW_1ST = -1;

	if(EXTERNALVIEW) {
		ARX_INTERACTIVE_Show_Hide_1st(entities.player(), 0);
	} else {
		ARX_INTERACTIVE_Show_Hide_1st(entities.player(), 1);
	}

	ARX_INTERACTIVE_HideGore(entities.player(), 1);
	EERIE_Object_Precompute_Fast_Access(hero);
	EERIE_Object_Precompute_Fast_Access(entities.player()->obj);

	ARX_INTERACTIVE_RemoveGoreOnIO(entities.player()); 
}

void ARX_EQUIPMENT_UnEquipAllPlayer() {
	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] && ValidIONum(player.equiped[i])) {
			ARX_EQUIPMENT_UnEquip(entities.player(), entities[player.equiped[i]]);
		}
	}

	ARX_PLAYER_ComputePlayerFullStats();
}

bool ARX_EQUIPMENT_IsPlayerEquip(Entity * _pIO)
{
	Entity * io = entities.player();
	if(!io)
		return false;

	if (io != entities.player()) return false;

	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] && ValidIONum(player.equiped[i])) {
			Entity * toequip = entities[player.equiped[i]];

			if(toequip == _pIO) {
				return true;
			}
		}
	}

	return false;
}


//***********************************************************************************************
// flags & 1 == destroyed !
//***********************************************************************************************
void ARX_EQUIPMENT_UnEquip(Entity * target, Entity * tounequip, long flags)
{
	if(!target || !tounequip)
		return;

	if(target != entities.player())
		return;

	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] && ValidIONum(player.equiped[i]) && entities[player.equiped[i]] == tounequip) {
			EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, tounequip->obj);
			ARX_EQUIPMENT_Release(player.equiped[i]);
			target->bbox2D.min.x = 9999;
			target->bbox2D.max.x = -9999;
			
			if(!flags & 1) {
				if(!DRAGINTER) {
					ARX_SOUND_PlayInterface(SND_INVSTD);
					Set_DragInter(tounequip);
				} else {
					giveToPlayer(tounequip);
				}
			}
			
			EVENT_SENDER = tounequip;
			SendIOScriptEvent(entities.player(), SM_EQUIPOUT);
			EVENT_SENDER = entities.player();
			SendIOScriptEvent(tounequip, SM_EQUIPOUT);
		}
	}

	if(tounequip->type_flags & (OBJECT_TYPE_HELMET | OBJECT_TYPE_ARMOR | OBJECT_TYPE_LEGGINGS))
		ARX_EQUIPMENT_RecreatePlayerMesh();
}

void ARX_EQUIPMENT_AttachPlayerWeaponToHand()
{
	Entity * target = entities.player();
	if(!target)
		return;

	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] && ValidIONum(player.equiped[i])) {
			Entity *toequip = entities[player.equiped[i]];

			if(toequip) {
				if(toequip->type_flags & (OBJECT_TYPE_DAGGER | OBJECT_TYPE_1H | OBJECT_TYPE_2H | OBJECT_TYPE_BOW)) {
					EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, toequip->obj);
					EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "primary_attach", "primary_attach", toequip);
					return;
				}
			}
		}
	}
}

void ARX_EQUIPMENT_AttachPlayerWeaponToBack()
{
	Entity * target = entities.player();

	if (!target) return;

	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] && ValidIONum(player.equiped[i])) {
			Entity *toequip = entities[player.equiped[i]];

			if(toequip) {
				if ((toequip->type_flags & OBJECT_TYPE_DAGGER)
				        ||	(toequip->type_flags & OBJECT_TYPE_1H)
				        ||	(toequip->type_flags & OBJECT_TYPE_2H)
				        ||	(toequip->type_flags & OBJECT_TYPE_BOW)
				   )
				{
					if(toequip->type_flags & OBJECT_TYPE_BOW) {
						EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, toequip->obj);
						EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "weapon_attach", "test", toequip); //
						return;
					}

					EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, toequip->obj);
					EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "weapon_attach", "primary_attach", toequip); //
					return;
				}
			}
		}
	}
}

WeaponType ARX_EQUIPMENT_GetPlayerWeaponType()
{
	Entity * io = entities.player();
	if(!io)
		return WEAPON_BARE;

	if(player.equiped[EQUIP_SLOT_WEAPON] && ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
		Entity * toequip = entities[player.equiped[EQUIP_SLOT_WEAPON]];

		if(toequip) {
			if(toequip->type_flags & OBJECT_TYPE_DAGGER)
				return WEAPON_DAGGER;

			if(toequip->type_flags & OBJECT_TYPE_1H)
				return WEAPON_1H;

			if(toequip->type_flags & OBJECT_TYPE_2H)
				return WEAPON_2H;

			if(toequip->type_flags & OBJECT_TYPE_BOW)
				return WEAPON_BOW;
		}
	}

	return WEAPON_BARE;
}

void ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon()
{
	Entity * io = entities.player();
	if(!io)
		return;

	ANIM_HANDLE * anim;
	WeaponType type = ARX_EQUIPMENT_GetPlayerWeaponType();

	switch(type) {
		case WEAPON_DAGGER:
			anim = io->anims[ANIM_DAGGER_UNREADY_PART_1];
			break;
		case WEAPON_1H:
			anim = io->anims[ANIM_1H_UNREADY_PART_1];
			break;
		case WEAPON_2H:
			anim = io->anims[ANIM_2H_UNREADY_PART_1];
			break;
		case WEAPON_BOW:
			anim = io->anims[ANIM_MISSILE_UNREADY_PART_1];

			if(arrowobj)
				EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
			break;
		default:
			anim = io->anims[ANIM_BARE_UNREADY];
			break;
	}

	AcquireLastAnim(io);
	ANIM_Set(&io->animlayer[1], anim);
}

float ARX_EQUIPMENT_ComputeDamages(Entity * io_source, Entity * io_target, float ratioaim, Vec3f * position)
{
	EVENT_SENDER = io_source;
	SendIOScriptEvent(io_target, SM_AGGRESSION);

	if(!io_source || !io_target)
		return 0.f;

	if(!(io_target->ioflags & IO_NPC)) {
		if(io_target->ioflags & IO_FIX) {
			if (io_source == entities.player())
				ARX_DAMAGES_DamageFIX(io_target, player.Full_damages, 0, 0);
			else if (io_source->ioflags & IO_NPC)
				ARX_DAMAGES_DamageFIX(io_target, io_source->_npcdata->damages, io_source->index(), 0);
			else
				ARX_DAMAGES_DamageFIX(io_target, 1, io_source->index(), 0);
		}

		return 0.f;
	}

	float attack, ac, damages;
	float backstab = 1.f;

	string _wmat = "bare";
	const string * wmat = &_wmat;
	
	string _amat = "flesh";
	const string * amat = &_amat;

	bool critical = false;

	if(io_source == entities.player()) {
		
		if(player.equiped[EQUIP_SLOT_WEAPON] != 0 && ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
			Entity * io = entities[player.equiped[EQUIP_SLOT_WEAPON]];
			if(io && !io->weaponmaterial.empty()) {
				wmat = &io->weaponmaterial;
			}
		}

		attack = player.Full_damages;

		if(rnd() * 100 <= player.Full_Critical_Hit)
		{
			if(SendIOScriptEvent(io_source, SM_CRITICAL) != REFUSE)
				critical = true;
		}
		else
			critical = false;

		damages = attack * ratioaim; 

		if(io_target->_npcdata->npcflags & NPCFLAG_BACKSTAB) {
			if(rnd() * 100.f <= player.Full_Skill_Stealth * ( 1.0f / 2 )) {
				if(SendIOScriptEvent(io_source, SM_BACKSTAB) != REFUSE)
					backstab = 1.5f; 
			}
		}
	} else {
		if(!(io_source->ioflags & IO_NPC)) // no NPC source...
			return 0.f;

		if(!io_source->weaponmaterial.empty()) {
			wmat = &io_source->weaponmaterial;
		}
		
		if(io_source->_npcdata->weapon) {
			Entity * iow = io_source->_npcdata->weapon;
			if(!iow->weaponmaterial.empty()) {
				wmat = &iow->weaponmaterial;
			}
		}
		
		attack = io_source->_npcdata->tohit;
		
		damages = io_source->_npcdata->damages * ratioaim * (rnd() * ( 1.0f / 2 ) + 0.5f);

		long value = ARX_SPELLS_GetSpellOn(io_source, SPELL_CURSE);

		if(value >= 0) {
			damages *= (1 - spells[value].caster_level * 0.05f);
		}

		if(rnd() * 100 <= io_source->_npcdata->critical) {
			if(SendIOScriptEvent(io_source, SM_CRITICAL) != REFUSE)
				critical = true;
		}
		else
			critical = false;

		if(rnd() * 100.f <= (float)io_source->_npcdata->backstab_skill) {
			if(SendIOScriptEvent(io_source, SM_BACKSTAB) != REFUSE)
				backstab = 1.5f; 
		}
	}

	float absorb;

	if(io_target == entities.player()) {
		ac = player.Full_armor_class;
		absorb = player.Full_Skill_Defense * ( 1.0f / 2 );
	} else {
		ac = ARX_INTERACTIVE_GetArmorClass(io_target);
		absorb = io_target->_npcdata->absorb;
		long value = ARX_SPELLS_GetSpellOn(io_target, SPELL_CURSE);

		if(value >= 0) {
			float modif = (1 - spells[value].caster_level * 0.05f);
			ac *= modif;
			absorb *= modif;
		}
	}
	
	if(!io_target->armormaterial.empty()) {
		amat = &io_target->armormaterial;
	}
	
	if(io_target == entities.player()) {
		if(player.equiped[EQUIP_SLOT_ARMOR] > 0
		   && ValidIONum(player.equiped[EQUIP_SLOT_ARMOR])) {
			Entity * io = entities[player.equiped[EQUIP_SLOT_ARMOR]];
			if(io && !io->armormaterial.empty()) {
				amat = &io->armormaterial;
			}
		}
	}
	
	float dmgs = damages * backstab;
	dmgs -= dmgs * absorb * 0.01f;
	
	Vec3f pos = io_target->pos;
	float power = std::min(1.f, dmgs * 0.05f) * 0.1f + 0.9f;
	
	ARX_SOUND_PlayCollision(*amat, *wmat, power, 1.f, &pos, io_source);
	
	float chance = 100.f - (ac - attack); 
	if(rnd() * 100.f > chance) {
		return 0.f;
	}
	
	ARX_SOUND_PlayCollision("flesh", *wmat, power, 1.f, &pos, io_source);
	
	if(dmgs > 0.f) {
		
		if(critical) {
			dmgs *= 1.5f; 
		}
		
		if(io_target == entities.player()) {
			
			// TODO should this be player.pos - player.baseOffset() = player.basePosition()?
			Vec3f ppos = io_source->pos - (player.pos + player.baseOffset());
			fnormalize(ppos);
			
			// Push the player
			PUSH_PLAYER_FORCE += ppos * -dmgs * Vec3f(1.0f / 11, 1.0f / 30, 1.0f / 11);
			
			ppos *= 60.f;
			ppos += ACTIVECAM->orgTrans.pos;
			ARX_DAMAGES_DamagePlayer(dmgs, 0, io_source->index());
			ARX_DAMAGES_DamagePlayerEquipment(dmgs);
			
		} else {
			
			Vec3f ppos = io_source->pos - io_target->pos;
			fnormalize(ppos);
			
			// Push the NPC
			io_target->forcedmove += ppos * -dmgs;
			
			Vec3f * pos = position ? position : &io_target->pos;
			ARX_DAMAGES_DamageNPC(io_target, dmgs, io_source->index(), 0, pos);
		}
	}
	
	return dmgs;
}

static float ARX_EQUIPMENT_GetSpecialValue(Entity * io, long val) {
	
	if(!io || !(io->ioflags & IO_ITEM) || !io->_itemdata->equipitem)
		return -1;

	for(long i = IO_EQUIPITEM_ELEMENT_SPECIAL_1; i <= IO_EQUIPITEM_ELEMENT_SPECIAL_4; i++) {
		if(io->_itemdata->equipitem->elements[i].special == val) {
			return io->_itemdata->equipitem->elements[i].value;
		}
	}

	return -1;
}

//***********************************************************************************************
// flags & 1 = blood spawn only
//-----------------------------------------------------------------------------------------------
//***********************************************************************************************
bool ARX_EQUIPMENT_Strike_Check(Entity * io_source, Entity * io_weapon, float ratioaim, long flags, long targ)
{
	
	bool ret = false;
	long source = (io_source == NULL) ? -1 : io_source->index();
	long weapon = io_weapon->index();
	EERIE_SPHERE sphere;

	Vec3f * v0;
	EXCEPTIONS_LIST_Pos = 0;
	float rad;

	long nbact = io_weapon->obj->actionlist.size();
	float drain_life = ARX_EQUIPMENT_GetSpecialValue(io_weapon, IO_SPECIAL_ELEM_DRAIN_LIFE);
	float paralyse = ARX_EQUIPMENT_GetSpecialValue(io_weapon, IO_SPECIAL_ELEM_PARALYZE);

	for (long j = 0; j < nbact; j++) // TODO iterator
	{
		if(!ValidIONum(weapon))
			return false;

		rad = GetHitValue(io_weapon->obj->actionlist[j].name);

		if(rad == -1)
			continue;
		
		v0 = &io_weapon->obj->vertexlist3[io_weapon->obj->actionlist[j].idx].v;
		sphere.origin = *v0;
		
		sphere.radius = rad; 

		if(source != 0)
			sphere.radius += 15.f;

		std::vector<long> sphereContent;

		if(CheckEverythingInSphere(&sphere, source, targ, sphereContent)) {
			for(size_t jj = 0; jj < sphereContent.size(); jj++) {
				if(ValidIONum(sphereContent[jj])
						&& !(entities[sphereContent[jj]]->ioflags & IO_BODY_CHUNK))
				{
					long HIT_SPARK = 0;
					EXCEPTIONS_LIST[EXCEPTIONS_LIST_Pos] = sphereContent[jj];
					EXCEPTIONS_LIST_Pos++;

					if(EXCEPTIONS_LIST_Pos >= MAX_IN_SPHERE)
						EXCEPTIONS_LIST_Pos--;

					Entity * target = entities[sphereContent[jj]];
			
					Vec3f pos;
					Color color = Color::white;
					long hitpoint = -1;
					float curdist = 999999.f;
					
					Vec3f vector = (sphere.origin - target->pos) * Vec3f(1.f, 0.5f, 1.f);
					vector = glm::normalize(vector);

					for(size_t ii = 0; ii < target->obj->facelist.size(); ii++) {
						if(target->obj->facelist[ii].facetype & POLY_HIDE)
							continue;

						float d = glm::distance(sphere.origin, target->obj->vertexlist3[target->obj->facelist[ii].vid[0]].v);

						if(d < curdist) {
							hitpoint = target->obj->facelist[ii].vid[0];
							curdist = d;
						}
					}

					if(hitpoint >= 0) {
						color = (target->ioflags & IO_NPC) ? target->_npcdata->blood_color : Color::white;
						pos = target->obj->vertexlist3[hitpoint].v;
					}
					else ARX_DEAD_CODE(); 
					
					float dmgs = 0.f;
					if(!(flags & 1)) {
						Vec3f posi;

						if(hitpoint >= 0) {
							posi = target->obj->vertexlist3[hitpoint].v;
							dmgs = ARX_EQUIPMENT_ComputeDamages(io_source, target, ratioaim, &posi);
						} else {
							dmgs = ARX_EQUIPMENT_ComputeDamages(io_source, target, ratioaim);
						}

						if(target->ioflags & IO_NPC) {
							ret = true;
							target->spark_n_blood = 0;
							target->_npcdata->SPLAT_TOT_NB = 0;

							if(drain_life > 0.f) {
								float life_gain = min(dmgs, drain_life);
								life_gain = min(life_gain, target->_npcdata->life);
								life_gain = max(life_gain, 0.f);
								ARX_DAMAGES_HealInter(io_source, life_gain);
							}

							if(paralyse > 0.f) {
								float ptime = min(dmgs * 1000.f, paralyse);
								ARX_SPELLS_Launch(SPELL_PARALYSE, weapon, SPELLCAST_FLAG_NOMANA | SPELLCAST_FLAG_NOCHECKCANCAST
												  , 5, sphereContent[jj], (long)(ptime));
							}
						}

						if(io_source == entities.player())
							ARX_DAMAGES_DurabilityCheck(io_weapon, 0.2f);
					}

					if(dmgs > 0.f || ((target->ioflags & IO_NPC) && target->spark_n_blood == SP_BLOODY)) {
						if(target->ioflags & IO_NPC) {
							target->spark_n_blood = SP_BLOODY;

							if(!(flags & 1)) {
								ARX_PARTICLES_Spawn_Splat(pos, dmgs, color);

								EERIE_SPHERE sp;
								float power;
								power = (dmgs * ( 1.0f / 40 )) + 0.7f;
								Vec3f vect;
								vect.x = target->obj->vertexlist3[hitpoint].v.x - io_source->pos.x;
								vect.y = 0;
								vect.z = target->obj->vertexlist3[hitpoint].v.z - io_source->pos.z;
								fnormalize(vect);
								sp.origin.x = target->obj->vertexlist3[hitpoint].v.x + vect.x * 30.f;
								sp.origin.y = target->obj->vertexlist3[hitpoint].v.y;
								sp.origin.z = target->obj->vertexlist3[hitpoint].v.z + vect.z * 30.f;
								sp.radius = 3.5f * power * 20;

								if(CheckAnythingInSphere(&sp, 0, CAS_NO_NPC_COL)) {
									Color3f rgb = color.to<float>();
									SpawnGroundSplat(&sp, &rgb, 30, 1);
								}
							}

							ARX_PARTICLES_Spawn_Blood2(pos, dmgs, color, target);

							if(!ValidIONum(weapon))
								io_weapon = NULL;
						} else {
							if(target->ioflags & IO_ITEM)
								ARX_PARTICLES_Spawn_Spark(&pos, rnd() * 3.f, 0);
							else
								ARX_PARTICLES_Spawn_Spark(&pos, rnd() * 30.f, 0);

							ARX_NPC_SpawnAudibleSound(&pos, io_source);

							if(io_source == entities.player())
								HIT_SPARK = 1;
						}
					} else if((target->ioflags & IO_NPC) && (dmgs <= 0.f || target->spark_n_blood == SP_SPARKING)) {
						long nb;

						if(target->spark_n_blood == SP_SPARKING)
							nb = Random::get(0, 3);
						else
							nb = 30;

						if(target->ioflags & IO_ITEM)
							nb = 1;

						ARX_PARTICLES_Spawn_Spark(&pos, (float)nb, 0); 
						ARX_NPC_SpawnAudibleSound(&pos, io_source);
						target->spark_n_blood = SP_SPARKING;

						if(!(target->ioflags & IO_NPC))
							HIT_SPARK = 1;
					} else if(dmgs <= 0.f && ((target->ioflags & IO_FIX) || (target->ioflags & IO_ITEM))) {
						long  nb;

						if(target->spark_n_blood == SP_SPARKING)
							nb = Random::get(0, 3);
						else
							nb = 30;

						if(target->ioflags & IO_ITEM)
							nb = 1;

						ARX_PARTICLES_Spawn_Spark(&pos, (float)nb, 0);
						ARX_NPC_SpawnAudibleSound(&pos, io_source);
						target->spark_n_blood = SP_SPARKING;

						if (!(target->ioflags & IO_NPC))
							HIT_SPARK = 1;
					}

					if(HIT_SPARK) {
						if(!io_source->isHit) {
							ARX_DAMAGES_DurabilityCheck(io_weapon, 1.f);
							io_source->isHit = true;
							
							if(!ValidIONum(weapon)) {
								io_weapon = NULL;
							} else {
								string _weapon_material = "metal";
								const string * weapon_material = &_weapon_material;

								if(io_weapon && !io_weapon->weaponmaterial.empty()) {
									weapon_material = &io_weapon->weaponmaterial;
								}

								char bkg_material[128];

								if(ARX_MATERIAL_GetNameById(target->material, bkg_material))
									ARX_SOUND_PlayCollision(*weapon_material, bkg_material, 1.f, 1.f, &sphere.origin, NULL);
							}
						}
					}
				}
			}
		}

		EERIEPOLY * ep = CheckBackgroundInSphere(&sphere);
		if(ep) {
			if(io_source == entities.player()) {
				if(!io_source->isHit) {
					ARX_DAMAGES_DurabilityCheck(io_weapon, 1.f);
					io_source->isHit = true;

					if(!ValidIONum(weapon)) {
						io_weapon = NULL;
					} else {
						string _weapon_material = "metal";
						const string * weapon_material = &_weapon_material;
						if(io_weapon && !io_weapon->weaponmaterial.empty()) {
							weapon_material = &io_weapon->weaponmaterial;
						}

						std::string bkg_material = "earth";

						if(ep && ep->tex && !ep->tex->m_texName.empty())
							bkg_material = GetMaterialString(ep->tex->m_texName);

						ARX_SOUND_PlayCollision(*weapon_material, bkg_material, 1.f, 1.f, &sphere.origin, io_source);
					}
				}
			}

			ARX_PARTICLES_Spawn_Spark(&sphere.origin, rnd() * 10.f, 0);
			ARX_NPC_SpawnAudibleSound(&sphere.origin, io_source);
		}
	}

	return ret;
}

void ARX_EQUIPMENT_LaunchPlayerReadyWeapon()
{
	Entity *io = entities.player();
	if(!io)
		return;

	WeaponType type = ARX_EQUIPMENT_GetPlayerWeaponType();
	ANIM_HANDLE * anim = NULL;

	switch(type) {
		case WEAPON_DAGGER:
			anim = io->anims[ANIM_DAGGER_READY_PART_1];
			break;
		case WEAPON_1H:
			anim = io->anims[ANIM_1H_READY_PART_1];
			break;
		case WEAPON_2H:
			if(player.equiped[EQUIP_SLOT_SHIELD] == 0)
				anim = io->anims[ANIM_2H_READY_PART_1];

			break;
		case WEAPON_BOW:
			if(player.equiped[EQUIP_SLOT_SHIELD] == 0)
				anim = io->anims[ANIM_MISSILE_READY_PART_1];

			break;
		default:
			anim = io->anims[ANIM_BARE_READY];
			break;
	}

	AcquireLastAnim(io);
	ANIM_Set(&io->animlayer[1], anim);
}

void ARX_EQUIPMENT_UnEquipPlayerWeapon()
{
	if(player.equiped[EQUIP_SLOT_WEAPON] && ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
		Entity * pioOldDragInter;
		pioOldDragInter = DRAGINTER;
		DRAGINTER = entities[player.equiped[EQUIP_SLOT_WEAPON]];

		if(DRAGINTER)
			ARX_SOUND_PlayInterface(SND_INVSTD);

		ARX_EQUIPMENT_UnEquip(entities.player(), entities[player.equiped[EQUIP_SLOT_WEAPON]]);
		DRAGINTER = pioOldDragInter;
	}

	player.equiped[EQUIP_SLOT_WEAPON] = 0;
}

bool bRing = false;

void ARX_EQUIPMENT_Equip(Entity * target, Entity * toequip)
{
	if(!target || !toequip || target != entities.player())
		return;

	long validid = -1;

	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i] == toequip) {
			validid = i;
			break;
		}
	}

	if(validid == -1)
		return;

	RemoveFromAllInventories(toequip);
	toequip->show = SHOW_FLAG_ON_PLAYER; // on player

	if(toequip == DRAGINTER)
		Set_DragInter(NULL);

	if(toequip->type_flags & (OBJECT_TYPE_DAGGER | OBJECT_TYPE_1H | OBJECT_TYPE_2H |OBJECT_TYPE_BOW)) {
		if(player.equiped[EQUIP_SLOT_WEAPON] && ValidIONum(player.equiped[EQUIP_SLOT_WEAPON]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_WEAPON]]);

		player.equiped[EQUIP_SLOT_WEAPON] = (short)validid;

		if(toequip->type_flags & OBJECT_TYPE_BOW)
			EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "weapon_attach", "test", toequip);
		else
			EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "weapon_attach", "primary_attach", toequip);

		if((toequip->type_flags & (OBJECT_TYPE_2H | OBJECT_TYPE_BOW)) && player.equiped[EQUIP_SLOT_SHIELD])
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_SHIELD]]);
	} else if(toequip->type_flags & OBJECT_TYPE_SHIELD) {
		if(player.equiped[EQUIP_SLOT_SHIELD] && ValidIONum(player.equiped[EQUIP_SLOT_SHIELD]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_SHIELD]]);

		player.equiped[EQUIP_SLOT_SHIELD] = (short)validid;
		EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "shield_attach", "shield_attach", toequip);

		if(player.equiped[EQUIP_SLOT_WEAPON] && ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
			if(entities[player.equiped[EQUIP_SLOT_WEAPON]]->type_flags & (OBJECT_TYPE_2H | OBJECT_TYPE_BOW))
				ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_WEAPON]]);
		}
	} else if(toequip->type_flags & OBJECT_TYPE_RING) {
		// check first, if not already equiped
		if (!((ValidIONum(player.equiped[EQUIP_SLOT_RING_LEFT]) && (toequip == entities[player.equiped[EQUIP_SLOT_RING_LEFT]]))
		        ||	(ValidIONum(player.equiped[EQUIP_SLOT_RING_RIGHT]) && (toequip == entities[player.equiped[EQUIP_SLOT_RING_RIGHT]]))))
		{
			long willequip = -1;

			if(player.equiped[EQUIP_SLOT_RING_LEFT] == 0)
				willequip = EQUIP_SLOT_RING_LEFT;

			if(player.equiped[EQUIP_SLOT_RING_RIGHT] == 0)
				willequip = EQUIP_SLOT_RING_RIGHT;

			if(willequip == -1) {
				if(bRing) {
					if(ValidIONum(player.equiped[EQUIP_SLOT_RING_RIGHT]))
						ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_RING_RIGHT]]);

					willequip = EQUIP_SLOT_RING_RIGHT;
				} else {
					if(ValidIONum(player.equiped[EQUIP_SLOT_RING_LEFT]))
						ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_RING_LEFT]]);

					willequip = EQUIP_SLOT_RING_LEFT;
				}
				bRing = !bRing;
			}
			player.equiped[willequip] = (short)validid;
		}
	} else if(toequip->type_flags & OBJECT_TYPE_ARMOR) {
		if(player.equiped[EQUIP_SLOT_ARMOR] && ValidIONum(player.equiped[EQUIP_SLOT_ARMOR]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_ARMOR]]);

		player.equiped[EQUIP_SLOT_ARMOR] = (short)validid;
	} else if(toequip->type_flags & OBJECT_TYPE_LEGGINGS) {
		if(player.equiped[EQUIP_SLOT_LEGGINGS] && ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_LEGGINGS]]);

		player.equiped[EQUIP_SLOT_LEGGINGS] = (short)validid;
	} else if(toequip->type_flags & OBJECT_TYPE_HELMET) {
		if(player.equiped[EQUIP_SLOT_HELMET] && ValidIONum(player.equiped[EQUIP_SLOT_HELMET]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_HELMET]]);

		player.equiped[EQUIP_SLOT_HELMET] = (short)validid;
	}

	if(toequip->type_flags & (OBJECT_TYPE_HELMET | OBJECT_TYPE_ARMOR | OBJECT_TYPE_LEGGINGS))
		ARX_EQUIPMENT_RecreatePlayerMesh();

	ARX_PLAYER_ComputePlayerFullStats();
}

bool ARX_EQUIPMENT_SetObjectType(Entity & io, const string & temp, bool set) {
	
	ItemType flag = ARX_EQUIPMENT_GetObjectTypeFlag(temp);
	
	if(set) {
		io.type_flags |= flag;
	} else {
		io.type_flags &= ~flag;
	}
	
	return (flag != 0);
}

//! \brief Initializes Equipment infos
void ARX_EQUIPMENT_Init()
{
	// IO_EQUIPITEM_ELEMENT_... are Defined in EERIEPOLY.h
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_STRENGTH].name, "strength");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_DEXTERITY].name, "dexterity");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_CONSTITUTION].name, "constitution");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_MIND].name, "intelligence");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Stealth].name, "stealth");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Mecanism].name, "mecanism");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Intuition].name, "intuition");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Etheral_Link].name, "etheral_link");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Object_Knowledge].name, "object_knowledge");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Casting].name, "casting");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Projectile].name, "projectile");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Close_Combat].name, "close_combat");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Defense].name, "defense");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Armor_Class].name, "armor_class");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Resist_Magic].name, "resist_magic");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Resist_Poison].name, "resist_poison");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Critical_Hit].name, "critical_hit");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Damages].name, "damages");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Duration].name, "duration");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_AimTime].name, "aim_time");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Identify_Value].name, "identify_value");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Life].name, "life");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_Mana].name, "mana");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_MaxLife].name, "maxlife");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_MaxMana].name, "maxmana");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_1].name, "special1");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_2].name, "special2");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_3].name, "special3");
	strcpy(equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_4].name, "special4");
}

//! \brief Removes All special equipement properties
void ARX_EQUIPMENT_Remove_All_Special(Entity * io)
{
	if(!io || !(io->ioflags & IO_ITEM))
		return;

	io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_SPECIAL_1].special = IO_SPECIAL_ELEM_NONE;
	io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_SPECIAL_2].special = IO_SPECIAL_ELEM_NONE;
	io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_SPECIAL_3].special = IO_SPECIAL_ELEM_NONE;
	io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_SPECIAL_4].special = IO_SPECIAL_ELEM_NONE;
}

float getEquipmentBaseModifier(EquipmentModifierType modifier, bool getRelative) {
	
	float sum = 0;
	
	for(long i = 0; i < MAX_EQUIPED; i++) {
		if(player.equiped[i] && ValidIONum(player.equiped[i])) {
			Entity * toequip = entities[player.equiped[i]];
			if(toequip && (toequip->ioflags & IO_ITEM) && toequip->_itemdata->equipitem) {
				IO_EQUIPITEM_ELEMENT * elem = &toequip->_itemdata->equipitem->elements[modifier];
				bool isRelative = elem->flags.has(IO_ELEMENT_FLAG_PERCENT);
				if(isRelative == getRelative) {
					sum += elem->value;
				}
			}
		}
	}
	
	if(getRelative) {
		// Convert from percent to ratio
		sum *= 0.01f;
	}
	
	return sum;
}

float getEquipmentModifier(EquipmentModifierType modifier, float baseval) {
	float modabs = getEquipmentBaseModifier(modifier, false);
	float modrel = getEquipmentBaseModifier(modifier, true);
	return modabs + modrel * std::max(0.f, baseval + modabs);
}

void ARX_EQUIPMENT_SetEquip(Entity * io, bool special,
                            const std::string & param2, float val,
                            EquipmentModifierFlags flags) {
	
	if (io == NULL) return;

	if (!(io->ioflags & IO_ITEM)) return;

	if (!io->_itemdata->equipitem)
	{
		io->_itemdata->equipitem = (IO_EQUIPITEM *) malloc(sizeof(IO_EQUIPITEM));

		if (io->_itemdata->equipitem == NULL) return;

		memset(io->_itemdata->equipitem, 0, sizeof(IO_EQUIPITEM));
		io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Duration].value = 10;
		io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_AimTime].value = 10;
		io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Damages].value = 0;
		io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value = 0;
	}

	if(special) {
		for (long i = IO_EQUIPITEM_ELEMENT_SPECIAL_1; i <= IO_EQUIPITEM_ELEMENT_SPECIAL_4; i++)
		{
			if (io->_itemdata->equipitem->elements[i].special == IO_SPECIAL_ELEM_NONE)
			{
				if(param2 == "paralyse") {
					io->_itemdata->equipitem->elements[i].special = IO_SPECIAL_ELEM_PARALYZE;
				} else if(param2 == "drainlife") {
					io->_itemdata->equipitem->elements[i].special = IO_SPECIAL_ELEM_DRAIN_LIFE;
				}

				io->_itemdata->equipitem->elements[i].value = val;
				io->_itemdata->equipitem->elements[i].flags = flags;
				return;
			}
		}

		return;
		
	} else {
		for(long i = 0; i < IO_EQUIPITEM_ELEMENT_Number; i++) {
			if(param2 == equipinfo[i].name) {
				io->_itemdata->equipitem->elements[i].value = val;
				io->_itemdata->equipitem->elements[i].special = IO_SPECIAL_ELEM_NONE;
				io->_itemdata->equipitem->elements[i].flags = flags;
				return;
			}
		}
	}
}

void ARX_EQUIPMENT_IdentifyAll()
{
	Entity * io = entities.player();

	if (io == NULL) return;

	if (io != entities.player()) return;

	for (long i = 0; i < MAX_EQUIPED; i++)
	{
		if ((player.equiped[i] != 0)
		        &&	ValidIONum(player.equiped[i]))
		{
			Entity * toequip = entities[player.equiped[i]];

			if ((toequip) && (toequip->ioflags & IO_ITEM) && (toequip->_itemdata->equipitem))
			{
				if (player.Full_Skill_Object_Knowledge + player.Full_Attribute_Mind
				        >= toequip->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Identify_Value].value)
				{
					SendIOScriptEvent(toequip, SM_IDENTIFY);
				}
			}
		}
	}
}

float GetHitValue( const std::string & name) {
	
	if(boost::starts_with(name, "hit_")) {
		// Get the number after the first 4 characters in the string
		try {
			return float(boost::lexical_cast<long>(name.substr(4)));
		} catch(...) { /* ignore */ }
	}
	
	return -1;
}
