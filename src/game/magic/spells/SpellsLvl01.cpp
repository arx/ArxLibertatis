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

#include "game/magic/spells/SpellsLvl01.h"

#include "core/Application.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/spell/Cheat.h"

#include "graphics/spells/Spells01.h"
#include "graphics/spells/Spells03.h"

#include "scene/GameSound.h"

void MagicSightSpellLaunch(long duration, long i)
{
	spells[i].exist = true;
	spells[i].fManaCostPerSecond = 0.36f;
	spells[i].bDuration = true;
	spells[i].tolive = (duration > -1) ? duration : 6000000l;
	
	ARX_SOUND_PlaySFX(SND_SPELL_VISION_START, &spells[i].caster_pos);
	
	if(spells[i].caster == 0) {
		Project.improve = 1;
		spells[i].snd_loop = SND_SPELL_VISION_LOOP;
		ARX_SOUND_PlaySFX(spells[i].snd_loop, &spells[i].caster_pos, 1.f,
		                  ARX_SOUND_PLAY_LOOPED);
	}
}

void MagicSightSpellEnd(long i)
{
	if(spells[i].caster == 0) {
		Project.improve = 0;
		ARX_SOUND_Stop(spells[i].snd_loop);
	}
	ARX_SOUND_PlaySFX(SND_SPELL_VISION_START, &entities[spells[i].caster]->pos);
}

void MagicMissileSpellLaunch(long i)
{
	spells[i].exist = true;
	spells[i].tolive = 20000; // TODO probably never read
	
	long number;
	if(sp_max || cur_rf == 3) {
		number = long(spells[i].caster_level);
	} else {
		number = clamp(long(spells[i].caster_level + 1) / 2, 1l, 5l);
	}
	
	CMultiMagicMissile * effect = new CMultiMagicMissile(number);
	effect->spellinstance = i;
	effect->SetDuration(6000ul);
	effect->Create();
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
}

void MagicMissileSpellEnd(long i)
{
	lightHandleDestroy(spells[i].longinfo_light);
}

void IgnitSpellLaunch(/*long target, */long i)
{
	spells[i].exist = true;
	spells[i].tolive = 20000; // TODO probably never read
	
	CIgnit * effect = new CIgnit();
	effect->spellinstance = i;
	
	Vec3f target;
	if(spells[i].hand_group != -1) {
		target = spells[i].hand_pos;
	} else {
		target = spells[i].caster_pos - Vec3f(0.f, 50.f, 0.f);
	}
	
	LightHandle id = GetFreeDynLight();
	if(lightHandleIsValid(id)) {
		EERIE_LIGHT * light = lightHandleGet(id);
		
		light->intensity = 1.8f;
		light->fallend   = 450.f;
		light->fallstart = 380.f;
		light->rgb       = Color3f(1.f, 0.75f, 0.5f);
		light->pos       = target;
		light->duration  = 300;
	}
	
	float fPerimeter = 400.f + spells[i].caster_level * 30.f;
	
	effect->Create(&target, fPerimeter, 500);
	CheckForIgnition(&target, fPerimeter, 1, 1);
	
	for(size_t ii = 0; ii < MAX_LIGHTS; ii++) {
		
		if(!GLight[ii] || !(GLight[ii]->extras & EXTRAS_EXTINGUISHABLE)) {
			continue;
		}
		
		if(spells[i].caster == 0 && (GLight[ii]->extras & EXTRAS_NO_IGNIT)) {
			continue;
		}
		
		if(!(GLight[ii]->extras & EXTRAS_SEMIDYNAMIC)
		  && !(GLight[ii]->extras & EXTRAS_SPAWNFIRE)
		  && !(GLight[ii]->extras & EXTRAS_SPAWNSMOKE)) {
			continue;
		}
		
		if(GLight[ii]->status) {
			continue;
		}
		
		if(!fartherThan(target, GLight[ii]->pos, effect->GetPerimetre())) {
			effect->AddLight(ii);
		}
	}
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		if(!spells[n].exist) {
			continue;
		}
		if(spells[n].type == SPELL_FIREBALL) {
			CSpellFx * pCSpellFX = spells[n].pSpellFx;
			if(pCSpellFX) {
				CFireBall * pCF = (CFireBall *)pCSpellFX;
				float radius = std::max(spells[i].caster_level * 2.f, 12.f);
				if(closerThan(target, pCF->eCurPos,
				              effect->GetPerimetre() + radius)) {
					spells[n].caster_level += 1; 
				}
			}
		}
	}
	
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
}

void IgnitSpellEnd(long i)
{
	CIgnit *pIgnit = (CIgnit *)spells[i].pSpellFx;
	pIgnit->Action(1);
}

void DouseSpellLaunch(long i)
{
	spells[i].exist = true;
	spells[i].tolive = 20000;
	
	CDoze * effect = new CDoze();
	effect->spellinstance = i;
	
	Vec3f target;
	if(spells[i].hand_group >= 0) {
		target = spells[i].hand_pos;
	} else {
		target = spells[i].caster_pos;
		target.y -= 50.f;
	}
	
	float fPerimeter = 400.f + spells[i].caster_level * 30.f;
	effect->CreateDoze(&target, fPerimeter, 500);
	CheckForIgnition(&target, fPerimeter, 0, 1);
	
	for(size_t ii = 0; ii < MAX_LIGHTS; ii++) {
		
		if(!GLight[ii] || !(GLight[ii]->extras & EXTRAS_EXTINGUISHABLE)) {
			continue;
		}
		
		if(!(GLight[ii]->extras & EXTRAS_SEMIDYNAMIC)
		  && !(GLight[ii]->extras & EXTRAS_SPAWNFIRE)
		  && !(GLight[ii]->extras & EXTRAS_SPAWNSMOKE)) {
			continue;
		}
		
		if(!GLight[ii]->status) {
			continue;
		}
		
		if(!fartherThan(target, GLight[ii]->pos, effect->GetPerimetre())) {
			effect->AddLightDoze(ii);	
		}
	}
	
	if(player.torch && closerThan(target, player.pos, effect->GetPerimetre())) {
		ARX_PLAYER_ClickedOnTorch(player.torch);
	}
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		
		if(!spells[n].exist) {
			continue;
		}
		
		switch(spells[n].type) {
			
			case SPELL_FIREBALL: {
				CSpellFx * pCSpellFX = spells[n].pSpellFx;
				if(pCSpellFX) {
					CFireBall * pCF = (CFireBall *)pCSpellFX;
					float radius = std::max(spells[i].caster_level * 2.f, 12.f);
					if(closerThan(target, pCF->eCurPos,
					              effect->GetPerimetre() + radius)) {
						spells[n].caster_level -= spells[i].caster_level;
						if(spells[n].caster_level < 1) {
							spells[n].tolive = 0;
						}
					}
				}
				break;
			}
			
			case SPELL_FIRE_FIELD: {
				Vec3f pos;
				if(GetSpellPosition(&pos, n)) {
					if(closerThan(target, pos, effect->GetPerimetre() + 200)) {
						spells[n].caster_level -= spells[i].caster_level;
						if(spells[n].caster_level < 1) {
							spells[n].tolive=0;
						}
					}
				}
				break;
			}
			
			default: break;
		}
	}
	
	spells[i].pSpellFx = effect;
	spells[i].tolive = effect->GetDuration();
}

void DouseSpellEnd(long i)
{
	CDoze *pDoze = (CDoze *)spells[i].pSpellFx;
	pDoze->Action(0);
}

void ActivatePortalSpellLaunch(long i)
{
	ARX_SOUND_PlayInterface(SND_SPELL_ACTIVATE_PORTAL);
	spells[i].exist = true;
	spells[i].tolive = 20;
}
