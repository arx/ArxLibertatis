/*
 * Copyright 2014-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/magic/Precast.h"

#include <algorithm>

#include "core/GameTime.h"
#include "core/Localisation.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "gui/Interface.h"
#include "gui/Notification.h"
#include "gui/Speech.h"
#include "scene/GameSound.h"

const size_t MAX_PRECAST = 3;

GameInstant LAST_PRECAST_TIME = 0;

util::HandleVector<PrecastHandle, PRECAST_STRUCT> g_precast;

void ARX_SPELLS_Precast_Reset() {
	LAST_PRECAST_TIME = 0;
	g_precast.clear();
}

void ARX_SPELLS_Precast_Add(SpellType typ, long _level, SpellcastFlags flags, GameDuration duration) {
	
	if(g_precast.size() >= MAX_PRECAST) {
		g_precast.erase(g_precast.begin());
	}
	
	if(typ == SPELL_NONE) {
		return;
	}
	
	PRECAST_STRUCT & precast = g_precast.emplace_back();
	precast.typ = typ;
	precast.level = _level;
	precast.launch_time = 0;
	precast.flags = flags;
	precast.duration = duration;
	
}

void ARX_SPELLS_Precast_Launch(PrecastHandle handle) {
	
	if(!handle || size_t(handle) >= g_precast.size()) {
		return;
	}
	
	GameDuration elapsed = g_gameTime.now() - LAST_PRECAST_TIME;
	if(elapsed < 1s) {
		return;
	}
	
	PRECAST_STRUCT & precast = g_precast[handle];
	
	if(precast.typ == SPELL_NONE) {
		return;
	}
	
	// Calculate the player's magic level
	float playerSpellLevel = player.m_skillFull.casting + player.m_attributeFull.mind;
	playerSpellLevel = glm::clamp(playerSpellLevel * 0.1f, 1.f, 10.f);
	
	float cost = ARX_SPELLS_GetManaCost(precast.typ, playerSpellLevel);
	
	if((precast.flags & SPELLCAST_FLAG_NOMANA) || player.manaPool.current >= cost) {
		LAST_PRECAST_TIME = g_gameTime.now();
		if(precast.launch_time == 0) {
			precast.launch_time = g_gameTime.now();
			ARX_SOUND_PlaySFX(g_snd.SPELL_CREATE_FIELD);
		}
	} else {
		ARX_SOUND_PlaySFX(g_snd.MAGIC_FIZZLE);
		notification_add("player_cantcast");
		ARX_SPEECH_AddSpeech(*entities.player(), "player_cantcast", ANIM_TALK_NEUTRAL);
	}
	
}

void ARX_SPELLS_Precast_Check() {
	
	for(PRECAST_STRUCT & precast : g_precast) {
		
		if(precast.launch_time <= 0 || g_gameTime.now() < precast.launch_time) {
			continue;
		}
		
		AnimLayer & layer1 = entities.player()->animlayer[1];
		if(player.Interface & INTER_COMBATMODE) {
			WILLRETURNTOCOMBATMODE = true;
			ARX_INTERFACE_setCombatMode(COMBAT_MODE_OFF);
			ResetAnim(layer1);
			layer1.flags &= ~EA_LOOP;
		}
		
		if(!layer1.cur_anim || layer1.cur_anim != entities.player()->anims[ANIM_CAST]) {
			changeAnimation(entities.player(), 1, entities.player()->anims[ANIM_CAST]);
			continue;
		}
		
		if(layer1.ctime + 550ms > layer1.currentAltAnim()->anim_time) {
			ARX_SPELLS_Launch(precast.typ, *entities.player(), precast.flags | SPELLCAST_FLAG_LAUNCHPRECAST,
			                  precast.level, nullptr, precast.duration);
			precast.typ = SPELL_NONE;
		}
		
	}
	
	g_precast.erase(std::remove_if(g_precast.begin(), g_precast.end(), [](const PRECAST_STRUCT & precast) {
		return precast.typ == SPELL_NONE;
	}), g_precast.end());
	
}
