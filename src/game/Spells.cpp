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

using std::abs;
using std::string;

static const float DEC_FOCAL = 50.0f;
static const float IMPROVED_FOCAL = 320.0f;

extern bool TRUE_PLAYER_MOUSELOOK_ON;

bool WILLRETURNTOFREELOOK = false;
bool GLOBAL_MAGIC_MODE = true;

extern long MAGICMODE;
extern float GLOBAL_SLOWDOWN;

extern bool bRenderInCursorMode;

bool bOldLookToggle;
extern float SLID_START;

extern bool FrustrumsClipSphere(EERIE_FRUSTRUM_DATA * frustrums,EERIE_SPHERE * sphere);

void ARX_INTERFACE_Combat_Mode(long i);



///////////////Spell Interpretation
SPELL spells[MAX_SPELLS];
short ARX_FLARES_broken(1);

float LASTTELEPORT = 0.0F;
long snip=0;
static Vec2s Lm;


unsigned char ucFlick=0;

bool GetSpellPosition(Vec3f * pos,long i)
{
	switch(spells[i].type) {
		//****************************************************************************
		// LEVEL 1
		case SPELL_MAGIC_SIGHT:
		break;
		case SPELL_MAGIC_MISSILE:
		break;
		case SPELL_IGNIT:
		break;
		case SPELL_DOUSE:
		break;
		case SPELL_ACTIVATE_PORTAL:
		break;			
		//****************************************************************************
		// LEVEL 2
		case SPELL_HEAL:
		break;
		case SPELL_DETECT_TRAP:
		break;
		case SPELL_ARMOR:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		case SPELL_LOWER_ARMOR:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		case SPELL_HARM:
		break;			
		//****************************************************************************
		// LEVEL 3
		case SPELL_SPEED:// Launching SPEED
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		case SPELL_DISPELL_ILLUSION:// Launching DISPELL_ILLUSION (REVEAL)
		break;
		case SPELL_FIREBALL:
		break;
		case SPELL_CREATE_FOOD:
		break;
		case SPELL_ICE_PROJECTILE:
		break;
		//****************************************************************************
		// LEVEL 4
		case SPELL_BLESS:// Launching BLESS
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		case SPELL_DISPELL_FIELD:
		break;
		case SPELL_FIRE_PROTECTION:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		case SPELL_COLD_PROTECTION:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		case SPELL_TELEKINESIS:
		break;
		case SPELL_CURSE:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//****************************************************************************
		// LEVEL 5
		case SPELL_RUNE_OF_GUARDING: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if (pCSpellFX)
			{
				CRuneOfGuarding *pCRG = (CRuneOfGuarding *) pCSpellFX;
					
				*pos = pCRG->eSrc;
				return true;
			}
		}
		break;
		case SPELL_LEVITATE:
		break;
		case SPELL_CURE_POISON:
		break;
		case SPELL_REPEL_UNDEAD:
		break;
		case SPELL_POISON_PROJECTILE:
		break;
		//****************************************************************************
		// LEVEL 6
		case SPELL_RISE_DEAD:
		break;
		case SPELL_PARALYSE:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		case SPELL_CREATE_FIELD: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if (pCSpellFX)
			{
				CCreateField *pCreateField = (CCreateField *) pCSpellFX;
					
				*pos = pCreateField->eSrc;
				return true;
			}
		}
		break;
		case SPELL_DISARM_TRAP:
		break;
		case SPELL_SLOW_DOWN:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		//****************************************************************************
		// LEVEL 7
		case SPELL_FLYING_EYE:
		{	
			*pos = eyeball.pos;
			return true;
		}	
		break;
		case SPELL_FIRE_FIELD: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if (pCSpellFX)
			{
				CFireField *pFireField = (CFireField *) pCSpellFX;
					
				*pos = pFireField->pos;
				return true;
			}
		}
		break;
		case SPELL_ICE_FIELD: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if (pCSpellFX)
			{
				CIceField *pIceField = (CIceField *) pCSpellFX;
					
				*pos = pIceField->eSrc;
				return true;
			}
		}
		break;
		case SPELL_LIGHTNING_STRIKE:
		break;
		case SPELL_CONFUSE:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		//****************************************************************************
		// LEVEL 8
		case SPELL_INVISIBILITY:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		case SPELL_MANA_DRAIN:				
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		case SPELL_EXPLOSION:
		break;
		case SPELL_ENCHANT_WEAPON:
		break;			
		case SPELL_LIFE_DRAIN:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		//****************************************************************************
		// LEVEL 9
		case SPELL_SUMMON_CREATURE:
		break;
		case SPELL_NEGATE_MAGIC:
		break;
		case SPELL_INCINERATE:
			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}
		break;
		case SPELL_MASS_PARALYSE:
		break;
		//****************************************************************************
		// LEVEL 10
		case SPELL_MASS_LIGHTNING_STRIKE:
		break;
		case SPELL_CONTROL_TARGET:
		break;
		case SPELL_FREEZE_TIME:
		break;
		case SPELL_MASS_INCINERATE:
		break;
		case SPELL_TELEPORT:
		break;
		default:
		break;
	}

	if(ValidIONum(spells[i].caster)) {
		*pos = entities[spells[i].caster]->pos;
		return true;
	}

	return false;
}

bool spellHandleIsValid(long handle) {
	return (long)handle >= 0 && ((size_t)handle < MAX_SPELLS) && spells[handle].exist;
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
			SPELL * spell = &spells[spellHandle];
			
			if(spell->type == spellid) {
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

void ARX_SPELLS_RemoveMultiSpellOn(long spell_id) {
	for(size_t i = 0; i < entities.size(); i++) {
		ARX_SPELLS_RemoveSpellOn(i, spells[spell_id].type);
	}
}

void ARX_SPELLS_RemoveAllSpellsOn(Entity *io) {
	io->spellsOn.clear();
}

static bool MakeSpellName(char * spell, SpellType num) {
	
	// TODO(spells) use map
	
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
	long source = spells[num].caster;

	if(!MakeSpellName(spell, spells[num].type))
		return;
	
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i] != NULL) {
			EVENT_SENDER = (source >= 0) ? entities[source] : NULL;
			char param[256];
			sprintf(param, "%s %ld", spell, (long)spells[num].caster_level);
			SendIOScriptEvent(entities[i], SM_SPELLCAST, param);
		}
	}
}

void SPELLCAST_NotifyOnlyTarget(long num)
{
	if(num < 0 || size_t(num) >= MAX_SPELLS)
		return;

	if(spells[num].target<0)
		return;

	char spell[128];
	long source = spells[num].caster;

	if(MakeSpellName(spell, spells[num].type)) {
		if(source >= 0)
			EVENT_SENDER = entities[source];
		else
			EVENT_SENDER = NULL;

		char param[256];
		sprintf(param,"%s %ld",spell,(long)spells[num].caster_level);
		SendIOScriptEvent(entities[spells[num].target], SM_SPELLCAST, param);
	}	
}

void SPELLEND_Notify(SPELL & spell) {
	
	char spellName[128];
	long source = spell.caster;

	if(spell.type == SPELL_CONFUSE) {
		EVENT_SENDER = ValidIONum(source) ? entities[source] : NULL;
		
		if(ValidIONum(spell.target)) {
			if(MakeSpellName(spellName,spell.type)) {
				Entity * targ = entities[spell.target];
				char param[128];
				sprintf(param,"%s %ld", spellName, (long)spell.caster_level);
				SendIOScriptEvent(targ, SM_SPELLEND, param);
			}
		}
		return;
	}
	
	// we only notify player spells end.
	if(!MakeSpellName(spellName, spell.type)) {
		return;
	}
	
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i]) {
			EVENT_SENDER = ValidIONum(source) ? entities[source] : NULL;
			
			char param[128];
			sprintf(param, "%s %ld", spellName, (long)spell.caster_level);
			SendIOScriptEvent(entities[i], SM_SPELLEND, param);
		}
	}
}



//! Plays the sound of Fizzling spell
void ARX_SPELLS_Fizzle(long num) {
	if(num < 0) {
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE); // player fizzle
	} else {
		spells[num].tolive = 0;
		
		if(spells[num].caster >= 0) {
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &spells[num].caster_pos);
		}
	}
}


bool No_MagicAllowed()
{
	ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
	return false;
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
				MAGICMODE = 1;
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
	if(spells[num].caster >= 0) {
		spells[num].tolive = 0;
		ARX_SPELLS_Fizzle(num);
	}
}

long CanPayMana(long num, float cost, bool _bSound = true) {
	
	if(num < 0)
		return 0;

	if(spells[num].flags & SPELLCAST_FLAG_NOMANA)
		return 1;

	if(spells[num].caster == 0) {
		if(player.mana < cost) {
			ARX_SPELLS_FizzleNoMana(num);

			if(_bSound) {
				ARX_SPEECH_Add(getLocalised("player_cantcast"));
				ARX_SPEECH_AddSpeech(entities.player(), "player_cantcast", ANIM_TALK_NEUTRAL);
			}
			return 0;
		}

		player.mana -= cost;
		return 1;
	} else if(ValidIONum(spells[num].caster)) {
		if(entities[spells[num].caster]->ioflags & IO_NPC) {
			if(entities[spells[num].caster]->_npcdata->mana < cost) {
				ARX_SPELLS_FizzleNoMana(num);
				return 0;
			}
			entities[spells[num].caster]->_npcdata->mana -= cost;
			return 1;
		}
	}

	return 0;
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
		spells[i].tolive = 0;
		spells[i].exist = false;
		spells[i].pSpellFx = NULL;
	}
	
	spellRecognitionInit();
	
	RuneInfosFill();
	ARX_SPELLS_Init_Rects();
}

// Clears All Spells.
void ARX_SPELLS_ClearAll() {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].exist) {
			spells[i].tolive = 0;
			spells[i].exist = false;
			delete spells[i].pSpellFx, spells[i].pSpellFx = NULL;
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
		if(!spells[i].exist) {
			spells[i].longinfo_entity = -1;
			spells[i].longinfo_damage = -1;
			spells[i].longinfo_time = -1;
			spells[i].longinfo_summon_creature = -1;
			spells[i].longinfo_lower_armor = -1;
			spells[i].longinfo_light = -1;
			
			spells[i].longinfo2_light = -1;
			spells[i].misc = NULL;
			return i;
		}
	}
	
	return -1;
}

long ARX_SPELLS_GetInstance(SpellType typ) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].exist && spells[i].type == typ) {
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
		if(spells[i].exist && spells[i].type == typ && spells[i].caster == caster) {
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
		if(spells[i].exist && spells[i].caster == num_caster) {
			spells[i].tolive = 0;
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

float ARX_SPELLS_GetManaCost(SpellType spell, long index) {
	
	// Calculate the player's magic level
	float playerCasterLevel = player.Full_Skill_Casting + player.Full_Attribute_Mind;
	playerCasterLevel = clamp(playerCasterLevel * 0.1f, 1.f, 10.f);
	
	float casterLevel = ((index < 0) ? playerCasterLevel : spells[index].caster_level);
	
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
		level = std::max( level, 15L );
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
			CurrSpellSymbol					= 0;
			player.SpellToMemorize.bSpell	= false;
		}

		ARX_PLAYER_ComputePlayerFullStats();

		if(level == -1) {
			Player_Magic_Level = (float) player.Full_Skill_Casting + player.Full_Attribute_Mind;
			Player_Magic_Level = clamp(Player_Magic_Level * 0.1f, 1.0f, 10.0f);
		} else {
			Player_Magic_Level = static_cast<float>(level);
		}
	}

	arx_assert( !( source && (flags & SPELLCAST_FLAG_PRECAST) ) );

	if(flags & SPELLCAST_FLAG_PRECAST) {
		int l = level;

		if(l <= 0) {
			l = checked_range_cast<int>(Player_Magic_Level);
		}

		SpellcastFlags flgs=flags;
		flgs&=~SPELLCAST_FLAG_PRECAST;
		ARX_SPELLS_Precast_Add( typ, l, flgs, duration);
		return true;
	}
	
	static TextureContainer * tc4 = TextureContainer::Load("graph/particles/smoke");

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
				if(ioo && (ioo->ioflags & IO_NPC) && ioo->_npcdata->life > 0.f
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
	
	if(ValidIONum(source) && spellicons[typ].bAudibleAtStart) {
		ARX_NPC_SpawnAudibleSound(entities[source]->pos, entities[source]);
	}
	
	spells[i].caster = source;	// Caster...
	spells[i].target = target;	// No target if <0

	if(target < 0)
		spells[i].target = TemporaryGetSpellTarget( &entities[spells[i].caster]->pos );

	// Create hand position if a hand is defined
	if(spells[i].caster == 0) {
		spells[i].hand_group = entities[spells[i].caster]->obj->fastaccess.primary_attach;
	} else {
		spells[i].hand_group = entities[spells[i].caster]->obj->fastaccess.left_attach;
	}

	if(spells[i].hand_group != -1) {
		spells[i].hand_pos = entities[spells[i].caster]->obj->vertexlist3[spells[i].hand_group].v;
	}
	
	if(source == 0) {
		// Player source
		spells[i].caster_level = Player_Magic_Level; // Level of caster
		spells[i].caster_pos = player.pos;
	} else {
		// IO source
		spells[i].caster_level = (float)clamp(level, 1l, 10l);
		spells[i].caster_pos = entities[source]->pos;
	}

	if(flags & SPELLCAST_FLAG_LAUNCHPRECAST) {
		spells[i].caster_level = static_cast<float>(level);
	}
	
	if(cur_rf == 3) {
		spells[i].caster_level += 2;
	}

	// Checks target TODO if ( target < 0 ) is already handled above!
	if (target<0) // no target... targeted by sight
	{
		if (source==0) // no target... player spell targeted by sight
		{
			spells[i].target_pos.x = player.pos.x - std::sin(radians(player.angle.getPitch()))*60.f;
			spells[i].target_pos.y = player.pos.y + std::sin(radians(player.angle.getYaw()))*60.f;
			spells[i].target_pos.z = player.pos.z + std::cos(radians(player.angle.getPitch()))*60.f;
		}
		else
		{
			// TODO entities[target] with target < 0 ??? - uh oh!
			spells[i].target_pos.x = entities[target]->pos.x - std::sin(radians(entities[target]->angle.getPitch()))*60.f;
			spells[i].target_pos.y=entities[target]->pos.y-120.f;
			spells[i].target_pos.z = entities[target]->pos.z + std::cos(radians(entities[target]->angle.getPitch()))*60.f;
		}
	} else if (target==0) {
		// player target
		spells[i].target_pos = player.pos;
	} else {
		// IO target
		spells[i].target_pos = entities[target]->pos;
	}
	
	spells[i].flags=flags;
	spells[i].pSpellFx=NULL;
	spells[i].type = typ;
	spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
	spells[i].fManaCostPerSecond = 0.f;
	
	
	// Check spell-specific preconditions
	switch(typ) {
		case SPELL_MAGIC_SIGHT:
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster))
				return false;

			break;
		case SPELL_HEAL:
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster))
				return false;

			break;
		case SPELL_BLESS:
			if(ARX_SPELLS_ExistAnyInstance(typ))
				return false;

			break;
		case SPELL_TELEKINESIS:
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster))
				return false;

			break;
		case SPELL_FLYING_EYE:
			if(eyeball.exist)
				return false;

			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster))
				return false;

			break;		
		case SPELL_INVISIBILITY:
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster))
				return false;

			break;		
		case SPELL_MANA_DRAIN:
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster))
				return false;

			break;
		case SPELL_LIFE_DRAIN:
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster))
				return false;

			break;		
		case SPELL_CONTROL_TARGET:
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster))
				return false;

			break;		
		case SPELL_FREEZE_TIME:
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster))
				return false;

			break;
		case SPELL_TELEPORT:
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster))
				return false;

			break;		
	default:
		break; // no preconditions to check
	}
	
	if(!CanPayMana(i, ARX_SPELLS_GetManaCost(typ, i))) {
		return false;
	}
	
	if(!GLOBAL_MAGIC_MODE) {
		return No_MagicAllowed();
	}
	
	bool notifyAll = true;
	
	switch(typ) {
		case SPELL_NONE:
		return true;
		//****************************************************************************
		// LEVEL 1
		case SPELL_MAGIC_SIGHT: {
			MagicSightSpellLaunch(duration, i);
			break;
		}
		case SPELL_MAGIC_MISSILE: {
			MagicMissileSpellLaunch(i);
			break;
		}
		case SPELL_IGNIT: {
			IgnitSpellLaunch(i);
			break;
		}
		case SPELL_DOUSE: {
			DouseSpellLaunch(i);
			break;
		}
		case SPELL_ACTIVATE_PORTAL: {
			ActivatePortalSpellLaunch(i);
			break;
		}
		//****************************************************************************
		// LEVEL 2
		case SPELL_HEAL: {
			HealSpellLaunch(i, duration);
			break;
		}
		case SPELL_DETECT_TRAP: {
			DetectTrapSpellLaunch(i, typ);
			break;
		}
		case SPELL_ARMOR: {
			ArmorSpellLaunch(typ, duration, i);
			break;
		}
		case SPELL_LOWER_ARMOR: {
			LowerArmorSpellLaunch(typ, duration, i);
			break;
		}
		case SPELL_HARM: {
			HarmSpellLaunch(duration, i);
			break;
		}
		//****************************************************************************
		// LEVEL 3
		case SPELL_SPEED: {
			SpeedSpellLaunch(i, duration);
			break;
		}
		case SPELL_DISPELL_ILLUSION: {
			DispellIllusionSpellLaunch(i);
			break;
		}
		case SPELL_FIREBALL: {
			FireballSpellLaunch(i);
			break;
		}
		case SPELL_CREATE_FOOD: {
			CreateFoodSpellLaunch(duration, i);
			break;
		}
		case SPELL_ICE_PROJECTILE: {
			IceProjectileSpellLaunch(i);
			break;
		}
		//****************************************************************************
		// LEVEL 4
		case SPELL_BLESS: {
			BlessSpellLaunch(i, duration, typ);
			break;
		}
		case SPELL_DISPELL_FIELD: {
			DispellFieldSpellLaunch(i);
			break;
		}
		case SPELL_FIRE_PROTECTION: {
			FireProtectionSpellLaunch(i, typ, duration);
			break;
		}
		case SPELL_COLD_PROTECTION: {
			ColdProtectionSpellLaunch(i, duration, typ);
			break;
		}
		case SPELL_TELEKINESIS: {
			TelekinesisSpellLaunch(i, duration);
			break;
		}
		case SPELL_CURSE: {
			CurseSpellLaunch(duration, typ, i);
			break;
		}
		//****************************************************************************
		// LEVEL 5
		case SPELL_RUNE_OF_GUARDING: {
			RuneOfGuardingSpellLaunch(i, typ, duration);
			break;
		}
		case SPELL_LEVITATE: {
			LevitateSpellLaunch(duration, i, typ);
			break;
		}
		case SPELL_CURE_POISON: {
			CurePoisonSpellLaunch(i);
			break;
		}
		case SPELL_REPEL_UNDEAD: {
			RepelUndeadSpellLaunch(duration, i);
			break;
		}
		case SPELL_POISON_PROJECTILE: {
			PoisonProjectileSpellLaunch(i);
			break;
		}
		//****************************************************************************
		// LEVEL 6
		case SPELL_RISE_DEAD: {
			bool result = RiseDeadSpellLaunch(typ, i, duration);
			if(!result)
				return false;
			
			break;
		}
		case SPELL_PARALYSE: {
			ParalyseSpellLaunch(i, duration);
			break;
		}
		case SPELL_CREATE_FIELD: {
			CreateFieldSpellLaunch(flags, i, duration);
			break;
		}
		case SPELL_DISARM_TRAP: {
			DisarmTrapSpellLaunch(i);
			break;
		}
		case SPELL_SLOW_DOWN: {
			bool result = SlowDownSpellLaunch(duration, i);
			if(!result)
				return false;
			
			break;
		}
		//****************************************************************************
		// LEVEL 7
		case SPELL_FLYING_EYE: {
			bool result = FlyingEyeSpellLaunch(i, tc4);
			if(!result)
				return false;
			
			break;
		}
		case SPELL_FIRE_FIELD: {
			FireFieldSpellLaunch(i, typ, duration);
			break;
		}
		case SPELL_ICE_FIELD: {
			IceFieldSpellLaunch(i, duration, typ);
			break;
		}
		case SPELL_LIGHTNING_STRIKE: {
			LightningStrikeSpellLaunch(i);
			break;
		}
		case SPELL_CONFUSE: {
			ConfuseSpellLaunch(i, notifyAll, duration); // TODO inconsistent use of the SM_SPELLCAST event
			break;
		}
		//****************************************************************************
		// LEVEL 8
		case SPELL_INVISIBILITY: {
			InvisibilitySpellLaunch(i, duration);
			break;
		}
		case SPELL_MANA_DRAIN: {
			ManaDrainSpellLaunch(i, duration);
			break;
		}
		case SPELL_EXPLOSION: {
			ExplosionSpellLaunch(i);
			break;
		}
		case SPELL_ENCHANT_WEAPON: {
			EnchantWeaponSpellLaunch(notifyAll, i); // TODO inconsistent use of the SM_SPELLCAST event
			break;
		}
		case SPELL_LIFE_DRAIN: {
			LifeDrainSpellLaunch(duration, i);
			break;
		}
		//****************************************************************************
		// LEVEL 9
		case SPELL_SUMMON_CREATURE: {
			bool result = SummonCreatureSpellLaunch(i, duration);
			if(!result)
				return false;
			
			break;
		}
		case SPELL_FAKE_SUMMON: {
			bool result = FakeSummonSpellLaunch(i);
			if(!result)
				return false;
			
			break;
		}
		case SPELL_NEGATE_MAGIC: {
			NegateMagicSpellLaunch(duration, i);
			break;
		}
		case SPELL_INCINERATE: {
			bool result = IncinerateSpellLaunch(i);
			if(!result)
				return false;
			
			break;
		}
		case SPELL_MASS_PARALYSE: {
			MassParalyseSpellLaunch(i, duration);
			break;
		}
		//****************************************************************************
		// LEVEL 10
		case SPELL_MASS_LIGHTNING_STRIKE: {
			MassLightningStrikeSpellLaunch(i, typ);
			break;
		}
		case SPELL_CONTROL_TARGET: {
			bool result = ControlTargetSpellLaunch(i);
			if(!result)
				return false;
			
			break;
		}
		case SPELL_FREEZE_TIME: {
			FreezeTimeSpellLaunch(duration, i);
			break;
		}
		case SPELL_MASS_INCINERATE: {
			MassIncinerateSpellLaunch(i);
			break;
		}
		case SPELL_TELEPORT: {
			TeleportSpellLaunch(i);
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

// Used for specific Spell-End FX
void ARX_SPELLS_Kill(long i) {
	
	static TextureContainer * tc4=TextureContainer::Load("graph/particles/smoke");

	if (!spells[i].exist) return;

	spells[i].exist=false;

	// All Levels - Kill Light
	if(spells[i].pSpellFx && lightHandleIsValid(spells[i].pSpellFx->lLightId)) {
		EERIE_LIGHT * light = lightHandleGet(spells[i].pSpellFx->lLightId);
		
		light->duration = 500; 
		light->time_creation = (unsigned long)(arxtime);
	}

	switch(spells[i].type) {
		case SPELL_FIREBALL: {
			FireballSpellKill(i);
			break;
		}
		case SPELL_LIGHTNING_STRIKE: {
			LightningStrikeKill(i);
			break;
		}
		case SPELL_MASS_LIGHTNING_STRIKE: {
			MassLightningStrikeSpellKill(i);
			break;
		}
		case SPELL_REPEL_UNDEAD: {
			RepelUndeadSpellKill(i);
			break;
		}
		case SPELL_HARM: {
			HarmSpellKill(i);
			break;
		}
		case SPELL_LIFE_DRAIN: {
			LifeDrainSpellKill(i);
			break;
		}
		case SPELL_MANA_DRAIN: {
			ManaDrainSpellKill(i);
			break;
		}
		case SPELL_FLYING_EYE : {
			FlyingEyeSpellKill(tc4, i);
			break;
		}
		// Level 06
		case SPELL_PARALYSE: {
			ParalyseSpellKill();
			break;
		}
		// Level 7
		case SPELL_FIRE_FIELD: {
			FireFieldSpellKill(i);
			break;
		}
		case SPELL_ICE_FIELD: {
			IceFieldSpellKill(i); 
			break; 
		}
		case SPELL_MASS_PARALYSE: {
			MassParalyseSpellKill();
			break;
		}
		case SPELL_SUMMON_CREATURE: {
			SummonCreatureSpellKill(i);
			break;
		}
		case SPELL_FAKE_SUMMON: {
			FakeSummonSpellKill(i);
			break;
		}
		
		default: break;
	}
	
	delete spells[i].pSpellFx, spells[i].pSpellFx = NULL;
}

float ARX_SPELLS_ApplyFireProtection(Entity * io,float damages)
{
	if(io) {
		long idx=ARX_SPELLS_GetSpellOn(io,SPELL_FIRE_PROTECTION);

		if(idx >= 0) {
			float modif = 1.f-((float)spells[idx].caster_level*( 1.0f / 10 ));

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
		float modif=1.f-((float)spells[idx].caster_level*( 1.0f / 10 ));

		modif = clamp(modif, 0.f, 1.f);

		damages *= modif;
	}

	return damages;
}

void TeleportSpellEnd(size_t i)
{
	ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &spells[i].caster_pos);
}

void MagicSightSpellEnd(size_t i)
{
	if(spells[i].caster == 0) {
		Project.improve = 0;
		ARX_SOUND_Stop(spells[i].snd_loop);
	}
	ARX_SOUND_PlaySFX(SND_SPELL_VISION_START, &entities[spells[i].caster]->pos);
}

void MagicMissileSpellEnd(size_t i)
{
	lightHandleDestroy(spells[i].longinfo_light);
}

void IgnitSpellEnd(size_t i)
{
	CIgnit *pIgnit = (CIgnit *)spells[i].pSpellFx;
	pIgnit->Action(1);
}

void DouseSpellEnd(size_t i)
{
	CDoze *pDoze = (CDoze *)spells[i].pSpellFx;
	pDoze->Action(0);
}

void DetectTrapSpellEnd(size_t i)
{
	if(spells[i].caster == 0) {
		ARX_SOUND_Stop(spells[i].snd_loop);
	}
	ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
}

void ArmorSpellEnd(size_t i)
{
	ARX_SOUND_Stop(spells[i].snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_END, &entities[spells[i].target]->pos);
	
	if(ValidIONum(spells[i].target)) {
		ARX_HALO_SetToNative(entities[spells[i].target]);
	}
	
	ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
}

void LowerArmorSpellEnd(size_t i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_LOWER_ARMOR_END);
	Entity *io = entities[spells[i].target];
	
	if(spells[i].longinfo_lower_armor) {
		io->halo.flags &= ~HALO_ACTIVE;
		ARX_HALO_SetToNative(io);
	}
	
	ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
}

void SpeedSpellEnd(size_t i)
{
	ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
	
	if(spells[i].caster == 0)
		ARX_SOUND_Stop(spells[i].snd_loop);
	
	ARX_SOUND_PlaySFX(SND_SPELL_SPEED_END, &entities[spells[i].target]->pos);
}

void FireballSpellEnd(size_t i)
{
	ARX_SOUND_Stop(spells[i].snd_loop);
}

void BlessSpellEnd(size_t i)
{
	ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
}

void CurseSpellEnd(size_t i)
{
	ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
}

void TelekinesisSpellEnd(size_t i)
{
	if(spells[i].caster == 0)
		Project.telekinesis = 0;
	
	ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_END, &entities[spells[i].caster]->pos);
}

void FireProtectionSpellEnd(size_t i)
{
	ARX_SOUND_Stop(spells[i].snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION_END, &entities[spells[i].target]->pos);
	ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
	
	if(ValidIONum(spells[i].target))
		ARX_HALO_SetToNative(entities[spells[i].target]);
}

void ColdProtectionSpellEnd(size_t i)
{
	ARX_SOUND_Stop(spells[i].snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_END, &entities[spells[i].target]->pos);
	ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
	
	if(ValidIONum(spells[i].target))
		ARX_HALO_SetToNative(entities[spells[i].target]);
}

void ARX_SPELLS_Update_End(size_t i) {
	switch(spells[i].type) {
		case SPELL_TELEPORT: {
			TeleportSpellEnd(i);
			break;
		}
		//****************************************************************************
		// LEVEL 1 SPELLS
		case SPELL_MAGIC_SIGHT: {
			MagicSightSpellEnd(i);
			break;
		}
		case SPELL_MAGIC_MISSILE: {
			MagicMissileSpellEnd(i);
			break;
		}
		case SPELL_IGNIT: {
			IgnitSpellEnd(i);
			break;
		}
		case SPELL_DOUSE: {
			DouseSpellEnd(i);
			break;
		}
		//****************************************************************************
		// LEVEL 2
		case SPELL_DETECT_TRAP: {
			DetectTrapSpellEnd(i);
			break;
		}
		case SPELL_ARMOR: {
			ArmorSpellEnd(i);
			break;
		}
		case SPELL_LOWER_ARMOR: {
			LowerArmorSpellEnd(i);
			break;
		}
		//****************************************************************************
		// LEVEL 3
		case SPELL_SPEED: {
			SpeedSpellEnd(i);
			break;
		}
		case SPELL_FIREBALL: {
			FireballSpellEnd(i);
			break;
		}
		//****************************************************************************
		// LEVEL 4
		case SPELL_BLESS: {
			BlessSpellEnd(i);
			break;
		}
		case SPELL_CURSE: {
			CurseSpellEnd(i);
			break;
		}
		case SPELL_TELEKINESIS: {
			TelekinesisSpellEnd(i);
			break;
		}
		case SPELL_FIRE_PROTECTION: {
			FireProtectionSpellEnd(i);
			break;
		}
		case SPELL_COLD_PROTECTION: {
			ColdProtectionSpellEnd(i);
			break;
		}
		//****************************************************************************
		// LEVEL 5
		case SPELL_LEVITATE: {
			ARX_SOUND_Stop(spells[i].snd_loop);
			ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_END, &entities[spells[i].target]->pos);
			ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
			
			if(spells[i].target == 0)
				player.levitate = false;
			
			break;
		}
		//****************************************************************************
		// LEVEL 6 SPELLS
		case SPELL_PARALYSE: {
			ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
			entities[spells[i].target]->ioflags &= ~IO_FREEZESCRIPT;
			break;
		}
		case SPELL_RISE_DEAD: {
			if(ValidIONum(spells[i].longinfo_entity) && spells[i].longinfo_entity != 0) {
				
				ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &entities[spells[i].longinfo_entity]->pos);
				
				Entity *entity = entities[spells[i].longinfo_entity];

				if(entity->scriptload && (entity->ioflags & IO_NOSAVE)) {
					AddRandomSmoke(entity,100);
					Vec3f posi = entity->pos;
					posi.y-=100.f;
					MakeCoolFx(posi);
					
					LightHandle nn = GetFreeDynLight();

					if(lightHandleIsValid(nn)) {
						EERIE_LIGHT * light = lightHandleGet(nn);
						
						light->intensity = 0.7f + 2.f*rnd();
						light->fallend = 600.f;
						light->fallstart = 400.f;
						light->rgb = Color3f(1.0f, 0.8f, 0.f);
						light->pos = posi;
						light->duration = 600;
					}

					entity->destroyOne();
				}
			}
			break;
		}
		case SPELL_CREATE_FIELD: {
			CCreateField *pCreateField = (CCreateField *) spells[i].pSpellFx;

			if(pCreateField && lightHandleIsValid(pCreateField->lLightId)) {
				lightHandleGet(pCreateField->lLightId)->duration = 800;
			}

			if(ValidIONum(spells[i].longinfo_entity)) {
				delete entities[spells[i].longinfo_entity];
			}
			break;
		}
		case SPELL_SLOW_DOWN: {
			ARX_SOUND_PlaySFX(SND_SPELL_SLOW_DOWN_END);
			ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
			break;
		}
		//****************************************************************************
		// LEVEL 7
		case SPELL_ICE_FIELD: {
			if(spells[i].longinfo_damage != -1)
				damages[spells[i].longinfo_damage].exist = false;
			
			break;
		}
		case SPELL_FIRE_FIELD: {
			if(spells[i].longinfo_damage != -1)
				damages[spells[i].longinfo_damage].exist = false;
			
			break;
		}
		case SPELL_LIGHTNING_STRIKE: {
			ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &entities[spells[i].caster]->pos);					
			break;
		}
		case SPELL_FLYING_EYE: {
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &entities[spells[i].caster]->pos);
			break;
		}
		case SPELL_CONFUSE: {
			ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
			break;
		}
		//****************************************************************************
		// LEVEL 8
		case SPELL_EXPLOSION: {
			break;
		}
		case SPELL_INVISIBILITY: {
			if(ValidIONum(spells[i].target)) {
				entities[spells[i].target]->gameFlags &= ~GFLAG_INVISIBILITY;
				ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_END, &entities[spells[i].target]->pos);
				ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
			}
			break;
		}
		//****************************************************************************
		// LEVEL 9
		case SPELL_MASS_PARALYSE: {
			long *ptr = (long *) spells[i].misc;

			for(long in = 0; in < spells[i].longinfo2_entity; in++) {
				if(ValidIONum(ptr[in])) {
					ARX_SPELLS_RemoveSpellOn(ptr[in], i);
					entities[ptr[in]]->ioflags &= ~IO_FREEZESCRIPT;
				}
			}

			if(ptr)
				free(spells[i].misc);

			spells[i].misc=NULL;
			break;
		}
		case SPELL_SUMMON_CREATURE : {
			if(ValidIONum(spells[i].longinfo2_entity) && spells[i].longinfo2_entity != 0) {
				ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &entities[spells[i].longinfo2_entity]->pos);
			}

			lightHandleDestroy(spells[i].pSpellFx->lLightId);
			// need to killio
			break;
		}
		case SPELL_FAKE_SUMMON: {
			ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].target_pos);						
			
			lightHandleDestroy(spells[i].pSpellFx->lLightId);
			break;
		}
		case SPELL_INCINERATE: {
			ARX_SPELLS_RemoveSpellOn(spells[i].target, i);
			ARX_SOUND_Stop(spells[i].snd_loop);
			ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE_END);
			break;
		}
		//****************************************************************************
		// LEVEL 10
		case SPELL_FREEZE_TIME: {
			GLOBAL_SLOWDOWN += spells[i].siz;
			ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_END, &entities[spells[i].caster]->pos);
			break;
		}
		case SPELL_MASS_INCINERATE: {
			ARX_SPELLS_RemoveMultiSpellOn(i);
			ARX_SOUND_Stop(spells[i].snd_loop);
			ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE_END);
			break;
		}
		default:
			break;
	}				
}

void ARX_SPELLS_Update_Update(size_t i, unsigned long tim) {
	
	const long framediff3 = tim - spells[i].lastupdate;
	
	switch(spells[i].type) {
		case SPELL_DISPELL_FIELD: {
			break;
		}
		case SPELL_NONE: {
			break;
		}
		//****************************************************************************
		// LEVEL 1
		case SPELL_MAGIC_MISSILE: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if(pCSpellFX) {
				CMultiMagicMissile *pMMM = (CMultiMagicMissile *) pCSpellFX;
				pMMM->CheckCollision();

				// Update
				pCSpellFX->Update(framedelay);

				if(pMMM->CheckAllDestroyed())
					spells[i].tolive = 0;

				pCSpellFX->Render();
			}
			break;
		}
		case SPELL_IGNIT:
		case SPELL_DOUSE: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;
			if(pCSpellFX)
				pCSpellFX->Update(framedelay);
			
			break;
		} 
		case SPELL_ACTIVATE_PORTAL: {
			break;
		}
		//****************************************************************************
		// LEVEL 2
		case SPELL_HEAL: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}

			CHeal * ch=(CHeal *)pCSpellFX;

			if (ch)
			for(size_t ii = 0; ii < entities.size(); ii++) {
				if ((entities[ii])
					&& (entities[ii]->show==SHOW_FLAG_IN_SCENE) 
					&& (entities[ii]->gameFlags & GFLAG_ISINTREATZONE)
					&& (entities[ii]->ioflags & IO_NPC)
					&& (entities[ii]->_npcdata->life>0.f)
					)
				{
					float dist;

					if(long(ii) == spells[i].caster)
						dist=0;
					else
						dist=fdist(ch->eSrc, entities[ii]->pos);

					if(dist<300.f) {
						float gain=((rnd()*1.6f+0.8f)*spells[i].caster_level)*(300.f-dist)*( 1.0f / 300 )*framedelay*( 1.0f / 1000 );

						if(ii==0) {
							if (!BLOCK_PLAYER_CONTROLS)
								player.life=std::min(player.life+gain,player.Full_maxlife);									
						}
						else
							entities[ii]->_npcdata->life = std::min(entities[ii]->_npcdata->life+gain, entities[ii]->_npcdata->maxlife);
					}
				}
			}
			break;
		}
		case SPELL_DETECT_TRAP: {
			if(spells[i].caster == 0) {
				Vec3f pos;
				ARX_PLAYER_FrontPos(&pos);
				ARX_SOUND_RefreshPosition(spells[i].snd_loop, pos);
			}

			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
			break;
		}
		case SPELL_ARMOR:
		case SPELL_LOWER_ARMOR: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;
			
			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
			
			ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
			break;
		} 
		case SPELL_HARM: {
			if(cabal) {
				float refpos;
				float scaley;

				if(spells[i].caster==0)
					scaley=90.f;
				else
					scaley = EEfabs(entities[spells[i].caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;


				float mov=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;

				Vec3f cabalpos;
				if(spells[i].caster==0) {
					cabalpos.x = player.pos.x;
					cabalpos.y = player.pos.y + 60.f - mov;
					cabalpos.z = player.pos.z;
					refpos=player.pos.y+60.f;
				} else {
					cabalpos.x = entities[spells[i].caster]->pos.x;
					cabalpos.y = entities[spells[i].caster]->pos.y - scaley - mov;
					cabalpos.z = entities[spells[i].caster]->pos.z;
					refpos=entities[spells[i].caster]->pos.y-scaley;							
				}

				float Es=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + radians(scaley));

				if(lightHandleIsValid(spells[i].longinfo2_light)) {
					EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
					
					light->pos.x = cabalpos.x;
					light->pos.y = refpos;
					light->pos.z = cabalpos.z; 
					light->rgb.r=rnd()*0.2f+0.8f;
					light->rgb.g=rnd()*0.2f+0.6f;
					light->fallstart=Es*1.5f;
				}

				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				GRenderer->SetRenderState(Renderer::DepthWrite, false);

				Anglef cabalangle(0.f, 0.f, 0.f);
				cabalangle.setPitch(spells[i].fdata+(float)framedelay*0.1f);
				spells[i].fdata = cabalangle.getPitch();

				Vec3f cabalscale = Vec3f(Es);
				Color3f cabalcolor = Color3f(0.8f, 0.4f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y = refpos - mov;
				cabalcolor = Color3f(0.5f, 3.f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos-mov;
				cabalcolor = Color3f(0.25f, 0.1f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos-mov;
				cabalcolor = Color3f(0.15f, 0.1f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				GRenderer->SetRenderState(Renderer::AlphaBlending, false);		
				GRenderer->SetRenderState(Renderer::DepthWrite, true);	
				
				ARX_SOUND_RefreshPosition(spells[i].snd_loop, cabalpos);
			}
			break;
		}
		//****************************************************************************
		// LEVEL 3 SPELLS
		case SPELL_FIREBALL: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if(pCSpellFX) {
				CFireBall *pCF = (CFireBall*) pCSpellFX;
					
				if(!lightHandleIsValid(spells[i].longinfo_light))
					spells[i].longinfo_light = GetFreeDynLight();

				if(lightHandleIsValid(spells[i].longinfo_light)) {
					EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo_light);
					
					light->pos = pCF->eCurPos;
					light->intensity = 2.2f;
					light->fallend = 500.f;
					light->fallstart = 400.f;
					light->rgb.r = 1.0f-rnd()*0.3f;
					light->rgb.g = 0.6f-rnd()*0.1f;
					light->rgb.b = 0.3f-rnd()*0.1f;
				}

				EERIE_SPHERE sphere;
				sphere.origin = pCF->eCurPos;
				sphere.radius=std::max(spells[i].caster_level*2.f,12.f);
				#define MIN_TIME_FIREBALL 2000 

				if(pCF->pPSFire.iParticleNbMax) {
					if(pCF->ulCurrentTime > MIN_TIME_FIREBALL) {
						SpawnFireballTail(&pCF->eCurPos,&pCF->eMove,(float)spells[i].caster_level,0);
					} else {
						if(rnd()<0.9f) {
							Vec3f move = Vec3f_ZERO;
							float dd=(float)pCF->ulCurrentTime / (float)MIN_TIME_FIREBALL*10;

							if(dd > spells[i].caster_level)
								dd = spells[i].caster_level;

							if(dd < 1)
								dd = 1;

							SpawnFireballTail(&pCF->eCurPos,&move,(float)dd,1);
						}
					}
				}

				if(!pCF->bExplo)
				if(CheckAnythingInSphere(&sphere, spells[i].caster, CAS_NO_SAME_GROUP)) {
					ARX_BOOMS_Add(pCF->eCurPos);
					LaunchFireballBoom(&pCF->eCurPos,(float)spells[i].caster_level);
					pCF->pPSFire.iParticleNbMax = 0;
					pCF->pPSFire2.iParticleNbMax = 0;
					pCF->eMove *= 0.5f;
					pCF->pPSSmoke.iParticleNbMax = 0;
					pCF->SetTTL(1500);
					pCF->bExplo = true;
					
					DoSphericDamage(&pCF->eCurPos,3.f*spells[i].caster_level,30.f*spells[i].caster_level,DAMAGE_AREA,DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL,spells[i].caster);
					spells[i].tolive=0;
					ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &sphere.origin);
					ARX_NPC_SpawnAudibleSound(sphere.origin, entities[spells[i].caster]);
				}

				pCSpellFX->Update(framedelay);
				ARX_SOUND_RefreshPosition(spells[i].snd_loop, pCF->eCurPos);
			}
			break;
		}
		case SPELL_SPEED: {
			if(spells[i].pSpellFx) {
				if(spells[i].caster == 0)
					ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
				
				spells[i].pSpellFx->Update(framedelay);
				spells[i].pSpellFx->Render();
			}
			break;
		}
		case SPELL_CREATE_FOOD:
		case SPELL_ICE_PROJECTILE:
		case SPELL_DISPELL_ILLUSION: {
			if(spells[i].pSpellFx) {
				spells[i].pSpellFx->Update(framedelay);
				spells[i].pSpellFx->Render();
			}
			break;
		}
		//****************************************************************************
		// LEVEL 4 SPELLS
		case SPELL_BLESS: {
			if(spells[i].pSpellFx) {
				CBless * pBless=(CBless *)spells[i].pSpellFx;

				if(pBless) {
					if(ValidIONum(spells[i].target)) {
						pBless->eSrc = entities[spells[i].target]->pos;
						Anglef angle = Anglef::ZERO;

						if(spells[i].target == 0)
							angle.setPitch(player.angle.getPitch());
						else 
							angle.setPitch(entities[spells[i].target]->angle.getPitch());

						pBless->Set_Angle(angle);
					}
				}

				spells[i].pSpellFx->Update(framedelay);
				spells[i].pSpellFx->Render();
			}
			break;
		}
		case SPELL_CURSE: {
			if(spells[i].pSpellFx) {
				CCurse * curse=(CCurse *)spells[i].pSpellFx;
				Vec3f target = Vec3f_ZERO;
					
				if(spells[i].target >= 0 && entities[spells[i].target]) {
					target = entities[spells[i].target]->pos;
	
					if(spells[i].target == 0)
						target.y -= 200.f;
					else
						target.y += entities[spells[i].target]->physics.cyl.height - 30.f;
				}
				
				curse->Update(checked_range_cast<unsigned long>(framedelay));
				
				curse->eTarget = target;
				curse->Render();
			}
			break;
		}
		case SPELL_FIRE_PROTECTION: {
			spells[i].pSpellFx->Update(framedelay);
			spells[i].pSpellFx->Render();
			ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
			break;
		}
		case SPELL_COLD_PROTECTION: {
			spells[i].pSpellFx->Update(framedelay);
			spells[i].pSpellFx->Render();
			ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
			break;
		}
		//****************************************************************************
		// LEVEL 5 SPELLS
		case SPELL_CURE_POISON: {
			if(spells[i].pSpellFx) {
				spells[i].pSpellFx->Update(framedelay);
				spells[i].pSpellFx->Render();					
			}
			break;
		}
		case SPELL_RUNE_OF_GUARDING: {
			if(spells[i].pSpellFx) {
				spells[i].pSpellFx->Update(framedelay);
				spells[i].pSpellFx->Render();
				CRuneOfGuarding * pCRG=(CRuneOfGuarding *)spells[i].pSpellFx;

				if (pCRG)
				{
					EERIE_SPHERE sphere;
					sphere.origin = pCRG->eSrc;
					sphere.radius=std::max(spells[i].caster_level*15.f,50.f);

					if (CheckAnythingInSphere(&sphere,spells[i].caster,CAS_NO_SAME_GROUP | CAS_NO_BACKGROUND_COL | CAS_NO_ITEM_COL| CAS_NO_FIX_COL | CAS_NO_DEAD_COL))
					{
						ARX_BOOMS_Add(pCRG->eSrc);
						LaunchFireballBoom(&pCRG->eSrc,(float)spells[i].caster_level);
						DoSphericDamage(&pCRG->eSrc,4.f*spells[i].caster_level,30.f*spells[i].caster_level,DAMAGE_AREA,DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL,spells[i].caster);
						spells[i].tolive=0;
						ARX_SOUND_PlaySFX(SND_SPELL_RUNE_OF_GUARDING_END, &sphere.origin);
					}
				}
			}
			break;
		}
		case SPELL_REPEL_UNDEAD: {
			if(spells[i].pSpellFx) {
				spells[i].pSpellFx->Update(framedelay);
				spells[i].pSpellFx->Render();					

				if (spells[i].target == 0)
					ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
			}
			break;
		}
		case SPELL_POISON_PROJECTILE: {
			if(spells[i].pSpellFx) {
				spells[i].pSpellFx->Update(framedelay);
				spells[i].pSpellFx->Render();
			}
			break;
		}
		case SPELL_LEVITATE: {
			CLevitate *pLevitate=(CLevitate *)spells[i].pSpellFx;
			Vec3f target;

			if(spells[i].target == 0) {
				target.x=player.pos.x;
				target.y=player.pos.y+150.f;
				target.z=player.pos.z;
				player.levitate = true;
			} else {
				target.x = entities[spells[i].caster]->pos.x;
				target.y = entities[spells[i].caster]->pos.y;
				target.z = entities[spells[i].caster]->pos.z;
			}

			pLevitate->ChangePos(&target);
				
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
			ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
			break;
		}
		//****************************************************************************
		// LEVEL 6 SPELLS
		case SPELL_RISE_DEAD: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if(pCSpellFX) {
				if(spells[i].longinfo_entity == -2) {
					pCSpellFX->lLightId=-1;
					break;
				}

				spells[i].tolive+=200;
			
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();

				if(lightHandleIsValid(pCSpellFX->lLightId)) {
					EERIE_LIGHT * light = lightHandleGet(pCSpellFX->lLightId);
					
					light->intensity = 0.7f + 2.3f;
					light->fallend = 500.f;
					light->fallstart = 400.f;
					light->rgb.r = 0.8f;
					light->rgb.g = 0.2f;
					light->rgb.b = 0.2f;
					light->duration=800;
					light->time_creation = (unsigned long)(arxtime);
				}

				unsigned long tim=pCSpellFX->getCurrentTime();

				if(tim > 3000 && spells[i].longinfo_entity == -1) {
					ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].target_pos);
					CRiseDead *prise = (CRiseDead *)spells[i].pSpellFx;

					if(prise) {
						EERIE_CYLINDER phys;
						phys.height=-200;
						phys.radius=50;
						phys.origin=spells[i].target_pos;

						float anything = CheckAnythingInCylinder(&phys, NULL, CFLAG_JUST_TEST);

						if(EEfabs(anything) < 30) {
							
							const char * cls = "graph/obj3d/interactive/npc/undead_base/undead_base";
							Entity * io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
							
							if(io) {
								ARX_INTERACTIVE_HideGore(io);
								RestoreInitialIOStatusOfIO(io);
								
								long lSpellsCaster = spells[i].caster;
								io->summoner = checked_range_cast<short>(lSpellsCaster);
								
								io->ioflags|=IO_NOSAVE;
								spells[i].longinfo_entity = io->index();
								io->scriptload=1;
								
								ARX_INTERACTIVE_Teleport(io, phys.origin);
								SendInitScriptEvent(io);

								if(ValidIONum(spells[i].caster)) {
									EVENT_SENDER = entities[spells[i].caster];
								} else {
									EVENT_SENDER = NULL;
								}

								SendIOScriptEvent(io,SM_SUMMONED);
									
								Vec3f pos;
								pos.x=prise->eSrc.x+rnd()*100.f-50.f;
								pos.y=prise->eSrc.y+100+rnd()*100.f-50.f;
								pos.z=prise->eSrc.z+rnd()*100.f-50.f;
								MakeCoolFx(pos);
							}

							pCSpellFX->lLightId=-1;
						} else {
							ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
							spells[i].longinfo_entity = -2;
							spells[i].tolive=0;
						}
					}
				} else if(!arxtime.is_paused() && tim < 4000) {
				  if(rnd() > 0.95f) {
						CRiseDead *pRD = (CRiseDead*)pCSpellFX;
						Vec3f pos = pRD->eSrc;
						MakeCoolFx(pos);
					}
				}

			}
			break;
		}
		case SPELL_SLOW_DOWN: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
			break;
		}
		case SPELL_DISARM_TRAP: {
			break;
		}
		case SPELL_PARALYSE: {
			break;
		}
		case SPELL_CREATE_FIELD: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;
			
			if(pCSpellFX) {
				if(ValidIONum(spells[i].longinfo_entity)) {
					Entity * io = entities[spells[i].longinfo_entity];
					
					CCreateField * ccf=(CCreateField *)pCSpellFX;
					io->pos = ccf->eSrc;

					if (IsAnyNPCInPlatform(io))
					{
						spells[i].tolive=0;
					}
				
					pCSpellFX->Update(framedelay);			
					pCSpellFX->Render();
				}
			}
			break;
		}
		//****************************************************************************
		// LEVEL 7 SPELLS
		case SPELL_CONFUSE: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;
			
			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
			break;
		}
		case SPELL_FIRE_FIELD: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;
			
			if(pCSpellFX) {
				CFireField *pf = (CFireField *) pCSpellFX;
				pCSpellFX->Update(framedelay);
				
				if(!lightHandleIsValid(spells[i].longinfo2_light))
					spells[i].longinfo2_light = GetFreeDynLight();

				if(lightHandleIsValid(spells[i].longinfo2_light)) {
					EERIE_LIGHT * el = lightHandleGet(spells[i].longinfo2_light);
					
					el->pos.x = pf->pos.x;
					el->pos.y = pf->pos.y-120.f;
					el->pos.z = pf->pos.z;
					el->intensity = 4.6f;
					el->fallstart = 150.f+rnd()*30.f;
					el->fallend   = 290.f+rnd()*30.f;
					el->rgb.r = 1.f-rnd()*( 1.0f / 10 );
					el->rgb.g = 0.8f;
					el->rgb.b = 0.6f;
					el->duration = 600;
					el->extras=0;
				}
				
				if(VisibleSphere(pf->pos - Vec3f(0.f, 120.f, 0.f), 350.f)) {
					
					pCSpellFX->Render();
					float fDiff = framedelay / 8.f;
					int nTime = checked_range_cast<int>(fDiff);
					
					for(long nn=0;nn<=nTime+1;nn++) {
						
						PARTICLE_DEF * pd = createParticle();
						if(!pd) {
							break;
						}
						
						float t = rnd() * (PI * 2.f) - PI;
						float ts = std::sin(t);
						float tc = std::cos(t);
						pd->ov = pf->pos + Vec3f(120.f * ts, 15.f * ts, 120.f * tc) * randomVec();
						pd->move = Vec3f(2.f - 4.f * rnd(), 1.f - 8.f * rnd(), 2.f - 4.f * rnd());
						pd->siz = 7.f;
						pd->tolive = Random::get(500, 1500);
						pd->tc = fire2;
						pd->special = ROTATING | MODULATE_ROTATION | FIRE_TO_SMOKE;
						pd->fparam = 0.1f - rnd() * 0.2f;
						pd->scale = Vec3f(-8.f);
						
						PARTICLE_DEF * pd2 = createParticle();
						if(!pd2) {
							break;
						}
						
						*pd2 = *pd;
						pd2->delay = Random::get(60, 210);
					}
					
				}
			}
			break;
		}
		case SPELL_ICE_FIELD: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;
			
			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				
				CIceField *pf = (CIceField *) pCSpellFX;

				if(!lightHandleIsValid(spells[i].longinfo2_light))
					spells[i].longinfo2_light = GetFreeDynLight();

				if(lightHandleIsValid(spells[i].longinfo2_light)) {
					EERIE_LIGHT * el = lightHandleGet(spells[i].longinfo2_light);
					
					el->pos.x = pf->eSrc.x;
					el->pos.y = pf->eSrc.y-120.f;
					el->pos.z = pf->eSrc.z;
					el->intensity = 4.6f;
					el->fallstart = 150.f+rnd()*30.f;
					el->fallend   = 290.f+rnd()*30.f;
					el->rgb.r = 0.76f;
					el->rgb.g = 0.76f;
					el->rgb.b = 1.0f-rnd()*( 1.0f / 10 );
					el->duration = 600;
					el->extras=0;						
				}

				pCSpellFX->Render();
			}
			break;
		}
		//-----------------------------------------------------------------------------------------
		case SPELL_LIGHTNING_STRIKE: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
			
			ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].caster]->pos);
			break;
		}
		//****************************************************************************
		// LEVEL 8 SPELLS
		case SPELL_ENCHANT_WEAPON: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;
			
			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
		}
		//TODO Missing break ?
		case SPELL_EXPLOSION: {
			if(!lightHandleIsValid(spells[i].longinfo2_light))
				spells[i].longinfo2_light = GetFreeDynLight();

			if(lightHandleIsValid(spells[i].longinfo2_light)) {
				EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
				
				light->rgb.r = 0.1f+rnd()*( 1.0f / 3 );;
				light->rgb.g = 0.1f+rnd()*( 1.0f / 3 );;
				light->rgb.b = 0.8f+rnd()*( 1.0f / 5 );;
				light->duration=200;
			
				float rr,r2;
				Vec3f pos;
				
				float choice = rnd();
				if(choice > .8f) {
					long lvl = Random::get(9, 13);
					rr=radians(rnd()*360.f);
					r2=radians(rnd()*360.f);
					pos.x=light->pos.x-std::sin(rr)*260;
					pos.y=light->pos.y-std::sin(r2)*260;
					pos.z=light->pos.z+std::cos(rr)*260;
					Color3f rgb(0.1f + rnd()*(1.f/3), 0.1f + rnd()*(1.0f/3), 0.8f + rnd()*(1.0f/5));
					LaunchFireballBoom(&pos, static_cast<float>(lvl), NULL, &rgb);
				} else if(choice > .6f) {
					rr=radians(rnd()*360.f);
					r2=radians(rnd()*360.f);
					pos.x=light->pos.x-std::sin(rr)*260;
					pos.y=light->pos.y-std::sin(r2)*260;
					pos.z=light->pos.z+std::cos(rr)*260;
					MakeCoolFx(pos);
				} else if(choice > 0.4f) {
					rr=radians(rnd()*360.f);
					r2=radians(rnd()*360.f);
					pos.x=light->pos.x-std::sin(rr)*160;
					pos.y=light->pos.y-std::sin(r2)*160;
					pos.z=light->pos.z+std::cos(rr)*160;
					ARX_PARTICLES_Add_Smoke(&pos, 2, 20); // flag 1 = randomize pos
				}
			}
			break;
		}
		//****************************************************************************
		// LEVEL 9 SPELLS
		case SPELL_SUMMON_CREATURE: {
			if(!arxtime.is_paused()) {
				if(float(arxtime) - (float)spells[i].timcreation <= 4000) {
					if(rnd() > 0.7f) {
						CSummonCreature * pSummon = (CSummonCreature *)spells[i].pSpellFx;
						if(pSummon) {
							Vec3f pos = pSummon->eSrc;
							MakeCoolFx(pos);
						}
					}

					CSpellFx *pCSpellFX = spells[i].pSpellFx;

					if(pCSpellFX) {
						pCSpellFX->Update(framedelay);
						pCSpellFX->Render();
					}	

					spells[i].longinfo_summon_creature = 1;
					spells[i].longinfo2_entity = -1;

				} else if(spells[i].longinfo_summon_creature) {
					lightHandleDestroy(spells[i].pSpellFx->lLightId);

					spells[i].longinfo_summon_creature = 0;
					ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].target_pos);
					CSummonCreature *pSummon;
					pSummon= (CSummonCreature *)spells[i].pSpellFx;

					if(pSummon) {
						EERIE_CYLINDER phys;
						phys.height=-200;
						phys.radius=50;
						phys.origin=spells[i].target_pos;
						float anything = CheckAnythingInCylinder(&phys, NULL, CFLAG_JUST_TEST);

						if(EEfabs(anything) < 30) {
						
						long tokeep;
						res::path cls;
						if(spells[i].fdata == 1.f) {
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
						} else if(spells[i].caster_level >= 9) {
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
							
							long lSpellsCaster = spells[i].caster ; 
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

							if(ValidIONum(spells[i].caster)) {
								EVENT_SENDER = entities[spells[i].caster];
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
								spells[i].longinfo2_entity = io->index();
							else
								spells[i].longinfo2_entity = -1;
						}
						}
					}
				} else if(spells[i].longinfo2_entity <= 0) {
					spells[i].tolive = 0;
				}
			}
			break;
		}
		case SPELL_FAKE_SUMMON: {
			if(!arxtime.is_paused()) {
				if(rnd() > 0.7f) {
					CSummonCreature * pSummon = (CSummonCreature *)spells[i].pSpellFx;
					if(pSummon) {
						Vec3f pos = pSummon->eSrc;
						MakeCoolFx(pos);
					}
				}
			}
			CSpellFx *pCSpellFX = spells[i].pSpellFx;
			
			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
			break;
		}
		case SPELL_INCINERATE: {
			if(ValidIONum(spells[i].target)) {
				ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].target]->pos);
			}
			break;
		}
		case SPELL_NEGATE_MAGIC: {
			if(ValidIONum(spells[i].target))
				LaunchAntiMagicField(i);

			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
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
			CSpellFx *pCSpellFX = spells[i].pSpellFx;
			
			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
			break;
		}
		case SPELL_MASS_INCINERATE: {
			if(ValidIONum(spells[i].caster)) {
				ARX_SOUND_RefreshPosition(spells[i].snd_loop, entities[spells[i].caster]->pos);
			}
			break;
		}
		case SPELL_MASS_LIGHTNING_STRIKE: {
			CSpellFx *pCSpellFX = spells[i].pSpellFx;

			if(pCSpellFX) {
				pCSpellFX->Update(framedelay);
				pCSpellFX->Render();
			}
			
			Vec3f _source = spells[i].vsource;
			float _fx;
			_fx = 0.5f;
			unsigned long _gct;
			_gct = 0;

			Vec3f position;

			spells[i].lastupdate=tim;

			position = _source + randomVec(-250.f, 250.f);
			ARX_SOUND_RefreshPosition(spells[i].snd_loop, position);
			ARX_SOUND_RefreshVolume(spells[i].snd_loop, _fx + 0.5F);
			ARX_SOUND_RefreshPitch(spells[i].snd_loop, 0.8F + 0.4F * rnd());
			
			if(rnd() > 0.62f) {
				position = _source  + randomVec(-250.f, 250.f);
				ARX_SOUND_PlaySFX(SND_SPELL_SPARK, &position, 0.8F + 0.4F * rnd());
			}
			
			if(rnd() > 0.82f) {
				position = _source + randomVec(-250.f, 250.f);
				ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &position, 0.8F + 0.4F * rnd());
			}
			
			if((_gct > spells[i].tolive - 1800) && (spells[i].siz == 0)) {
				spells[i].siz = 1;
				ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, NULL, 0.8F + 0.4F * rnd());
			}

			if(lightHandleIsValid(spells[i].longinfo_light)) {
				EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo_light);
				
				float fxx;

				if(_fx > 0.2f)
					fxx = 1.f;
				else
					fxx = _fx * 5.f;

				light->intensity = 1.3f + rnd() * 1.f;
				light->fallend = 850.f;
				light->fallstart = 500.f;
				light->rgb = Color3f::red * fxx;
			}
			break;
		}
		case SPELL_TELEPORT: {
			float TELEPORT = (float)(((float)tim-(float)spells[i].timcreation)/(float)spells[i].tolive);

			if(LASTTELEPORT < 0.5f && TELEPORT >= 0.5f) {
				Vec3f pos = lastteleport;
				lastteleport = player.pos;
				player.pos = pos;
				LASTTELEPORT = 32.f;
				ARX_SOUND_PlaySFX(SND_SPELL_TELEPORTED, &player.pos);
			} else {
				LASTTELEPORT = TELEPORT;
			}
			break;
		}
		case SPELL_MAGIC_SIGHT: {
			if(spells[i].caster == 0) {
				Vec3f pos;
				ARX_PLAYER_FrontPos(&pos);
				ARX_SOUND_RefreshPosition(spells[i].snd_loop, pos);
				
				if(subj.focal > IMPROVED_FOCAL)
					subj.focal -= DEC_FOCAL;
			}
			break;
		}
		case SPELL_TELEKINESIS: {
			break;
		}
		case SPELL_INVISIBILITY: {
			if(spells[i].target != 0) {
				if(!(entities[spells[i].target]->gameFlags & GFLAG_INVISIBILITY)) {
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
					ARX_SPELLS_Fizzle(i);
				}
			}
			break;
		}
		case SPELL_MANA_DRAIN: {
			if(cabal) {
				float refpos;
				float scaley;

				if(spells[i].caster==0)
					scaley=90.f;
				else
					scaley=EEfabs(entities[spells[i].caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;

				float mov=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;

				Vec3f cabalpos;
				if(spells[i].caster == 0) {
					cabalpos.x = player.pos.x;
					cabalpos.y = player.pos.y + 60.f - mov;
					cabalpos.z = player.pos.z;
					refpos=player.pos.y+60.f;
				} else {
					cabalpos.x = entities[spells[i].caster]->pos.x;
					cabalpos.y = entities[spells[i].caster]->pos.y - scaley - mov;
					cabalpos.z = entities[spells[i].caster]->pos.z;
					refpos=entities[spells[i].caster]->pos.y-scaley;
				}

				float Es=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + radians(scaley));

				if(lightHandleIsValid(spells[i].longinfo2_light)) {
					EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
					
					light->pos.x = cabalpos.x;
					light->pos.y = refpos;
					light->pos.z = cabalpos.z;
					light->rgb.b=rnd()*0.2f+0.8f;
					light->fallstart=Es*1.5f;
				}

				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				GRenderer->SetRenderState(Renderer::DepthWrite, false);

				Anglef cabalangle(0.f, 0.f, 0.f);
				cabalangle.setPitch(spells[i].fdata + (float)framedelay*0.1f);
				spells[i].fdata = cabalangle.getPitch();
										
				Vec3f cabalscale = Vec3f(Es);
				Color3f cabalcolor = Color3f(0.4f, 0.4f, 0.8f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y = refpos - mov;
				cabalcolor = Color3f(0.2f, 0.2f, 0.5f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos-mov;
				cabalcolor = Color3f(0.1f, 0.1f, 0.25f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos-mov;
				cabalcolor = Color3f(0.f, 0.f, 0.15f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				cabalangle.setPitch(-cabalangle.getPitch());
				cabalpos.y=refpos-mov;
				cabalscale = Vec3f(Es);
				cabalcolor = Color3f(0.f, 0.f, 0.15f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()+30.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos+mov;
				cabalcolor = Color3f(0.1f, 0.1f, 0.25f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()+60.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos+mov;
				cabalcolor = Color3f(0.2f, 0.2f, 0.5f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()+120.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos+mov;
				cabalcolor = Color3f(0.4f, 0.4f, 0.8f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				cabalangle.setPitch(-cabalangle.getPitch());
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);		
				GRenderer->SetRenderState(Renderer::DepthWrite, true);	

				ARX_SOUND_RefreshPosition(spells[i].snd_loop, cabalpos);
			}
			break;
		}
		case SPELL_LIFE_DRAIN: {
			if(cabal) {
				float refpos;
				float scaley;

				if(spells[i].caster==0)
					scaley=90.f;
				else
					scaley=EEfabs(entities[spells[i].caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;

				float mov=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;

				Vec3f cabalpos;
				if(spells[i].caster == 0) {
					cabalpos.x = player.pos.x;
					cabalpos.y = player.pos.y + 60.f - mov;
					cabalpos.z = player.pos.z;
					refpos=player.pos.y+60.f;							
				} else {
					cabalpos.x = entities[spells[i].caster]->pos.x;
					cabalpos.y = entities[spells[i].caster]->pos.y - scaley-mov;
					cabalpos.z = entities[spells[i].caster]->pos.z;
					refpos=entities[spells[i].caster]->pos.y-scaley;							
				}

				float Es=std::sin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + radians(scaley));

				if(lightHandleIsValid(spells[i].longinfo2_light)) {
					EERIE_LIGHT * light = lightHandleGet(spells[i].longinfo2_light);
					
					light->pos.x = cabalpos.x;
					light->pos.y = refpos;
					light->pos.z = cabalpos.z;
					light->rgb.r = rnd() * 0.2f + 0.8f;
					light->fallstart = Es * 1.5f;
				}

				GRenderer->SetCulling(Renderer::CullNone);
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
				GRenderer->SetRenderState(Renderer::AlphaBlending, true);
				GRenderer->SetRenderState(Renderer::DepthWrite, false);

				Anglef cabalangle(0.f, 0.f, 0.f);
				cabalangle.setPitch(spells[i].fdata+(float)framedelay*0.1f);
				spells[i].fdata=cabalangle.getPitch();

				Vec3f cabalscale = Vec3f(Es);
				Color3f cabalcolor = Color3f(0.8f, 0.f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos-mov;
				cabalcolor = Color3f(0.5f, 0.f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos-mov;
				cabalcolor = Color3f(0.25f, 0.f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos-mov;
				cabalcolor = Color3f(0.15f, 0.f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				cabalangle.setPitch(-cabalangle.getPitch());
				cabalpos.y=refpos-mov;
				cabalscale = Vec3f(Es);
				cabalcolor = Color3f(0.15f, 0.f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()+30.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos+mov;
				cabalcolor = Color3f(0.25f, 0.f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()+60.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos+mov;
				cabalcolor = Color3f(0.5f, 0.f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				mov=std::sin((float)(arxtime.get_frame_time()+120.f)*( 1.0f / 800 ))*scaley;
				cabalpos.y=refpos+mov;
				cabalcolor = Color3f(0.8f, 0.f, 0.f);
				DrawEERIEObjEx(cabal, cabalangle, cabalpos, cabalscale, cabalcolor);

				cabalangle.setPitch(-cabalangle.getPitch());
				GRenderer->SetRenderState(Renderer::AlphaBlending, false);		
				GRenderer->SetRenderState(Renderer::DepthWrite, true);	

				ARX_SOUND_RefreshPosition(spells[i].snd_loop, cabalpos);
			}
			break;
		}
		case SPELL_FLYING_EYE: {
			eyeball.floating = std::sin(spells[i].lastupdate-spells[i].timcreation * 0.001f);
			eyeball.floating *= 10.f;
			
			if(spells[i].lastupdate-spells[i].timcreation <= 3000) {
				eyeball.exist = spells[i].lastupdate - spells[i].timcreation * (1.0f / 30);
				eyeball.size = Vec3f(1.f - float(eyeball.exist) * 0.01f);
				eyeball.angle.setPitch(eyeball.angle.getPitch() + framediff3 * 0.6f);
			} else {
				eyeball.exist = 2;
			}
			
			spells[i].lastupdate=tim;
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
		SPELL & spell = spells[i];
		
		if(!GLOBAL_MAGIC_MODE) {
			spell.tolive = 0;
		}

		if(!spell.exist) {
			continue;
		}
		
		if(spell.bDuration && !CanPayMana(i, spell.fManaCostPerSecond * (float)framedelay * (1.0f/1000), false))
			ARX_SPELLS_Fizzle(i);
		
		const long framediff = spell.timcreation + spell.tolive - tim;
		
		if(framediff < 0) {
			SPELLEND_Notify(spell);
			ARX_SPELLS_Update_End(i);
			ARX_SPELLS_Kill(i);
			
			continue;
		}

		if(spell.exist) {
			ARX_SPELLS_Update_Update(i, tim);
		}
	}
}

void TryToCastSpell(Entity * io, SpellType spellid, long level, long target, SpellcastFlags flags, long duration)
{
	if(!io || io->spellcast_data.castingspell != SPELL_NONE)
		return;

	if(!(flags & SPELLCAST_FLAG_NOMANA) && (io->ioflags & IO_NPC) && (io->_npcdata->mana<=0.f))
		return;

	unsigned long i(0);

	for(; i < SPELL_TYPES_COUNT; i++)
		if(spellicons[i].spellid == spellid)
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

	io->spellcast_data.castingspell = spellid;

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

