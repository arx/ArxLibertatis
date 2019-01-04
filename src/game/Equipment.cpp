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
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "animation/Animation.h"

#include "core/Core.h"

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
#include "graphics/effects/PolyBoom.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/Spark.h"

#include "io/resource/ResourcePath.h"

#include "math/Random.h"
#include "math/Vector.h"

#include "physics/Collisions.h"

#include "platform/Platform.h"
#include "platform/profiler/Profiler.h"

#include "scene/Object.h"
#include "scene/LinkedObject.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

#include "script/Script.h"

struct EQUIP_INFO {
	const char * name;
};

#define SP_SPARKING 1
#define SP_BLOODY 2

extern Vec3f PUSH_PLAYER_FORCE;
extern bool EXTERNALVIEW;

extern EERIE_3DOBJ * arrowobj;

EQUIP_INFO equipinfo[IO_EQUIPITEM_ELEMENT_Number];

//! \brief Returns the object type flag corresponding to a string
ItemType ARX_EQUIPMENT_GetObjectTypeFlag(const std::string & temp) {
	
	if(temp.empty()) {
		return 0;
	}
	
	char c = temp[0];
	
	arx_assert(std::tolower(c) == c);
	
	switch(c) {
		case 'w': return OBJECT_TYPE_WEAPON;
		case 'd': return OBJECT_TYPE_DAGGER;
		case '1': return OBJECT_TYPE_1H;
		case '2': return OBJECT_TYPE_2H;
		case 'b': return OBJECT_TYPE_BOW;
		case 's': return OBJECT_TYPE_SHIELD;
		case 'f': return OBJECT_TYPE_FOOD;
		case 'g': return OBJECT_TYPE_GOLD;
		case 'r': return OBJECT_TYPE_RING;
		case 'a': return OBJECT_TYPE_ARMOR;
		case 'h': return OBJECT_TYPE_HELMET;
		case 'l': return OBJECT_TYPE_LEGGINGS;
		default:  return 0;
	}
	
}

//! \brief Releases Equiped Id from player
static void ARX_EQUIPMENT_Release(EntityHandle id) {
	if(ValidIONum(id)) {
		for(size_t i = 0; i < MAX_EQUIPED; i++) {
			if(player.equiped[i] == id) {
				player.equiped[i] = EntityHandle();
			}
		}
	}
}

//! \brief Releases Equipment Structure
void ARX_EQUIPMENT_ReleaseAll(Entity * io) {
	
	if(!io || !(io->ioflags & IO_ITEM)) {
		return;
	}
	
	delete io->_itemdata->equipitem;
	io->_itemdata->equipitem = NULL;
	
}

extern long EXITING;

//! \brief Recreates player mesh from scratch
static void applyTweak(EquipmentSlot equip, TweakType tw, const std::string & selection) {
	
	if(!ValidIONum(player.equiped[equip])) {
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
	
	ObjSelection sel = ObjSelection();
	for(size_t i = 0; i < io->obj->selections.size(); i++) {
		if(io->obj->selections[i].name == selection) {
			sel = ObjSelection(i);
			break;
		}
	}
	if(sel == ObjSelection()) {
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
		EERIE_FACE & face = io->obj->facelist[i];

		if(   IsInSelection(io->obj, face.vid[0], sel)
		   && IsInSelection(io->obj, face.vid[1], sel)
		   && IsInSelection(io->obj, face.vid[2], sel)
		) {
			if(face.texid == textochange) {
				face.texid = short(mapidx);
			}
		}
	}
	
}

void ARX_EQUIPMENT_RecreatePlayerMesh() {
	
	if(EXITING)
		return;
	
	arx_assert(entities.player());
	Entity * io = entities.player();
	
	if(io->obj != hero)
		delete io->obj;
	
	io->obj = loadObject("graph/obj3d/interactive/npc/human_base/human_base.teo", false);
	
	applyTweak(EQUIP_SLOT_HELMET, TWEAK_HEAD, "head");
	applyTweak(EQUIP_SLOT_ARMOR, TWEAK_TORSO, "chest");
	applyTweak(EQUIP_SLOT_LEGGINGS, TWEAK_LEGS, "leggings");
	
	Entity * target = entities.player();
	
	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(ValidIONum(player.equiped[i])) {
			Entity * toequip = entities[player.equiped[i]];
			if(toequip) {
				if(toequip->type_flags & (OBJECT_TYPE_DAGGER | OBJECT_TYPE_1H | OBJECT_TYPE_2H | OBJECT_TYPE_BOW)) {
					if(player.Interface & INTER_COMBATMODE) {
						ARX_EQUIPMENT_AttachPlayerWeaponToHand();
					} else {
						EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "weapon_attach", "primary_attach", toequip);
					}
				} else if(toequip->type_flags & OBJECT_TYPE_SHIELD) {
					if(ValidIONum(player.equiped[EQUIP_SLOT_SHIELD])) {
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
	
	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(ValidIONum(player.equiped[i])) {
			ARX_EQUIPMENT_UnEquip(entities.player(), entities[player.equiped[i]]);
		}
	}

	ARX_PLAYER_ComputePlayerFullStats();
}

bool ARX_EQUIPMENT_IsPlayerEquip(Entity * _pIO) {
	arx_assert(entities.player());
	
	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(ValidIONum(player.equiped[i])) {
			Entity * toequip = entities[player.equiped[i]];

			if(toequip == _pIO) {
				return true;
			}
		}
	}

	return false;
}

// flags & 1 == destroyed !
void ARX_EQUIPMENT_UnEquip(Entity * target, Entity * tounequip, long flags)
{
	if(!target || !tounequip)
		return;

	if(target != entities.player())
		return;

	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(ValidIONum(player.equiped[i]) && entities[player.equiped[i]] == tounequip) {
			EERIE_LINKEDOBJ_UnLinkObjectFromObject(target->obj, tounequip->obj);
			ARX_EQUIPMENT_Release(player.equiped[i]);
			target->bbox2D.min.x = 9999;
			target->bbox2D.max.x = -9999;
			
			if(!flags) {
				if(!DRAGINTER) {
					ARX_SOUND_PlayInterface(g_snd.INVSTD);
					Set_DragInter(tounequip);
				} else {
					giveToPlayer(tounequip);
				}
			}
			
			SendIOScriptEvent(tounequip, entities.player(), SM_EQUIPOUT);
			SendIOScriptEvent(entities.player(), tounequip, SM_EQUIPOUT);
			
		}
	}

	if(tounequip->type_flags & (OBJECT_TYPE_HELMET | OBJECT_TYPE_ARMOR | OBJECT_TYPE_LEGGINGS))
		ARX_EQUIPMENT_RecreatePlayerMesh();
}

void ARX_EQUIPMENT_AttachPlayerWeaponToHand() {
	
	arx_assert(entities.player());
	Entity * target = entities.player();
	
	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(ValidIONum(player.equiped[i])) {
			Entity * toequip = entities[player.equiped[i]];
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

void ARX_EQUIPMENT_AttachPlayerWeaponToBack() {
	
	arx_assert(entities.player());
	Entity * target = entities.player();
	
	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(ValidIONum(player.equiped[i])) {
			Entity * toequip = entities[player.equiped[i]];
			if(toequip) {
				if((toequip->type_flags & OBJECT_TYPE_DAGGER) || (toequip->type_flags & OBJECT_TYPE_1H)
				   || (toequip->type_flags & OBJECT_TYPE_2H) || (toequip->type_flags & OBJECT_TYPE_BOW)) {
					
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

WeaponType ARX_EQUIPMENT_GetPlayerWeaponType() {
	arx_assert(entities.player());
	
	if(ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
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

void ARX_EQUIPMENT_LaunchPlayerUnReadyWeapon() {
	arx_assert(entities.player());
	arx_assert(arrowobj);
	
	Entity * io = entities.player();
	
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
			EERIE_LINKEDOBJ_UnLinkObjectFromObject(io->obj, arrowobj);
			break;
		default:
			anim = io->anims[ANIM_BARE_UNREADY];
			break;
	}

	changeAnimation(io, 1, anim);
}

float ARX_EQUIPMENT_ComputeDamages(Entity * io_source, Entity * io_target, float ratioaim, Vec3f * position) {
	
	SendIOScriptEvent(io_source, io_target, SM_AGGRESSION);
	
	if(!io_source || !io_target) {
		return 0.f;
	}
	
	if(!(io_target->ioflags & IO_NPC)) {
		if(io_target->ioflags & IO_FIX) {
			if(io_source == entities.player()) {
				ARX_DAMAGES_DamageFIX(io_target, player.m_miscFull.damages, EntityHandle_Player, false);
			} else if(io_source->ioflags & IO_NPC) {
				ARX_DAMAGES_DamageFIX(io_target, io_source->_npcdata->damages, io_source->index(), false);
			} else {
				ARX_DAMAGES_DamageFIX(io_target, 1, io_source->index(), false);
			}
		}
		return 0.f;
	}

	float attack, ac, damages;
	float backstab = 1.f;

	std::string _wmat = "bare";
	const std::string * wmat = &_wmat;
	
	std::string _amat = "flesh";
	const std::string * amat = &_amat;

	bool critical = false;

	if(io_source == entities.player()) {
		
		if(ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
			Entity * io = entities[player.equiped[EQUIP_SLOT_WEAPON]];
			if(io && !io->weaponmaterial.empty()) {
				wmat = &io->weaponmaterial;
			}
		}
		
		attack = player.m_miscFull.damages;
		
		if(Random::getf(0.f, 100.f) <= player.m_miscFull.criticalHit
		   && SendIOScriptEvent(io_source, io_source, SM_CRITICAL) != REFUSE) {
			critical = true;
		}
		
		damages = attack * ratioaim;
		
		if(io_target->_npcdata->npcflags & NPCFLAG_BACKSTAB) {
			if(Random::getf(0.f, 100.f) <= player.m_skillFull.stealth * 0.5f) {
				if(SendIOScriptEvent(io_source, io_source, SM_BACKSTAB) != REFUSE)
					backstab = 1.5f;
			}
		}
		
	} else {
		
		if(!(io_source->ioflags & IO_NPC)) {
			// no NPC source...
			return 0.f;
		}
		
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
		
		damages = io_source->_npcdata->damages * ratioaim * Random::getf(0.5f, 1.0f);
		
		SpellBase * spell = spells.getSpellOnTarget(io_source->index(), SPELL_CURSE);
		if(spell) {
			damages *= (1 - spell->m_level * 0.05f);
		}
		
		if(Random::getf(0.f, 100) <= io_source->_npcdata->critical
		   && SendIOScriptEvent(io_source, io_source, SM_CRITICAL) != REFUSE) {
			critical = true;
		}
		
		if(Random::getf(0.f, 100.f) <= io_source->_npcdata->backstab_skill) {
			if(SendIOScriptEvent(io_source, io_source, SM_BACKSTAB) != REFUSE)
				backstab = 1.5f;
		}
		
	}
	
	float absorb;

	if(io_target == entities.player()) {
		ac = player.m_miscFull.armorClass;
		absorb = player.m_skillFull.defense * 0.5f;
	} else {
		ac = ARX_INTERACTIVE_GetArmorClass(io_target);
		absorb = io_target->_npcdata->absorb;
		
		SpellBase * spell = spells.getSpellOnTarget(io_target->index(), SPELL_CURSE);
		if(spell) {
			float modif = (1 - spell->m_level * 0.05f);
			ac *= modif;
			absorb *= modif;
		}
	}
	
	if(!io_target->armormaterial.empty()) {
		amat = &io_target->armormaterial;
	}
	
	if(io_target == entities.player()) {
		if(ValidIONum(player.equiped[EQUIP_SLOT_ARMOR])) {
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
	
	ARX_SOUND_PlayCollision(*amat, *wmat, power, 1.f, pos, io_source);
	
	float chance = 100.f - (ac - attack);
	if(Random::getf(0.f, 100.f) > chance) {
		return 0.f;
	}
	
	ARX_SOUND_PlayCollision("flesh", *wmat, power, 1.f, pos, io_source);
	
	if(dmgs > 0.f) {
		
		if(critical) {
			dmgs *= 1.5f;
		}
		
		if(io_target == entities.player()) {
			
			// TODO should this be player.pos - player.baseOffset() = player.basePosition()?
			Vec3f ppos = io_source->pos - (player.pos + player.baseOffset());
			ppos = glm::normalize(ppos);
			
			// Push the player
			PUSH_PLAYER_FORCE += ppos * -dmgs * Vec3f(1.0f / 11, 1.0f / 30, 1.0f / 11);
			
			ARX_DAMAGES_DamagePlayer(dmgs, 0, io_source->index());
			ARX_DAMAGES_DamagePlayerEquipment(dmgs);
			
		} else {
			
			Vec3f ppos = io_source->pos - io_target->pos;
			ppos = glm::normalize(ppos);
			
			// Push the NPC
			io_target->forcedmove += ppos * -dmgs;
			
			Vec3f * targetPosition = position ? position : &io_target->pos;
			ARX_DAMAGES_DamageNPC(io_target, dmgs, io_source->index(), false, targetPosition);
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

// flags & 1 = blood spawn only
bool ARX_EQUIPMENT_Strike_Check(Entity * io_source, Entity * io_weapon, float ratioaim, long flags, EntityHandle targ) {
	
	ARX_PROFILE_FUNC();
	
	arx_assert(io_source);
	arx_assert(io_weapon);
	
	bool ret = false;
	EntityHandle source = io_source->index();
	EntityHandle weapon = io_weapon->index();
	
	EXCEPTIONS_LIST_Pos = 0;

	float drain_life = ARX_EQUIPMENT_GetSpecialValue(io_weapon, IO_SPECIAL_ELEM_DRAIN_LIFE);
	float paralyse = ARX_EQUIPMENT_GetSpecialValue(io_weapon, IO_SPECIAL_ELEM_PARALYZE);

	BOOST_FOREACH(const EERIE_ACTIONLIST & action, io_weapon->obj->actionlist) {
		
		float rad = GetHitValue(action.name);

		if(rad == -1)
			continue;
		
		Sphere sphere;
		sphere.origin = actionPointPosition(io_weapon->obj, action.idx);
		sphere.radius = rad;
		
		if(source != EntityHandle_Player)
			sphere.radius += 15.f;

		std::vector<EntityHandle> sphereContent;

		if(CheckEverythingInSphere(sphere, source, targ, sphereContent)) {
			BOOST_FOREACH(const EntityHandle & content, sphereContent) {
				if(ValidIONum(content) && !(entities[content]->ioflags & IO_BODY_CHUNK)) {
					
					bool HIT_SPARK = false;
					EXCEPTIONS_LIST[EXCEPTIONS_LIST_Pos] = content;
					EXCEPTIONS_LIST_Pos++;

					if(EXCEPTIONS_LIST_Pos >= MAX_IN_SPHERE)
						EXCEPTIONS_LIST_Pos--;
					
					Entity * target = entities[content];
					
					long hitpoint = -1;
					float curdist = 999999.f;
					
					Vec3f vector = (sphere.origin - target->pos) * Vec3f(1.f, 0.5f, 1.f);
					vector = glm::normalize(vector);

					for(size_t ii = 0; ii < target->obj->facelist.size(); ii++) {
						if(target->obj->facelist[ii].facetype & POLY_HIDE)
							continue;

						float d = glm::distance(sphere.origin, target->obj->vertexWorldPositions[target->obj->facelist[ii].vid[0]].v);

						if(d < curdist) {
							hitpoint = target->obj->facelist[ii].vid[0];
							curdist = d;
						}
					}
					
					arx_assert(hitpoint >= 0);
					Color color = (target->ioflags & IO_NPC) ? target->_npcdata->blood_color : Color::white;
					Vec3f pos = target->obj->vertexWorldPositions[hitpoint].v;
					
					float dmgs = 0.f;
					if(!(flags & 1)) {
						
						if(hitpoint >= 0) {
							Vec3f posi = target->obj->vertexWorldPositions[hitpoint].v;
							dmgs = ARX_EQUIPMENT_ComputeDamages(io_source, target, ratioaim, &posi);
						} else {
							dmgs = ARX_EQUIPMENT_ComputeDamages(io_source, target, ratioaim);
						}
						
						if(target->ioflags & IO_NPC) {
							ret = true;
							target->spark_n_blood = 0;
							target->_npcdata->SPLAT_TOT_NB = 0;

							if(drain_life > 0.f) {
								float life_gain = std::min(dmgs, drain_life);
								life_gain = std::min(life_gain, target->_npcdata->lifePool.current);
								life_gain = std::max(life_gain, 0.f);
								ARX_DAMAGES_HealInter(io_source, life_gain);
							}

							if(paralyse > 0.f) {
								GameDuration ptime = GameDurationMsf(std::min(dmgs * 1000.f, paralyse));
								ARX_SPELLS_Launch(SPELL_PARALYSE,
								                  weapon,
								                  SPELLCAST_FLAG_NOMANA | SPELLCAST_FLAG_NOCHECKCANCAST,
								                  5,
								                  content,
								                  ptime);
							}
						}

						if(io_source == entities.player()) {
							ARX_DAMAGES_DurabilityCheck(io_weapon, g_framedelay * 0.006f);
						}
					}
					
					if((target->ioflags & IO_NPC) && (dmgs > 0.f || target->spark_n_blood == SP_BLOODY)) {
						target->spark_n_blood = SP_BLOODY;
						
						if(!(flags & 1)) {
							ARX_PARTICLES_Spawn_Splat(pos, dmgs, color);
							
							Vec3f vertPos = target->obj->vertexWorldPositions[hitpoint].v;
							
							float power = (dmgs * 0.025f) + 0.7f;
							
							Vec3f vect(vertPos.x - io_source->pos.x, 0.f, vertPos.z - io_source->pos.z);
							vect = glm::normalize(vect);
							
							Sphere sp;
							sp.origin.x = vertPos.x + vect.x * 30.f;
							sp.origin.y = vertPos.y;
							sp.origin.z = vertPos.z + vect.z * 30.f;
							sp.radius = 3.5f * power * 20;
							
							if(CheckAnythingInSphere(sp, EntityHandle_Player, CAS_NO_NPC_COL)) {
								Sphere splatSphere;
								splatSphere.origin = sp.origin;
								splatSphere.radius = 30.f;
								PolyBoomAddSplat(splatSphere, Color3f(color), 1);
							}
						}
						
						ARX_PARTICLES_Spawn_Blood2(pos, dmgs, color, target);
					} else if(!(target->ioflags & IO_NPC) && dmgs > 0.f) {
						if(target->ioflags & IO_ITEM)
							ParticleSparkSpawnContinous(pos, Random::getu(0, 3), SpawnSparkType_Default);
						else
							ParticleSparkSpawnContinous(pos, Random::getu(0, 30), SpawnSparkType_Default);
						
						ARX_NPC_SpawnAudibleSound(pos, io_source);
						
						if(io_source == entities.player())
							HIT_SPARK = true;
					} else if(target->ioflags & IO_NPC) {
						unsigned int nb;

						if(target->spark_n_blood == SP_SPARKING)
							nb = Random::getu(0, 3);
						else
							nb = 30;

						if(target->ioflags & IO_ITEM)
							nb = 1;

						ParticleSparkSpawnContinous(pos, nb, SpawnSparkType_Default);
						ARX_NPC_SpawnAudibleSound(pos, io_source);
						target->spark_n_blood = SP_SPARKING;

						if(!(target->ioflags & IO_NPC))
							HIT_SPARK = true;
					} else if((target->ioflags & IO_FIX) || (target->ioflags & IO_ITEM)) {
						unsigned int nb;

						if(target->spark_n_blood == SP_SPARKING)
							nb = Random::getu(0, 3);
						else
							nb = 30;

						if(target->ioflags & IO_ITEM)
							nb = 1;

						ParticleSparkSpawnContinous(pos, nb, SpawnSparkType_Default);
						ARX_NPC_SpawnAudibleSound(pos, io_source);
						target->spark_n_blood = SP_SPARKING;

						if (!(target->ioflags & IO_NPC))
							HIT_SPARK = true;
					}

					if(HIT_SPARK) {
						if(!io_source->isHit) {
							ARX_DAMAGES_DurabilityCheck(io_weapon, 1.f);
							io_source->isHit = true;
							
							std::string _weapon_material = "metal";
							const std::string * weapon_material = &_weapon_material;
							
							if(!io_weapon->weaponmaterial.empty()) {
								weapon_material = &io_weapon->weaponmaterial;
							}
							
							if(target->material != MATERIAL_NONE) {
								const char * matStr = ARX_MATERIAL_GetNameById(target->material);
								ARX_SOUND_PlayCollision(*weapon_material, matStr, 1.f, 1.f, sphere.origin, NULL);
							}
						}
					}
				}
			}
		}

		const EERIEPOLY * ep = CheckBackgroundInSphere(sphere);
		if(ep) {
			if(io_source == entities.player()) {
				if(!io_source->isHit) {
					ARX_DAMAGES_DurabilityCheck(io_weapon, 1.f);
					io_source->isHit = true;
					
					std::string _weapon_material = "metal";
					const std::string * weapon_material = &_weapon_material;
					if(!io_weapon->weaponmaterial.empty()) {
						weapon_material = &io_weapon->weaponmaterial;
					}
					
					std::string bkg_material = "earth";
					
					if(ep && ep->tex && !ep->tex->m_texName.empty())
						bkg_material = GetMaterialString(ep->tex->m_texName);
					
					ARX_SOUND_PlayCollision(*weapon_material, bkg_material, 1.f, 1.f, sphere.origin, io_source);
				}
			}

			ParticleSparkSpawnContinous(sphere.origin, Random::getu(0, 10), SpawnSparkType_Default);
			ARX_NPC_SpawnAudibleSound(sphere.origin, io_source);
		}
	}

	return ret;
}

void ARX_EQUIPMENT_LaunchPlayerReadyWeapon() {
	
	arx_assert(entities.player());
	Entity * io = entities.player();
	
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
			if(!ValidIONum(player.equiped[EQUIP_SLOT_SHIELD]))
				anim = io->anims[ANIM_2H_READY_PART_1];

			break;
		case WEAPON_BOW:
			if(!ValidIONum(player.equiped[EQUIP_SLOT_SHIELD]))
				anim = io->anims[ANIM_MISSILE_READY_PART_1];

			break;
		default:
			anim = io->anims[ANIM_BARE_READY];
			break;
	}

	changeAnimation(io, 1, anim);
}

void ARX_EQUIPMENT_UnEquipPlayerWeapon()
{
	if(ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
		Entity * pioOldDragInter = DRAGINTER;
		DRAGINTER = entities[player.equiped[EQUIP_SLOT_WEAPON]];

		if(DRAGINTER)
			ARX_SOUND_PlayInterface(g_snd.INVSTD);

		ARX_EQUIPMENT_UnEquip(entities.player(), entities[player.equiped[EQUIP_SLOT_WEAPON]]);
		DRAGINTER = pioOldDragInter;
	}

	player.equiped[EQUIP_SLOT_WEAPON] = EntityHandle();
}

bool bRing = false;

void ARX_EQUIPMENT_Equip(Entity * target, Entity * toequip)
{
	if(!target || !toequip || target != entities.player())
		return;

	EntityHandle validid = EntityHandle();

	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e == toequip) {
			validid = handle;
			break;
		}
	}

	if(validid == EntityHandle())
		return;

	RemoveFromAllInventories(toequip);
	toequip->show = SHOW_FLAG_ON_PLAYER; // on player

	if(toequip == DRAGINTER)
		Set_DragInter(NULL);
	
	if(toequip->type_flags & (OBJECT_TYPE_DAGGER | OBJECT_TYPE_1H | OBJECT_TYPE_2H | OBJECT_TYPE_BOW)) {
		
		if(ValidIONum(player.equiped[EQUIP_SLOT_WEAPON]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_WEAPON]]);

		player.equiped[EQUIP_SLOT_WEAPON] = validid;

		if(toequip->type_flags & OBJECT_TYPE_BOW)
			EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "weapon_attach", "test", toequip);
		else
			EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "weapon_attach", "primary_attach", toequip);

		if((toequip->type_flags & (OBJECT_TYPE_2H | OBJECT_TYPE_BOW)) && ValidIONum(player.equiped[EQUIP_SLOT_SHIELD]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_SHIELD]]);
	} else if(toequip->type_flags & OBJECT_TYPE_SHIELD) {
		if(ValidIONum(player.equiped[EQUIP_SLOT_SHIELD]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_SHIELD]]);

		player.equiped[EQUIP_SLOT_SHIELD] = validid;
		EERIE_LINKEDOBJ_LinkObjectToObject(target->obj, toequip->obj, "shield_attach", "shield_attach", toequip);

		if(ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
			if(entities[player.equiped[EQUIP_SLOT_WEAPON]]->type_flags & (OBJECT_TYPE_2H | OBJECT_TYPE_BOW))
				ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_WEAPON]]);
		}
	} else if(toequip->type_flags & OBJECT_TYPE_RING) {
		// check first, if not already equiped
		if (!((ValidIONum(player.equiped[EQUIP_SLOT_RING_LEFT])
		       && (toequip == entities[player.equiped[EQUIP_SLOT_RING_LEFT]]))
		      || (ValidIONum(player.equiped[EQUIP_SLOT_RING_RIGHT])
		          && (toequip == entities[player.equiped[EQUIP_SLOT_RING_RIGHT]])))) {
			
			long willequip = -1;
			
			if(player.equiped[EQUIP_SLOT_RING_LEFT] == EntityHandle())
				willequip = EQUIP_SLOT_RING_LEFT;

			if(player.equiped[EQUIP_SLOT_RING_RIGHT] == EntityHandle())
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
			player.equiped[willequip] = validid;
		}
	} else if(toequip->type_flags & OBJECT_TYPE_ARMOR) {
		if(ValidIONum(player.equiped[EQUIP_SLOT_ARMOR]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_ARMOR]]);

		player.equiped[EQUIP_SLOT_ARMOR] = validid;
	} else if(toequip->type_flags & OBJECT_TYPE_LEGGINGS) {
		if(ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_LEGGINGS]]);

		player.equiped[EQUIP_SLOT_LEGGINGS] = validid;
	} else if(toequip->type_flags & OBJECT_TYPE_HELMET) {
		if(ValidIONum(player.equiped[EQUIP_SLOT_HELMET]))
			ARX_EQUIPMENT_UnEquip(target, entities[player.equiped[EQUIP_SLOT_HELMET]]);

		player.equiped[EQUIP_SLOT_HELMET] = validid;
	}

	if(toequip->type_flags & (OBJECT_TYPE_HELMET | OBJECT_TYPE_ARMOR | OBJECT_TYPE_LEGGINGS))
		ARX_EQUIPMENT_RecreatePlayerMesh();

	ARX_PLAYER_ComputePlayerFullStats();
}

bool ARX_EQUIPMENT_SetObjectType(Entity & io, const std::string & temp, bool set) {
	
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
	equipinfo[IO_EQUIPITEM_ELEMENT_STRENGTH].name = "strength";
	equipinfo[IO_EQUIPITEM_ELEMENT_DEXTERITY].name = "dexterity";
	equipinfo[IO_EQUIPITEM_ELEMENT_CONSTITUTION].name = "constitution";
	equipinfo[IO_EQUIPITEM_ELEMENT_MIND].name = "intelligence";
	equipinfo[IO_EQUIPITEM_ELEMENT_Stealth].name = "stealth";
	equipinfo[IO_EQUIPITEM_ELEMENT_Mecanism].name = "mecanism";
	equipinfo[IO_EQUIPITEM_ELEMENT_Intuition].name = "intuition";
	equipinfo[IO_EQUIPITEM_ELEMENT_Etheral_Link].name = "etheral_link";
	equipinfo[IO_EQUIPITEM_ELEMENT_Object_Knowledge].name = "object_knowledge";
	equipinfo[IO_EQUIPITEM_ELEMENT_Casting].name = "casting";
	equipinfo[IO_EQUIPITEM_ELEMENT_Projectile].name = "projectile";
	equipinfo[IO_EQUIPITEM_ELEMENT_Close_Combat].name = "close_combat";
	equipinfo[IO_EQUIPITEM_ELEMENT_Defense].name = "defense";
	equipinfo[IO_EQUIPITEM_ELEMENT_Armor_Class].name = "armor_class";
	equipinfo[IO_EQUIPITEM_ELEMENT_Resist_Magic].name = "resist_magic";
	equipinfo[IO_EQUIPITEM_ELEMENT_Resist_Poison].name = "resist_poison";
	equipinfo[IO_EQUIPITEM_ELEMENT_Critical_Hit].name = "critical_hit";
	equipinfo[IO_EQUIPITEM_ELEMENT_Damages].name = "damages";
	equipinfo[IO_EQUIPITEM_ELEMENT_Duration].name = "duration";
	equipinfo[IO_EQUIPITEM_ELEMENT_AimTime].name = "aim_time";
	equipinfo[IO_EQUIPITEM_ELEMENT_Identify_Value].name = "identify_value";
	equipinfo[IO_EQUIPITEM_ELEMENT_Life].name = "life";
	equipinfo[IO_EQUIPITEM_ELEMENT_Mana].name = "mana";
	equipinfo[IO_EQUIPITEM_ELEMENT_MaxLife].name = "maxlife";
	equipinfo[IO_EQUIPITEM_ELEMENT_MaxMana].name = "maxmana";
	equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_1].name = "special1";
	equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_2].name = "special2";
	equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_3].name = "special3";
	equipinfo[IO_EQUIPITEM_ELEMENT_SPECIAL_4].name = "special4";
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
	
	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(ValidIONum(player.equiped[i])) {
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
                            const std::string & modifierName, float val,
                            EquipmentModifierFlags flags) {
	
	if(io == NULL) {
		return;
	}

	if(!(io->ioflags & IO_ITEM)) {
		return;
	}
	
	if(!io->_itemdata->equipitem) {
		io->_itemdata->equipitem = new IO_EQUIPITEM;
		io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_Duration].value = 10;
		io->_itemdata->equipitem->elements[IO_EQUIPITEM_ELEMENT_AimTime].value = 10;
	}
	
	if(special) {
		for(long i = IO_EQUIPITEM_ELEMENT_SPECIAL_1; i <= IO_EQUIPITEM_ELEMENT_SPECIAL_4; i++) {
			if(io->_itemdata->equipitem->elements[i].special == IO_SPECIAL_ELEM_NONE) {
				if(modifierName == "paralyse") {
					io->_itemdata->equipitem->elements[i].special = IO_SPECIAL_ELEM_PARALYZE;
				} else if(modifierName == "drainlife") {
					io->_itemdata->equipitem->elements[i].special = IO_SPECIAL_ELEM_DRAIN_LIFE;
				}
				
				io->_itemdata->equipitem->elements[i].value = val;
				io->_itemdata->equipitem->elements[i].flags = flags;
				return;
			}
		}
		return;
	}
	
	for(long i = 0; i < IO_EQUIPITEM_ELEMENT_Number; i++) {
		if(modifierName == equipinfo[i].name) {
			io->_itemdata->equipitem->elements[i].value = val;
			io->_itemdata->equipitem->elements[i].special = IO_SPECIAL_ELEM_NONE;
			io->_itemdata->equipitem->elements[i].flags = flags;
			return;
		}
	}
}

void ARX_EQUIPMENT_IdentifyAll() {
	arx_assert(entities.player());
	
	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		if(ValidIONum(player.equiped[i])) {
			Entity * toequip = entities[player.equiped[i]];
			
			ARX_INVENTORY_IdentifyIO(toequip);
		}
	}
}

float GetHitValue(const std::string & name) {
	
	if(boost::starts_with(name, "hit_")) {
		// Get the number after the first 4 characters in the string
		try {
			return float(boost::lexical_cast<long>(name.substr(4)));
		} catch(...) { /* ignore */ }
	}
	
	return -1;
}
