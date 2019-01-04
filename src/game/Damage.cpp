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
	short exist;
	GameInstant start_time;
	GameInstant lastupd;
	
	DamageParameters params;
};

const size_t MAX_DAMAGES = 200;
static DAMAGE_INFO g_damages[MAX_DAMAGES];

DamageHandle DamageCreate(const DamageParameters & params) {
	for(size_t i = 0; i < MAX_DAMAGES; i++) {
		if(!g_damages[i].exist) {
			DAMAGE_INFO & damage = g_damages[i];
			damage.params = params;
			damage.start_time = g_gameTime.now();
			damage.lastupd = 0;
			damage.exist = true;
			return DamageHandle(i);
		}
	}

	return DamageHandle(-1);
}

void DamageRequestEnd(DamageHandle handle) {
	if(handle.handleData() >= 0) {
		g_damages[handle.handleData()].exist = 0;
	}
}



extern Vec3f PUSH_PLAYER_FORCE;

static float Blood_Pos = 0.f;
static GameDuration Blood_Duration = 0;

static void ARX_DAMAGES_IgnitIO(Entity * source, Entity * io, float dmg) {
	
	if(!io || (io->ioflags & IO_INVULNERABILITY)) {
		return;
	}
	
	if(io->ignition <= 0.f && io->ignition + dmg > 1.f) {
		SendIOScriptEvent(source, io, SM_ENTERZONE, "cook_s");
	}
	
	if(io->ioflags & IO_FIX) {
		io->ignition += dmg * 0.1f;
	} else if(io->ioflags & IO_ITEM) {
		io->ignition += dmg * 0.125f;
	} else if(io->ioflags & IO_NPC) {
		io->ignition += dmg * 0.25f;
	}
	
}

void ARX_DAMAGE_Reset_Blood_Info()
{
	Blood_Pos = 0.f;
	Blood_Duration = 0;
}

void ARX_DAMAGE_Show_Hit_Blood()
{
	Color color;
	static float Last_Blood_Pos = 0.f;
	static GameDuration duration;

	if(Blood_Pos > 2.f) { // end of blood flash
		Blood_Pos = 0.f;
		duration = 0;
	} else if(Blood_Pos > 1.f) {
		
		if(player.poison > 1.f)
			color = Color::rgb(Blood_Pos - 1.f, 1.f, Blood_Pos - 1.f);
		else
			color = Color::rgb(1.f, Blood_Pos - 1.f, Blood_Pos - 1.f);
		
		UseRenderState state(render2D().blend(BlendZero, BlendSrcColor));
		EERIEDrawBitmap(Rectf(g_size), 0.00009f, NULL, color);
	} else if(Blood_Pos > 0.f) {
		
		if(player.poison > 1.f)
			color = Color::rgb(1.f - Blood_Pos, 1.f, 1.f - Blood_Pos);
		else
			color = Color::rgb(1.f, 1.f - Blood_Pos, 1.f - Blood_Pos);
		
		UseRenderState state(render2D().blend(BlendZero, BlendSrcColor));
		EERIEDrawBitmap(Rectf(g_size), 0.00009f, NULL, color);
	}
	
	if(Blood_Pos > 0.f) {
		if(Blood_Pos > 1.f) {
			if(Last_Blood_Pos <= 1.f) {
				Blood_Pos = 1.0001f;
				duration = 0;
			}
			if(duration > Blood_Duration) {
				Blood_Pos += g_gameTime.lastFrameDuration() / GameDurationMs(300);
			}
			duration += g_gameTime.lastFrameDuration();
		} else {
			Blood_Pos += g_gameTime.lastFrameDuration() / GameDurationMs(40);
		}
	}
	
	Last_Blood_Pos = Blood_Pos;
}

static ScriptParameters getOuchEventParameter(const Entity * entity) {
	
	std::ostringstream oss;
	oss.setf(std::ios_base::fixed, std::ios_base::floatfield);
	oss.precision(2);
	
	oss << entity->dmg_sum;
	
	return oss.str();
}

float ARX_DAMAGES_DamagePlayer(float dmg, DamageType type, EntityHandle source) {
	if (player.playerflags & PLAYERFLAGS_INVULNERABILITY)
		return 0;

	float damagesdone = 0.f;

	if(player.lifePool.current == 0.f)
		return damagesdone;

	if(dmg > player.lifePool.current)
		damagesdone = dmg;
	else
		damagesdone = player.lifePool.current;

	entities.player()->dmg_sum += dmg;
	
	Entity * sender = ValidIONum(source) ? entities[source] : NULL;
	
	GameDuration elapsed = g_gameTime.now() - entities.player()->ouch_time;
	if(elapsed > GameDurationMs(500)) {
		entities.player()->ouch_time = g_gameTime.now();
		SendIOScriptEvent(sender, entities.player(), SM_OUCH, getOuchEventParameter(entities.player()));
		float power = entities.player()->dmg_sum / player.lifePool.max * 220.f;
		AddQuakeFX(power * 3.5f, GameDurationMsf(500 + power * 3), Random::getf(200.f, 300.f) + power, false);
		entities.player()->dmg_sum = 0.f;
	}

	if(dmg > 0.f) {
		if(ValidIONum(source)) {
			Entity * pio = NULL;

			if(entities[source]->ioflags & IO_NPC) {
				pio = entities[source]->_npcdata->weapon;

				if(pio && (pio->poisonous == 0 || pio->poisonous_count == 0))
					pio = NULL;
			}

			if(!pio)
				pio = entities[source];

			if(pio && pio->poisonous && pio->poisonous_count != 0) {
				if(Random::getf(0.f, 100.f) > player.m_miscFull.resistPoison) {
					player.poison += pio->poisonous;
				}

				if(pio->poisonous_count != -1)
					pio->poisonous_count--;
			}
		}

		long alive;

		if(player.lifePool.current > 0)
			alive = 1;
		else
			alive = 0;

		if(!BLOCK_PLAYER_CONTROLS)
			player.lifePool.current -= dmg;

		if(player.lifePool.current <= 0.f) {
			player.lifePool.current = 0.f;

			if(alive) {
				
				ARX_PLAYER_BecomesDead();

				if((type & DAMAGE_TYPE_FIRE) || (type & DAMAGE_TYPE_FAKEFIRE)) {
					ARX_SOUND_PlayInterface(g_snd.PLAYER_DEATH_BY_FIRE);
				}

				SendIOScriptEvent(sender, entities.player(), SM_DIE);

				for(size_t i = 1; i < entities.size(); i++) {
					const EntityHandle handle = EntityHandle(i);
					Entity * ioo = entities[handle];
					
					if(ioo && (ioo->ioflags & IO_NPC)) {
						if(ioo->targetinfo == EntityHandle(TARGET_PLAYER)) {
							std::string killer;
							if(source == EntityHandle_Player) {
								killer = "player";
							} else if(source.handleData() <= EntityHandle().handleData()) {
								killer = "none";
							} else if(ValidIONum(source)) {
								killer = entities[source]->idString();
							}
							SendIOScriptEvent(entities.player(), entities[handle], "target_death", killer);
						}
					}
				}
			}
		}

		if(player.lifePool.max <= 0.f)
			return damagesdone;

		float t = dmg / player.lifePool.max;

		if(Blood_Pos == 0.f) {
			Blood_Pos = 0.000001f;
			Blood_Duration = GameDurationMsf(100.f + t * 200.f);
		} else {
			Blood_Duration += GameDurationMsf(t * 800.f);
		}
	}

	// revient les barres
	ResetPlayerInterface();

	return damagesdone;
}

static void ARX_DAMAGES_HealPlayer(float dmg) {
	
	if(player.lifePool.current == 0.f)
		return;

	if(dmg > 0.f) {
		if(!BLOCK_PLAYER_CONTROLS)
			player.lifePool.current += dmg;

		if(player.lifePool.current > player.Full_maxlife)
			player.lifePool.current = player.Full_maxlife;
	}
}

void ARX_DAMAGES_HealInter(Entity * io, float dmg)
{
	if(!io || !(io->ioflags & IO_NPC))
		return;

	if(io->_npcdata->lifePool.current <= 0.f)
		return;

	if(io == entities.player())
		ARX_DAMAGES_HealPlayer(dmg);

	if(dmg > 0.f) {
		io->_npcdata->lifePool.current += dmg;

		if(io->_npcdata->lifePool.current > io->_npcdata->lifePool.max)
			io->_npcdata->lifePool.current = io->_npcdata->lifePool.max;
	}
}

static void ARX_DAMAGES_HealManaPlayer(float dmg) {
	
	if(player.lifePool.current == 0.f)
		return;

	if(dmg > 0.f) {
		player.manaPool.current += dmg;

		if(player.manaPool.current > player.Full_maxmana)
			player.manaPool.current = player.Full_maxmana;
	}
}

static void ARX_DAMAGES_HealManaInter(Entity * io, float dmg) {
	
	if(!io || !(io->ioflags & IO_NPC))
		return;

	if(io == entities.player())
		ARX_DAMAGES_HealManaPlayer(dmg);

	if(io->_npcdata->lifePool.current <= 0.f)
		return;

	if(dmg > 0.f) {
		io->_npcdata->manaPool.current += dmg;

		if(io->_npcdata->manaPool.current > io->_npcdata->manaPool.max)
			io->_npcdata->manaPool.current = io->_npcdata->manaPool.max;
	}
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

void ARX_DAMAGES_DamageFIX(Entity * io, float dmg, EntityHandle source, bool isSpellHit)
{
	if(   !io
	   || !io->show
	   || !(io->ioflags & IO_FIX)
	   || (io->ioflags & IO_INVULNERABILITY)
	   || !io->script.valid
	) {
		return;
	}

	io->dmg_sum += dmg;
	
	Entity * sender = ValidIONum(source) ? entities[source] : NULL;
	
	GameDuration elapsed = g_gameTime.now() - io->ouch_time;
	if(elapsed > GameDurationMs(500)) {
		io->ouch_time = g_gameTime.now();
		SendIOScriptEvent(sender, io, SM_OUCH, getOuchEventParameter(io));
		io->dmg_sum = 0.f;
	}
	
	if(Random::getf(0.f, 100.f) > io->durability) {
		io->durability -= dmg * 0.5f;
	}
	
	if(io->durability <= 0.f) {
		io->durability = 0.f;
		SendIOScriptEvent(sender, io, SM_BREAK);
	} else {
		
		ScriptParameters parameters(dmg);
		if(source == EntityHandle_Player) {
			if(isSpellHit) {
				parameters.push_back("spell");
			} else {
				switch(ARX_EQUIPMENT_GetPlayerWeaponType()) {
					case WEAPON_BARE:
						parameters.push_back("bare");
						break;
					case WEAPON_DAGGER:
						parameters.push_back("dagger");
						break;
					case WEAPON_1H:
						parameters.push_back("1h");
						break;
					case WEAPON_2H:
						parameters.push_back("2h");
						break;
					case WEAPON_BOW:
						parameters.push_back("arrow");
						break;
					default: break;
				}
			}
		}
		
		SendIOScriptEvent(sender, io, SM_HIT, parameters);
		
	}
	
}

void ARX_DAMAGES_ForceDeath(Entity & io_dead, Entity * io_killer) {
	
	if(io_dead.mainevent == SM_DEAD) {
		return;
	}
	
	if(&io_dead == DRAGINTER) {
		Set_DragInter(NULL);
	}
	
	if(&io_dead == FlyingOverIO) {
		FlyingOverIO = NULL;
	}
	
	if(g_cameraEntity == &io_dead) {
		g_cameraEntity = NULL;
	}
	
	lightHandleDestroy(io_dead.dynlight);
	
	ARX_NPC_Behaviour_Reset(&io_dead);
	
	ARX_SPEECH_ReleaseIOSpeech(&io_dead);
	
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
		io_dead.animlayer[0].ctime = AnimationDurationMs(9999999);
		io_dead.animBlend.lastanimtime = 0;
	}
	
	if(io_dead.ioflags & IO_NPC) {
		io_dead._npcdata->weaponinhand = 0;
	}
	
	ARX_INTERACTIVE_DestroyDynamicInfo(&io_dead);
	
	ScriptParameters killer;
	if(io_killer == entities.player()) {
		killer.push_back("player");
	} else if(io_killer) {
		killer.push_back(io_killer->idString());
	}
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * ioo = entities[handle];
		
		if(ioo == &io_dead) {
			continue;
		}
		
		if(ioo && (ioo->ioflags & IO_NPC)) {
			if(ValidIONum(ioo->targetinfo))
				if(entities[ioo->targetinfo] == &io_dead) {
					Stack_SendIOScriptEvent(&io_dead, entities[handle], "target_death", killer);
					ioo->targetinfo = EntityHandle(TARGET_NONE);
					ioo->_npcdata->reachedtarget = 0;
				}

			if(ValidIONum(ioo->_npcdata->pathfind.truetarget))
				if(entities[ioo->_npcdata->pathfind.truetarget] == &io_dead) {
					Stack_SendIOScriptEvent(&io_dead, entities[handle], "target_death", killer);
					ioo->_npcdata->pathfind.truetarget = EntityHandle(TARGET_NONE);
					ioo->_npcdata->reachedtarget = 0;
				}
		}
	}

	io_dead.animlayer[1].cur_anim = NULL;
	io_dead.animlayer[2].cur_anim = NULL;
	io_dead.animlayer[3].cur_anim = NULL;

	if(io_dead.ioflags & IO_NPC) {
		io_dead._npcdata->lifePool.current = 0;

		if(io_dead._npcdata->weapon) {
			Entity * ioo = io_dead._npcdata->weapon;
			if(ValidIOAddress(ioo)) {
				ioo->show = SHOW_FLAG_IN_SCENE;
				ioo->ioflags |= IO_NO_NPC_COLLIDE;
				ioo->pos = ioo->obj->vertexWorldPositions[ioo->obj->origin].v;
				// TODO old broken code suggested that physics sim might be enabled here
			}
		}
	}
	
}

static void ARX_DAMAGES_PushIO(Entity * io_target, EntityHandle source, float power) {
	
	if(power > 0.f && ValidIONum(source)) {
		
		power *= 0.05f;
		Entity * io = entities[source];
		Vec3f vect = io_target->pos - io->pos;
		vect = glm::normalize(vect);
		vect *= power;
		arx_assert(isallfinite(vect));
		
		if(io_target == entities.player()) {
			PUSH_PLAYER_FORCE = vect; // TODO why not +=?
		} else {
			io_target->move += vect;
		}
		
	}
	
}

void ARX_DAMAGES_DealDamages(EntityHandle target, float dmg, EntityHandle source, DamageType flags, Vec3f * pos) {
	if(!ValidIONum(target) || !ValidIONum(source)) {
		return;
	}

	Entity * io_target = entities[target];
	Entity * io_source = entities[source];
	float damagesdone;

	if(flags & DAMAGE_TYPE_PER_SECOND) {
		dmg = dmg * (g_gameTime.lastFrameDuration() / GameDurationMs(1000));
	}

	if(target == EntityHandle_Player) {
		
		if(flags & DAMAGE_TYPE_POISON) {
			if(Random::getf(0.f, 100.f) > player.m_miscFull.resistPoison) {
				damagesdone = dmg;
				player.poison += damagesdone;
			} else {
				damagesdone = 0;
			}
		} else {
			if(flags & DAMAGE_TYPE_DRAIN_MANA) {
				damagesdone = ARX_DAMAGES_DrainMana(io_target, dmg);
			} else {
				ARX_DAMAGES_DamagePlayerEquipment(dmg);
				damagesdone = ARX_DAMAGES_DamagePlayer(dmg, flags, source);
			}
		}
		
		if(flags & DAMAGE_TYPE_FIRE) {
			ARX_DAMAGES_IgnitIO(io_source, io_target, damagesdone);
		}
		
	} else {
		if(io_target->ioflags & IO_NPC) {
			
			if(flags & DAMAGE_TYPE_POISON) {
				
				if(Random::getf(0.f, 100.f) > io_target->_npcdata->resist_poison) {
					damagesdone = dmg;
					io_target->_npcdata->poisonned += damagesdone;
				} else {
					damagesdone = 0;
				}
				
			} else {
				
				if(flags & DAMAGE_TYPE_FIRE) {
					if(Random::getf(0.f, 100.f) <= io_target->_npcdata->resist_fire) {
						dmg = 0;
					}
					ARX_DAMAGES_IgnitIO(io_source, io_target, dmg);
				}
				
				if(flags & DAMAGE_TYPE_DRAIN_MANA) {
					damagesdone = ARX_DAMAGES_DrainMana(io_target, dmg);
				} else {
					damagesdone = ARX_DAMAGES_DamageNPC(io_target, dmg, source, true, pos);
				}
			}
		} else {
			return;
		}
	}

	if(flags & DAMAGE_TYPE_DRAIN_LIFE) {
		ARX_DAMAGES_HealInter(io_source, damagesdone);
	}

	if(flags & DAMAGE_TYPE_DRAIN_MANA) {
		ARX_DAMAGES_HealManaInter(io_source, damagesdone);
	}

	if(flags & DAMAGE_TYPE_PUSH) {
		ARX_DAMAGES_PushIO(io_target, source, damagesdone * (1.0f / 2));
	}
}

float ARX_DAMAGES_DamageNPC(Entity * io, float dmg, EntityHandle source, bool isSpellHit, const Vec3f * pos) {
	
	if(   !io
	   || !io->show
	   || !(io->ioflags & IO_NPC)
	   || (io->ioflags & IO_INVULNERABILITY)
	) {
		return 0.f;
	}

	float damagesdone = 0.f;

	if(io->_npcdata->lifePool.current <= 0.f) {
		if(source != EntityHandle_Player || ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
			if(dmg >= io->_npcdata->lifePool.max * 0.4f && pos)
				ARX_NPC_TryToCutSomething(io, pos);

			return damagesdone;
		}

		return damagesdone;
	}

	io->dmg_sum += dmg;
	
	GameDuration elapsed = g_gameTime.now() - io->ouch_time;
	
	if(elapsed > GameDurationMs(500)) {
		
		Entity * sender = ValidIONum(source) ? entities[source] : NULL;
		
		io->ouch_time = g_gameTime.now();
		
		ScriptParameters parameters = getOuchEventParameter(io);
		if(sender && sender->summoner == EntityHandle_Player) {
			sender = entities.player();
			parameters.push_back("summoned");
		}
		
		SendIOScriptEvent(sender, io, SM_OUCH, parameters);
		
		io->dmg_sum = 0.f;
		
		spells.endByTarget(io->index(), SPELL_CONFUSE);
	}
	
	if(dmg >= 0.f) {
		if(ValidIONum(source)) {
			Entity * pio = NULL;

			if(source == EntityHandle_Player) {
				if(ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
					pio = entities[player.equiped[EQUIP_SLOT_WEAPON]];

					if((pio && (pio->poisonous == 0 || pio->poisonous_count == 0)) || isSpellHit) {
						pio = NULL;
					}
				}
			} else {
				if(entities[source]->ioflags & IO_NPC) {
					pio = entities[source]->_npcdata->weapon;
					if(pio && (pio->poisonous == 0 || pio->poisonous_count == 0)) {
						pio = NULL;
					}
				}
			}

			if(!pio)
				pio = entities[source];

			if(pio && pio->poisonous && (pio->poisonous_count != 0)) {
				if(Random::getf(0.f, 100.f) > io->_npcdata->resist_poison) {
					io->_npcdata->poisonned += pio->poisonous;
				}

				if (pio->poisonous_count != -1)
					pio->poisonous_count--;
			}
		}

		if(io->script.valid) {
			if(source.handleData() >= EntityHandle_Player.handleData()) {
				
				ScriptParameters parameters(dmg);
				if(source == EntityHandle_Player) {
					if(isSpellHit) {
						parameters.push_back("spell");
					} else {
						switch (ARX_EQUIPMENT_GetPlayerWeaponType()) {
							case WEAPON_BARE:
								parameters.push_back("bare");
								break;
							case WEAPON_DAGGER:
								parameters.push_back("dagger");
								break;
							case WEAPON_1H:
								parameters.push_back("1h");
								break;
							case WEAPON_2H:
								parameters.push_back("2h");
								break;
							case WEAPON_BOW:
								parameters.push_back("arrow");
								break;
							default: break;
						}
					}
				}
				
				Entity * sender = ValidIONum(source) ? entities[source] : NULL;
				if(sender && sender->summoner == EntityHandle_Player) {
					sender = entities.player();
					parameters.push_back("summoned");
				}
				
				if(SendIOScriptEvent(sender, io, SM_HIT, parameters) != ACCEPT) {
					return damagesdone;
				}
				
			}
		}

		damagesdone = std::min(dmg, io->_npcdata->lifePool.current);
		io->_npcdata->lifePool.current -= dmg;
		
		float fHitFlash = 0;
		if(io->_npcdata->lifePool.current <= 0) {
			fHitFlash = 0;
		} else {
			fHitFlash = io->_npcdata->lifePool.current / io->_npcdata->lifePool.max;
		}
		g_hudRoot.hitStrengthGauge.requestFlash(fHitFlash);
		
		
		if(io->_npcdata->lifePool.current <= 0.f) {
			io->_npcdata->lifePool.current = 0.f;
			if(source != EntityHandle_Player || ValidIONum(player.equiped[EQUIP_SLOT_WEAPON])) {
				if((dmg >= io->_npcdata->lifePool.max * ( 1.0f / 2 )) && pos)
					ARX_NPC_TryToCutSomething(io, pos);
			}
			if(ValidIONum(source)) {
				long xp = io->_npcdata->xpvalue;
				ARX_DAMAGES_ForceDeath(*io, entities[source]);
				if(source == EntityHandle_Player || entities[source]->summoner == EntityHandle_Player) {
					ARX_PLAYER_Modify_XP(xp);
				}
			} else {
				ARX_DAMAGES_ForceDeath(*io, NULL);
			}
		}
		
	}
	
	return damagesdone;
}

void ARX_DAMAGES_Reset() {
	memset(g_damages, 0, sizeof(DAMAGE_INFO) * MAX_DAMAGES);
}

extern TextureContainer * TC_fire2;

static void ARX_DAMAGES_AddVisual(DAMAGE_INFO & di, const Vec3f & pos, float dmg, Entity * io) {
	
	arx_assert(io);
	
	if(!(di.params.type & DAMAGE_TYPE_FAKEFIRE)) {
		return;
	}
	
	GameInstant now = g_gameTime.now();
	if(di.lastupd + GameDurationMs(200) < now) {
		di.lastupd = now;
		if(di.params.type & DAMAGE_TYPE_MAGICAL) {
			ARX_SOUND_PlaySFX(g_snd.SPELL_MAGICAL_HIT, &pos, Random::getf(0.8f, 1.2f));
		} else {
			ARX_SOUND_PlaySFX(g_snd.SPELL_FIRE_HIT, &pos, Random::getf(0.8f, 1.2f));
		}
	}
	
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
		pd->tc = TC_fire2;
		pd->m_rotation = Random::getf(-0.1f, 0.1f);
	}
}

// source = -1 no source but valid pos
// source = 0  player
// source > 0  IO
static void ARX_DAMAGES_UpdateDamage(DamageHandle j, GameInstant now) {
	
	ARX_PROFILE_FUNC();
	
	DAMAGE_INFO & damage = g_damages[j.handleData()];
	
	if(!damage.exist) {
		return;
	}
		
	if(damage.params.flags & DAMAGE_FLAG_FOLLOW_SOURCE) {
		if(damage.params.source == EntityHandle_Player) {
			damage.params.pos = player.pos;
		} else if(ValidIONum(damage.params.source)) {
			damage.params.pos = entities[damage.params.source]->pos;
		}
	}
	
	float dmg;
	if(damage.params.flags & DAMAGE_NOT_FRAME_DEPENDANT) {
		dmg = damage.params.damages;
	} else if(damage.params.duration == GameDuration::ofRaw(-1)) {
		dmg = damage.params.damages;
	} else {
		GameDuration FD = g_gameTime.lastFrameDuration();
		
		if(now > damage.start_time + damage.params.duration) {
			FD -= damage.start_time + damage.params.duration - now;
		}
		
		dmg = damage.params.damages * (FD / GameDurationMs(1000));
	}
	
	bool validsource = ValidIONum(damage.params.source);
	float divradius = 1.f / damage.params.radius;
	
	// checking for IO damages
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];
		
		if(io
		   && (io->gameFlags & GFLAG_ISINTREATZONE)
		   && (io->show == SHOW_FLAG_IN_SCENE)
		   && (damage.params.source != handle || !(damage.params.flags & DAMAGE_FLAG_DONT_HURT_SOURCE))
		){
			if(io->ioflags & IO_NPC) {
				if(   handle != EntityHandle_Player
				   && damage.params.source != EntityHandle_Player
				   && validsource
				   && HaveCommonGroup(io, entities[damage.params.source])
				) {
					continue;
				}
				
				Sphere sphere;
				sphere.origin = damage.params.pos;
				sphere.radius = damage.params.radius - 10.f;
				
				if(CheckIOInSphere(sphere, *io, true)) {
					Vec3f sub = io->pos + Vec3f(0.f, -60.f, 0.f);
					
					float dist = fdist(damage.params.pos, sub);
					
					if(damage.params.type & DAMAGE_TYPE_FIELD) {
						GameDuration elapsed = g_gameTime.now() - io->collide_door_time;
						if(elapsed > GameDurationMs(500)) {
							io->collide_door_time = g_gameTime.now();
							
							ScriptParameters parameters;
							if(damage.params.type & DAMAGE_TYPE_COLD) {
								parameters = "cold";
							} else if(damage.params.type & DAMAGE_TYPE_FIRE) {
								parameters = "fire";
							}
							
							SendIOScriptEvent(NULL, io, SM_COLLIDE_FIELD, parameters);
						}
					}
					
					switch(damage.params.area) {
						case DAMAGE_AREA: {
							float ratio = (damage.params.radius - dist) * divradius;
							ratio = glm::clamp(ratio, 0.f, 1.f);
							dmg = dmg * ratio + 1.f;
						}
						break;
						case DAMAGE_AREAHALF: {
							float ratio = (damage.params.radius - dist * 0.5f) * divradius;
							ratio = glm::clamp(ratio, 0.f, 1.f);
							dmg = dmg * ratio + 1.f;
						}
						break;
						case DAMAGE_FULL:
						break;
					}
					
					if(dmg <= 0.f)
						continue;
					
					if(   (damage.params.flags & DAMAGE_FLAG_ADD_VISUAL_FX)
					   && (io->ioflags & IO_NPC)
					   && (io->_npcdata->lifePool.current > 0.f)
					) {
						ARX_DAMAGES_AddVisual(damage, sub, dmg, io);
					}
					
					if(damage.params.type & DAMAGE_TYPE_DRAIN_MANA) {
						float manadrained;
						
						if(handle == EntityHandle_Player) {
							manadrained = std::min(dmg, player.manaPool.current);
							player.manaPool.current -= manadrained;
						} else {
							manadrained = dmg;
							
							if(io && io->_npcdata) {
								manadrained = std::min(dmg, io->_npcdata->manaPool.current);
								io->_npcdata->manaPool.current -= manadrained;
							}
						}
						
						if (damage.params.source == EntityHandle_Player) {
							player.manaPool.current = std::min(player.manaPool.current + manadrained, player.Full_maxmana);
						} else {
							if(ValidIONum(damage.params.source) && (entities[damage.params.source]->_npcdata)) {
								entities[damage.params.source]->_npcdata->manaPool.current = std::min(entities[damage.params.source]->_npcdata->manaPool.current + manadrained, entities[damage.params.source]->_npcdata->manaPool.max);
							}
						}
					} else {
						float damagesdone;
						
						// TODO copy-paste
						if(handle == EntityHandle_Player) {
							if(damage.params.type & DAMAGE_TYPE_POISON) {
								if(Random::getf(0.f, 100.f) > player.m_miscFull.resistPoison) {
									// Failed Saving Throw
									damagesdone = dmg;
									player.poison += damagesdone;
								} else {
									damagesdone = 0;
								}
							} else {
								if(   (damage.params.type & DAMAGE_TYPE_MAGICAL)
								   && !(damage.params.type & DAMAGE_TYPE_FIRE)
								   && !(damage.params.type & DAMAGE_TYPE_COLD)
								) {
									dmg -= player.m_miscFull.resistMagic * 0.01f * dmg;
									dmg = std::max(0.0f, dmg);
								}
								if(damage.params.type & DAMAGE_TYPE_FIRE) {
									dmg = ARX_SPELLS_ApplyFireProtection(entities.player(), dmg);
									ARX_DAMAGES_IgnitIO(entities.get(damage.params.source), entities.player(), dmg);
								}
								if(damage.params.type & DAMAGE_TYPE_COLD) {
									dmg = ARX_SPELLS_ApplyColdProtection(entities.player(), dmg);
								}
								damagesdone = ARX_DAMAGES_DamagePlayer(dmg, damage.params.type, damage.params.source);
							}
						} else {
							if(   (io->ioflags & IO_NPC)
							   && (damage.params.type & DAMAGE_TYPE_POISON)
							) {
								if(Random::getf(0.f, 100.f) > io->_npcdata->resist_poison) {
									// Failed Saving Throw
									damagesdone = dmg;
									io->_npcdata->poisonned += damagesdone;
								} else {
									damagesdone = 0;
								}
							} else {
								if(damage.params.type & DAMAGE_TYPE_FIRE) {
									dmg = ARX_SPELLS_ApplyFireProtection(io, dmg);
									ARX_DAMAGES_IgnitIO(entities.get(damage.params.source), io, dmg);
								}
								if(   (damage.params.type & DAMAGE_TYPE_MAGICAL)
								   && !(damage.params.type & DAMAGE_TYPE_FIRE)
								   && !(damage.params.type & DAMAGE_TYPE_COLD)
								) {
									dmg -= io->_npcdata->resist_magic * 0.01f * dmg;
									dmg = std::max(0.0f, dmg);
								}
								if(damage.params.type & DAMAGE_TYPE_COLD) {
									dmg = ARX_SPELLS_ApplyColdProtection(io, dmg);
								}
								damagesdone = ARX_DAMAGES_DamageNPC(io, dmg, damage.params.source, true, &damage.params.pos);
							}
							if(damagesdone > 0 && (damage.params.flags & DAMAGE_SPAWN_BLOOD)) {
								ARX_PARTICLES_Spawn_Blood(damage.params.pos, damagesdone, damage.params.source);
							}
						}
						if(damage.params.type & DAMAGE_TYPE_DRAIN_LIFE) {
							if(ValidIONum(damage.params.source))
								ARX_DAMAGES_HealInter(entities[damage.params.source], damagesdone);
						}
					}
				}
			} else if((io->ioflags & IO_FIX) && !(damage.params.type & DAMAGE_TYPE_NO_FIX)) {
				Sphere sphere;
				sphere.origin = damage.params.pos;
				sphere.radius = damage.params.radius + 15.f;
				
				if(CheckIOInSphere(sphere, *io)) {
					ARX_DAMAGES_DamageFIX(io, dmg, damage.params.source, true);
				}
			}
		}
	}
	
	if(damage.params.duration == GameDuration::ofRaw(-1))
		damage.exist = false;
	else if(now > damage.start_time + damage.params.duration)
		damage.exist = false;
}

void ARX_DAMAGES_UpdateAll() {
	
	ARX_PROFILE_FUNC();
	
	for (size_t j = 0; j < MAX_DAMAGES; j++)
		ARX_DAMAGES_UpdateDamage(DamageHandle(j), g_gameTime.now());
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

bool ARX_DAMAGES_TryToDoDamage(const Vec3f & pos, float dmg, float radius, EntityHandle source)
{
	bool ret = false;

	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];

		if(io != NULL
		   && (entities[handle]->gameFlags & GFLAG_ISINTREATZONE)
		   && io->show == SHOW_FLAG_IN_SCENE
		   && source != handle
		) {
			float threshold;
			float rad = radius + 5.f;

			if(io->ioflags & IO_FIX) {
				threshold = 510;
				rad += 10.f;
			} else if(io->ioflags & IO_NPC) {
				threshold = 250;
			} else {
				threshold = 350;
			}

			if(closerThan(pos, io->pos, threshold) && SphereInIO(io, Sphere(pos, rad))) {
				if(io->ioflags & IO_NPC) {
					if(ValidIONum(source))
						ARX_EQUIPMENT_ComputeDamages(entities[source], io, 1.f);

					ret = true;
				}

				if(io->ioflags & IO_FIX) {
					ARX_DAMAGES_DamageFIX(io, dmg, source, false);
					ret = true;
				}
			}
		}
	}

	return ret;
}

void CheckForIgnition(const Sphere & sphere, bool mode, long flag) {
	
	if(!(flag & 1))
		for(size_t i = 0; i < g_staticLightsMax; i++) {
			EERIE_LIGHT * el = g_staticLights[i];

			if(el == NULL)
				continue;

			if((el->extras & EXTRAS_EXTINGUISHABLE) && (el->extras & (EXTRAS_SEMIDYNAMIC | EXTRAS_SPAWNFIRE | EXTRAS_SPAWNSMOKE)))
			{
				if((el->extras & EXTRAS_FIREPLACE) && (flag & 2))
					continue;

				if(!fartherThan(sphere.origin, el->pos, sphere.radius)) {
					if(mode) {
						if (!(el->extras & EXTRAS_NO_IGNIT))
							el->m_ignitionStatus = true;
					} else {
						el->m_ignitionStatus = false;
					}
				}

			}
		}

	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * io = entities[handle];

		if(   io
		   && io->show == SHOW_FLAG_IN_SCENE
		   && io->obj
		   && !(io->ioflags & IO_UNDERWATER)
		   && io->obj->fastaccess.fire != ActionPoint()
		) {
			if(closerThan(sphere.origin, actionPointPosition(io->obj, io->obj->fastaccess.fire), sphere.radius)) {

				if(mode && io->ignition <= 0) {
					io->ignition = 1;
				} else if(!mode && io->ignition > 0) {
					io->ignition = 0;
					lightHandleDestroy(io->ignit_light);
					
					ARX_SOUND_Stop(io->ignit_sound);
					io->ignit_sound = audio::SourcedSample();
				}
			}
		}
	}
}

void DoSphericDamage(const Sphere & sphere, float dmg, DamageArea flags, DamageType typ, EntityHandle numsource) {
	
	if(sphere.radius <= 0.f)
		return;
	
	float rad = 1.f / sphere.radius;
	bool validsource = ValidIONum(numsource);
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * ioo = entities[handle];
		
		if(!ioo || handle == numsource || !ioo->obj)
			continue;
			
		if(   handle != EntityHandle_Player
		   && numsource != EntityHandle_Player
		   && validsource
		   && HaveCommonGroup(ioo, entities[numsource])
		) {
			continue;
		}
		
		if((ioo->ioflags & IO_CAMERA) || (ioo->ioflags & IO_MARKER))
			continue;
		
		long count = 0;
		long count2 = 0;
		float mindist = std::numeric_limits<float>::max();
		
		for(size_t k = 0; k < ioo->obj->vertexlist.size(); k += 1) {
			if(ioo->obj->vertexlist.size() < 120) {
				for(size_t kk = 0; kk < ioo->obj->vertexlist.size(); kk += 1) {
					if(kk != k) {
						Vec3f posi = (entities[handle]->obj->vertexWorldPositions[k].v
						             + entities[handle]->obj->vertexWorldPositions[kk].v) * 0.5f;
						float dist = fdist(sphere.origin, posi);
						if(dist <= sphere.radius) {
							count2++;
							if(dist < mindist)
								mindist = dist;
						}
					}
				}
			}
			
			{
			float dist = fdist(sphere.origin, entities[handle]->obj->vertexWorldPositions[k].v);
			
			if(dist <= sphere.radius) {
				count++;
				
				if(dist < mindist)
					mindist = dist;
			}
			}
		}
		
		float ratio = glm::max(count, count2) / (float(ioo->obj->vertexlist.size()) / 2.f);
		
		if(ratio > 2.f)
			ratio = 2.f;
		
		if(ioo->ioflags & IO_NPC) {
			if(mindist <= sphere.radius + 30.f) {
				switch (flags) {
					case DAMAGE_AREA:
						dmg = dmg * (sphere.radius + 30 - mindist) * rad;
						break;
					case DAMAGE_AREAHALF:
						dmg = dmg * (sphere.radius + 30 - mindist * 0.5f) * rad;
						break;
					case DAMAGE_FULL: break;
				}
				
				if(handle == EntityHandle_Player) {
					if(typ & DAMAGE_TYPE_FIRE) {
						dmg = ARX_SPELLS_ApplyFireProtection(ioo, dmg);
						ARX_DAMAGES_IgnitIO(entities.get(numsource), entities.player(), dmg);
					}
					
					if(typ & DAMAGE_TYPE_COLD) {
						dmg = ARX_SPELLS_ApplyColdProtection(ioo, dmg);
					}
					
					ARX_DAMAGES_DamagePlayer(dmg, typ, numsource);
					ARX_DAMAGES_DamagePlayerEquipment(dmg);
				} else {
					if(typ & DAMAGE_TYPE_FIRE) {
						dmg = ARX_SPELLS_ApplyFireProtection(ioo, dmg * ratio);
						ARX_DAMAGES_IgnitIO(entities.get(numsource), ioo, dmg);
					}
					
					if(typ & DAMAGE_TYPE_COLD) {
						dmg = ARX_SPELLS_ApplyColdProtection(ioo, dmg * ratio);
					}
					
					ARX_DAMAGES_DamageNPC(ioo, dmg * ratio, numsource, true, &sphere.origin);
				}
			}
		} else {
			if(mindist <= sphere.radius + 30.f) {
				if(typ & DAMAGE_TYPE_FIRE) {
					dmg = ARX_SPELLS_ApplyFireProtection(ioo, dmg * ratio);
					ARX_DAMAGES_IgnitIO(entities.get(numsource), entities[handle], dmg);
				}
				
				if(typ & DAMAGE_TYPE_COLD) {
					dmg = ARX_SPELLS_ApplyColdProtection(ioo, dmg * ratio);
				}
				
				if(entities[handle]->ioflags & IO_FIX)
					ARX_DAMAGES_DamageFIX(entities[handle], dmg * ratio, numsource, true);
			}
		}
	}
	
	if(typ & DAMAGE_TYPE_FIRE) {
		CheckForIgnition(sphere, true, 0);
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
	if(!io)
		return;

	if(Random::getf(0.f, 100.f) > io->durability) {
		ARX_DAMAGES_DurabilityLoss(io, ratio);
	}
}

void ARX_DAMAGES_DurabilityLoss(Entity * io, float loss) {
	
	arx_assert(io);
	
	io->durability -= loss;
	
	if(io->durability <= 0) {
		SendIOScriptEvent(NULL, io, SM_BREAK);
	}
	
}

void ARX_DAMAGES_DamagePlayerEquipment(float damages) {
	
	float ratio = damages * 0.05f;
	if(ratio > 1.f) {
		ratio = 1.f;
	}
	
	for(size_t i = 0; i < MAX_EQUIPED; i++) {
		Entity * todamage = entities.get(player.equiped[i]);
		if(todamage) {
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
	
	for(size_t i = 0; i < MAX_DAMAGES; i++) {
		if(!g_damages[i].exist)
			continue;
		
		DAMAGE_INFO & d = g_damages[i];
		
		drawLineSphere(Sphere(d.params.pos, d.params.radius), Color::red);
	}
}
