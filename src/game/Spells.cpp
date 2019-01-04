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

#include "game/Spells.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <utility>

#include <boost/container/flat_set.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "animation/Animation.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "core/Localisation.h"

#include "game/Damage.h"
#include "game/EntityManager.h"
#include "game/Equipment.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Inventory.h"
#include "game/spell/FlyingEye.h"
#include "game/spell/Cheat.h"
#include "game/effect/Quake.h"

#include "game/magic/spells/SpellsLvl01.h"
#include "game/magic/spells/SpellsLvl02.h"
#include "game/magic/spells/SpellsLvl03.h"
#include "game/magic/spells/SpellsLvl04.h"
#include "game/magic/spells/SpellsLvl05.h"
#include "game/magic/spells/SpellsLvl06.h"
#include "game/magic/spells/SpellsLvl07.h"
#include "game/magic/spells/SpellsLvl08.h"
#include "game/magic/spells/SpellsLvl09.h"
#include "game/magic/spells/SpellsLvl10.h"

#include "gui/Speech.h"
#include "gui/Menu.h"
#include "gui/Interface.h"
#include "gui/MiniMap.h"
#include "gui/Notification.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "graphics/Draw.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/effects/SpellEffects.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleSystem.h"
#include "graphics/particle/MagicFlare.h"

#include "graphics/spells/Spells05.h"

#include "input/Input.h"

#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Vector.h"

#include "physics/Collisions.h"

#include "platform/Platform.h"
#include "platform/profiler/Profiler.h"

#include "scene/Light.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

#include "script/Script.h"

bool WILLRETURNTOFREELOOK = false;
bool GLOBAL_MAGIC_MODE = true;

short ARX_FLARES_broken(1);

long snip = 0;
static Vec2f g_LastFlarePosition;
static PlatformInstant g_LastFlareTime = 0;

SpellManager spells;

void SpellManager::init() {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		m_spells[i] = NULL;
	}
	
	spellRecognitionInit();
	
	RuneInfosFill();
	ARX_SPELLS_Init_Rects();
}

void SpellManager::clearAll() {
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		delete m_spells[i];
		m_spells[i] = NULL;
	}
}

SpellBase * SpellManager::operator[](const SpellHandle handle) {
	return m_spells[handle.handleData()];
}

static void SPELLEND_Notify(const SpellBase & spell);

void SpellManager::endSpell(SpellBase * spell)
{
	spell->m_duration = 0;
	spell->m_hasDuration = true;
}

void SpellManager::endByCaster(EntityHandle caster) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = m_spells[i];
		
		if(spell && spell->m_caster == caster) {
			spells.endSpell(spell);
		}
	}
}

void SpellManager::endByTarget(EntityHandle target, SpellType type) {
	SpellBase * spell = spells.getSpellOnTarget(target, type);
	if(spell) {
		spells.endSpell(spell);
	}
}

void SpellManager::endByType(SpellType type)
{
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = m_spells[i];
		
		if(spell && spell->m_type == type) {
			spells.endSpell(spell);
		}
	}
}

void SpellManager::endByCaster(EntityHandle caster, SpellType type) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = m_spells[i];
		
		if(spell && spell->m_type == type && spell->m_caster == caster) {
			spells.endSpell(spell);
			return;
		}
	}
}

bool SpellManager::ExistAnyInstanceForThisCaster(SpellType typ, EntityHandle caster) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		const SpellBase * spell = m_spells[i];
		
		if(spell && spell->m_type == typ && spell->m_caster == caster) {
			return true;
		}
	}
	
	return false;
}

SpellBase * SpellManager::getSpellByCaster(EntityHandle caster, SpellType type) {
	
	if(caster == EntityHandle())
		return NULL;
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = m_spells[i];
		if(!spell)
			continue;
		
		if(spell->m_type != type)
			continue;
		
		if(spell->m_caster == caster)
			return spell;
	}
	
	return NULL;
}

SpellBase * SpellManager::getSpellOnTarget(EntityHandle target, SpellType type)
{
	if(target == EntityHandle())
		return NULL;
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = m_spells[i];
		if(!spell)
			continue;
		
		if(spell->m_type != type)
			continue;
		
		if(std::find(spell->m_targets.begin(), spell->m_targets.end(), target) != spell->m_targets.end()) {
			return spell;
		}
	}
	
	return NULL;
}

void SpellManager::replaceCaster(EntityHandle oldCaster, EntityHandle newCaster) {
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = m_spells[i];
		
		if(spell && spell->m_caster == oldCaster) {
			spell->m_caster = newCaster;
		}
	}
}

void SpellManager::removeTarget(Entity * io) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = m_spells[i];
		if(!spell) {
			continue;
		}
		spell->m_targets.erase(std::remove(spell->m_targets.begin(), spell->m_targets.end(), io->index()),
		                       spell->m_targets.end());
	}
	
}

bool SpellManager::hasFreeSlot()
{
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase * spell = m_spells[i];
		
		if(!spell) {
			return true;
		}
	}
	return false;
}

void SpellManager::addSpell(SpellBase * spell)
{
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(!m_spells[i]) {
			m_spells[i] = spell;
			spell->m_thisHandle = SpellHandle(i);
			return;
		}
	}
}

void SpellManager::freeSlot(SpellBase * spell)
{
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(m_spells[i] == spell) {
			delete m_spells[i];
			m_spells[i] = NULL;
			return;
		}
	}
}

static const char * MakeSpellName(SpellType num) {
	
	switch(num) {
		// Level 1
		case SPELL_MAGIC_SIGHT           : return "magic_sight";
		case SPELL_MAGIC_MISSILE         : return "magic_missile";
		case SPELL_IGNIT                 : return "ignit";
		case SPELL_DOUSE                 : return "douse";
		case SPELL_ACTIVATE_PORTAL       : return "activate_portal";
		// Level 2
		case SPELL_HEAL                  : return "heal";
		case SPELL_DETECT_TRAP           : return "detect_trap";
		case SPELL_ARMOR                 : return "armor";
		case SPELL_LOWER_ARMOR           : return "lower_armor";
		case SPELL_HARM                  : return "harm";
		// Level 3
		case SPELL_SPEED                 : return "speed";
		case SPELL_DISPELL_ILLUSION      : return "dispell_illusion";
		case SPELL_FIREBALL              : return "fireball";
		case SPELL_CREATE_FOOD           : return "create_food";
		case SPELL_ICE_PROJECTILE        : return "ice_projectile";
		// Level 4
		case SPELL_BLESS                 : return "bless";
		case SPELL_DISPELL_FIELD         : return "dispell_field";
		case SPELL_FIRE_PROTECTION       : return "fire_protection";
		case SPELL_TELEKINESIS           : return "telekinesis";
		case SPELL_CURSE                 : return "curse";
		case SPELL_COLD_PROTECTION       : return "cold_protection";
		// Level 5
		case SPELL_RUNE_OF_GUARDING      : return "rune_of_guarding";
		case SPELL_LEVITATE              : return "levitate";
		case SPELL_CURE_POISON           : return "cure_poison";
		case SPELL_REPEL_UNDEAD          : return "repel_undead";
		case SPELL_POISON_PROJECTILE     : return "poison_projectile";
		// Level 6
		case SPELL_RISE_DEAD             : return "raise_dead";
		case SPELL_PARALYSE              : return "paralyse";
		case SPELL_CREATE_FIELD          : return "create_field";
		case SPELL_DISARM_TRAP           : return "disarm_trap";
		case SPELL_SLOW_DOWN             : return "slowdown";
		// Level 7
		case SPELL_FLYING_EYE            : return "flying_eye";
		case SPELL_FIRE_FIELD            : return "fire_field";
		case SPELL_ICE_FIELD             : return "ice_field";
		case SPELL_LIGHTNING_STRIKE      : return "lightning_strike";
		case SPELL_CONFUSE               : return "confuse";
		// Level 8
		case SPELL_INVISIBILITY          : return "invisibility";
		case SPELL_MANA_DRAIN            : return "mana_drain";
		case SPELL_EXPLOSION             : return "explosion";
		case SPELL_ENCHANT_WEAPON        : return "enchant_weapon";
		case SPELL_LIFE_DRAIN            : return "life_drain";
		// Level 9
		case SPELL_SUMMON_CREATURE       : return "summon_creature";
		case SPELL_FAKE_SUMMON           : return "fake_summon";
		case SPELL_NEGATE_MAGIC          : return "negate_magic";
		case SPELL_INCINERATE            : return "incinerate";
		case SPELL_MASS_PARALYSE         : return "mass_paralyse";
		// Level 10
		case SPELL_MASS_LIGHTNING_STRIKE : return "mass_lightning_strike";
		case SPELL_CONTROL_TARGET        : return "control";
		case SPELL_FREEZE_TIME           : return "freeze_time";
		case SPELL_MASS_INCINERATE       : return "mass_incinerate";
		default :
			return NULL;
	}
}

static void SPELLCAST_Notify(const SpellBase & spell) {
	
	EntityHandle source = spell.m_caster;
	
	const char * spellName = MakeSpellName(spell.m_type);
	if(!spellName)
		return;
	
	Entity * sender = (source != EntityHandle()) ? entities[source] : NULL;
	ScriptParameters parameters;
	parameters.push_back(spellName);
	parameters.push_back(long(spell.m_level));
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		if(entities[handle] != NULL) {
			SendIOScriptEvent(sender, entities[handle], SM_SPELLCAST, parameters);
		}
	}
	
}

static void SPELLCAST_NotifyOnlyTarget(const SpellBase & spell) {
	
	if(!ValidIONum(spell.m_target))
		return;
	
	EntityHandle source = spell.m_caster;
	const char * spellName = MakeSpellName(spell.m_type);
	
	if(spellName) {
		Entity * sender = (source != EntityHandle()) ? entities[source] : NULL;
		ScriptParameters parameters;
		parameters.push_back(spellName);
		parameters.push_back(long(spell.m_level));
		SendIOScriptEvent(sender, entities[spell.m_target], SM_SPELLCAST, parameters);
	}
}

static void SPELLEND_Notify(const SpellBase & spell) {
	
	EntityHandle source = spell.m_caster;
	const char * spellName = MakeSpellName(spell.m_type);

	if(spell.m_type == SPELL_CONFUSE) {
		if(ValidIONum(spell.m_target) && spellName) {
			Entity * sender = ValidIONum(source) ? entities[source] : NULL;
			ScriptParameters parameters;
			parameters.push_back(spellName);
			parameters.push_back(long(spell.m_level));
			SendIOScriptEvent(sender, entities[spell.m_target], SM_SPELLEND, parameters);
		}
		return;
	}
	
	// we only notify player spells end.
	if(!spellName) {
		return;
	}
	
	Entity * sender = ValidIONum(source) ? entities[source] : NULL;
	ScriptParameters parameters;
	parameters.push_back(spellName);
	parameters.push_back(long(spell.m_level));
	
	for(size_t i = 0; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		if(entities[handle]) {
			SendIOScriptEvent(sender, entities[handle], SM_SPELLEND, parameters);
		}
	}
	
}

//! Plays the sound of Fizzling spell
void ARX_SPELLS_Fizzle(SpellBase * spell) {
	
	if(ValidIONum(spell->m_caster)) {
		ARX_SOUND_PlaySFX(g_snd.MAGIC_FIZZLE, &spell->m_caster_pos);
	}
}


void ARX_SPELLS_ManageMagic() {
	
	arx_assert(entities.player());
	Entity * io = entities.player();
	
	const ANIM_HANDLE * anim = io->animlayer[1].cur_anim;
	
	if(   anim == io->anims[ANIM_BARE_UNREADY]
	   || anim == io->anims[ANIM_DAGGER_UNREADY_PART_1]
	   || anim == io->anims[ANIM_1H_UNREADY_PART_1]
	   || anim == io->anims[ANIM_2H_UNREADY_PART_1]
	   || anim == io->anims[ANIM_MISSILE_UNREADY_PART_1]
	   || anim == io->anims[ANIM_DAGGER_UNREADY_PART_2]
	   || anim == io->anims[ANIM_1H_UNREADY_PART_2]
	   || anim == io->anims[ANIM_2H_UNREADY_PART_2]
	   || anim == io->anims[ANIM_MISSILE_UNREADY_PART_2]
	) {
		return;
	}

	snip++;

	if(   !(player.m_currentMovement & PLAYER_CROUCH)
	   && !BLOCK_PLAYER_CONTROLS
	   && GInput->actionPressed(CONTROLS_CUST_MAGICMODE)
	   && !player.m_paralysed
	) {
		if(player.Interface & INTER_COMBATMODE) {
			WILLRETURNTOCOMBATMODE = true;

			ARX_INTERFACE_setCombatMode(COMBAT_MODE_OFF);

			ResetAnim(io->animlayer[1]);
			io->animlayer[1].flags &= ~EA_LOOP;
		}

		if(TRUE_PLAYER_MOUSELOOK_ON) {
			WILLRETURNTOFREELOOK = true;
			TRUE_PLAYER_MOUSELOOK_ON = false;
		}

		if(player.doingmagic != 2) {
			player.doingmagic = 2;
			if(io->anims[ANIM_CAST_START]) {
				changeAnimation(io, 1, io->anims[ANIM_CAST_START]);
				MAGICMODE = true;
			}
		}
		
		if(snip >= 2) {
			if(!eeMousePressed1() && ARX_FLARES_broken == 0) {
				ARX_FLARES_broken = 2;
				MagicFlareChangeColor();
			}
			
			if(eeMousePressed1()) {
				Vec2f pos = Vec2f(DANAEMouse);
				if(TRUE_PLAYER_MOUSELOOK_ON) {
					pos = Vec2f(MemoMouse);
				}
				
				PlatformInstant now = g_platformTime.frameStart();
				
				const PlatformDuration interval = PlatformDurationMs(1000 / 60);
				
				if(ARX_FLARES_broken) {
					g_LastFlarePosition = pos;
					g_LastFlareTime = now - interval;
				}
				
				if(now - g_LastFlareTime >= interval) {
					
					if(glm::distance(pos, g_LastFlarePosition) > 14 * g_sizeRatio.y) {
						FlareLine(g_LastFlarePosition, pos);
						g_LastFlarePosition = pos;
					}
					
					if(Random::getf() > 0.6f)
						AddFlare(pos, 1.f, -1);
					else
						AddFlare(pos, 1.f, 3);
					
					g_LastFlareTime = now - std::min(now - g_LastFlareTime - interval, interval);
				}
				
				ARX_FLARES_broken = 0;
				
				if(!ARX_SOUND_IsPlaying(player.magic_draw)) {
					player.magic_draw = ARX_SOUND_PlaySFX_loop(g_snd.MAGIC_DRAW_LOOP, NULL, 1.f);
				}
				
			} else {
				ARX_SOUND_Stop(player.magic_draw);
				player.magic_draw = audio::SourcedSample();
			}
			
			snip = 0;
		}
	} else {
		ARX_FLARES_broken = 1;
		MagicFlareChangeColor();
		
		if(player.doingmagic != 0) {
			player.doingmagic = 0;
			if(io->anims[ANIM_CAST_END]) {
				changeAnimation(io, 1, io->anims[ANIM_CAST_END]);
			}
			ARX_FLARES_broken = 3;
		}
	}
	
	if(ARX_FLARES_broken == 3) {
		CheatDetectionReset();
		
		if(CurrSpellSymbol != 0) {
			if(!ARX_SPELLS_AnalyseSPELL()) {
				if(io->anims[ANIM_CAST]) {
					changeAnimation(io, 1, io->anims[ANIM_CAST]);
				}
			}
		}

		ARX_FLARES_broken = 1;

		if(WILLRETURNTOCOMBATMODE) {
			player.Interface |= INTER_COMBATMODE;
			player.Interface |= INTER_NO_STRIKE;

			ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
			player.doingmagic = 0;
			WILLRETURNTOCOMBATMODE = false;

			TRUE_PLAYER_MOUSELOOK_ON = true;
			bRenderInCursorMode = false;
		}

		if(WILLRETURNTOFREELOOK) {
			TRUE_PLAYER_MOUSELOOK_ON = true;
			WILLRETURNTOFREELOOK = false;
		}

		ARX_SPELLS_ResetRecognition();
	} else if(ARX_FLARES_broken == 2) {

		if(!config.input.useAltRuneRecognition) {
			ARX_SPELLS_Analyse();
			if(!SpellMoves.empty())
				ARX_SPELLS_AnalyseSYMBOL();
		} else {
			ARX_SPELLS_Analyse_Alt();
		}
	
		ARX_FLARES_broken = 1;
	}
}

static bool CanPayMana(SpellBase * spell, float cost) {
	
	if(spell->m_flags & SPELLCAST_FLAG_NOMANA) {
		return true;
	}
	if(spell->m_caster == EntityHandle_Player) {
		if(player.manaPool.current < cost) {
			return false;
		}
		
		player.manaPool.current -= cost;
		return true;
	}
	
	if(ValidIONum(spell->m_caster)) {
		if(entities[spell->m_caster]->ioflags & IO_NPC) {
			if(entities[spell->m_caster]->_npcdata->manaPool.current < cost) {
				return false;
			}
			entities[spell->m_caster]->_npcdata->manaPool.current -= cost;
			return true;
		}
	}

	return false;
}

static EntityHandle TemporaryGetSpellTarget(const Vec3f & from) {
	
	float mindist = std::numeric_limits<float>::max();
	EntityHandle found = EntityHandle_Player;
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * e = entities[handle];
		
		if(e && e->ioflags & IO_NPC) {
			float dist = arx::distance2(from, e->pos);
			if(dist < mindist) {
				found = handle;
				mindist = dist;
			}
		}
	}
	
	return found;
}

struct TARGETING_SPELL {
	SpellType typ;
	SpellcastFlags flags;
	long level;
	EntityHandle target;
	GameDuration duration;
};

static TARGETING_SPELL t_spell;

long LOOKING_FOR_SPELL_TARGET = 0;
GameInstant LOOKING_FOR_SPELL_TARGET_TIME = 0;

void ARX_SPELLS_CancelSpellTarget() {
	t_spell.typ = SPELL_NONE;
	LOOKING_FOR_SPELL_TARGET = 0;
}

void ARX_SPELLS_LaunchSpellTarget(Entity * io) {
	if(io) {
		ARX_SPELLS_Launch(t_spell.typ, EntityHandle_Player, t_spell.flags, t_spell.level, io->index(),
		                  t_spell.duration);
	}
}

float ARX_SPELLS_ApplyFireProtection(Entity * io, float damages) {
	
	if(io) {
		
		SpellBase * spell = spells.getSpellOnTarget(io->index(), SPELL_FIRE_PROTECTION);
		if(spell) {
			damages *= glm::clamp(1.f - (spell->m_level * 0.1f), 0.f, 1.f);
		}
		
		if(io->ioflags & IO_NPC) {
			damages -= io->_npcdata->resist_fire * 0.01f * damages;
			if(damages < 0.f) {
				damages = 0.f;
			}
		}
		
	}
	
	return damages;
}

float ARX_SPELLS_ApplyColdProtection(Entity * io, float damages) {
	
	SpellBase * spell = spells.getSpellOnTarget(io->index(), SPELL_COLD_PROTECTION);
	if(spell) {
		damages *= glm::clamp(1.f - (spell->m_level * 0.1f), 0.f, 1.f);
	}
	
	return damages;
}

float ARX_SPELLS_GetManaCost(SpellType spell, float casterLevel) {
	
	// TODO this data should not be hardcoded
	
	switch(spell)  {
		
		default:                          return   0.f;
		
		case SPELL_TELEKINESIS:           return   0.001f;
		case SPELL_CURSE:                 return   0.001f;
		case SPELL_ARMOR:                 return   0.01f;
		case SPELL_LOWER_ARMOR:           return   0.01f;
		case SPELL_SPEED:                 return   0.01f;
		case SPELL_BLESS:                 return   0.01f;
		case SPELL_DETECT_TRAP:           return   0.03f;
		case SPELL_MAGIC_SIGHT:           return   0.3f;
		case SPELL_HARM:                  return   0.4f;
		case SPELL_MANA_DRAIN:            return   0.4f;
		case SPELL_IGNIT:                 return   1.f;
		case SPELL_DOUSE:                 return   1.f;
		case SPELL_FIRE_PROTECTION:       return   1.f;
		case SPELL_COLD_PROTECTION:       return   1.f;
		case SPELL_LEVITATE:              return   1.f;
		case SPELL_CREATE_FIELD:          return   1.2f;
		case SPELL_SLOW_DOWN:             return   1.2f;
		case SPELL_ACTIVATE_PORTAL:       return   2.f;
		case SPELL_NEGATE_MAGIC:          return   2.f;
		case SPELL_INVISIBILITY:          return   3.f;
		case SPELL_LIFE_DRAIN:            return   3.f;
		case SPELL_HEAL:                  return   4.f;
		case SPELL_FLYING_EYE:            return   4.f;
		case SPELL_CREATE_FOOD:           return   5.f;
		case SPELL_DISPELL_ILLUSION:      return   7.f;
		case SPELL_DISPELL_FIELD:         return   7.f;
		case SPELL_RUNE_OF_GUARDING:      return   9.f;
		case SPELL_CURE_POISON:           return  10.f;
		case SPELL_RISE_DEAD:             return  12.f;
		case SPELL_DISARM_TRAP:           return  15.f;
		case SPELL_FIRE_FIELD:            return  15.f;
		case SPELL_ICE_FIELD:             return  15.f;
		case SPELL_REPEL_UNDEAD:          return  18.f;
		case SPELL_ENCHANT_WEAPON:        return  35.f;
		case SPELL_INCINERATE:            return  40.f;
		case SPELL_CONTROL_TARGET:        return  40.f;
		case SPELL_EXPLOSION:             return  45.f;
		case SPELL_FREEZE_TIME:           return  60.f;
		case SPELL_MASS_INCINERATE:       return 160.f;
		
		case SPELL_CONFUSE:               return casterLevel * 0.1f;
		case SPELL_MAGIC_MISSILE:         return casterLevel * 1.f;
		case SPELL_ICE_PROJECTILE:        return casterLevel * 1.5f;
		case SPELL_POISON_PROJECTILE:     return casterLevel * 2.f;
		case SPELL_FIREBALL:              return casterLevel * 3.f;
		case SPELL_PARALYSE:              return casterLevel * 3.f;
		case SPELL_MASS_PARALYSE:         return casterLevel * 3.f;
		case SPELL_LIGHTNING_STRIKE:      return casterLevel * 6.f;
		case SPELL_MASS_LIGHTNING_STRIKE: return casterLevel * 8.f;
		
		case SPELL_SUMMON_CREATURE:       return (casterLevel < 9) ? 20.f : 80.f;
		case SPELL_FAKE_SUMMON:           return (casterLevel < 9) ? 20.f : 80.f;
		
	}
}

static SpellBase * createSpellInstance(SpellType type) {
	switch(type) {
		case SPELL_NONE: return NULL;
		// LEVEL 1
		case SPELL_MAGIC_SIGHT: return new MagicSightSpell();
		case SPELL_MAGIC_MISSILE: return new MagicMissileSpell();
		case SPELL_IGNIT: return new IgnitSpell();
		case SPELL_DOUSE: return new DouseSpell();
		case SPELL_ACTIVATE_PORTAL: return new ActivatePortalSpell();
		// LEVEL 2
		case SPELL_HEAL: return new HealSpell();
		case SPELL_DETECT_TRAP: return new DetectTrapSpell();
		case SPELL_ARMOR: return new ArmorSpell();
		case SPELL_LOWER_ARMOR: return new LowerArmorSpell();
		case SPELL_HARM: return new HarmSpell();
		// LEVEL 3
		case SPELL_SPEED: return new SpeedSpell();
		case SPELL_DISPELL_ILLUSION: return new DispellIllusionSpell();
		case SPELL_FIREBALL: return new FireballSpell();
		case SPELL_CREATE_FOOD: return new CreateFoodSpell();
		case SPELL_ICE_PROJECTILE: return new IceProjectileSpell();
		// LEVEL 4
		case SPELL_BLESS: return new BlessSpell();
		case SPELL_DISPELL_FIELD: return new DispellFieldSpell();
		case SPELL_FIRE_PROTECTION: return new FireProtectionSpell();
		case SPELL_COLD_PROTECTION: return new ColdProtectionSpell();
		case SPELL_TELEKINESIS: return new TelekinesisSpell();
		case SPELL_CURSE: return new CurseSpell();
		// LEVEL 5
		case SPELL_RUNE_OF_GUARDING: return new RuneOfGuardingSpell();
		case SPELL_LEVITATE: return new LevitateSpell();
		case SPELL_CURE_POISON: return new CurePoisonSpell();
		case SPELL_REPEL_UNDEAD: return new RepelUndeadSpell();
		case SPELL_POISON_PROJECTILE: return new PoisonProjectileSpell();
		// LEVEL 6
		case SPELL_RISE_DEAD: return new RiseDeadSpell();
		case SPELL_PARALYSE: return new ParalyseSpell();
		case SPELL_CREATE_FIELD: return new CreateFieldSpell();
		case SPELL_DISARM_TRAP: return new DisarmTrapSpell();
		case SPELL_SLOW_DOWN: return new SlowDownSpell();
		// LEVEL 7
		case SPELL_FLYING_EYE: return new FlyingEyeSpell();
		case SPELL_FIRE_FIELD: return new FireFieldSpell();
		case SPELL_ICE_FIELD: return new IceFieldSpell();
		case SPELL_LIGHTNING_STRIKE: return new LightningStrikeSpell();
		case SPELL_CONFUSE: return new ConfuseSpell();
		// LEVEL 8
		case SPELL_INVISIBILITY: return new InvisibilitySpell();
		case SPELL_MANA_DRAIN: return new ManaDrainSpell();
		case SPELL_EXPLOSION: return new ExplosionSpell();
		case SPELL_ENCHANT_WEAPON: return new EnchantWeaponSpell();
		case SPELL_LIFE_DRAIN: return new LifeDrainSpell();
		// LEVEL 9
		case SPELL_SUMMON_CREATURE: return new SummonCreatureSpell();
		case SPELL_FAKE_SUMMON: return new FakeSummonSpell();
		case SPELL_NEGATE_MAGIC: return new NegateMagicSpell();
		case SPELL_INCINERATE: return new IncinerateSpell();
		case SPELL_MASS_PARALYSE: return new MassParalyseSpell();
		// LEVEL 10
		case SPELL_MASS_LIGHTNING_STRIKE: return new MassLightningStrikeSpell();
		case SPELL_CONTROL_TARGET: return new ControlTargetSpell();
		case SPELL_FREEZE_TIME: return new FreezeTimeSpell();
		case SPELL_MASS_INCINERATE: return new MassIncinerateSpell();
	}
	
	return NULL;
}



bool ARX_SPELLS_Launch(SpellType typ, EntityHandle source, SpellcastFlags flags, long level, EntityHandle target, GameDuration duration) {
	
	if(cur_rf == 3) {
		flags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NOMANA;
	}

	if(sp_max) {
		level = std::max(level, 15l);
	}
	
	if(   source == EntityHandle_Player
	   && !(flags & SPELLCAST_FLAG_NOCHECKCANCAST)
	) {
		for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
			if(SpellSymbol[i] != RUNE_NONE) {
				if(!player.hasRune(SpellSymbol[i])) {
					ARX_SOUND_PlaySpeech("player_cantcast");
					CurrSpellSymbol = 0;
					ARX_SPELLS_ResetRecognition();
					return false;
				}
			}
		}
	}
	
	float playerSpellLevel = 0;
	
	if(source == EntityHandle_Player) {
		ARX_SPELLS_ResetRecognition();
		
		if(player.SpellToMemorize.bSpell) {
			CurrSpellSymbol = 0;
			player.SpellToMemorize.bSpell = false;
		}
		
		ARX_PLAYER_ComputePlayerFullStats();
		
		if(level == -1) {
			playerSpellLevel = player.m_skillFull.casting + player.m_attributeFull.mind;
			playerSpellLevel = glm::clamp(playerSpellLevel * 0.1f, 1.0f, 10.0f);
		} else {
			playerSpellLevel = static_cast<float>(level);
		}
	}
	
	// Todo what was this assert supposed to do ?
	// arx_assert(!(source && (flags & SPELLCAST_FLAG_PRECAST)));
	
	if(flags & SPELLCAST_FLAG_PRECAST) {
		int l = level;
		
		if(l <= 0) {
			l = checked_range_cast<int>(playerSpellLevel);
		}
		
		SpellcastFlags flgs = flags;
		flgs &= ~SPELLCAST_FLAG_PRECAST;
		ARX_SPELLS_Precast_Add(typ, l, flgs, duration);
		return true;
	}
	
	if(target == EntityHandle() && source == EntityHandle_Player) {
		switch(typ) {
			case SPELL_LOWER_ARMOR:
			case SPELL_CURSE:
			case SPELL_PARALYSE:
			case SPELL_INCINERATE:
			case SPELL_SLOW_DOWN:
			case SPELL_CONFUSE: {
				LOOKING_FOR_SPELL_TARGET_TIME = g_gameTime.now();
				LOOKING_FOR_SPELL_TARGET = 1;
				t_spell.typ = typ;
				t_spell.flags = flags;
				t_spell.level = level;
				t_spell.target = target;
				t_spell.duration = duration;
				return false;
			}
			case SPELL_ENCHANT_WEAPON: {
				LOOKING_FOR_SPELL_TARGET_TIME = g_gameTime.now();
				LOOKING_FOR_SPELL_TARGET = 2;
				t_spell.typ = typ;
				t_spell.flags = flags;
				t_spell.level = level;
				t_spell.target = target;
				t_spell.duration = duration;
				return false;
			}
			case SPELL_CONTROL_TARGET: {
				long tcount = 0;
				
				if(!ValidIONum(source)) {
					return false;
				}
				
				Vec3f cpos = entities[source]->pos;
				
				for(size_t ii = 1; ii < entities.size(); ii++) {
					const EntityHandle handle = EntityHandle(ii);
					Entity * ioo = entities[handle];
					
					if(ioo && (ioo->ioflags & IO_NPC) && ioo->_npcdata->lifePool.current > 0.f
						&& ioo->show == SHOW_FLAG_IN_SCENE
						&& ioo->groups.find("demon") != ioo->groups.end()
						&& closerThan(ioo->pos, cpos, 900.f)) {
						tcount++;
					}
				}
				
				if(tcount == 0) {
					ARX_SOUND_PlaySFX(g_snd.MAGIC_FIZZLE, &cpos);
					return false;
				}
				
				ARX_SOUND_PlaySpeech("player_follower_attack");
				
				LOOKING_FOR_SPELL_TARGET_TIME = g_gameTime.now();
				LOOKING_FOR_SPELL_TARGET = 1;
				t_spell.typ = typ;
				t_spell.flags = flags;
				t_spell.level = level;
				t_spell.target = target;
				t_spell.duration = duration;
				return false;
			}
			default: break;
		}
	}
	
	if(source == EntityHandle_Player) {
		ARX_SPELLS_CancelSpellTarget();
	}
	
	if(!spells.hasFreeSlot()) {
		return false;
	}
	
	SpellBase * spell = createSpellInstance(typ);
	if(!spell)
		return false;
	
	if(ValidIONum(source) && spellicons[typ].bAudibleAtStart) {
		ARX_NPC_SpawnAudibleSound(entities[source]->pos, entities[source]);
	}
	
	spell->m_caster = source; // Caster...
	spell->m_target = target;
	
	if(target == EntityHandle())
		spell->m_target = TemporaryGetSpellTarget(entities[spell->m_caster]->pos);
	
	spell->updateCasterHand();
	spell->updateCasterPosition();
	
	float spellLevel;
	
	if(source == EntityHandle_Player) {
		// Player source
		spellLevel = playerSpellLevel; // Level of caster
	} else {
		// IO source
		spellLevel = float(glm::clamp(level, 1l, 10l));
	}
	
	if(flags & SPELLCAST_FLAG_LAUNCHPRECAST) {
		spellLevel = static_cast<float>(level);
	}
	
	if(cur_rf == 3) {
		spellLevel += 2;
	}
	
	spell->m_level = spellLevel;
	spell->m_flags = flags;
	spell->m_type = typ;
	spell->m_timcreation = g_gameTime.now();
	spell->m_fManaCostPerSecond = 0.f;
	spell->m_launchDuration = duration;
	
	if(!CanPayMana(spell, ARX_SPELLS_GetManaCost(typ, spell->m_level))) {
		if(spell->m_caster == EntityHandle_Player) {
			notification_add(getLocalised("player_cantcast"));
			ARX_SPEECH_AddSpeech(entities.player(), "player_cantcast", ANIM_TALK_NEUTRAL);
		}
		ARX_SPELLS_Fizzle(spell);
		delete spell;
		return false;
	}
	
	if(!GLOBAL_MAGIC_MODE) {
		ARX_SPELLS_Fizzle(spell);
		delete spell;
		return false;
	}
	
	if(!spell->CanLaunch()) {
		delete spell;
		return false;
	}
	
	spell->Launch();
	
	spells.addSpell(spell);
	
	// TODO inconsistent use of the SM_SPELLCAST event
	if(typ == SPELL_CONFUSE || typ == SPELL_ENCHANT_WEAPON) {
		SPELLCAST_NotifyOnlyTarget(*spell);
	} else {
		SPELLCAST_Notify(*spell);
	}
	
	return true;
}


/*!
 * \brief Updates all currently working spells.
 */
void ARX_SPELLS_Update() {
	
	ARX_PROFILE_FUNC();
	
	for(size_t u = 0; u < MAX_SPELLS; u++) {
		SpellBase * spell = spells[SpellHandle(u)];
		if(!spell)
			continue;
		
		if(!GLOBAL_MAGIC_MODE) {
			spells.endSpell(spell);
		}
		
		if(!CanPayMana(spell, spell->m_fManaCostPerSecond * (g_gameTime.lastFrameDuration() / GameDurationMs(1000)))) {
			ARX_SPELLS_Fizzle(spell);
			spells.endSpell(spell);
		}
		
		spell->m_elapsed += g_gameTime.lastFrameDuration();
		
		if(spell->m_hasDuration && spell->m_elapsed > spell->m_duration) {
			SPELLEND_Notify(*spell);
			spell->End();
			spells.freeSlot(spell);
			continue;
		}

		if(spell) {
			spell->Update();
		}
	}
}

void TryToCastSpell(Entity * io, SpellType spellType, long level, EntityHandle target, SpellcastFlags flags, GameDuration duration)
{
	if(!io || io->spellcast_data.castingspell != SPELL_NONE)
		return;
	
	if(!(flags & SPELLCAST_FLAG_NOMANA) && (io->ioflags & IO_NPC) && io->_npcdata->manaPool.current <= 0.f) {
		return;
	}
	
	unsigned long i(0);

	for(; i < SPELL_TYPES_COUNT; i++)
		if(spellicons[i].spellid == spellType)
			break;

	if(i >= SPELL_TYPES_COUNT)
		return; // not an existing spell...

	for(unsigned long j(0); j < 4; j++)
		io->spellcast_data.symb[j] = RUNE_NONE;

	// checks for symbol drawing...
	if(!flags.has(SPELLCAST_FLAG_NOANIM) && io->ioflags.has(IO_NPC)) {
		changeAnimation(io, 1, io->anims[ANIM_CAST_START]);
		for(unsigned long j = 0; j < 4; j++) {
			io->spellcast_data.symb[j] = spellicons[i].symbols[j];
		}
	}

	io->spellcast_data.castingspell = spellType;

	io->spellcast_data.spell_flags = flags;
	io->spellcast_data.spell_level = checked_range_cast<short>(level);

	io->spellcast_data.duration = duration;
	io->spellcast_data.target = target;
	
	io->gameFlags &= ~GFLAG_INVISIBILITY;
	
	if (((io->spellcast_data.spell_flags & SPELLCAST_FLAG_NOANIM)
	     && (io->spellcast_data.spell_flags & SPELLCAST_FLAG_NODRAW))
	    || (io->spellcast_data.spell_flags & SPELLCAST_FLAG_PRECAST)) {
		
		ARX_SPELLS_Launch(io->spellcast_data.castingspell,
		                  io->index(),
		                  io->spellcast_data.spell_flags,
		                  io->spellcast_data.spell_level,
		                  io->spellcast_data.target,
		                  io->spellcast_data.duration);
		
		io->spellcast_data.castingspell = SPELL_NONE;
	}
	
	io->spellcast_data.spell_flags &= ~SPELLCAST_FLAG_NODRAW; // temporary, removes colored flares
}
