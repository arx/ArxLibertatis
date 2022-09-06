/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/Damage.h"

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include <sstream>

#include "ai/Paths.h"

#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Camera.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/Inventory.h"
#include "game/Item.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/effect/Quake.h"
#include "game/npc/Dismemberment.h"

#include "gui/Dragging.h"
#include "gui/Speech.h"
#include "gui/Hud.h"
#include "gui/Interface.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleTextures.h"

#include "io/resource/ResourcePath.h"

#include "math/Random.h"
#include "math/RandomVector.h"

#include "physics/Collisions.h"
#include "platform/profiler/Profiler.h"

#include "scene/GameSound.h"
#include "scene/Light.h"
#include "scene/Interactive.h"

#include "script/Script.h"

class TextureContainer;

struct DAMAGE_INFO {
	
	bool exist;
	GameInstant start_time;
	GameInstant lastupd;
	
	DamageParameters params;
	Spell * spell;
	
	DAMAGE_INFO()
		: exist(false)
	{ }
	
};

static std::vector<DAMAGE_INFO> g_damages;

void damageClearSpell(Spell * spell) {
	for(DAMAGE_INFO & damage : g_damages) {
		if(damage.spell == spell) {
			damage.spell = nullptr;
			damage.params.type |= DAMAGE_TYPE_FAKESPELL;
		}
	}
}

DamageHandle DamageCreate(Spell * spell, const DamageParameters & params) {
	
	size_t i = std::find_if(g_damages.begin(), g_damages.end(), [](const DAMAGE_INFO & damage) {
		return !damage.exist;
	}) - g_damages.begin();
	if(i == g_damages.size()) {
		g_damages.emplace_back();
	}
	
	DAMAGE_INFO & damage = g_damages[i];
	damage.spell = spell;
	damage.params = params;
	damage.start_time = g_gameTime.now();
	damage.lastupd = 0;
	damage.exist = true;
	
	return DamageHandle(i);
}

DamageParameters & damageGet(Spell * spell, DamageHandle & handle) {
	
	if(handle.handleData() >= 0 && size_t(handle.handleData()) < g_damages.size()) {
		g_damages[handle.handleData()].spell = spell;
		return g_damages[handle.handleData()].params;
	}
	
	handle = DamageCreate(spell, DamageParameters());
	return g_damages[handle.handleData()].params;
}

void DamageRequestEnd(DamageHandle handle) {
	if(handle.handleData() >= 0 && size_t(handle.handleData()) < g_damages.size()) {
		g_damages[handle.handleData()].exist = false;
		while(!g_damages.empty() && !g_damages.back().exist) {
			g_damages.pop_back();
		}
	}
}

extern Vec3f PUSH_PLAYER_FORCE;

static void igniteEntity(Entity & entity, Entity * source, float dmg) {
	
	if(entity.ioflags & IO_INVULNERABILITY) {
		return;
	}
	
	if(entity.ignition <= 0.f && entity.ignition + dmg > 1.f) {
		SendIOScriptEvent(source, &entity, SM_ENTERZONE, "cook_s");
	}
	
	if(entity.ioflags & IO_FIX) {
		entity.ignition += dmg * 0.1f;
	} else if(entity.ioflags & IO_ITEM) {
		entity.ignition += dmg * 0.125f;
	} else if(entity.ioflags & IO_NPC) {
		entity.ignition += dmg * 0.25f;
	}
	
}

class ScreenFxBloodSplash {
	
	GameDuration Blood_Duration;
	GameDuration duration;
	float Blood_Pos;
	float Last_Blood_Pos;
	
public:
	
	ScreenFxBloodSplash()
		: Blood_Pos(0.f)
		, Last_Blood_Pos(0.f)
	{ }
	
	void reset();
	void hit(float strength);
	void render();
	
};

static ScreenFxBloodSplash g_screenFxBloodSplash;

void ScreenFxBloodSplash::reset() {
	Blood_Pos = 0.f;
	Blood_Duration = 0;
}

void ScreenFxBloodSplash::hit(float strength) {
	if(Blood_Pos == 0.f) {
		Blood_Pos = 0.000001f;
		Blood_Duration = 100ms + 200ms * strength;
	} else {
		Blood_Duration += 800ms * strength;
	}
}

void ScreenFxBloodSplash::render() {
	
	Color color;
	if(Blood_Pos > 2.f) { // end of blood flash
		Blood_Pos = 0.f;
		duration = 0;
	} else if(Blood_Pos > 1.f) {
		
		if(player.poison > 1.f)
			color = Color::rgb(Blood_Pos - 1.f, 1.f, Blood_Pos - 1.f);
		else
			color = Color::rgb(1.f, Blood_Pos - 1.f, Blood_Pos - 1.f);
		
		UseRenderState state(render2D().blend(BlendZero, BlendSrcColor));
		EERIEDrawBitmap(Rectf(g_size), 0.00009f, nullptr, color);
	} else if(Blood_Pos > 0.f) {
		
		if(player.poison > 1.f)
			color = Color::rgb(1.f - Blood_Pos, 1.f, 1.f - Blood_Pos);
		else
			color = Color::rgb(1.f, 1.f - Blood_Pos, 1.f - Blood_Pos);
		
		UseRenderState state(render2D().blend(BlendZero, BlendSrcColor));
		EERIEDrawBitmap(Rectf(g_size), 0.00009f, nullptr, color);
	}
	
	if(Blood_Pos > 0.f) {
		if(Blood_Pos > 1.f) {
			if(Last_Blood_Pos <= 1.f) {
				Blood_Pos = 1.0001f;
				duration = 0;
			}
			if(duration > Blood_Duration) {
				Blood_Pos += g_gameTime.lastFrameDuration() / 300ms;
			}
			duration += g_gameTime.lastFrameDuration();
		} else {
			Blood_Pos += g_gameTime.lastFrameDuration() / 40ms;
		}
	}
	
	Last_Blood_Pos = Blood_Pos;
}

void ARX_DAMAGE_Reset_Blood_Info() {
	g_screenFxBloodSplash.reset();
}

void ARX_DAMAGE_Show_Hit_Blood() {
	g_screenFxBloodSplash.render();
}


static ScriptParameters getOuchEventParameter(const Entity * entity) {
	
	std::ostringstream oss;
	oss.setf(std::ios_base::fixed, std::ios_base::floatfield);
	oss.precision(2);
	
	oss << entity->dmg_sum;
	
	return oss.str();
}

static ScriptParameters getHitEventParameters(float dmg, Entity * source, Spell * spell, DamageType type) {
	
	// First parameter: damage amount
	ScriptParameters parameters(dmg);
	
	// Second parameter: weapon category
	if(source == entities.player()) {
		if(spell || (type & DAMAGE_TYPE_FAKESPELL)) {
			parameters.emplace_back("spell");
		} else {
			switch(ARX_EQUIPMENT_GetPlayerWeaponType()) {
				case WEAPON_BARE: {
					parameters.emplace_back("bare");
					break;
				}
				case WEAPON_DAGGER: {
					parameters.emplace_back("dagger");
					break;
				}
				case WEAPON_1H: {
					parameters.emplace_back("1h");
					break;
				}
				case WEAPON_2H: {
					parameters.emplace_back("2h");
					break;
				}
				case WEAPON_BOW: {
					parameters.emplace_back("arrow");
					break;
				}
				default: break;
			}
		}
	} else if(source && (source->ioflags & IO_NPC) && source->_npcdata->summoner == EntityHandle_Player) {
		parameters.push_back("summoned");
	}
	if(parameters.size() < 2) {
		parameters.emplace_back(std::string());
	}
	
	// Third parameter: weapon entity
	if(spell) {
		parameters.push_back(spell->idString());
	} else if(source && (source->ioflags & IO_NPC) && source->_npcdata->summoner == EntityHandle_Player) {
		parameters.push_back(source->idString());
	} else if(source && !(type & DAMAGE_TYPE_FAKESPELL)) {
		if(Entity * weapon = getWeapon(*source)) {
			parameters.push_back(weapon->idString());
		}
	}
	if(parameters.size() < 3) {
		parameters.emplace_back("none");
	}
	
	// Fourth parameter: damage type
	if(type) {
		std::ostringstream oss;
		// These should be kept in sync with the dodamage and damager script command flags
		if(type & DAMAGE_TYPE_FIRE) {
			oss << 'f';
		}
		if(type & DAMAGE_TYPE_MAGICAL) {
			oss << 'm';
		}
		if(type & DAMAGE_TYPE_POISON) {
			oss << 'p';
		}
		if(type & DAMAGE_TYPE_LIGHTNING) {
			oss << 'l';
		}
		if(type & DAMAGE_TYPE_COLD) {
			oss << 'c';
		}
		if(type & DAMAGE_TYPE_GAS) {
			oss << 'g';
		}
		if(type & DAMAGE_TYPE_METAL) {
			oss << 'e';
		}
		if(type & DAMAGE_TYPE_WOOD) {
			oss << 'w';
		}
		if(type & DAMAGE_TYPE_STONE) {
			oss << 's';
		}
		if(type & DAMAGE_TYPE_ACID) {
			oss << 'a';
		}
		if(type & DAMAGE_TYPE_ORGANIC) {
			oss << 'o';
		}
		if(type & DAMAGE_TYPE_DRAIN_LIFE) {
			oss << 'r';
		}
		if(type & DAMAGE_TYPE_DRAIN_MANA) {
			oss << 'n';
		}
		if(type & DAMAGE_TYPE_PUSH) {
			oss << 'p';
		}
		parameters.push_back(oss.str());
	}
	
	return parameters;
}

float damagePlayer(float dmg, DamageType type, Entity * source) {
	
	if(player.playerflags & PLAYERFLAGS_INVULNERABILITY) {
		return 0;
	}
	
	if(player.lifePool.current == 0.f) {
		return 0.f;
	}
	
	// TODO was this intended to be std::min(dmg, player.lifePool.current)
	float damagesdone = 0.f;
	if(dmg > player.lifePool.current) {
		damagesdone = dmg;
	} else {
		damagesdone = player.lifePool.current;
	}
	
	entities.player()->dmg_sum += dmg;
	
	GameDuration elapsed = g_gameTime.now() - entities.player()->ouch_time;
	if(elapsed > 500ms) {
		entities.player()->ouch_time = g_gameTime.now();
		SendIOScriptEvent(source, entities.player(), SM_OUCH, getOuchEventParameter(entities.player()));
		float power = entities.player()->dmg_sum / player.m_lifeMaxWithoutMods * 220.f;
		AddQuakeFX(power * 3.5f, 500ms + 3ms * power, Random::getf(200.f, 300.f) + power, false);
		entities.player()->dmg_sum = 0.f;
	}
	
	if(dmg > 0.f) {
		
		if(source) {
			Entity * pio = source;
			if((source->ioflags & IO_NPC)
			   && source->_npcdata->weapon
			   && source->_npcdata->weapon->poisonous != 0
			   && source->_npcdata->weapon->poisonous_count != 0) {
				pio = source->_npcdata->weapon;
			}
			if(pio && pio->poisonous && pio->poisonous_count != 0) {
				if(Random::getf(0.f, 100.f) > player.m_miscFull.resistPoison) {
					player.poison += pio->poisonous;
				}
				if(pio->poisonous_count != -1)
					pio->poisonous_count--;
			}
		}
		
		bool wasAlive = (player.lifePool.current > 0);
		
		if(!BLOCK_PLAYER_CONTROLS) {
			player.lifePool.current -= dmg;
		}
		
		if(player.lifePool.current <= 0.f) {
			player.lifePool.current = 0.f;
			if(wasAlive) {
				ARX_PLAYER_BecomesDead();
				if((type & DAMAGE_TYPE_FIRE) || (type & DAMAGE_TYPE_FAKEFIRE)) {
					ARX_SOUND_PlayInterface(g_snd.PLAYER_DEATH_BY_FIRE);
				}
				SendIOScriptEvent(source, entities.player(), SM_DIE);
				for(Entity & npc : entities(IO_NPC)) {
					if(npc.targetinfo == EntityHandle_Player) {
						SendIOScriptEvent(entities.player(), &npc, "target_death", idString(source));
					}
				}
			}
		}
		
		arx_assert(player.m_lifeMaxWithoutMods > 0.f);
		g_screenFxBloodSplash.hit(dmg / player.m_lifeMaxWithoutMods);
	}
	
	// revient les barres
	ResetPlayerInterface();
	
	return damagesdone;
}

static void ARX_DAMAGES_HealPlayer(float dmg) {
	
	if(player.lifePool.current == 0.f || dmg <= 0.f) {
		return;
	}
	
	if(!BLOCK_PLAYER_CONTROLS) {
		player.lifePool.current += dmg;
	}
	
	player.lifePool.current = std::min(player.lifePool.current, player.lifePool.max);
	
}

void healCharacter(Entity & entity, float dmg) {
	
	arx_assert(entity.ioflags & IO_NPC);
	
	if(entity._npcdata->lifePool.current <= 0.f || dmg <= 0.f) {
		return;
	}
	
	if(entity == *entities.player()) {
		ARX_DAMAGES_HealPlayer(dmg);
	}
	
	entity._npcdata->lifePool.current = std::min(entity._npcdata->lifePool.current + dmg,
	                                             entity._npcdata->lifePool.max);
	
}

static void ARX_DAMAGES_HealManaPlayer(float dmg) {
	
	if(player.lifePool.current == 0.f || dmg <= 0.f) {
		return;
	}
	
	player.manaPool.current = std::min(player.manaPool.current + dmg, player.manaPool.max);
	
}

static void restoreMana(Entity & entity, float dmg) {
	
	arx_assert(entity.ioflags & IO_NPC);
	
	if(entity == *entities.player()) {
		ARX_DAMAGES_HealManaPlayer(dmg);
	}
	
	if(entity._npcdata->lifePool.current <= 0.f || dmg <= 0.f) {
		return;
	}
	
	entity._npcdata->manaPool.current = std::min(entity._npcdata->manaPool.current + dmg,
	                                             entity._npcdata->manaPool.max);
	
}

static float ARX_DAMAGES_DrainMana(Entity * io, float dmg) {
	
	if(!io || !(io->ioflags & IO_NPC))
		return 0;

	if(io == entities.player()) {
		if(player.playerflags & PLAYERFLAGS_NO_MANA_DRAIN)
			return 0;

		if(player.manaPool.current >= dmg) {
			player.manaPool.current -= dmg;
			return dmg;
		}

		float d = player.manaPool.current;
		player.manaPool.current = 0;
		return d;
	}

	if(io->_npcdata->manaPool.current >= dmg) {
		io->_npcdata->manaPool.current -= dmg;
		return dmg;
	}

	float d = io->_npcdata->manaPool.current;
	io->_npcdata->manaPool.current = 0;
	return d;
}

void damageProp(Entity & prop, float dmg, Entity * source, Spell * spell, DamageType type) {
	
	arx_assert(prop.ioflags & IO_FIX);
	
	if(!prop.show || (prop.ioflags & IO_INVULNERABILITY) || !prop.script.valid) {
		return;
	}
	
	prop.dmg_sum += dmg;
	
	GameDuration elapsed = g_gameTime.now() - prop.ouch_time;
	if(elapsed > 500ms) {
		prop.ouch_time = g_gameTime.now();
		SendIOScriptEvent(source, &prop, SM_OUCH, getOuchEventParameter(&prop));
		prop.dmg_sum = 0.f;
	}
	
	if(!prop.isInvulnerable() && Random::getf(0.f, 100.f) > prop.durability) {
		prop.durability -= dmg * 0.5f;
	}
	
	if(prop.durability <= 0.f) {
		prop.durability = 0.f;
		SendIOScriptEvent(source, &prop, SM_BREAK);
		return;
	}
	
	SendIOScriptEvent(source, &prop, SM_HIT, getHitEventParameters(dmg, source, spell, type));
	
}

void ARX_DAMAGES_ForceDeath(Entity & io_dead, Entity * io_killer) {
	
	if(io_dead.mainevent == SM_DEAD) {
		return;
	}
	
	if(&io_dead == g_draggedEntity) {
		setDraggedEntity(nullptr);
	}
	
	if(&io_dead == FlyingOverIO) {
		FlyingOverIO = nullptr;
	}
	
	if(g_cameraEntity == &io_dead) {
		g_cameraEntity = nullptr;
	}
	
	lightHandleDestroy(io_dead.dynlight);
	
	if(io_dead.ioflags & IO_NPC) {
		resetNpcBehavior(io_dead);
	}
	
	ARX_SCRIPT_Timer_Clear_For_IO(&io_dead);
	
	if(io_dead.mainevent != SM_DEAD) {
		if(SendIOScriptEvent(io_killer, &io_dead, SM_DIE) != REFUSE && ValidIOAddress(&io_dead)) {
			io_dead.infracolor = Color3f::blue;
		}
	}
	
	if(!ValidIOAddress(&io_dead)) {
		return;
	}
	
	io_dead.mainevent = SM_DEAD;
	
	if(fartherThan(io_dead.pos, g_camera->m_pos, 3200.f)) {
		io_dead.animlayer[0].ctime = 9999999ms;
		io_dead.animBlend.lastanimtime = 0;
	}
	
	if(io_dead.ioflags & IO_NPC) {
		io_dead._npcdata->weaponinhand = 0;
	}
	
	ARX_INTERACTIVE_DestroyDynamicInfo(&io_dead);
	
	ScriptParameters killer;
	if(io_killer) {
		killer.emplace_back(io_killer->idString());
	}
	
	for(Entity & follower : entities(IO_NPC)) {
		
		if(follower == io_dead) {
			continue;
		}
		
		if(follower.targetinfo == io_dead.index()) {
			Stack_SendIOScriptEvent(&io_dead, &follower, "target_death", killer);
			follower.targetinfo = EntityHandle(TARGET_NONE);
			follower._npcdata->reachedtarget = 0;
		}
		
		if(follower._npcdata->pathfind.truetarget == io_dead.index()) {
			Stack_SendIOScriptEvent(&io_dead, &follower, "target_death", killer);
			follower._npcdata->pathfind.truetarget = EntityHandle(TARGET_NONE);
			follower._npcdata->reachedtarget = 0;
		}
		
	}
	
	io_dead.animlayer[1].cur_anim = nullptr;
	io_dead.animlayer[2].cur_anim = nullptr;
	io_dead.animlayer[3].cur_anim = nullptr;

	if(io_dead.ioflags & IO_NPC) {
		io_dead._npcdata->lifePool.current = 0;

		if(io_dead._npcdata->weapon) {
			Entity * ioo = io_dead._npcdata->weapon;
			if(ValidIOAddress(ioo)) {
				removeFromInventories(ioo);
				ioo->show = SHOW_FLAG_IN_SCENE;
				ioo->ioflags |= IO_NO_NPC_COLLIDE;
				ioo->pos = ioo->obj->vertexWorldPositions[ioo->obj->origin].v;
				// TODO old broken code suggested that physics sim might be enabled here
			}
		}
	}
	
}

static void pushEntity(Entity & entity, const Entity & source, float power) {
	
	if(power > 0.f) {
		
		Vec3f vect = glm::normalize(entity.pos - source.pos) * (power * 0.05f);
		arx_assert(isallfinite(vect));
		
		if(entity == *entities.player()) {
			PUSH_PLAYER_FORCE = vect; // TODO why not +=?
		} else {
			entity.move += vect;
		}
		
	}
	
}

void damageCharacter(Entity & entity, float dmg, Entity & source, Spell * spell, DamageType flags, Vec3f * pos) {
	
	if(flags & DAMAGE_TYPE_PER_SECOND) {
		dmg = dmg * (g_gameTime.lastFrameDuration() / 1s);
	}
	
	float damagesdone;
	if(entity == *entities.player()) {
		
		if(flags & DAMAGE_TYPE_POISON) {
			if(Random::getf(0.f, 100.f) > player.m_miscFull.resistPoison) {
				damagesdone = dmg;
				player.poison += damagesdone;
			} else {
				damagesdone = 0;
			}
		} else if(flags & DAMAGE_TYPE_DRAIN_MANA) {
			damagesdone = ARX_DAMAGES_DrainMana(&entity, dmg);
		} else {
			ARX_DAMAGES_DamagePlayerEquipment(dmg);
			damagesdone = damagePlayer(dmg, flags, &source);
		}
		
		if(flags & DAMAGE_TYPE_FIRE) {
			igniteEntity(entity, &source, damagesdone);
		}
		
	} else {
		
		if(flags & DAMAGE_TYPE_POISON) {
			
			if(Random::getf(0.f, 100.f) > entity._npcdata->resist_poison) {
				damagesdone = dmg;
				entity._npcdata->poisonned += damagesdone;
			} else {
				damagesdone = 0;
			}
			
		} else {
			
			if(flags & DAMAGE_TYPE_FIRE) {
				if(Random::getf(0.f, 100.f) <= entity._npcdata->resist_fire) {
					dmg = 0;
				}
				igniteEntity(entity, &source, dmg);
			}
			
			if(flags & DAMAGE_TYPE_DRAIN_MANA) {
				damagesdone = ARX_DAMAGES_DrainMana(&entity, dmg);
			} else {
				damagesdone = damageNpc(entity, dmg, &source, spell, flags, pos);
			}
			
		}
		
	}
	
	if((flags & DAMAGE_TYPE_DRAIN_LIFE) && (source.ioflags & IO_NPC)) {
		healCharacter(source, damagesdone);
	}
	
	if((flags & DAMAGE_TYPE_DRAIN_MANA) && (source.ioflags & IO_NPC)) {
		restoreMana(source, damagesdone);
	}
	
	if(flags & DAMAGE_TYPE_PUSH) {
		pushEntity(entity, source, damagesdone * 0.5f);
	}
	
}

float damageNpc(Entity & npc, float dmg, Entity * source, Spell * spell, DamageType type, const Vec3f * pos) {
	
	arx_assert(npc.ioflags & IO_NPC);
	arx_assert(npc != *entities.player());
	
	if(!npc.show || (npc.ioflags & IO_INVULNERABILITY)) {
		return 0.f;
	}
	
	if(npc._npcdata->lifePool.current <= 0.f) {
		if((source != entities.player() || entities.get(player.equiped[EQUIP_SLOT_WEAPON]))
		   && dmg >= npc._npcdata->lifePool.max * 0.4f && pos) {
			ARX_NPC_TryToCutSomething(&npc, pos);
		}
		return 0.f;
	}
	
	npc.dmg_sum += dmg;
	
	GameDuration elapsed = g_gameTime.now() - npc.ouch_time;
	if(elapsed > 500ms) {
		npc.ouch_time = g_gameTime.now();
		ScriptParameters parameters = getOuchEventParameter(&npc);
		Entity * sender = source;
		if(sender && (sender->ioflags & IO_NPC) && sender->_npcdata->summoner == EntityHandle_Player) {
			parameters.push_back("summoned");
			parameters.push_back(sender->idString());
			sender = entities.player();
		}
		SendIOScriptEvent(sender, &npc, SM_OUCH, parameters);
		npc.dmg_sum = 0.f;
		spells.endByTarget(npc.index(), SPELL_CONFUSE);
	}
	
	if(dmg < 0.f) {
		return 0.f;
	}
	
	if(source) {
		Entity * pio = nullptr;
		if(source == entities.player()) {
			if(Entity * weapon = entities.get(player.equiped[EQUIP_SLOT_WEAPON])) {
				pio = weapon;
				if((pio->poisonous == 0 || pio->poisonous_count == 0) || spell || (type & DAMAGE_TYPE_FAKESPELL)) {
					pio = nullptr;
				}
			}
		} else {
			if(source->ioflags & IO_NPC) {
				pio = source->_npcdata->weapon;
				if(pio && (pio->poisonous == 0 || pio->poisonous_count == 0)) {
					pio = nullptr;
				}
			}
		}
		if(!pio) {
			pio = source;
		}
		if(pio && pio->poisonous && (pio->poisonous_count != 0)) {
			if(Random::getf(0.f, 100.f) > npc._npcdata->resist_poison) {
				npc._npcdata->poisonned += pio->poisonous;
			}
			if(pio->poisonous_count != -1) {
				pio->poisonous_count--;
			}
		}
	}
	
	if(npc.script.valid && source) {
		Entity * sender = source;
		if((sender->ioflags & IO_NPC) && sender->_npcdata->summoner == EntityHandle_Player) {
			sender = entities.player();
		}
		if(SendIOScriptEvent(sender, &npc, SM_HIT, getHitEventParameters(dmg, source, spell, type)) != ACCEPT) {
			return 0.f;
		}
	}
	
	float damagesdone = std::min(dmg, npc._npcdata->lifePool.current);
	npc._npcdata->lifePool.current -= dmg;
	
	float fHitFlash = 0;
	if(npc._npcdata->lifePool.current <= 0) {
		fHitFlash = 0;
	} else {
		fHitFlash = npc._npcdata->lifePool.current / npc._npcdata->lifePool.max;
	}
	g_hudRoot.hitStrengthGauge.requestFlash(fHitFlash);
	
	if(npc._npcdata->lifePool.current <= 0.f) {
		npc._npcdata->lifePool.current = 0.f;
		if((source != entities.player() || entities.get(player.equiped[EQUIP_SLOT_WEAPON]))
		   && dmg >= npc._npcdata->lifePool.max * 0.5f && pos) {
			ARX_NPC_TryToCutSomething(&npc, pos);
		}
		long xp = npc._npcdata->xpvalue;
		ARX_DAMAGES_ForceDeath(npc, source);
		if(source == entities.player()
		   || (source && (source->ioflags & IO_NPC) && source->_npcdata->summoner == EntityHandle_Player)) {
			ARX_PLAYER_Modify_XP(xp);
		}
	}
	
	return damagesdone;
}

void ARX_DAMAGES_Reset() {
	g_damages.clear();
}

static void ARX_DAMAGES_AddVisual(DAMAGE_INFO & di, const Vec3f & pos, float dmg, Entity * io) {
	
	arx_assert(io && (io->ioflags & IO_NPC));
	
	if(!(di.params.type & DAMAGE_TYPE_FAKEFIRE)) {
		return;
	}
	
	GameInstant now = g_gameTime.now();
	if(di.lastupd + 200ms < now) {
		di.lastupd = now;
		if(di.params.type & DAMAGE_TYPE_MAGICAL) {
			ARX_SOUND_PlaySFX(g_snd.SPELL_MAGICAL_HIT, &pos, Random::getf(0.8f, 1.2f));
		} else {
			ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_HIT, &pos, Random::getf(0.8f, 1.2f));
		}
	}
	
	if(io->_npcdata->m_magicalDamageTime + 500ms < now) {
		// Make sure there is at least one particle
		io->_npcdata->m_magicalDamageQuantizer.reset();
		io->_npcdata->m_magicalDamageQuantizer.add(1.f);
	} else {
		// Add 60 new particles per second
		io->_npcdata->m_magicalDamageQuantizer.add(g_framedelay * 0.06f);
	}
	io->_npcdata->m_magicalDamageTime = now;
	
	int count = io->_npcdata->m_magicalDamageQuantizer.consume();
	
	for(int i = 0; i < count; i++) {
		
		long num = Random::get(0, io->obj->vertexlist.size() / 4 - 1) * 4 + 1;
		arx_assert(num >= 0);
		Vec3f vertPos = io->obj->vertexWorldPositions[num].v;
		
		for(long k = 0 ; k < 14 ; k++) {
			
			PARTICLE_DEF * pd = createParticle();
			if(!pd) {
				break;
			}
			
			pd->ov = vertPos + arx::randomVec(-5.f, 5.f);
			pd->siz = glm::clamp(dmg, 5.f, 15.f);
			pd->scale = Vec3f(-10.f);
			pd->m_flags = ROTATING | FIRE_TO_SMOKE;
			pd->tolive = Random::getu(500, 900);
			pd->move = Vec3f(1.f, 2.f, 1.f) - arx::randomVec3f() * Vec3f(2.f, 16.f, 2.f);
			if(di.params.type & DAMAGE_TYPE_MAGICAL) {
				pd->rgb = Color3f(0.3f, 0.3f, 0.8f);
			} else {
				pd->rgb = Color3f::gray(0.5f);
			}
			pd->tc = g_particleTextures.fire2;
			pd->m_rotation = Random::getf(-0.1f, 0.1f);
		}
		
	}
	
}

// source = -1 no source but valid pos
// source = 0  player
// source > 0  IO
static void updateDamage(DAMAGE_INFO & damage, GameInstant now) {
	
	ARX_PROFILE_FUNC();
	
	Entity * source = entities.get(damage.params.source);
	
	if(damage.params.flags & DAMAGE_FLAG_FOLLOW_SOURCE) {
		if(source == entities.player()) {
			damage.params.pos = player.pos;
		} else if(source) {
			damage.params.pos = source->pos;
		}
	}
	
	float dmg;
	if(damage.params.flags & DAMAGE_NOT_FRAME_DEPENDANT) {
		dmg = damage.params.damages;
	} else {
		GameDuration FD = g_gameTime.lastFrameDuration();
		
		if(!(damage.params.flags & DAMAGE_ONCE) && now - damage.start_time > damage.params.duration) {
			FD -= damage.start_time + damage.params.duration - now;
		}
		
		dmg = damage.params.damages * (FD / 1s);
	}
	
	float divradius = 1.f / damage.params.radius;
	
	EntityFlags flags = IO_NPC;
	if(!(damage.params.type & DAMAGE_TYPE_NO_FIX)) {
		flags |= IO_FIX;
	}
	for(Entity & entity : entities.inScene(flags)) {
		
		if(!(entity.gameFlags & GFLAG_ISINTREATZONE)
		   || (&entity == source && (damage.params.flags & DAMAGE_FLAG_DONT_HURT_SOURCE))) {
			continue;
		}
		
		Sphere sphere;
		sphere.origin = damage.params.pos;
		sphere.radius = damage.params.radius + ((entity.ioflags & IO_NPC) ? -10.f : 15.f);
		if(!CheckIOInSphere(sphere, entity, (entity.ioflags & IO_NPC))) {
			continue;
		}
		
		if(entity.ioflags & IO_NPC) {
			
			if(entity != *entities.player() && source != entities.player() && source
			   && HaveCommonGroup(&entity, source)) {
				continue;
			}
			
			Vec3f sub = entity.pos + Vec3f(0.f, -60.f, 0.f);
			float dist = fdist(damage.params.pos, sub);
			
			if(damage.params.type & DAMAGE_TYPE_FIELD) {
				GameDuration elapsed = g_gameTime.now() - entity.collide_door_time;
				if(elapsed > 500ms) {
					entity.collide_door_time = g_gameTime.now();
					ScriptParameters parameters;
					if(damage.params.type & DAMAGE_TYPE_COLD) {
						parameters = "cold";
					} else if(damage.params.type & DAMAGE_TYPE_FIRE) {
						parameters = "fire";
					}
					SendIOScriptEvent(nullptr, &entity, SM_COLLIDE_FIELD, parameters);
				}
			}
			
			// TODO damage ratio applied multiple times if there are multiple NPCs in range
			switch(damage.params.area) {
				case DAMAGE_AREA: {
					float ratio = (damage.params.radius - dist) * divradius;
					ratio = glm::clamp(ratio, 0.f, 1.f);
					dmg = dmg * ratio + 1.f;
					break;
				}
				case DAMAGE_AREAHALF: {
					float ratio = (damage.params.radius - dist * 0.5f) * divradius;
					ratio = glm::clamp(ratio, 0.f, 1.f);
					dmg = dmg * ratio + 1.f;
					break;
				}
				case DAMAGE_FULL: {
					break;
				}
			}
			if(dmg <= 0.f) {
				continue;
			}
			
			if((damage.params.flags & DAMAGE_FLAG_ADD_VISUAL_FX) && (entity._npcdata->lifePool.current > 0.f)) {
				ARX_DAMAGES_AddVisual(damage, sub, dmg, &entity);
			}
			
			if(damage.params.type & DAMAGE_TYPE_DRAIN_MANA) {
				
				float manadrained;
				if(entity == *entities.player()) {
					manadrained = std::min(dmg, player.manaPool.current);
					player.manaPool.current -= manadrained;
				} else {
					manadrained = std::min(dmg, entity._npcdata->manaPool.current);
					entity._npcdata->manaPool.current -= manadrained;
				}
				
				if(source == entities.player()) {
					player.manaPool.current = std::min(player.manaPool.current + manadrained, player.manaPool.max);
				} else if(source && (source->ioflags & IO_NPC)) {
					source->_npcdata->manaPool.current = std::min(source->_npcdata->manaPool.current + manadrained,
					                                              source->_npcdata->manaPool.max);
				}
				
			} else {
				
				// TODO copy-paste
				float damagesdone;
				if(entity == *entities.player()) {
					if(damage.params.type & DAMAGE_TYPE_POISON) {
						if(Random::getf(0.f, 100.f) > player.m_miscFull.resistPoison) {
							// Failed Saving Throw
							damagesdone = dmg;
							player.poison += damagesdone;
						} else {
							damagesdone = 0;
						}
					} else {
						if((damage.params.type & DAMAGE_TYPE_MAGICAL)
						   && !(damage.params.type & DAMAGE_TYPE_FIRE)
						   && !(damage.params.type & DAMAGE_TYPE_COLD)) {
							dmg -= player.m_miscFull.resistMagic * 0.01f * dmg;
							dmg = std::max(0.0f, dmg);
						}
						if(damage.params.type & DAMAGE_TYPE_FIRE) {
							dmg = ARX_SPELLS_ApplyFireProtection(&entity, dmg);
							igniteEntity(entity, source, dmg);
						}
						if(damage.params.type & DAMAGE_TYPE_COLD) {
							dmg = ARX_SPELLS_ApplyColdProtection(&entity, dmg);
						}
						damagesdone = damagePlayer(dmg, damage.params.type, source);
					}
				} else {
					if(damage.params.type & DAMAGE_TYPE_POISON) {
						if(Random::getf(0.f, 100.f) > entity._npcdata->resist_poison) {
							// Failed Saving Throw
							damagesdone = dmg;
							entity._npcdata->poisonned += damagesdone;
						} else {
							damagesdone = 0;
						}
					} else {
						if(damage.params.type & DAMAGE_TYPE_FIRE) {
							dmg = ARX_SPELLS_ApplyFireProtection(&entity, dmg);
							igniteEntity(entity, source, dmg);
						}
						if((damage.params.type & DAMAGE_TYPE_MAGICAL)
						    && !(damage.params.type & DAMAGE_TYPE_FIRE)
						    && !(damage.params.type & DAMAGE_TYPE_COLD)) {
							dmg -= entity._npcdata->resist_magic * 0.01f * dmg;
							dmg = std::max(0.0f, dmg);
						}
						if(damage.params.type & DAMAGE_TYPE_COLD) {
							dmg = ARX_SPELLS_ApplyColdProtection(&entity, dmg);
						}
						damagesdone = damageNpc(entity, dmg, source, damage.spell, damage.params.type, &damage.params.pos);
					}
					if(damagesdone > 0 && (damage.params.flags & DAMAGE_SPAWN_BLOOD)) {
						ARX_PARTICLES_Spawn_Blood(damage.params.pos, damagesdone, damage.params.source);
					}
				}
				
				if((damage.params.type & DAMAGE_TYPE_DRAIN_LIFE) && source && (source->ioflags & IO_NPC)) {
					healCharacter(*source, damagesdone);
				}
				
			}
			
		} else if(entity.ioflags & IO_FIX) {
			
			damageProp(entity, dmg, source, damage.spell, damage.params.type);
			
		}
		
	}
	
	if((damage.params.flags & DAMAGE_ONCE) || now - damage.start_time > damage.params.duration) {
		damage.exist = false;
	}
	
}

void ARX_DAMAGES_UpdateAll() {
	
	ARX_PROFILE_FUNC();
	
	for(DAMAGE_INFO & damage : g_damages) {
		if(damage.exist) {
			updateDamage(damage, g_gameTime.now());
		}
	}
	
}

static bool SphereInIO(Entity * io, const Sphere & sphere) {
	
	if(!io || !io->obj)
		return false;

	long step;
	long nbv = io->obj->vertexlist.size();
	
	if(nbv < 150)
		step = 1;
	else if(nbv < 300)
		step = 2;
	else if(nbv < 600)
		step = 4;
	else if(nbv < 1200)
		step = 6;
	else
		step = 7;

	for(size_t i = 0; i < io->obj->vertexlist.size(); i += step) {
		if(!fartherThan(sphere.origin, io->obj->vertexWorldPositions[i].v, sphere.radius)) {
			return true;
		}
	}
	
	return false;
}

bool tryToDoDamage(const Vec3f & pos, float dmg, float radius, Entity & source) {
	
	bool ret = false;
	for(Entity & entity : entities.inScene(IO_NPC | IO_FIX)) {
		
		if(!(entity.gameFlags & GFLAG_ISINTREATZONE) || entity == source) {
			continue;
		}
		
		float threshold;
		float rad = radius + 5.f;
		if(entity.ioflags & IO_FIX) {
			threshold = 510;
			rad += 10.f;
		} else if(entity.ioflags & IO_NPC) {
			threshold = 250;
		} else {
			arx_unreachable();
		}
		
		if(closerThan(pos, entity.pos, threshold) && SphereInIO(&entity, Sphere(pos, rad))) {
			if(entity.ioflags & IO_NPC) {
				ARX_EQUIPMENT_ComputeDamages(&source, &entity, 1.f);
				ret = true;
			}
			if(entity.ioflags & IO_FIX) {
				damageProp(entity, dmg, &source, nullptr, DAMAGE_TYPE_GENERIC);
				ret = true;
			}
		}
		
	}
	
	return ret;
}

void igniteLights(const Sphere & sphere, bool ignite, bool ignoreFireplaces) {
	
	for(EERIE_LIGHT & light : g_staticLights) {
		
		if(!(light.extras & EXTRAS_EXTINGUISHABLE)
		   || !(light.extras & (EXTRAS_SEMIDYNAMIC | EXTRAS_SPAWNFIRE | EXTRAS_SPAWNSMOKE))
		   || ((light.extras & EXTRAS_FIREPLACE) && ignoreFireplaces)
		   || fartherThan(sphere.origin, light.pos, sphere.radius)) {
			continue;
		}
		
		if(ignite) {
			if(!(light.extras & EXTRAS_NO_IGNIT)) {
				light.m_ignitionStatus = true;
			}
		} else {
			light.m_ignitionStatus = false;
		}
		
	}
	
}

void igniteEntities(const Sphere & sphere, bool ignite) {
	
	for(Entity & entity : entities.inScene()) {
		
		if(!entity.obj
		   || (entity.ioflags & IO_UNDERWATER)
		   || entity.obj->fastaccess.fire == ActionPoint()
		   || !closerThan(sphere.origin, actionPointPosition(entity.obj, entity.obj->fastaccess.fire),
		                  sphere.radius)) {
			continue;
		}
		
		if(ignite && entity.ignition <= 0) {
			entity.ignition = 1;
		} else if(!ignite && entity.ignition > 0) {
			entity.ignition = 0;
			lightHandleDestroy(entity.ignit_light);
			ARX_SOUND_Stop(entity.ignit_sound);
			entity.ignit_sound = audio::SourcedSample();
		}
		
	}
	
}

void doSphericDamage(const Sphere & sphere, float dmg, DamageArea flags, Spell * spell, DamageType typ, Entity * source) {
	
	if(sphere.radius <= 0.f) {
		return;
	}
	
	float rad = 1.f / sphere.radius;
	
	for(Entity & entity : entities) {
		
		if(&entity == source || !entity.obj || (entity.ioflags & (IO_CAMERA | IO_MARKER))) {
			continue;
		}
		
		if(source && entity != *entities.player() && source != entities.player()
		   && HaveCommonGroup(&entity, source)) {
			continue;
		}
		
		long count = 0;
		long count2 = 0;
		float mindist = std::numeric_limits<float>::max();
		for(size_t k = 0; k < entity.obj->vertexWorldPositions.size(); k += 1) {
			if(entity.obj->vertexlist.size() < 120) {
				for(size_t kk = 0; kk < entity.obj->vertexWorldPositions.size(); kk += 1) {
					if(kk != k) {
						Vec3f pos = (entity.obj->vertexWorldPositions[k].v + entity.obj->vertexWorldPositions[kk].v) * 0.5f;
						float dist = fdist(sphere.origin, pos);
						if(dist <= sphere.radius) {
							count2++;
							if(dist < mindist) {
								mindist = dist;
							}
						}
					}
				}
			}
			float dist = fdist(sphere.origin, entity.obj->vertexWorldPositions[k].v);
			if(dist <= sphere.radius) {
				count++;
				if(dist < mindist) {
					mindist = dist;
				}
			}
		}
		
		if(mindist > sphere.radius + 30.f) {
			continue;
		}
		
		// TODO damage is scaled multiple times if there are multiple NPCs in the sphere
		if(entity.ioflags & IO_NPC) {
			switch (flags) {
				case DAMAGE_AREA:
					dmg = dmg * (sphere.radius + 30 - mindist) * rad;
					break;
				case DAMAGE_AREAHALF:
					dmg = dmg * (sphere.radius + 30 - mindist * 0.5f) * rad;
					break;
				case DAMAGE_FULL: break;
			}
		}
		
		float ratio = glm::max(count, count2) / (float(entity.obj->vertexWorldPositions.size()) / 2.f);
		if(ratio > 2.f) {
			ratio = 2.f;
		}
		if(entity == *entities.player()) {
			ratio = 1.f;
		}
		
		if(typ & DAMAGE_TYPE_FIRE) {
			dmg = ARX_SPELLS_ApplyFireProtection(&entity, dmg * ratio);
			igniteEntity(entity, source, dmg);
		}
		if(typ & DAMAGE_TYPE_COLD) {
			dmg = ARX_SPELLS_ApplyColdProtection(&entity, dmg * ratio);
		}
		
		if(entity == *entities.player()) {
			damagePlayer(dmg, typ, source);
			ARX_DAMAGES_DamagePlayerEquipment(dmg);
		} else if(entity.ioflags & IO_NPC) {
			damageNpc(entity, dmg * ratio, source, spell, typ, &sphere.origin);
		} else if(entity.ioflags & IO_FIX) {
			damageProp(entity, dmg * ratio, source, spell, typ);
		}
		
	}
	
	if(typ & DAMAGE_TYPE_FIRE) {
		igniteLights(sphere);
		igniteEntities(sphere);
	}
	
}

void ARX_DAMAGES_DurabilityRestore(Entity * io, float percent)
{
	if(!io)
		return;

	if (io->durability <= 0) return;

	if (io->durability == io->max_durability) return;

	if (percent >= 100.f) {
		io->durability = io->max_durability;
	} else {
		
		float ratio = percent * 0.01f;
		float to_restore = (io->max_durability - io->durability) * ratio;
		float v = Random::getf(0.f, 100.f) - percent;
		
		if (v <= 0.f) {
			float mloss = 1.f;

			if(io->ioflags & IO_ITEM) {
				io->_itemdata->price -= checked_range_cast<long>(float(io->_itemdata->price) / io->max_durability);
			}

			io->max_durability -= mloss;
		} else {
			
			if(v > 50.f) {
				v = 50.f;
			}
			v *= 0.01f;
			
			float mloss = io->max_durability * v;
			if(io->ioflags & IO_ITEM) {
				io->_itemdata->price -= static_cast<long>(float(io->_itemdata->price) * v);
			}

			io->max_durability -= mloss;
		}

		io->durability += to_restore;

		if(io->durability > io->max_durability)
			io->durability = io->max_durability;

		if(io->max_durability <= 0.f)
			ARX_DAMAGES_DurabilityLoss(io, 100);
	}

}

void ARX_DAMAGES_DurabilityCheck(Entity * io, float ratio)
{
	if(!io || io->isInvulnerable())
		return;

	if(Random::getf(0.f, 100.f) > io->durability) {
		ARX_DAMAGES_DurabilityLoss(io, ratio);
	}
}

void ARX_DAMAGES_DurabilityLoss(Entity * io, float loss) {
	
	arx_assert(io);
	
	io->durability -= loss;
	
	if(io->durability <= 0) {
		SendIOScriptEvent(nullptr, io, SM_BREAK);
	}
	
}

void ARX_DAMAGES_DamagePlayerEquipment(float damages) {
	
	float ratio = damages * 0.05f;
	if(ratio > 1.f) {
		ratio = 1.f;
	}
	
	for(EntityHandle equipped : player.equiped) {
		if(Entity * todamage = entities.get(equipped)) {
			ARX_DAMAGES_DurabilityCheck(todamage, ratio);
		}
	}
	
}

float ARX_DAMAGES_ComputeRepairPrice(const Entity * torepair, const Entity * blacksmith)
{
	if(!torepair || !blacksmith) return -1.f;

	if(!(torepair->ioflags & IO_ITEM)) return -1.f;

	if(torepair->max_durability <= 0.f) return -1.f;

	if(torepair->durability == torepair->max_durability) return -1.f;

	float ratio = (torepair->max_durability - torepair->durability) / torepair->max_durability;
	float price = float(torepair->_itemdata->price) * ratio;

	if(blacksmith->shop_multiply != 0.f)
		price *= blacksmith->shop_multiply;

	if(price > 0.f && price < 1.f)
		price = 1.f;

	return price;
}

void ARX_DAMAGES_DrawDebug() {
	
	for(DAMAGE_INFO & damage : g_damages) {
		if(damage.exist) {
			drawLineSphere(Sphere(damage.params.pos, damage.params.radius), Color::red);
		}
	}
	
}
