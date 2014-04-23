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

#include "game/magic/spells/SpellsLvl09.h"

#include "core/Application.h"
#include "core/Config.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "game/effect/Quake.h"
#include "game/spell/Cheat.h"

#include "graphics/particle/ParticleEffects.h"

#include "graphics/spells/Spells09.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

bool SummonCreatureSpell::Launch(long i, long duration)
{
	spells[i].m_exist = true;
	spells[i].m_lastupdate = spells[i].m_timcreation = (unsigned long)(arxtime);
	spells[i].m_bDuration = true;
	spells[i].m_fManaCostPerSecond = 1.9f;
	spells[i].m_longinfo_summon_creature = 0;
	spells[i].m_longinfo2_entity = 0;
	spells[i].m_tolive = (duration > -1) ? duration : 2000000;
	
	Vec3f target;
	float beta;
	bool displace = false;
	if(spells[i].m_caster == 0) {
		target = player.basePosition();
		beta = player.angle.getPitch();
		displace = true;
	} else {
		target = entities[spells[i].m_caster]->pos;
		beta = entities[spells[i].m_caster]->angle.getPitch();
		displace = (entities[spells[i].m_caster]->ioflags & IO_NPC) == IO_NPC;
	}
	if(displace) {
		target.x -= std::sin(radians(MAKEANGLE(beta))) * 300.f;
		target.z += std::cos(radians(MAKEANGLE(beta))) * 300.f;
	}
	
	if(!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target)) {
		spells[i].m_exist = false;
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
		return false;
	}
	
	spells[i].m_fdata = (spells[i].m_caster == 0 && cur_mega == 10) ? 1.f : 0.f;
	spells[i].m_target_pos = target;
	ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE, &spells[i].m_target_pos);
	CSummonCreature * effect = new CSummonCreature();
	effect->spellinstance = i;
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(2000, 500, 1500);
	effect->SetColorBorder(Color3f::red);
	effect->SetColorRays1(Color3f::red);
	effect->SetColorRays2(Color3f::yellow * .5f);
	
	effect->lLightId = GetFreeDynLight();
	if(lightHandleIsValid(effect->lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(effect->lLightId);
		
		light->intensity = 0.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f::red;
		light->pos = effect->eSrc;
	}
	
	spells[i].m_pSpellFx = effect;
	
	return true;
}

void SummonCreatureSpell::End(size_t i)
{
	if(ValidIONum(spells[i].m_longinfo2_entity) && spells[i].m_longinfo2_entity != 0) {
		ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &entities[spells[i].m_longinfo2_entity]->pos);
	}

	lightHandleDestroy(spells[i].m_pSpellFx->lLightId);
	// need to killio
}

void SummonCreatureSpell::Kill(long i)
{
	lightHandleDestroy(spells[i].m_pSpellFx->lLightId);
	
	if(ValidIONum(spells[i].m_longinfo2_entity) && spells[i].m_longinfo2_entity != 0) {
		
		if(entities[spells[i].m_longinfo2_entity]->scriptload
		   && (entities[spells[i].m_longinfo2_entity]->ioflags & IO_NOSAVE)) {
			
			AddRandomSmoke(entities[spells[i].m_longinfo2_entity], 100);
			Vec3f posi = entities[spells[i].m_longinfo2_entity]->pos;
			posi.y -= 100.f;
			MakeCoolFx(posi);
		
			LightHandle nn = GetFreeDynLight();
			if(lightHandleIsValid(nn)) {
				EERIE_LIGHT * light = lightHandleGet(nn);
				
				light->intensity = 0.7f + 2.f * rnd();
				light->fallend = 600.f;
				light->fallstart = 400.f;
				light->rgb = Color3f(1.0f, 0.8f, 0.0f);
				light->pos = posi;
				light->duration = 600;
			}
			
			entities[spells[i].m_longinfo2_entity]->destroyOne();
		}
	}
	
	spells[i].m_longinfo2_entity = 0;
}

void SummonCreatureSpell::Update(size_t i, float timeDelta)
{
	if(!arxtime.is_paused()) {
		if(float(arxtime) - (float)spells[i].m_timcreation <= 4000) {
			if(rnd() > 0.7f) {
				CSummonCreature * pSummon = (CSummonCreature *)spells[i].m_pSpellFx;
				if(pSummon) {
					Vec3f pos = pSummon->eSrc;
					MakeCoolFx(pos);
				}
			}

			CSpellFx *pCSpellFX = spells[i].m_pSpellFx;

			if(pCSpellFX) {
				pCSpellFX->Update(timeDelta);
				pCSpellFX->Render();
			}	

			spells[i].m_longinfo_summon_creature = 1;
			spells[i].m_longinfo2_entity = -1;

		} else if(spells[i].m_longinfo_summon_creature) {
			lightHandleDestroy(spells[i].m_pSpellFx->lLightId);

			spells[i].m_longinfo_summon_creature = 0;
			ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].m_target_pos);
			CSummonCreature *pSummon;
			pSummon= (CSummonCreature *)spells[i].m_pSpellFx;

			if(pSummon) {
				EERIE_CYLINDER phys;
				phys.height=-200;
				phys.radius=50;
				phys.origin=spells[i].m_target_pos;
				float anything = CheckAnythingInCylinder(&phys, NULL, CFLAG_JUST_TEST);

				if(EEfabs(anything) < 30) {
				
				long tokeep;
				res::path cls;
				if(spells[i].m_fdata == 1.f) {
					if(rnd() > 0.5) {
						tokeep = -1;
						cls = "graph/obj3d/interactive/npc/wrat_base/wrat_base";
					} else {
						tokeep = 0;
						cls = "graph/obj3d/interactive/npc/y_mx/y_mx";
					}
				} else if(rnd() > 0.997f || (sp_max && rnd() > 0.8f)
				   || (cur_mr >= 3 && rnd() > 0.3f)) {
					tokeep = 0;
					cls = "graph/obj3d/interactive/npc/y_mx/y_mx";
				} else if(rnd() > 0.997f || (cur_rf >= 3 && rnd() > 0.8f)
				   || (cur_mr >= 3 && rnd() > 0.3f)) {
					tokeep = -1;
					cls = "graph/obj3d/interactive/npc/wrat_base/wrat_base";
				} else if(spells[i].m_caster_level >= 9) {
					tokeep = 1;
					cls = "graph/obj3d/interactive/npc/demon/demon";
				} else if(rnd() > 0.98f) {
					tokeep = -1;
					cls = "graph/obj3d/interactive/npc/wrat_base/wrat_base";
				} else {
					tokeep = 0;
					cls = "graph/obj3d/interactive/npc/chicken_base/chicken_base";
				}
				
				Entity * io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
				if(!io) {
					cls = "graph/obj3d/interactive/npc/chicken_base/chicken_base";
					tokeep = 0;
					io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
				}
				
				if(io) {
					RestoreInitialIOStatusOfIO(io);
					
					long lSpellsCaster = spells[i].m_caster ; 
					io->summoner = checked_range_cast<short>(lSpellsCaster);

					io->scriptload = 1;
					
					if(tokeep == 1) {
						io->ioflags |= IO_NOSAVE;
					}
					
					io->pos = phys.origin;
					SendInitScriptEvent(io);

					if(tokeep < 0) {
						io->scale=1.65f;
						io->physics.cyl.radius=25;
						io->physics.cyl.height=-43;
						io->speed_modif=1.f;
					}

					if(ValidIONum(spells[i].m_caster)) {
						EVENT_SENDER = entities[spells[i].m_caster];
					} else {
						EVENT_SENDER = NULL;
					}

					SendIOScriptEvent(io,SM_SUMMONED);
					
					Vec3f pos;
					
					for(long j = 0; j < 3; j++) {
						pos.x=pSummon->eSrc.x+rnd()*100.f-50.f;
						pos.y=pSummon->eSrc.y+100+rnd()*100.f-50.f;
						pos.z=pSummon->eSrc.z+rnd()*100.f-50.f;
						MakeCoolFx(pos);
					}

					if(tokeep==1)
						spells[i].m_longinfo2_entity = io->index();
					else
						spells[i].m_longinfo2_entity = -1;
				}
				}
			}
		} else if(spells[i].m_longinfo2_entity <= 0) {
			spells[i].m_tolive = 0;
		}
	}	
}

bool FakeSummonSpell::Launch(long i)
{
	if(spells[i].m_caster <= 0 || !ValidIONum(spells[i].m_target)) {
		return false;
	}
	
	spells[i].m_exist = true;
	spells[i].m_lastupdate = spells[i].m_timcreation = (unsigned long)(arxtime);
	spells[i].m_bDuration = true;
	spells[i].m_fManaCostPerSecond = 1.9f;
	spells[i].m_tolive = 4000;
	
	Vec3f target = entities[spells[i].m_target]->pos;
	if(spells[i].m_target != 0) {
		target.y += player.baseHeight();
	}
	spells[i].m_target_pos = target;
	ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE, &spells[i].m_target_pos);
	CSummonCreature * effect = new CSummonCreature();
	effect->spellinstance = i;
	effect->Create(target, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(2000, 500, 1500);
	effect->SetColorBorder(Color3f::red);
	effect->SetColorRays1(Color3f::red);
	effect->SetColorRays2(Color3f::yellow * .5f);
	
	effect->lLightId = GetFreeDynLight();
	
	if(lightHandleIsValid(effect->lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(effect->lLightId);
		
		light->intensity = 0.3f;
		light->fallend = 500.f;
		light->fallstart = 400.f;
		light->rgb = Color3f::red;
		light->pos = effect->eSrc;
	}
	
	spells[i].m_pSpellFx = effect;
	
	return true;
}

void FakeSummonSpell::End(size_t i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].m_target_pos);
	
	lightHandleDestroy(spells[i].m_pSpellFx->lLightId);
}

void FakeSummonSpell::Kill(long i)
{
	lightHandleDestroy(spells[i].m_pSpellFx->lLightId);
}

void FakeSummonSpell::Update(size_t i, float timeDelta)
{
	if(!arxtime.is_paused()) {
		if(rnd() > 0.7f) {
			CSummonCreature * pSummon = (CSummonCreature *)spells[i].m_pSpellFx;
			if(pSummon) {
				Vec3f pos = pSummon->eSrc;
				MakeCoolFx(pos);
			}
		}
	}
	CSpellFx *pCSpellFX = spells[i].m_pSpellFx;
	
	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}	
}


void LaunchAntiMagicField(size_t ident) {
	
	for(size_t n = 0; n < MAX_SPELLS; n++) {
		
		if(!spells[n].m_exist || n == ident)
			continue;
		
		if(spells[ident].m_caster_level < spells[n].m_caster_level)
			continue;
		
		Vec3f pos;
		GetSpellPosition(&pos,n);
		if(closerThan(pos, entities[spells[ident].m_target]->pos, 600.f)) {
			if(spells[n].m_type != SPELL_CREATE_FIELD) {
				spells[n].m_tolive = 0;
			} else if(spells[ident].m_target == 0 && spells[n].m_caster == 0) {
				spells[n].m_tolive = 0;
			}
		}
	}
}

void NegateMagicSpell::Launch(long duration, long i)
{
	if(spells[i].m_caster == 0) {
		spells[i].m_target = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_NEGATE_MAGIC, &entities[spells[i].m_target]->pos);
	
	spells[i].m_exist = true;
	spells[i].m_lastupdate = spells[i].m_timcreation = (unsigned long)(arxtime);
	spells[i].m_bDuration = true;
	spells[i].m_fManaCostPerSecond = 2.f;
	spells[i].m_tolive = (duration > -1) ? duration : 1000000;
	
	CNegateMagic * effect = new CNegateMagic();
	effect->spellinstance = i;
	effect->Create(spells[i].m_target_pos, MAKEANGLE(entities[spells[i].m_target]->angle.getPitch()));
	effect->SetDuration(spells[i].m_tolive);
	spells[i].m_pSpellFx = effect;
	spells[i].m_tolive = effect->GetDuration();
	
	if(ValidIONum(spells[i].m_target)) {
		LaunchAntiMagicField(i);
	}
}

void NegateMagicSpell::Update(size_t i, float timeDelta)
{
	if(ValidIONum(spells[i].m_target))
		LaunchAntiMagicField(i);

	CSpellFx *pCSpellFX = spells[i].m_pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}	
}

bool IncinerateSpell::Launch(long i)
{
	Entity * tio = entities[spells[i].m_target];
	if((tio->ioflags & IO_NPC) && tio->_npcdata->life <= 0.f) {
		return false;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE, &entities[spells[i].m_target]->pos);
	
	spells[i].m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE_LOOP, 
	                                       &entities[spells[i].m_target]->pos, 1.f, 
	                                       ARX_SOUND_PLAY_LOOPED);
	
	spells[i].m_exist = true;
	spells[i].m_lastupdate = spells[i].m_timcreation = (unsigned long)(arxtime);
	spells[i].m_tolive = 20000;
	
	tio->sfx_flag |= SFX_TYPE_YLSIDE_DEATH | SFX_TYPE_INCINERATE;
	tio->sfx_time = (unsigned long)(arxtime);
	
	ARX_SPELLS_AddSpellOn(spells[i].m_target, i);
	
	return true;
}

void IncinerateSpell::End(size_t i)
{
	ARX_SPELLS_RemoveSpellOn(spells[i].m_target, i);
	ARX_SOUND_Stop(spells[i].m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE_END);
}

void IncinerateSpell::Update(size_t i)
{
	if(ValidIONum(spells[i].m_target)) {
		ARX_SOUND_RefreshPosition(spells[i].m_snd_loop, entities[spells[i].m_target]->pos);
	}	
}

void MassParalyseSpell::Launch(long i, long duration)
{
	ARX_SOUND_PlaySFX(SND_SPELL_MASS_PARALYSE);
	
	spells[i].m_exist = true;
	spells[i].m_tolive = (duration > -1) ? duration : 10000;
	spells[i].m_longinfo2_entity = 0;
	
	for(size_t ii = 0; ii < entities.size(); ii++) {
		
		Entity * tio = entities[ii];
		if(long(ii) == spells[i].m_caster || !tio || !(tio->ioflags & IO_NPC)) {
			continue;
		}
		
		if(tio->show != SHOW_FLAG_IN_SCENE) {
			continue;
		}
		
		if(tio->ioflags & IO_FREEZESCRIPT) {
			continue;
		}
		
		if(fartherThan(tio->pos, entities[spells[i].m_caster]->pos, 500.f)) {
			continue;
		}
		
		tio->ioflags |= IO_FREEZESCRIPT;
		
		ARX_NPC_Kill_Spell_Launch(tio);
		ARX_SPELLS_AddSpellOn(ii, i);
		
		spells[i].m_longinfo2_entity ++;
		spells[i].m_misc = realloc(spells[i].m_misc,
		                         sizeof(long) * spells[i].m_longinfo2_entity);
		long * ptr = (long *)spells[i].m_misc;
		ptr[spells[i].m_longinfo2_entity - 1] = ii;
	}
}

void MassParalyseSpell::End(size_t i)
{
	long *ptr = (long *) spells[i].m_misc;

	for(long in = 0; in < spells[i].m_longinfo2_entity; in++) {
		if(ValidIONum(ptr[in])) {
			ARX_SPELLS_RemoveSpellOn(ptr[in], i);
			entities[ptr[in]]->ioflags &= ~IO_FREEZESCRIPT;
		}
	}

	if(ptr)
		free(spells[i].m_misc);

	spells[i].m_misc=NULL;
}

void MassParalyseSpell::Kill()
{
	ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE_END);
}

