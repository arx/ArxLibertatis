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

#include <glm/gtx/norm.hpp>

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
#include "graphics/spells/Spells01.h"
#include "graphics/spells/Spells02.h"
#include "graphics/spells/Spells03.h"
#include "graphics/spells/Spells04.h"
#include "graphics/spells/Spells05.h"
#include "graphics/spells/Spells06.h"
#include "graphics/spells/Spells07.h"
#include "graphics/spells/Spells09.h"
#include "graphics/spells/Spells10.h"

#include "input/Input.h"

#include "io/resource/ResourcePath.h"
#include "io/log/Logger.h"

#include "math/Angle.h"
#include "math/Vector.h"

#include "physics/Collisions.h"

#include "platform/Platform.h"

#include "scene/Light.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

#include "script/Script.h"

extern bool TRUE_PLAYER_MOUSELOOK_ON;

bool WILLRETURNTOFREELOOK = false;
bool GLOBAL_MAGIC_MODE = true;

extern bool MAGICMODE;

extern bool bRenderInCursorMode;

void ARX_INTERFACE_Combat_Mode(long i);

///////////////Spell Interpretation
SpellBase spells[MAX_SPELLS];
short ARX_FLARES_broken(1);

long snip=0;
static Vec2s Lm;


unsigned char ucFlick=0;

bool GetSpellPosition(Vec3f * pos,long i)
{
	switch(spells[i].m_type) {
		case SPELL_ARMOR:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_LOWER_ARMOR:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_SPEED:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_BLESS:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_FIRE_PROTECTION:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_COLD_PROTECTION:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_CURSE:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_RUNE_OF_GUARDING: {
			CSpellFx *pCSpellFX = spells[i].m_pSpellFx;

			if(pCSpellFX) {
				CRuneOfGuarding *pCRG = (CRuneOfGuarding *) pCSpellFX;
					
				*pos = pCRG->eSrc;
				return true;
			}
		}
		break;
		case SPELL_PARALYSE:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_CREATE_FIELD: {
			CSpellFx *pCSpellFX = spells[i].m_pSpellFx;

			if(pCSpellFX) {
				CCreateField *pCreateField = (CCreateField *) pCSpellFX;
					
				*pos = pCreateField->eSrc;
				return true;
			}
		}
		break;
		case SPELL_SLOW_DOWN:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_FLYING_EYE:
		{	
			*pos = eyeball.pos;
			return true;
		}	
		break;
		case SPELL_FIRE_FIELD: {
			CSpellFx *pCSpellFX = spells[i].m_pSpellFx;

			if(pCSpellFX) {
				CFireField *pFireField = (CFireField *) pCSpellFX;
					
				*pos = pFireField->pos;
				return true;
			}
		}
		break;
		case SPELL_ICE_FIELD: {
			CSpellFx *pCSpellFX = spells[i].m_pSpellFx;

			if(pCSpellFX) {
				CIceField *pIceField = (CIceField *) pCSpellFX;
					
				*pos = pIceField->eSrc;
				return true;
			}
		}
		break;
		case SPELL_CONFUSE:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_INVISIBILITY:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_MANA_DRAIN:				
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_LIFE_DRAIN:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		case SPELL_INCINERATE:
			if(ValidIONum(spells[i].m_target)) {
				*pos = entities[spells[i].m_target]->pos;
				return true;
			}
		break;
		default:
		break;
	}

	if(ValidIONum(spells[i].m_caster)) {
		*pos = entities[spells[i].m_caster]->pos;
		return true;
	}

	return false;
}

bool spellHandleIsValid(long handle) {
	return (long)handle >= 0 && ((size_t)handle < MAX_SPELLS) && spells[handle].m_exist;
}

void ARX_SPELLS_AddSpellOn(const long &caster, const long &spell)
{
	if(caster < 0 ||  spell < 0 || !entities[caster])
		return;

	Entity *io = entities[caster];
	
	io->spellsOn.insert(spell);
}

long ARX_SPELLS_GetSpellOn(const Entity * io, SpellType spellid)
{
	if(!io)
		return -1;

	boost::container::flat_set<long>::const_iterator it;
	for(it = io->spellsOn.begin(); it != io->spellsOn.end(); ++it) {
		long spellHandle = *it;
		if(spellHandleIsValid(spellHandle)) {
			SpellBase * spell = &spells[spellHandle];
			
			if(spell->m_type == spellid) {
				return spellHandle;
			}
		}
	}
	
	return -1;
}

void ARX_SPELLS_RemoveSpellOn(const long & entityHandle, const long & spellHandle)
{
	if(entityHandle < 0 || spellHandle < 0)
		return;

	Entity *io = entities[entityHandle];

	if(!io || io->spellsOn.empty())
		return;

	if(io->spellsOn.size() == 1) {
		io->spellsOn.clear();
		return;
	}
	
	io->spellsOn.erase(spellHandle);
}

void ARX_SPELLS_RemoveAllSpellsOn(Entity *io) {
	io->spellsOn.clear();
}

static bool MakeSpellName(char * spell, SpellType num) {
	
	switch (num)
	{
		// Level 1
		case SPELL_MAGIC_SIGHT           :
			strcpy(spell, "magic_sight");
			break;
		case SPELL_MAGIC_MISSILE         :
			strcpy(spell, "magic_missile");
			break;
		case SPELL_IGNIT                 :
			strcpy(spell, "ignit");
			break;
		case SPELL_DOUSE                 :
			strcpy(spell, "douse");
			break;
		case SPELL_ACTIVATE_PORTAL       :
			strcpy(spell, "activate_portal");
			break;

		// Level 2
		case SPELL_HEAL                  :
			strcpy(spell, "heal");
			break;
		case SPELL_DETECT_TRAP           :
			strcpy(spell, "detect_trap");
			break;
		case SPELL_ARMOR                 :
			strcpy(spell, "armor");
			break;
		case SPELL_LOWER_ARMOR           :
			strcpy(spell, "lower_armor");
			break;
		case SPELL_HARM                  :
			strcpy(spell, "harm");
			break;

		// Level 3
		case SPELL_SPEED                 :
			strcpy(spell, "speed");
			break;
		case SPELL_DISPELL_ILLUSION      :
			strcpy(spell, "dispell_illusion");
			break;
		case SPELL_FIREBALL              :
			strcpy(spell, "fireball");
			break;
		case SPELL_CREATE_FOOD           :
			strcpy(spell, "create_food");
			break;
		case SPELL_ICE_PROJECTILE        :
			strcpy(spell, "ice_projectile");
			break;

		// Level 4 
		case SPELL_BLESS                 :
			strcpy(spell, "bless");
			break;
		case SPELL_DISPELL_FIELD         :
			strcpy(spell, "dispell_field");
			break;
		case SPELL_FIRE_PROTECTION       :
			strcpy(spell, "fire_protection");
			break;
		case SPELL_TELEKINESIS           :
			strcpy(spell, "telekinesis");
			break;
		case SPELL_CURSE                 :
			strcpy(spell, "curse");
			break;
		case SPELL_COLD_PROTECTION       :
			strcpy(spell, "cold_protection");
			break;

		// Level 5 
		case SPELL_RUNE_OF_GUARDING      :
			strcpy(spell, "rune_of_guarding");
			break;
		case SPELL_LEVITATE              :
			strcpy(spell, "levitate");
			break;
		case SPELL_CURE_POISON           :
			strcpy(spell, "cure_poison");
			break;
		case SPELL_REPEL_UNDEAD          :
			strcpy(spell, "repel_undead");
			break;
		case SPELL_POISON_PROJECTILE     :
			strcpy(spell, "poison_projectile");
			break;

		// Level 6 
		case SPELL_RISE_DEAD             :
			strcpy(spell, "raise_dead");
			break;
		case SPELL_PARALYSE              :
			strcpy(spell, "paralyse");
			break;
		case SPELL_CREATE_FIELD          :
			strcpy(spell, "create_field");
			break;
		case SPELL_DISARM_TRAP           :
			strcpy(spell, "disarm_trap");
			break;
		case SPELL_SLOW_DOWN             :
			strcpy(spell, "slowdown");
			break;

		// Level 7  
		case SPELL_FLYING_EYE            :
			strcpy(spell, "flying_eye");
			break;
		case SPELL_FIRE_FIELD            :
			strcpy(spell, "fire_field");
			break;
		case SPELL_ICE_FIELD             :
			strcpy(spell, "ice_field");
			break;
		case SPELL_LIGHTNING_STRIKE      :
			strcpy(spell, "lightning_strike");
			break;
		case SPELL_CONFUSE               :
			strcpy(spell, "confuse");
			break;

		// Level 8
		case SPELL_INVISIBILITY          :
			strcpy(spell, "invisibility");
			break;
		case SPELL_MANA_DRAIN            :
			strcpy(spell, "mana_drain");
			break;
		case SPELL_EXPLOSION             :
			strcpy(spell, "explosion");
			break;
		case SPELL_ENCHANT_WEAPON        :
			strcpy(spell, "enchant_weapon");
			break;
		case SPELL_LIFE_DRAIN            :
			strcpy(spell, "life_drain");
			break;

		// Level 9
		case SPELL_SUMMON_CREATURE       :
			strcpy(spell, "summon_creature");
			break;
		case SPELL_FAKE_SUMMON		     :
			strcpy(spell, "fake_summon");
			break;
		case SPELL_NEGATE_MAGIC          :
			strcpy(spell, "negate_magic");
			break;
		case SPELL_INCINERATE            :
			strcpy(spell, "incinerate");
			break;
		case SPELL_MASS_PARALYSE         :
			strcpy(spell, "mass_paralyse");
			break;

		// Level 10
		case SPELL_MASS_LIGHTNING_STRIKE :
			strcpy(spell, "mass_lightning_strike");
			break;
		case SPELL_CONTROL_TARGET        :
			strcpy(spell, "control");
			break;
		case SPELL_FREEZE_TIME           :
			strcpy(spell, "freeze_time");
			break;
		case SPELL_MASS_INCINERATE       :
			strcpy(spell, "mass_incinerate");
			break;
		default :
			return false;
	}

	return true;
}

void SPELLCAST_Notify(long num) {
	
	if(num < 0 || size_t(num) >= MAX_SPELLS)
		return;
		
	char spell[128];
	long source = spells[num].m_caster;

	if(!MakeSpellName(spell, spells[num].m_type))
		return;
	
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i] != NULL) {
			EVENT_SENDER = (source >= 0) ? entities[source] : NULL;
			char param[256];
			sprintf(param, "%s %ld", spell, (long)spells[num].m_caster_level);
			SendIOScriptEvent(entities[i], SM_SPELLCAST, param);
		}
	}
}

void SPELLCAST_NotifyOnlyTarget(long num)
{
	if(num < 0 || size_t(num) >= MAX_SPELLS)
		return;

	if(spells[num].m_target<0)
		return;

	char spell[128];
	long source = spells[num].m_caster;

	if(MakeSpellName(spell, spells[num].m_type)) {
		if(source >= 0)
			EVENT_SENDER = entities[source];
		else
			EVENT_SENDER = NULL;

		char param[256];
		sprintf(param,"%s %ld",spell,(long)spells[num].m_caster_level);
		SendIOScriptEvent(entities[spells[num].m_target], SM_SPELLCAST, param);
	}	
}

void SPELLEND_Notify(SpellBase & spell) {
	
	char spellName[128];
	long source = spell.m_caster;

	if(spell.m_type == SPELL_CONFUSE) {
		EVENT_SENDER = ValidIONum(source) ? entities[source] : NULL;
		
		if(ValidIONum(spell.m_target)) {
			if(MakeSpellName(spellName,spell.m_type)) {
				Entity * targ = entities[spell.m_target];
				char param[128];
				sprintf(param,"%s %ld", spellName, (long)spell.m_caster_level);
				SendIOScriptEvent(targ, SM_SPELLEND, param);
			}
		}
		return;
	}
	
	// we only notify player spells end.
	if(!MakeSpellName(spellName, spell.m_type)) {
		return;
	}
	
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i]) {
			EVENT_SENDER = ValidIONum(source) ? entities[source] : NULL;
			
			char param[128];
			sprintf(param, "%s %ld", spellName, (long)spell.m_caster_level);
			SendIOScriptEvent(entities[i], SM_SPELLEND, param);
		}
	}
}



//! Plays the sound of Fizzling spell
void ARX_SPELLS_Fizzle(long num) {
	if(num < 0) {
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE); // player fizzle
	} else {
		spells[num].m_tolive = 0;
		
		if(spells[num].m_caster >= 0) {
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &spells[num].m_caster_pos);
		}
	}
}

extern long PLAYER_PARALYSED;

extern long passwall;

void ARX_SPELLS_ManageMagic() {
	if(ARXmenu.currentmode!=AMCM_OFF)
		return;

	Entity *io = entities.player();

	if(!io)
		return;

	if ((io->animlayer[1].cur_anim == io->anims[ANIM_BARE_UNREADY]) ||
		(io->animlayer[1].cur_anim == io->anims[ANIM_DAGGER_UNREADY_PART_1]) ||
		(io->animlayer[1].cur_anim == io->anims[ANIM_1H_UNREADY_PART_1]) ||
		(io->animlayer[1].cur_anim == io->anims[ANIM_2H_UNREADY_PART_1]) ||
		(io->animlayer[1].cur_anim == io->anims[ANIM_MISSILE_UNREADY_PART_1]) ||
		(io->animlayer[1].cur_anim == io->anims[ANIM_DAGGER_UNREADY_PART_2]) ||
		(io->animlayer[1].cur_anim == io->anims[ANIM_1H_UNREADY_PART_2]) ||
		(io->animlayer[1].cur_anim == io->anims[ANIM_2H_UNREADY_PART_2]) ||
		(io->animlayer[1].cur_anim == io->anims[ANIM_MISSILE_UNREADY_PART_2]))
		return;

	snip++;

	if(   !(player.Current_Movement & PLAYER_CROUCH)
	   && !BLOCK_PLAYER_CONTROLS
	   && GInput->actionPressed(CONTROLS_CUST_MAGICMODE)
	   && !PLAYER_PARALYSED
	) {
		if(player.Interface & INTER_COMBATMODE) {
			WILLRETURNTOCOMBATMODE = true;

			ARX_INTERFACE_Combat_Mode(0);

			ResetAnim(&io->animlayer[1]);
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
			if(!(EERIEMouseButton & 1) && ARX_FLARES_broken == 0) {
				ARX_FLARES_broken = 2;
				PIPOrgb++;

				if(PIPOrgb > 2)
					PIPOrgb = 0;
			}
			
			if(EERIEMouseButton & 1) {
				Vec2s pos = DANAEMouse;
				if(TRUE_PLAYER_MOUSELOOK_ON) {
					pos = MemoMouse;
				}
				
				Vec2s pos2 = Lm;
				
				if(!ARX_FLARES_broken)
					FlareLine(pos2, pos);

				if(rnd() > 0.6)
					AddFlare(pos, 1.f, -1);
				else
					AddFlare(pos, 1.f, 3);
				
				OPIPOrgb = PIPOrgb;
				
				Lm = DANAEMouse;
				if(TRUE_PLAYER_MOUSELOOK_ON) {
					Lm = MemoMouse;
				}
				
				ARX_FLARES_broken=0;
				
				if(!ARX_SOUND_IsPlaying(SND_MAGIC_DRAW))
					ARX_SOUND_PlaySFX(SND_MAGIC_DRAW, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
			} else {
				ARX_SOUND_Stop(SND_MAGIC_DRAW);				
			}
			
			snip = 0;
		}
	} else {
		ARX_FLARES_broken = 1;
		PIPOrgb++;

		if(PIPOrgb > 2)
			PIPOrgb = 0;

		if(player.doingmagic != 0) { //==2)
			player.doingmagic = 0;//1
			if(io->anims[ANIM_CAST_END]) {
				changeAnimation(io, 1, io->anims[ANIM_CAST_END]);
			}
			ARX_FLARES_broken = 3;
		}
	}
	
	if(ARX_FLARES_broken == 3) {
		cur_arm=0;
		cur_mega=0;
		passwall=0;

		if(cur_mr != 3)
			cur_mr=0;

		if(cur_mx != 3)
			cur_mx=0;

		if(cur_rf != 3)
			cur_rf=0;

		if(cur_pom != 3)
			cur_pom=0;

		if(cur_pnux < 3)
			cur_pnux=0;

		if(cur_sm < 3)
			cur_sm=0;

		cur_bh=0;
		cur_sos=0;

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
		ARX_SPELLS_Analyse();

		if(!SpellMoves.empty())
			ARX_SPELLS_AnalyseSYMBOL();
	
		ARX_FLARES_broken = 1;
	}
}

/*!
 * Plays the sound of Fizzling spell plus "NO MANA" speech
 */
void ARX_SPELLS_FizzleNoMana(long num) {
	if(num < 0) {
		return;
	}
	if(spells[num].m_caster >= 0) {
		spells[num].m_tolive = 0;
		ARX_SPELLS_Fizzle(num);
	}
}

bool CanPayMana(long num, float cost, bool _bSound = true) {
	
	if(num < 0)
		return false;

	if(spells[num].m_flags & SPELLCAST_FLAG_NOMANA)
		return true;

	if(spells[num].m_caster == 0) {
		if(player.manaPool.current < cost) {
			ARX_SPELLS_FizzleNoMana(num);

			if(_bSound) {
				ARX_SPEECH_Add(getLocalised("player_cantcast"));
				ARX_SPEECH_AddSpeech(entities.player(), "player_cantcast", ANIM_TALK_NEUTRAL);
			}
			return false;
		}

		player.manaPool.current -= cost;
		return true;
	} else if(ValidIONum(spells[num].m_caster)) {
		if(entities[spells[num].m_caster]->ioflags & IO_NPC) {
			if(entities[spells[num].m_caster]->_npcdata->manaPool.current < cost) {
				ARX_SPELLS_FizzleNoMana(num);
				return false;
			}
			entities[spells[num].m_caster]->_npcdata->manaPool.current -= cost;
			return true;
		}
	}

	return false;
}

long TemporaryGetSpellTarget(const Vec3f * from) {
	
	float mindist = std::numeric_limits<float>::max();
	long found = 0;
	for(size_t i = 1; i < entities.size(); i++) {
		if(entities[i] && entities[i]->ioflags & IO_NPC) {
			float dist = glm::distance2(*from, entities[i]->pos);
			if(dist < mindist) {
				found = i;
				mindist = dist;
			}
		}
	}
	
	return found;
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_Init() {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		spells[i].m_tolive = 0;
		spells[i].m_exist = false;
		spells[i].m_pSpellFx = NULL;
	}
	
	spellRecognitionInit();
	
	RuneInfosFill();
	ARX_SPELLS_Init_Rects();
}

// Clears All Spells.
void ARX_SPELLS_ClearAll() {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].m_exist) {
			spells[i].m_tolive = 0;
			spells[i].m_exist = false;
			delete spells[i].m_pSpellFx, spells[i].m_pSpellFx = NULL;
		}
	}
	
	BOOST_FOREACH(Entity * e, entities) {
		if(e) {
			ARX_SPELLS_RemoveAllSpellsOn(e);
		}
	}
}

// Obtains a Free Spell slot
static long ARX_SPELLS_GetFree() {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(!spells[i].m_exist) {
			spells[i].m_longinfo_entity = -1;
			spells[i].m_longinfo_damage = -1;
			spells[i].m_longinfo_time = -1;
			spells[i].m_longinfo_summon_creature = -1;
			spells[i].m_longinfo_lower_armor = -1;
			spells[i].m_longinfo_light = -1;
			
			spells[i].m_longinfo2_light = -1;
			
			spells[i].m_targetHandles.clear();
			return i;
		}
	}
	
	return -1;
}

long ARX_SPELLS_GetInstance(SpellType typ) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].m_exist && spells[i].m_type == typ) {
			return i;
		}
	}
	
	return -1;
}

// Checks for an existing instance of this spelltype
bool ARX_SPELLS_ExistAnyInstance(SpellType typ) {
	return (ARX_SPELLS_GetInstance(typ) != -1);
}

long ARX_SPELLS_GetInstanceForThisCaster(SpellType typ, long caster) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].m_exist && spells[i].m_type == typ && spells[i].m_caster == caster) {
			return i;
		}
	}
	
	return -1;
}

static bool ARX_SPELLS_ExistAnyInstanceForThisCaster(SpellType typ, long caster) {
	return (ARX_SPELLS_GetInstanceForThisCaster(typ, caster) != -1);
}

// Plays the sound of aborted spell
void ARX_SPELLS_AbortSpellSound() {
	ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
}

void ARX_SPELLS_FizzleAllSpellsFromCaster(long num_caster) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].m_exist && spells[i].m_caster == num_caster) {
			spells[i].m_tolive = 0;
		}
	}
}

struct TARGETING_SPELL {
	SpellType typ;
	SpellcastFlags flags;
	long level;
	long target;
	long duration;
};

static TARGETING_SPELL t_spell;

long LOOKING_FOR_SPELL_TARGET=0;
unsigned long LOOKING_FOR_SPELL_TARGET_TIME=0;
void ARX_SPELLS_CancelSpellTarget() {
	t_spell.typ = SPELL_NONE;
	LOOKING_FOR_SPELL_TARGET=0;
}

void ARX_SPELLS_LaunchSpellTarget(Entity * io) {
	if(io) {
		ARX_SPELLS_Launch(t_spell.typ, 0 /* player */, t_spell.flags, t_spell.level,
		                  io->index(), t_spell.duration);
	}
}

float ARX_SPELLS_ApplyFireProtection(Entity * io,float damages)
{
	if(io) {
		long idx=ARX_SPELLS_GetSpellOn(io,SPELL_FIRE_PROTECTION);

		if(idx >= 0) {
			float modif = 1.f-((float)spells[idx].m_caster_level*( 1.0f / 10 ));

			modif = clamp(modif, 0.f, 1.f);

			damages *= modif;
		}

		if(io->ioflags & IO_NPC) {
			damages -= io->_npcdata->resist_fire*( 1.0f / 100 )*damages;

			if(damages < 0.f)
				damages=0.f;
		}
	}

	return damages;
}

float ARX_SPELLS_ApplyColdProtection(Entity * io,float damages)
{
	long idx=ARX_SPELLS_GetSpellOn(io,SPELL_COLD_PROTECTION);

	if(idx >= 0) {
		float modif=1.f-((float)spells[idx].m_caster_level*( 1.0f / 10 ));

		modif = clamp(modif, 0.f, 1.f);

		damages *= modif;
	}

	return damages;
}

float ARX_SPELLS_GetManaCost(SpellType spell, long index) {
	
	// Calculate the player's magic level
	float playerCasterLevel = player.m_skillFull.casting + player.m_attributeFull.mind;
	playerCasterLevel = clamp(playerCasterLevel * 0.1f, 1.f, 10.f);
	
	float casterLevel = ((index < 0) ? playerCasterLevel : spells[index].m_caster_level);
	
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
		case SPELL_TELEPORT:              return  10.f;
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

bool ARX_SPELLS_Launch(SpellType typ, long source, SpellcastFlags flagss, long levell, long target, long duration) {
	
	SpellcastFlags flags = flagss;
	long level = levell;

	if(cur_rf == 3) {
		flags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NOMANA;	
	}

	if(sp_max) {
		level = std::max(level, 15L);
	}
	
	if(source == 0 && !(flags & SPELLCAST_FLAG_NOCHECKCANCAST)) {
		for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
			if(SpellSymbol[i] != RUNE_NONE) {
				if(!( player.rune_flags & (RuneFlag)(1 << SpellSymbol[i]))) {
					ARX_SOUND_PlaySpeech("player_cantcast");
					CurrSpellSymbol = 0;
					ARX_SPELLS_ResetRecognition();
					return false;
				}
			}
		}
	}
	
	float Player_Magic_Level = 0;
	
	if(!source) {
		ARX_SPELLS_ResetRecognition();

		if(player.SpellToMemorize.bSpell) {
			CurrSpellSymbol = 0;
			player.SpellToMemorize.bSpell = false;
		}

		ARX_PLAYER_ComputePlayerFullStats();

		if(level == -1) {
			Player_Magic_Level = player.m_skillFull.casting + player.m_attributeFull.mind;
			Player_Magic_Level = clamp(Player_Magic_Level * 0.1f, 1.0f, 10.0f);
		} else {
			Player_Magic_Level = static_cast<float>(level);
		}
	}

	arx_assert(!(source && (flags & SPELLCAST_FLAG_PRECAST)));

	if(flags & SPELLCAST_FLAG_PRECAST) {
		int l = level;

		if(l <= 0) {
			l = checked_range_cast<int>(Player_Magic_Level);
		}

		SpellcastFlags flgs = flags;
		flgs &= ~SPELLCAST_FLAG_PRECAST;
		ARX_SPELLS_Precast_Add(typ, l, flgs, duration);
		return true;
	}
	
	if(target < 0 && source == 0)
	switch(typ) {
		case SPELL_LOWER_ARMOR:
		case SPELL_CURSE:
		case SPELL_PARALYSE:
		case SPELL_INCINERATE:
		case SPELL_SLOW_DOWN:
		case SPELL_CONFUSE:
		{
			LOOKING_FOR_SPELL_TARGET_TIME	= (unsigned long)(arxtime);
			LOOKING_FOR_SPELL_TARGET		= 1;
			t_spell.typ						= typ;
			t_spell.flags					= flags;
			t_spell.level					= level;
			t_spell.target					= target;
			t_spell.duration				= duration;
			return false;
		}			
		case SPELL_ENCHANT_WEAPON:		
		{
			LOOKING_FOR_SPELL_TARGET_TIME	= (unsigned long)(arxtime);
			LOOKING_FOR_SPELL_TARGET		= 2;
			t_spell.typ						= typ;
			t_spell.flags					= flags;
			t_spell.level					= level;
			t_spell.target					= target;
			t_spell.duration				= duration;
			return false;
		}	
		break;
		case SPELL_CONTROL_TARGET:
		{
			long tcount = 0;

			if(!ValidIONum(source))
				return false;

			Vec3f cpos = entities[source]->pos;

			for(size_t ii = 1 ; ii < entities.size(); ii++) {
				Entity * ioo = entities[ii];
				if(ioo && (ioo->ioflags & IO_NPC) && ioo->_npcdata->lifePool.current > 0.f
				   && ioo->show == SHOW_FLAG_IN_SCENE
				   && ioo->groups.find("demon") != ioo->groups.end()
				   && closerThan(ioo->pos, cpos, 900.f)) {
					tcount++;
				}
			}

			if(tcount == 0) {
				ARX_SOUND_PlaySFX( SND_MAGIC_FIZZLE, &cpos );
				return false;
			}

			ARX_SOUND_PlaySpeech("player_follower_attack");

			LOOKING_FOR_SPELL_TARGET_TIME	= (unsigned long)(arxtime);
			LOOKING_FOR_SPELL_TARGET		= 1;
			t_spell.typ						= typ;
			t_spell.flags					= flags;
			t_spell.level					= level;
			t_spell.target					= target;
			t_spell.duration				= duration;
			return false;
		}
		break;
		default: break;
	}

	if(source == 0) {
		ARX_SPELLS_CancelSpellTarget();
	}

	// Try to create a new spell instance
	long i = ARX_SPELLS_GetFree();

	if(i < 0) {
		return false;
	}
	
	SpellBase & spell = spells[i];
	
	if(ValidIONum(source) && spellicons[typ].bAudibleAtStart) {
		ARX_NPC_SpawnAudibleSound(entities[source]->pos, entities[source]);
	}
	
	spell.m_caster = source; // Caster...
	spell.m_target = target; // No target if <0

	if(target < 0)
		spell.m_target = TemporaryGetSpellTarget(&entities[spell.m_caster]->pos);

	// Create hand position if a hand is defined
	if(spell.m_caster == 0) {
		spell.m_hand_group = entities[spell.m_caster]->obj->fastaccess.primary_attach;
	} else {
		spell.m_hand_group = entities[spell.m_caster]->obj->fastaccess.left_attach;
	}

	if(spell.m_hand_group != -1) {
		spell.m_hand_pos = entities[spell.m_caster]->obj->vertexlist3[spell.m_hand_group].v;
	}
	
	if(source == 0) {
		// Player source
		spell.m_caster_level = Player_Magic_Level; // Level of caster
		spell.m_caster_pos = player.pos;
	} else {
		// IO source
		spell.m_caster_level = (float)clamp(level, 1l, 10l);
		spell.m_caster_pos = entities[source]->pos;
	}

	if(flags & SPELLCAST_FLAG_LAUNCHPRECAST) {
		spell.m_caster_level = static_cast<float>(level);
	}
	
	if(cur_rf == 3) {
		spell.m_caster_level += 2;
	}

	// Checks target TODO if ( target < 0 ) is already handled above!
	if(target < 0) {
		// no target... targeted by sight
		if(source == 0) {
			// no target... player spell targeted by sight
			spell.m_target_pos.x = player.pos.x - std::sin(radians(player.angle.getPitch()))*60.f;
			spell.m_target_pos.y = player.pos.y + std::sin(radians(player.angle.getYaw()))*60.f;
			spell.m_target_pos.z = player.pos.z + std::cos(radians(player.angle.getPitch()))*60.f;
		} else {
			// TODO entities[target] with target < 0 ??? - uh oh!
			spell.m_target_pos.x = entities[target]->pos.x - std::sin(radians(entities[target]->angle.getPitch()))*60.f;
			spell.m_target_pos.y = entities[target]->pos.y - 120.f;
			spell.m_target_pos.z = entities[target]->pos.z + std::cos(radians(entities[target]->angle.getPitch()))*60.f;
		}
	} else if (target==0) {
		// player target
		spell.m_target_pos = player.pos;
	} else {
		// IO target
		spell.m_target_pos = entities[target]->pos;
	}
	
	spell.m_flags = flags;
	spell.m_pSpellFx = NULL;
	spell.m_type = typ;
	spell.m_timcreation = (unsigned long)(arxtime);
	spell.m_fManaCostPerSecond = 0.f;

	if(!CanPayMana(i, ARX_SPELLS_GetManaCost(typ, i))) {
		return false;
	}
	
	if(!GLOBAL_MAGIC_MODE) {
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
		return false;
	}
	
	bool notifyAll = true;
	
	switch(typ) {
		case SPELL_NONE:
		return true;
		//****************************************************************************
		// LEVEL 1
		case SPELL_MAGIC_SIGHT: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].m_caster))
				return false;
			
			static_cast<MagicSightSpell &>(spell).Launch(duration);
			break;
		}
		case SPELL_MAGIC_MISSILE: {
			static_cast<MagicMissileSpell &>(spell).Launch(i);
			break;
		}
		case SPELL_IGNIT: {
			static_cast<IgnitSpell &>(spell).Launch(i);
			break;
		}
		case SPELL_DOUSE: {
			static_cast<DouseSpell &>(spell).Launch(i);
			break;
		}
		case SPELL_ACTIVATE_PORTAL: {
			static_cast<ActivatePortalSpell &>(spell).Launch();
			break;
		}
		//****************************************************************************
		// LEVEL 2
		case SPELL_HEAL: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].m_caster))
				return false;
			
			static_cast<HealSpell &>(spell).Launch(i, duration);
			break;
		}
		case SPELL_DETECT_TRAP: {
			static_cast<DetectTrapSpell &>(spell).Launch(i);
			break;
		}
		case SPELL_ARMOR: {
			static_cast<ArmorSpell &>(spell).Launch(duration, i);
			break;
		}
		case SPELL_LOWER_ARMOR: {
			static_cast<LowerArmorSpell &>(spell).Launch(duration, i);
			break;
		}
		case SPELL_HARM: {
			static_cast<HarmSpell &>(spell).Launch(duration);
			break;
		}
		//****************************************************************************
		// LEVEL 3
		case SPELL_SPEED: {
			static_cast<SpeedSpell &>(spell).Launch(i, duration);
			break;
		}
		case SPELL_DISPELL_ILLUSION: {
			static_cast<DispellIllusionSpell &>(spell).Launch();
			break;
		}
		case SPELL_FIREBALL: {
			static_cast<FireballSpell &>(spell).Launch(i);
			break;
		}
		case SPELL_CREATE_FOOD: {
			static_cast<CreateFoodSpell &>(spell).Launch(duration);
			break;
		}
		case SPELL_ICE_PROJECTILE: {
			static_cast<IceProjectileSpell &>(spell).Launch(i);
			break;
		}
		//****************************************************************************
		// LEVEL 4
		case SPELL_BLESS: {
			if(ARX_SPELLS_ExistAnyInstance(typ))
				return false;
			
			static_cast<BlessSpell &>(spell).Launch(i, duration);
			break;
		}
		case SPELL_DISPELL_FIELD: {
			static_cast<DispellFieldSpell &>(spell).Launch();
			break;
		}
		case SPELL_FIRE_PROTECTION: {
			static_cast<FireProtectionSpell &>(spell).Launch(i, duration);
			break;
		}
		case SPELL_COLD_PROTECTION: {
			static_cast<ColdProtectionSpell &>(spell).Launch(i, duration);
			break;
		}
		case SPELL_TELEKINESIS: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].m_caster))
				return false;
			
			static_cast<TelekinesisSpell &>(spell).Launch(duration);
			break;
		}
		case SPELL_CURSE: {
			static_cast<CurseSpell &>(spell).Launch(duration, i);
			break;
		}
		//****************************************************************************
		// LEVEL 5
		case SPELL_RUNE_OF_GUARDING: {
			static_cast<RuneOfGuardingSpell &>(spell).Launch(duration);
			break;
		}
		case SPELL_LEVITATE: {
			static_cast<LevitateSpell &>(spell).Launch(duration, i);
			break;
		}
		case SPELL_CURE_POISON: {
			static_cast<CurePoisonSpell &>(spell).Launch(i);
			break;
		}
		case SPELL_REPEL_UNDEAD: {
			static_cast<RepelUndeadSpell &>(spell).Launch(duration, i);
			break;
		}
		case SPELL_POISON_PROJECTILE: {
			static_cast<PoisonProjectileSpell &>(spell).Launch(i);
			break;
		}
		//****************************************************************************
		// LEVEL 6
		case SPELL_RISE_DEAD: {
			bool result = static_cast<RiseDeadSpell &>(spell).Launch(duration);
			if(!result)
				return false;
			
			break;
		}
		case SPELL_PARALYSE: {
			static_cast<ParalyseSpell &>(spell).Launch(i, duration);
			break;
		}
		case SPELL_CREATE_FIELD: {
			static_cast<CreateFieldSpell &>(spell).Launch(flags, duration);
			break;
		}
		case SPELL_DISARM_TRAP: {
			static_cast<DisarmTrapSpell &>(spell).Launch();
			break;
		}
		case SPELL_SLOW_DOWN: {
			bool result = static_cast<SlowDownSpell &>(spell).Launch(duration, i);
			if(!result)
				return false;
			
			break;
		}
		//****************************************************************************
		// LEVEL 7
		case SPELL_FLYING_EYE: {
			if(eyeball.exist)
				return false;

			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].m_caster))
				return false;
			
			bool result = static_cast<FlyingEyeSpell &>(spell).Launch();
			if(!result)
				return false;
			
			break;
		}
		case SPELL_FIRE_FIELD: {
			static_cast<FireFieldSpell &>(spell).Launch(duration);
			break;
		}
		case SPELL_ICE_FIELD: {
			static_cast<IceFieldSpell &>(spell).Launch(duration);
			break;
		}
		case SPELL_LIGHTNING_STRIKE: {
			static_cast<LightningStrikeSpell &>(spell).Launch(i);
			break;
		}
		case SPELL_CONFUSE: {
			static_cast<ConfuseSpell &>(spell).Launch(i, notifyAll, duration); // TODO inconsistent use of the SM_SPELLCAST event
			break;
		}
		//****************************************************************************
		// LEVEL 8
		case SPELL_INVISIBILITY: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].m_caster))
				return false;
			
			static_cast<InvisibilitySpell &>(spell).Launch(i, duration);
			break;
		}
		case SPELL_MANA_DRAIN: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].m_caster))
				return false;
			
			static_cast<ManaDrainSpell &>(spell).Launch(duration);
			break;
		}
		case SPELL_EXPLOSION: {
			static_cast<ExplosionSpell &>(spell).Launch();
			break;
		}
		case SPELL_ENCHANT_WEAPON: {
			static_cast<EnchantWeaponSpell &>(spell).Launch(notifyAll); // TODO inconsistent use of the SM_SPELLCAST event
			break;
		}
		case SPELL_LIFE_DRAIN: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].m_caster))
				return false;
			
			static_cast<LifeDrainSpell &>(spell).Launch(duration);
			break;
		}
		//****************************************************************************
		// LEVEL 9
		case SPELL_SUMMON_CREATURE: {
			bool result = static_cast<SummonCreatureSpell &>(spell).Launch(duration);
			if(!result)
				return false;
			
			break;
		}
		case SPELL_FAKE_SUMMON: {
			bool result = static_cast<FakeSummonSpell &>(spell).Launch();
			if(!result)
				return false;
			
			break;
		}
		case SPELL_NEGATE_MAGIC: {
			static_cast<NegateMagicSpell &>(spell).Launch(duration, i);
			break;
		}
		case SPELL_INCINERATE: {
			bool result = static_cast<IncinerateSpell &>(spell).Launch(i);
			if(!result)
				return false;
			
			break;
		}
		case SPELL_MASS_PARALYSE: {
			static_cast<MassParalyseSpell &>(spell).Launch(i, duration);
			break;
		}
		//****************************************************************************
		// LEVEL 10
		case SPELL_MASS_LIGHTNING_STRIKE: {
			static_cast<MassLightningStrikeSpell &>(spell).Launch(i);
			break;
		}
		case SPELL_CONTROL_TARGET: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].m_caster))
				return false;
			
			bool result = static_cast<ControlTargetSpell &>(spell).Launch();
			if(!result)
				return false;
			
			break;
		}
		case SPELL_FREEZE_TIME: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].m_caster))
				return false;
			
			static_cast<FreezeTimeSpell &>(spell).Launch(duration);
			break;
		}
		case SPELL_MASS_INCINERATE: {
			static_cast<MassIncinerateSpell &>(spell).Launch(i);
			break;
		}
		case SPELL_TELEPORT: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].m_caster))
				return false;
			
			static_cast<TeleportSpell &>(spell).Launch();
			break;
		}
	}
	
	if(notifyAll) {
		SPELLCAST_Notify(i);
	} else {
		SPELLCAST_NotifyOnlyTarget(i);
	}
	
	return true;
}

void ARX_SPELLS_Update_End(size_t i) {
	
	SpellBase & spell = spells[i];
	
	switch(spells[i].m_type) {
		case SPELL_TELEPORT: {
			static_cast<TeleportSpell &>(spell).End();
			break;
		}
		//****************************************************************************
		// LEVEL 1 SPELLS
		case SPELL_MAGIC_SIGHT: {
			static_cast<MagicSightSpell &>(spell).End();
			break;
		}
		case SPELL_MAGIC_MISSILE: {
			static_cast<MagicMissileSpell &>(spell).End();
			break;
		}
		case SPELL_IGNIT: {
			static_cast<IgnitSpell &>(spell).End();
			break;
		}
		case SPELL_DOUSE: {
			static_cast<DouseSpell &>(spell).End();
			break;
		}
		//****************************************************************************
		// LEVEL 2
		case SPELL_HARM: {
			static_cast<HarmSpell &>(spell).End();
			break;
		}
		case SPELL_DETECT_TRAP: {
			static_cast<DetectTrapSpell &>(spell).End(i);
			break;
		}
		case SPELL_ARMOR: {
			static_cast<ArmorSpell &>(spell).End(i);
			break;
		}
		case SPELL_LOWER_ARMOR: {
			static_cast<LowerArmorSpell &>(spell).End(i);
			break;
		}
		//****************************************************************************
		// LEVEL 3
		case SPELL_SPEED: {
			static_cast<SpeedSpell &>(spell).End(i);
			break;
		}
		case SPELL_FIREBALL: {
			static_cast<FireballSpell &>(spell).End();
			break;
		}
		//****************************************************************************
		// LEVEL 4
		case SPELL_BLESS: {
			static_cast<BlessSpell &>(spell).End(i);
			break;
		}
		case SPELL_CURSE: {
			static_cast<CurseSpell &>(spell).End(i);
			break;
		}
		case SPELL_TELEKINESIS: {
			static_cast<TelekinesisSpell &>(spell).End();
			break;
		}
		case SPELL_FIRE_PROTECTION: {
			static_cast<FireProtectionSpell &>(spell).End(i);
			break;
		}
		case SPELL_COLD_PROTECTION: {
			static_cast<ColdProtectionSpell &>(spell).End(i);
			break;
		}
		//****************************************************************************
		// LEVEL 5
		case SPELL_LEVITATE: {
			static_cast<LevitateSpell &>(spell).End(i);
			break;
		}
		case SPELL_REPEL_UNDEAD: {
			static_cast<RepelUndeadSpell &>(spell).End();
			break;
		}
		//****************************************************************************
		// LEVEL 6 SPELLS
		case SPELL_PARALYSE: {
			static_cast<ParalyseSpell &>(spell).End(i);
			break;
		}
		case SPELL_RISE_DEAD: {
			static_cast<RiseDeadSpell &>(spell).End();
			break;
		}
		case SPELL_CREATE_FIELD: {
			static_cast<CreateFieldSpell &>(spell).End();
			break;
		}
		case SPELL_SLOW_DOWN: {
			static_cast<SlowDownSpell &>(spell).End(i);
			break;
		}
		//****************************************************************************
		// LEVEL 7
		case SPELL_ICE_FIELD: {
			static_cast<IceFieldSpell &>(spell).End();
			break;
		}
		case SPELL_FIRE_FIELD: {
			static_cast<FireFieldSpell &>(spell).End();
			break;
		}
		case SPELL_LIGHTNING_STRIKE: {
			static_cast<LightningStrikeSpell &>(spell).End();
			break;
		}
		case SPELL_FLYING_EYE: {
			static_cast<FlyingEyeSpell &>(spell).End();
			break;
		}
		case SPELL_CONFUSE: {
			static_cast<ConfuseSpell &>(spell).End(i);
			break;
		}
		//****************************************************************************
		// LEVEL 8
		case SPELL_EXPLOSION: {
			break;
		}
		case SPELL_INVISIBILITY: {
			static_cast<InvisibilitySpell &>(spell).End(i);
			break;
		}
		case SPELL_MANA_DRAIN: {
			static_cast<ManaDrainSpell &>(spell).End();
			break;
		}
		case SPELL_LIFE_DRAIN: {
			static_cast<LifeDrainSpell &>(spell).End();
			break;
		}
		//****************************************************************************
		// LEVEL 9
		case SPELL_MASS_PARALYSE: {
			static_cast<MassParalyseSpell &>(spell).End(i);
			break;
		}
		case SPELL_SUMMON_CREATURE : {
			static_cast<SummonCreatureSpell &>(spell).End();
			break;
		}
		case SPELL_FAKE_SUMMON: {
			static_cast<FakeSummonSpell &>(spell).End();
			break;
		}
		case SPELL_INCINERATE: {
			static_cast<IncinerateSpell &>(spell).End(i);
			break;
		}
		//****************************************************************************
		// LEVEL 10
		case SPELL_MASS_LIGHTNING_STRIKE: {
			static_cast<MassLightningStrikeSpell &>(spell).End();
			break;
		}
		case SPELL_FREEZE_TIME: {
			static_cast<FreezeTimeSpell &>(spell).End();
			break;
		}
		case SPELL_MASS_INCINERATE: {
			static_cast<MassIncinerateSpell &>(spell).End(i);
			break;
		}
		default:
			break;
	}
	
	spell.BaseEnd();
}

void ARX_SPELLS_Update_Update(size_t i, unsigned long tim) {
	
	SpellBase & spell = spells[i];
	
	switch(spells[i].m_type) {
		case SPELL_DISPELL_FIELD: {
			break;
		}
		case SPELL_NONE: {
			break;
		}
		//****************************************************************************
		// LEVEL 1
		case SPELL_MAGIC_MISSILE: {
			static_cast<MagicMissileSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_IGNIT: {
			static_cast<IgnitSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_DOUSE: {
			static_cast<DouseSpell &>(spell).Update(framedelay);
			break;
		} 
		case SPELL_ACTIVATE_PORTAL: {
			break;
		}
		//****************************************************************************
		// LEVEL 2
		case SPELL_HEAL: {
			static_cast<HealSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_DETECT_TRAP: {
			static_cast<DetectTrapSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_ARMOR: {
			static_cast<ArmorSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_LOWER_ARMOR: {
			static_cast<LowerArmorSpell &>(spell).Update(framedelay);
			break;
		} 
		case SPELL_HARM: {
			static_cast<HarmSpell &>(spell).Update(framedelay);
			break;
		}
		//****************************************************************************
		// LEVEL 3 SPELLS
		case SPELL_FIREBALL: {
			static_cast<FireballSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_SPEED: {
			static_cast<SpeedSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_CREATE_FOOD: {
			static_cast<CreateFoodSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_ICE_PROJECTILE: {
			static_cast<IceProjectileSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_DISPELL_ILLUSION: {
			static_cast<DispellIllusionSpell &>(spell).Update(framedelay);
			break;
		}
		//****************************************************************************
		// LEVEL 4 SPELLS
		case SPELL_BLESS: {
			static_cast<BlessSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_CURSE: {
			static_cast<CurseSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_FIRE_PROTECTION: {
			static_cast<FireProtectionSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_COLD_PROTECTION: {
			static_cast<ColdProtectionSpell &>(spell).Update(framedelay);
			break;
		}
		//****************************************************************************
		// LEVEL 5 SPELLS
		case SPELL_CURE_POISON: {
			static_cast<CurePoisonSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_RUNE_OF_GUARDING: {
			static_cast<RuneOfGuardingSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_REPEL_UNDEAD: {
			static_cast<RepelUndeadSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_POISON_PROJECTILE: {
			static_cast<PoisonProjectileSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_LEVITATE: {
			static_cast<LevitateSpell &>(spell).Update(framedelay);
			break;
		}
		//****************************************************************************
		// LEVEL 6 SPELLS
		case SPELL_RISE_DEAD: {
			static_cast<RiseDeadSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_SLOW_DOWN: {
			static_cast<SlowDownSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_DISARM_TRAP: {
			break;
		}
		case SPELL_PARALYSE: {
			break;
		}
		case SPELL_CREATE_FIELD: {
			static_cast<CreateFieldSpell &>(spell).Update(framedelay);
			break;
		}
		//****************************************************************************
		// LEVEL 7 SPELLS
		case SPELL_CONFUSE: {
			static_cast<ConfuseSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_FIRE_FIELD: {
			static_cast<FireFieldSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_ICE_FIELD: {
			static_cast<IceFieldSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_LIGHTNING_STRIKE: {
			static_cast<LightningStrikeSpell &>(spell).Update(framedelay);
			break;
		}
		//****************************************************************************
		// LEVEL 8 SPELLS
		case SPELL_ENCHANT_WEAPON: {
			static_cast<EnchantWeaponSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_EXPLOSION: {
			static_cast<ExplosionSpell &>(spell).Update();
			break;
		}
		//****************************************************************************
		// LEVEL 9 SPELLS
		case SPELL_SUMMON_CREATURE: {
			static_cast<SummonCreatureSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_FAKE_SUMMON: {
			static_cast<FakeSummonSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_INCINERATE: {
			static_cast<IncinerateSpell &>(spell).Update();
			break;
		}
		case SPELL_NEGATE_MAGIC: {
			static_cast<NegateMagicSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_MASS_PARALYSE: {
			break;
		}
		//****************************************************************************
		// LEVEL 10 SPELLS
		case SPELL_FREEZE_TIME: {
			break;
		}
		case SPELL_CONTROL_TARGET: {
			static_cast<ControlTargetSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_MASS_INCINERATE: {
			static_cast<MassIncinerateSpell &>(spell).Update();
			break;
		}
		case SPELL_MASS_LIGHTNING_STRIKE: {
			static_cast<MassLightningStrikeSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_TELEPORT: {
			static_cast<TeleportSpell &>(spell).Update(tim);
			break;
		}
		case SPELL_MAGIC_SIGHT: {
			static_cast<MagicSightSpell &>(spell).Update();
			break;
		}
		case SPELL_TELEKINESIS: {
			break;
		}
		case SPELL_INVISIBILITY: {
			static_cast<InvisibilitySpell &>(spell).Update(i);
			break;
		}
		case SPELL_MANA_DRAIN: {
			static_cast<ManaDrainSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_LIFE_DRAIN: {
			static_cast<LifeDrainSpell &>(spell).Update(framedelay);
			break;
		}
		case SPELL_FLYING_EYE: {
			static_cast<FlyingEyeSpell &>(spell).Update(tim);
			break;
		}
	}
}


/*!
 * \brief Updates all currently working spells.
 */
void ARX_SPELLS_Update() {
	
	ucFlick++;
	
	const unsigned long tim = (unsigned long)(arxtime);
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		SpellBase & spell = spells[i];
		
		if(!GLOBAL_MAGIC_MODE) {
			spell.m_tolive = 0;
		}

		if(!spell.m_exist) {
			continue;
		}
		
		if(spell.m_bDuration && !CanPayMana(i, spell.m_fManaCostPerSecond * (float)framedelay * (1.0f/1000), false))
			ARX_SPELLS_Fizzle(i);
		
		const long framediff = spell.m_timcreation + spell.m_tolive - tim;
		
		if(framediff < 0) {
			SPELLEND_Notify(spell);
			ARX_SPELLS_Update_End(i);
			
			continue;
		}

		if(spell.m_exist) {
			ARX_SPELLS_Update_Update(i, tim);
		}
	}
}

void TryToCastSpell(Entity * io, SpellType spellType, long level, long target, SpellcastFlags flags, long duration)
{
	if(!io || io->spellcast_data.castingspell != SPELL_NONE)
		return;

	if(!(flags & SPELLCAST_FLAG_NOMANA) && (io->ioflags & IO_NPC) && (io->_npcdata->manaPool.current<=0.f))
		return;

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
	
	io->gameFlags &=~GFLAG_INVISIBILITY;
	
	if (	((io->spellcast_data.spell_flags & SPELLCAST_FLAG_NOANIM)
		&&	(io->spellcast_data.spell_flags & SPELLCAST_FLAG_NODRAW) )
		||	(io->spellcast_data.spell_flags & SPELLCAST_FLAG_PRECAST))	
	{
		
		ARX_SPELLS_Launch(io->spellcast_data.castingspell, io->index(), io->spellcast_data.spell_flags,io->spellcast_data.spell_level,io->spellcast_data.target,io->spellcast_data.duration);
		io->spellcast_data.castingspell = SPELL_NONE;
	}

	io->spellcast_data.spell_flags &=~SPELLCAST_FLAG_NODRAW; // temporary, removes colored flares
}

