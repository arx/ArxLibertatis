/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#include <string.h>

#include "core/GameTime.h"
#include "core/Localisation.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "gui/Interface.h"
#include "gui/Speech.h"
#include "scene/GameSound.h"

const size_t MAX_PRECAST = 3;

extern void ARX_SPELLS_FizzleNoMana(long num);

std::vector<PRECAST_STRUCT> Precast;

void ARX_SPELLS_Precast_Reset() {
	Precast.clear();
}

void ARX_SPELLS_Precast_Add(SpellType typ, long _level, SpellcastFlags flags, long duration) {
	
	if(Precast.size() >= MAX_PRECAST) {
		Precast.erase(Precast.begin());
	}
	
	if(typ == SPELL_NONE)
		return;
	
	PRECAST_STRUCT precast;
	precast.typ = typ;
	precast.level = _level;
	precast.launch_time = 0;
	precast.flags = flags;
	precast.duration = duration;
	
	Precast.push_back(precast);
}

unsigned long LAST_PRECAST_TIME = 0;

long PrecastCheckCanPayMana(long num, float cost, bool _bSound = true) {
	if(num < 0)
		return 0;

	if(Precast[num].flags & SPELLCAST_FLAG_NOMANA)
		return 1;

	if(player.manaPool.current >= cost)
		return 1;
	
	ARX_SPELLS_FizzleNoMana(num);

	if(_bSound) {
		ARX_SPEECH_Add(getLocalised("player_cantcast"));
		ARX_SPEECH_AddSpeech(entities.player(), "player_cantcast", ANIM_TALK_NEUTRAL);
	}

	return 0;
}

void ARX_SPELLS_Precast_Launch(size_t num) {
	
	if(num >= Precast.size()) {
		return;
	}
	
	if(float(arxtime) >= LAST_PRECAST_TIME + 1000) {
		SpellType type = Precast[num].typ;
		float cost=ARX_SPELLS_GetManaCost(type,  -1);

		if(type != SPELL_NONE && !PrecastCheckCanPayMana(num,cost))
			return;

		LAST_PRECAST_TIME = (unsigned long)(arxtime);

		if(Precast[num].launch_time == 0) {
			Precast[num].launch_time = (unsigned long)(arxtime);
			ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FIELD);
		}
	}
}

void ARX_SPELLS_Precast_Check() {
	for(size_t i = 0; i < Precast.size(); i++) {
		if(Precast[i].launch_time > 0 && float(arxtime) >= Precast[i].launch_time) {
			ANIM_USE *ause1 = &entities.player()->animlayer[1];
			
			if(player.Interface & INTER_COMBATMODE) {
				WILLRETURNTOCOMBATMODE = true;
				ARX_INTERFACE_Combat_Mode(0);
				ResetAnim(&entities.player()->animlayer[1]);
				entities.player()->animlayer[1].flags&=~EA_LOOP;
			}

			if(ause1->cur_anim && ause1->cur_anim == entities.player()->anims[ANIM_CAST]) {
				if(ause1->ctime > ause1->cur_anim->anims[ause1->altidx_cur]->anim_time - 550)
				{
					ARX_SPELLS_Launch(	Precast[i].typ,
										PlayerEntityHandle,
										Precast[i].flags | SPELLCAST_FLAG_LAUNCHPRECAST, 
										Precast[i].level, 
										InvalidEntityHandle,
										Precast[i].duration);
					
					Precast.erase(Precast.begin() + i);
				}
			} else {
				changeAnimation(entities.player(), 1, entities.player()->anims[ANIM_CAST]);
			}
		}
	}
}
