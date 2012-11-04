/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <limits>
#include <map>
#include <set>
#include <utility>

#include <boost/foreach.hpp>

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
#include "math/Vector2.h"
#include "math/Vector3.h"

#include "physics/Collisions.h"

#include "platform/Platform.h"
#include "platform/String.h"

#include "scene/Light.h"
#include "scene/Scene.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

#include "script/Script.h"

using std::abs;
using std::string;

static const float DEC_FOCAL = 50.0f;
static const float IMPROVED_FOCAL = 320.0f;

void MakeSpCol();
extern bool TRUE_PLAYER_MOUSELOOK_ON;
long passwall=0;
long WILLRETURNTOFREELOOK = 0;
long GLOBAL_MAGIC_MODE=1;
bool bPrecastSpell = false;
extern std::string LAST_FAILED_SEQUENCE;
enum ARX_SPELLS_RuneDirection
{
	AUP,
	AUPRIGHT,
	ARIGHT,
	ADOWNRIGHT,
	ADOWN,
	ADOWNLEFT,
	ALEFT,
	AUPLEFT
};
long sp_arm=0;
long cur_arm=0;
long cur_sos=0;
static void ApplyPasswall();
static void ApplySPArm();
static void ApplySPuw();
static void ApplySPRf();
static void ApplySPMax();
static void ApplySPWep();
static void ApplySPBow();
static void ApplyCurPNux();
static void ApplyCurMr();
static void ApplyCurSOS(); 
 
 
extern long FistParticles;
extern long sp_max;
short uw_mode=0;
static short uw_mode_pos=0;
extern long MAGICMODE;
extern Entity * CURRENT_TORCH;
extern float GLOBAL_SLOWDOWN;
extern void ARX_SPSound();
extern float sp_max_y[64];
extern Color sp_max_col[64];
extern char	sp_max_ch[64];
extern long sp_max_nb;
long cur_mega=0;
float sp_max_start = 0;
long sp_wep=0;

extern bool bRenderInCursorMode;

bool bOldLookToggle;
extern float SLID_START;

long BH_MODE = 0;
void EERIE_OBJECT_SetBHMode()
{
	if (BH_MODE)
		BH_MODE=0;
	else
	{
		BH_MODE=1;
		MakeCoolFx(&player.pos);
		MakeSpCol();
		strcpy(sp_max_ch,"!!!_Super-Deformed_!!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
			}
}

struct SYMBOL_DRAW {
	unsigned long	starttime;
	Vec3f		lastpos;
	short			lasttim;
	short			duration;
	char			sequence[32];
	char			cPosStartX;
	char			cPosStartY;
};

extern bool FrustrumsClipSphere(EERIE_FRUSTRUM_DATA * frustrums,EERIE_SPHERE * sphere);
extern bool bGToggleCombatModeWithKey;
void ARX_INTERFACE_Combat_Mode(long i);

static float ARX_SPELLS_GetManaCost(Spell _lNumSpell,long _lNumSpellTab);

///////////////Spell Interpretation
SPELL spells[MAX_SPELLS];
short ARX_FLARES_broken(1);
long CurrPoint(0);
long cur_mx=0;
long cur_pnux=0;
long cur_pom=0;
long cur_rf=0;
long cur_mr=0;
long cur_sm=0;
long cur_bh=0;

static float LASTTELEPORT(0.0F);
long snip=0;
static Vec2s Lm;

static const long MAX_POINTS(200);
static Vec2s plist[MAX_POINTS];
static std::string SpellMoves;
Rune SpellSymbol[MAX_SPELL_SYMBOLS];
size_t CurrSpellSymbol=0;

static long lMaxSymbolDrawSizeX;
static long lMaxSymbolDrawSizeY;

unsigned char ucFlick=0;

bool GetSpellPosition(Vec3f * pos,long i)
{
 

	switch (spells[i].type)
	{
				//*********************************************************************************************
		// LEVEL 1 SPELLS -----------------------------------------------------------------------------
		case SPELL_MAGIC_SIGHT: // Launching MAGIC_SIGHT			
		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_MAGIC_MISSILE: // Launching MAGIC_MISSILE
		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_IGNIT:// Launching IGNIT
		break;
		//---------------------------------------------------------------------------------------------
		case SPELL_DOUSE:// Launching DOUSE
		break;
		//---------------------------------------------------------------------------------------------
		case SPELL_ACTIVATE_PORTAL:// Launching ACTIVATE_PORTAL
		break;			
		//*************************************************************************************************
		// LEVEL 2 SPELLS -----------------------------------------------------------------------------
		case SPELL_HEAL:// Launching HEAL
		break;
		//---------------------------------------------------------------------------------------------
		case SPELL_DETECT_TRAP:// Launching DETECT_TRAP
		break;
		//---------------------------------------------------------------------------------------------
		case SPELL_ARMOR:// Launching ARMOR

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//------------------------------------------------------------------------------------------------
		case SPELL_LOWER_ARMOR:// Launching LOWER_ARMOR

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//------------------------------------------------------------------------------------------------
		case SPELL_HARM:// Launching HARM
		break;			
		//**********************************************************************************************
		// LEVEL 3 SPELLS -----------------------------------------------------------------------------
		case SPELL_SPEED:// Launching SPEED

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//--------------------------------------------------------------------------------------------------
		case SPELL_DISPELL_ILLUSION:// Launching DISPELL_ILLUSION (REVEAL)
		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_FIREBALL:// Launching FIREBALL
		break;
		//-------------------------------------------------------------------------------------------------
		case SPELL_CREATE_FOOD:// Launching CREATE_FOOD
		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_ICE_PROJECTILE:// Launching ICE_PROJECTILE
		break;
		//***********************************************************************************************	
		// LEVEL 4 SPELLS -----------------------------------------------------------------------------
		case SPELL_BLESS:// Launching BLESS

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//-----------------------------------------------------------------------------------------------
		case SPELL_DISPELL_FIELD:// Launching DISPELL_FIELD
		break;
		//-----------------------------------------------------------------------------------------------
		case SPELL_FIRE_PROTECTION:// Launching FIRE_PROTECTION

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_COLD_PROTECTION:// Launching COLD_PROTECTION

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_TELEKINESIS:// Launching TELEKINESIS
		break;
		//-----------------------------------------------------------------------------------------------
		case SPELL_CURSE:// Launching CURSE

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//*********************************************************************
		// LEVEL 5 SPELLS -----------------------------------------------------------------------------
		case SPELL_RUNE_OF_GUARDING:
		break;
		//----------------------------------------------------------------------------
		case SPELL_LEVITATE:
		break;
		//----------------------------------------------------------------------------
		case SPELL_CURE_POISON:
		break;
		//----------------------------------------------------------------------------
		case SPELL_REPEL_UNDEAD:
		break;
		//----------------------------------------------------------------------------
		case SPELL_POISON_PROJECTILE:
		break;
		//***************************************************************************
		// LEVEL 6 -----------------------------------------------------------------------------
		case SPELL_RISE_DEAD:
		break;
		//----------------------------------------------------------------------------
		case SPELL_PARALYSE:

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//----------------------------------------------------------------------------
		case SPELL_CREATE_FIELD:
		break;
		//----------------------------------------------------------------------------
		case SPELL_DISARM_TRAP:
		break;
		//----------------------------------------------------------------------------
		case SPELL_SLOW_DOWN:

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//****************************************************************************************
		// LEVEL 7 SPELLS -----------------------------------------------------------------------------
		case SPELL_FLYING_EYE:
		{	
			*pos = eyeball.pos;
			return true;
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_FIRE_FIELD:
			CSpellFx *pCSpellFX;
			pCSpellFX = spells[i].pSpellFx;

			if (pCSpellFX)
			{
				CFireField *pFireField = (CFireField *) pCSpellFX;
					
				*pos = pFireField->pos;
				return true;
			}

		break;
		//----------------------------------------------------------------------------
		case SPELL_ICE_FIELD:
		break;
		//----------------------------------------------------------------------------
		case SPELL_LIGHTNING_STRIKE:
		break;
		//----------------------------------------------------------------------------
		case SPELL_CONFUSE:

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//*********************************************************************************
		// LEVEL 8 SPELLS -----------------------------------------------------------------------------
		case SPELL_INVISIBILITY:

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//----------------------------------------------------------------------------
		case SPELL_MANA_DRAIN:				

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//----------------------------------------------------------------------------
		case SPELL_EXPLOSION:
		break;
		//----------------------------------------------------------------------------
		case SPELL_ENCHANT_WEAPON:
		break;			
		//----------------------------------------------------------------------------
		case SPELL_LIFE_DRAIN:

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//*****************************************************************************************
		// LEVEL 9 SPELLS -----------------------------------------------------------------------------
		case SPELL_SUMMON_CREATURE:
		break;
		//----------------------------------------------------------------------------
		case SPELL_NEGATE_MAGIC:
		break;
		//----------------------------------------------------------------------------
		case SPELL_INCINERATE:

			if (ValidIONum(spells[i].target))
			{
				*pos = entities[spells[i].target]->pos;
				return true;
			}

		break;
		//----------------------------------------------------------------------------
		case SPELL_MASS_PARALYSE:
		break;
		//----------------------------------------------------------------------------
		//********************************************************************************************
		// LEVEL 10 SPELLS -----------------------------------------------------------------------------
		case SPELL_MASS_LIGHTNING_STRIKE:
		break;
		//----------------------------------------------------------------------------
		case SPELL_CONTROL_TARGET:
		break;
		//----------------------------------------------------------------------------
		case SPELL_FREEZE_TIME:
		break;
		//----------------------------------------------------------------------------
		case SPELL_MASS_INCINERATE:
		break;
		//----------------------------------------------------------------------------
		case SPELL_TELEPORT:
		break;
		//----------------------------------------------------------------------------
		default: break;
	}

	if (ValidIONum(spells[i].caster))
	{
		*pos = entities[spells[i].caster]->pos;
		return true;
	}

	return false;
}

void LaunchAntiMagicField(size_t ident) {
	
	for(size_t n = 0 ; n < MAX_SPELLS; n++) {
		
		if(!spells[n].exist || n == ident) {
			continue;
		}
		
		if(spells[ident].caster_level < spells[n].caster_level) {
			continue;
		}
		
		Vec3f pos;
		GetSpellPosition(&pos,n);
		if(closerThan(pos, entities[spells[ident].caster]->pos, 600.f)) {
			if(spells[n].type != SPELL_CREATE_FIELD) {
				spells[n].tolive = 0;
			} else if(spells[ident].caster == 0 && spells[n].caster == 0) {
				spells[n].tolive = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_AddSpellOn(const long &caster, const long &spell)
{
	if (caster < 0 ||  spell < 0 || !entities[caster]) return;

	Entity *io = entities[caster];
	void *ptr;

	ptr = realloc(io->spells_on, sizeof(long) * (io->nb_spells_on + 1));

	if (!ptr) return;

	io->spells_on = (long *)ptr;
	io->spells_on[io->nb_spells_on] = spell;
	io->nb_spells_on++;
}

//-----------------------------------------------------------------------------
long ARX_SPELLS_GetSpellOn(const Entity * io, Spell spellid)
{
	if (!io) return -1;

	for (long i(0); i < io->nb_spells_on; i++)
	{		
		if (	(spells[io->spells_on[i]].type == spellid)
			&&	(spells[io->spells_on[i]].exist)	)
			return io->spells_on[i];
	}

	return -1;
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_RemoveSpellOn(const long &caster, const long &spell)
{
	if (caster < 0 || spell < 0) return;

	Entity *io = entities[caster];

	if (!io || !io->nb_spells_on) return;

	if (io->nb_spells_on == 1) 
	{
		free(io->spells_on);
		io->spells_on = NULL;
		io->nb_spells_on = 0;
		return;
	}

	long i(0);

	for (; i < io->nb_spells_on; i++)
		if (io->spells_on[i] == spell) break;

	if ( i >= io->nb_spells_on) return;

	io->nb_spells_on--;
	memcpy(&io->spells_on[i], &io->spells_on[i + 1], sizeof(long) * (io->nb_spells_on - i));

	io->spells_on = (long *)realloc(io->spells_on, sizeof(long) * io->nb_spells_on);
}

void ARX_SPELLS_RemoveMultiSpellOn(long spell_id) {
	for(size_t i = 0; i < entities.size(); i++) {
		ARX_SPELLS_RemoveSpellOn(i, spells[spell_id].type);
	}
}

void ARX_SPELLS_RemoveAllSpellsOn(Entity *io) {
	free(io->spells_on), io->spells_on = NULL, io->nb_spells_on = 0;
}

void ARX_SPELLS_RequestSymbolDraw(Entity *io, const string & name, float duration) {
	
	const char * sequence;
	int iPosX = 0;
	int iPosY = 0;

	if(name == "aam")              iPosX = 0, iPosY = 2, sequence = "6666";
	else if(name == "cetrius")     iPosX = 1, iPosY = 1, sequence = "33388886666";
	else if(name == "comunicatum") iPosX = 0, iPosY = 0, sequence = "6666622244442226666";
	else if(name == "cosum")       iPosX = 0, iPosY = 2, sequence = "66666222244448888";
	else if(name == "folgora")     iPosX = 0, iPosY = 3, sequence = "99993333";
	else if(name == "fridd")       iPosX = 0, iPosY = 4, sequence = "888886662222";
	else if(name == "kaom")        iPosX = 3, iPosY = 0, sequence = "44122366";
	else if(name == "mega")        iPosX = 2, iPosY = 4, sequence = "88888";
	else if(name == "morte")       iPosX = 0, iPosY = 2, sequence = "66666222";
	else if(name == "movis")       iPosX = 0, iPosY = 0, sequence = "666611116666";
	else if(name == "nhi")         iPosX = 4, iPosY = 2, sequence = "4444";
	else if(name == "rhaa")        iPosX = 2, iPosY = 0, sequence = "22222";
	else if(name == "spacium")     iPosX = 4, iPosY = 0, sequence = "44444222266688";
	else if(name == "stregum")     iPosX = 0, iPosY = 4, sequence = "8888833338888";
	else if(name == "taar")        iPosX = 0, iPosY = 1, sequence = "666222666";
	else if(name == "tempus")      iPosX = 0, iPosY = 4, sequence = "88886662226668866";
	else if(name == "tera")        iPosX = 0, iPosY = 3, sequence = "99922266";
	else if(name == "vista")       iPosX = 1, iPosY = 0, sequence = "333111";
	else if(name == "vitae")       iPosX = 0, iPosY = 2, sequence = "66666888";
	else if(name == "yok")         iPosX = 0, iPosY = 0, sequence = "222226666888";
	else if(name == "akbaa")       iPosX = 0, iPosY = 0, sequence = "22666772222";
	else return;

	io->symboldraw = (SYMBOL_DRAW *)realloc(io->symboldraw, sizeof(SYMBOL_DRAW));

	if (!io->symboldraw) return;

	SYMBOL_DRAW *sd = io->symboldraw;

	sd->duration = (short)std::max(1l, long(duration));
	strcpy(sd->sequence, sequence);

	sd->starttime = (unsigned long)(arxtime);
	sd->lasttim = 0;
	sd->lastpos.x = io->pos.x - EEsin(radians(MAKEANGLE(io->angle.b - 45.0F + iPosX*2))) * 60.0F;
	sd->lastpos.y = io->pos.y - 120.0F - iPosY*5;
	sd->lastpos.z = io->pos.z + EEcos(radians(MAKEANGLE(io->angle.b - 45.0F + iPosX*2))) * 60.0F;
	
	sd->cPosStartX = checked_range_cast<char>(iPosX);
	sd->cPosStartY = checked_range_cast<char>(iPosY);
	
	io->gameFlags &= ~GFLAG_INVISIBILITY;
}

static void ARX_SPELLS_RequestSymbolDraw2(Entity *io, Rune symb, float duration)
{
	const char * sequence;
	int iPosX = 0;
	int iPosY = 0;

	switch (symb)
	{
		case RUNE_AAM   :
			iPosX = 0, iPosY = 2, sequence = "6666";
			break;
		case RUNE_CETRIUS:
			iPosX = 0, iPosY = 1, sequence = "33388886666";
			break;
		case RUNE_COMUNICATUM:
			iPosX = 0, iPosY = 0, sequence = "6666622244442226666";
			break;
		case RUNE_COSUM:
			iPosX = 0, iPosY = 2, sequence = "66666222244448888";
			break;
		case RUNE_FOLGORA:
			iPosX = 0, iPosY = 3, sequence = "99993333";
			break;
		case RUNE_FRIDD:
			iPosX = 0, iPosY = 4, sequence = "888886662222";
			break;
		case RUNE_KAOM:
			iPosX = 3, iPosY = 0, sequence = "44122366";
			break;
		case RUNE_MEGA:
			iPosX = 2, iPosY = 4, sequence = "88888";
			break;
		case RUNE_MORTE:
			iPosX = 0, iPosY = 2, sequence = "66666222";
			break;
		case RUNE_MOVIS:
			iPosX = 0, iPosY = 0, sequence = "666611116666";
			break;
		case RUNE_NHI:
			iPosX = 4, iPosY = 2, sequence = "4444";
			break;
		case RUNE_RHAA:
			iPosX = 2, iPosY = 0, sequence = "22222";
			break;
		case RUNE_SPACIUM:
			iPosX = 4, iPosY = 0, sequence = "44444222266688";
			break;
		case RUNE_STREGUM:
			iPosX = 0, iPosY = 4, sequence = "8888833338888";
			break;
		case RUNE_TAAR:
			iPosX = 0, iPosY = 1, sequence = "666222666";
			break;
		case RUNE_TEMPUS:
			iPosX = 0, iPosY = 4, sequence = "88886662226668866";
			break;
		case RUNE_TERA:
			iPosX = 0, iPosY = 3, sequence = "99922266";
			break;
		case RUNE_VISTA:
			iPosX = 1, iPosY = 0, sequence = "333111";
			break;
		case RUNE_VITAE:
			iPosX = 0, iPosY = 2, sequence = "66666888";
			break;
		case RUNE_YOK:
			iPosX = 0, iPosY = 0, sequence = "222226666888";
			break;
		default:
			return;
	}

	SYMBOL_DRAW * ptr;
	ptr = (SYMBOL_DRAW *)realloc(io->symboldraw, sizeof(SYMBOL_DRAW));

	if (!ptr) return;

	io->symboldraw = ptr;

	SYMBOL_DRAW *sd = io->symboldraw;
	sd->duration = duration < 1.0F ? 1 : (short)(long)duration;
	strcpy(sd->sequence, sequence);
	sd->starttime = (unsigned long)(arxtime);
	sd->lasttim = 0;
	
	sd->lastpos.x = io->pos.x - EEsin(radians(MAKEANGLE(io->angle.b - 45.0F + iPosX*2))) * 60.0F;
	sd->lastpos.y = io->pos.y - 120.0F - iPosY*5;
	sd->lastpos.z = io->pos.z + EEcos(radians(MAKEANGLE(io->angle.b - 45.0F + iPosX*2))) * 60.0F;
	
	sd->cPosStartX = checked_range_cast<char>(iPosX);
	sd->cPosStartY = checked_range_cast<char>(iPosY);

	io->gameFlags &= ~GFLAG_INVISIBILITY;

}

//-----------------------------------------------------------------------------
void ARX_SPELLS_RequestSymbolDraw3(const char *_pcName,char *_pcRes)
{
	if		(!strcmp(_pcName, "aam"))		strcpy(_pcRes, "6666");
	else if (!strcmp(_pcName, "cetrius"))	strcpy(_pcRes, "33388886666");
	else if (!strcmp(_pcName, "comunicatum")) 	strcpy(_pcRes, "6666622244442226666");
	else if (!strcmp(_pcName, "cosum"))     strcpy(_pcRes, "66666222244448888");
	else if (!strcmp(_pcName, "folgora"))   strcpy(_pcRes, "99993333");
	else if (!strcmp(_pcName, "fridd"))		strcpy(_pcRes, "888886662222");
	else if (!strcmp(_pcName, "kaom"))		strcpy(_pcRes, "44122366");
	else if (!strcmp(_pcName, "mega"))		strcpy(_pcRes, "88888");
	else if (!strcmp(_pcName, "morte"))		strcpy(_pcRes, "66666222");
	else if (!strcmp(_pcName, "movis"))		strcpy(_pcRes, "666611116666");
	else if (!strcmp(_pcName, "nhi"))		strcpy(_pcRes, "4444");
	else if (!strcmp(_pcName, "rhaa"))		strcpy(_pcRes, "22222");
	else if (!strcmp(_pcName, "spacium"))	strcpy(_pcRes, "44444222266688");
	else if (!strcmp(_pcName, "stregum"))	strcpy(_pcRes, "8888833338888");
	else if (!strcmp(_pcName, "taar"))		strcpy(_pcRes, "666222666");
	else if (!strcmp(_pcName, "tempus"))	strcpy(_pcRes, "88886662226668866");
	else if (!strcmp(_pcName, "tera"))		strcpy(_pcRes, "99922266");
	else if (!strcmp(_pcName, "vista"))		strcpy(_pcRes, "333111");
	else if (!strcmp(_pcName, "vitae"))		strcpy(_pcRes, "66666888");
	else if (!strcmp(_pcName, "yok"))		strcpy(_pcRes, "222226666888");
	else if (!strcmp(_pcName, "akbaa"))		strcpy(_pcRes, "22666772222");
}

#define OFFSET_X 8*2//0
#define OFFSET_Y 6*2//0

//-----------------------------------------------------------------------------
void GetSymbVector(char c,Vec2s * vec)
{
	switch (c)
	{
		case '1' :
			vec->x = -OFFSET_X, vec->y =  OFFSET_Y;
			break;
		case '2' :
			vec->x =         0, vec->y =  OFFSET_Y;
			break;
		case '3' :
			vec->x =  OFFSET_X, vec->y =  OFFSET_Y;
			break;
		case '4' :
			vec->x = -OFFSET_X, vec->y =         0;
			break;
		case '6' :
			vec->x =  OFFSET_X, vec->y =         0;
			break;
		case '7' :
			vec->x = -OFFSET_X, vec->y = -OFFSET_Y;
			break;
		case '8' :
			vec->x =         0, vec->y = -OFFSET_Y;
			break;
		case '9' :
			vec->x =  OFFSET_X, vec->y = -OFFSET_Y;
			break;
		default  :
			vec->x =         0, vec->y =         0;
			break;
	}
}

static bool MakeSpellName(char * spell, Spell num) {
	
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
	
	if(num < 0) {
		return;
	}
	
	if(size_t(num) >= MAX_SPELLS) {
		return;
	}
	
	char spell[128];
	long source = spells[num].caster;
	if(!MakeSpellName(spell, spells[num].type)) {
		return;
	}
	
	for(size_t i = 0; i < entities.size(); i++) {
		if(entities[i] != NULL) {
			EVENT_SENDER = (source >= 0) ? entities[source] : NULL;
			char param[256];
			sprintf(param, "%s %ld", spell, (long)spells[num].caster_level);
			SendIOScriptEvent(entities[i], SM_SPELLCAST, param);
		}
	}
}

//-----------------------------------------------------------------------------
void SPELLCAST_NotifyOnlyTarget(long num)
{
	if (num < 0) return;

	if ((size_t)num >= MAX_SPELLS) return;

	if(spells[num].target<0) return;

	char spell[128];
	long source = spells[num].caster;

	if (MakeSpellName(spell,spells[num].type))
	{
		if (source >= 0) EVENT_SENDER = entities[source];
		else EVENT_SENDER = NULL;

		char param[256];
		sprintf(param,"%s %ld",spell,(long)spells[num].caster_level);
		SendIOScriptEvent(entities[spells[num].target], SM_SPELLCAST, param);
	}	
}

//-----------------------------------------------------------------------------
void SPELLEND_Notify(long num)
{
	if(num < 0 || (size_t)num >= MAX_SPELLS) {
		return;
	}

	char spell[128];
	long source=spells[num].caster;

	if (spells[num].type==SPELL_CONFUSE)
	{
		if (ValidIONum(source))
			EVENT_SENDER = entities[source];
		else 
			EVENT_SENDER = NULL;

		if(ValidIONum(spells[num].target)) {
			if(MakeSpellName(spell,spells[num].type)) {
				Entity * targ = entities[spells[num].target];
				char param[128];
				sprintf(param,"%s %ld",spell,(long)spells[num].caster_level);
				SendIOScriptEvent(targ,SM_SPELLEND,param);
			}
		}
		return;
	}
	
	// we only notify player spells end.
	if(!MakeSpellName(spell,spells[num].type)) {
		return;
	}
	
	for (size_t i = 0; i < entities.size(); i++) {
		if(entities[i]) {
			EVENT_SENDER = ValidIONum(source) ? entities[source] : NULL;
			char param[128];
			sprintf(param,"%s %ld",spell,(long)spells[num].caster_level);
			SendIOScriptEvent(entities[i],SM_SPELLEND,param);
		}
	}
}

void ReCenterSequence(char *_pcSequence, int & _iMinX, int & _iMinY,
                      int & _iMaxX, int & _iMaxY) {
	
	int iSizeX=0,iSizeY=0;
	_iMinX=_iMinY=0;
	_iMaxX=_iMaxY=0;
	int iLenght=strlen(_pcSequence);

	for(int iI=0;iI<iLenght;iI++)
	{
		Vec2s es2dVector;
		GetSymbVector(_pcSequence[iI],&es2dVector);
		iSizeX+=es2dVector.x;
		iSizeY+=es2dVector.y;
		_iMinX=std::min(_iMinX,iSizeX);
		_iMinY=std::min(_iMinY,iSizeY);
		_iMaxX=std::max(_iMaxX,iSizeX);
		_iMaxY=std::max(_iMaxY,iSizeY);
	}
}

void ARX_SPELLS_UpdateSymbolDraw() {
	
	unsigned long curtime = (unsigned long)(arxtime);
	
	for(size_t i = 0; i < entities.size(); i++) {
		Entity * io = entities[i];
		if (io) 
		{
			if (io->spellcast_data.castingspell != SPELL_NONE)
			{
				if (io->symboldraw==NULL)
				{
					long tst=0;

					if (!(io->spellcast_data.spell_flags & SPELLCAST_FLAG_NOANIM) &&  (io->ioflags & IO_NPC))
					{
						ANIM_USE * ause1=&io->animlayer[1];

						if (ause1->cur_anim==io->anims[ANIM_CAST_START]  && (ause1->flags & EA_ANIMEND)) 
						{
							FinishAnim(io,ause1->cur_anim);
							ANIM_Set(ause1,io->anims[ANIM_CAST_CYCLE]);
							tst=1;
						}
						else if (ause1->cur_anim==io->anims[ANIM_CAST_CYCLE]) tst=1;
						else if (ause1->cur_anim!=io->anims[ANIM_CAST_START])
							io->spellcast_data.castingspell = SPELL_NONE;
					}
					else tst=1;

					if ((io->spellcast_data.symb[0] != RUNE_NONE)  && tst )
					{
						Rune symb = io->spellcast_data.symb[0];

						for (long j=0;j<3;j++)
							io->spellcast_data.symb[j]=io->spellcast_data.symb[j+1];

						io->spellcast_data.symb[3] = RUNE_NONE;
						ARX_SPELLS_RequestSymbolDraw2(io, symb, (1000-(io->spellcast_data.spell_level*60))*std::max(io->speed_modif+io->basespeed,0.01f));
						io->gameFlags &=~GFLAG_INVISIBILITY;
					}
					else if (tst)// cast spell !!!
					{					
						io->gameFlags &=~GFLAG_INVISIBILITY;
						ARX_SPELLS_Launch(io->spellcast_data.castingspell,i,io->spellcast_data.spell_flags,io->spellcast_data.spell_level,io->spellcast_data.target,io->spellcast_data.duration);

						if (!(io->spellcast_data.spell_flags & SPELLCAST_FLAG_NOANIM)
								&&  (io->ioflags & IO_NPC))
						{
							ANIM_USE * ause1=&io->animlayer[1];
							AcquireLastAnim(io);
							FinishAnim(io,ause1->cur_anim);
							ANIM_Set(ause1,io->anims[ANIM_CAST]);
						}

						io->spellcast_data.castingspell = SPELL_NONE;
					}
				}
			}

			float rr=rnd();

			if (io->flarecount)
			{
				if (io->dynlight==-1) io->dynlight=(short)GetFreeDynLight();

				if (io->dynlight!=-1)
				{
					DynLight[io->dynlight].pos.x=io->pos.x-EEsin(radians(MAKEANGLE(io->angle.b-45.f)))*60.f;
					DynLight[io->dynlight].pos.y=io->pos.y-120.f;
					DynLight[io->dynlight].pos.z=io->pos.z+EEcos(radians(MAKEANGLE(io->angle.b-45.f)))*60.f;
					DynLight[io->dynlight].fallstart=140.f+(float)io->flarecount*0.333333f+rr*5.f;
					DynLight[io->dynlight].fallend=220.f+(float)io->flarecount*0.5f+rr*5.f;
					DynLight[io->dynlight].intensity=1.6f;
					DynLight[io->dynlight].exist=1;	
					DynLight[io->dynlight].rgb.r=0.01f*io->flarecount*2;	
					DynLight[io->dynlight].rgb.g=0.009f*io->flarecount*2;
					DynLight[io->dynlight].rgb.b=0.008f*io->flarecount*2;
				}
			}
			else if (io->dynlight>-1) 
			{
				DynLight[io->dynlight].exist=0;
				io->dynlight=-1;
			}

			if(io->symboldraw) {
				SYMBOL_DRAW * sd = entities[i]->symboldraw;
				long tim=curtime-sd->starttime;
 


				if (tim>sd->duration)
				{
					if (io->dynlight!=-1)
					{
						DynLight[io->dynlight].time_creation = (unsigned long)(arxtime);
						DynLight[io->dynlight].duration = 600; 
						io->dynlight=-1;
					}			

					free(io->symboldraw);
					io->symboldraw=NULL;
					continue;
				}

				long nbcomponents=strlen(sd->sequence);

				if (nbcomponents<=0)
				{
					free(io->symboldraw);
					io->symboldraw=NULL;
					continue;
				}

				float ti=((float)sd->duration/(float)nbcomponents);

				if (ti<=0) ti=1;

				Vec2s pos1, vect, old_pos;
				long newtime=tim;
				long oldtime=sd->lasttim;

				if (oldtime>sd->duration) oldtime=sd->duration;

				if (newtime>sd->duration) newtime=sd->duration;

				sd->lasttim=(short)tim;

				pos1.x = (short)subj.centerx -OFFSET_X*2 + sd->cPosStartX*OFFSET_X;
				pos1.y = (short)subj.centery -OFFSET_Y*2 + sd->cPosStartY*OFFSET_Y;

				float div_ti=1.f/ti;

				if (io != entities.player())
				{
					old_pos = pos1;

					for (long j=0;j<nbcomponents;j++)
					{
						GetSymbVector(sd->sequence[j],&vect);
						vect += vect / 2;

						if(oldtime <= ti) {
							float ratio = float(oldtime)*div_ti;
							old_pos += (vect.to<float>() * ratio).to<short>();
							break;
						}

						old_pos += vect;
						oldtime-=(long)ti;
					}

					for (int j=0;j<nbcomponents;j++)
					{
						GetSymbVector(sd->sequence[j],&vect);
						vect += vect / 2;

						if (newtime<=ti)
						{
							float ratio = float(newtime) * div_ti;
							pos1 += (vect.to<float>() * ratio).to<short>();
							AddFlare(&pos1,0.1f,1,entities[i]);
							FlareLine(&old_pos,&pos1,entities[i]);
							break;
						}

						pos1 += vect;
						newtime-=(long)ti;
					}
				}
				else 
				{
					int iMinX,iMinY,iMaxX,iMaxY;
					int iSizeX,iSizeY;
					ReCenterSequence(sd->sequence,iMinX,iMinY,iMaxX,iMaxY);
					iSizeX=iMaxX-iMinX;
					iSizeY=iMaxY-iMinY;
					pos1.x = 97;
					pos1.y = 64;


					long lPosX	= (((513>>1)-lMaxSymbolDrawSizeX)>>1);
					long lPosY	= (313-(((313*3/4)-lMaxSymbolDrawSizeY)>>1));

					pos1.x = checked_range_cast<short>(pos1.x + lPosX);
					pos1.y = checked_range_cast<short>(pos1.y + lPosY);



					lPosX =  ((lMaxSymbolDrawSizeX-iSizeX)>>1);
					lPosY =  ((lMaxSymbolDrawSizeY-iSizeY)>>1);

					pos1.x = checked_range_cast<short>(pos1.x + lPosX);
					pos1.y = checked_range_cast<short>(pos1.y + lPosY);



					int iX = pos1.x-iMinX;
					int iY = pos1.y-iMinY;

					pos1.x = checked_range_cast<short>(iX);
					pos1.y = checked_range_cast<short>(iY);


					for (long j=0;j<nbcomponents;j++)
					{

						GetSymbVector(sd->sequence[j],&vect);

						if (newtime<ti)
						{
							float ratio = (float)(newtime) * div_ti;
							

							float fX = pos1.x + (ratio*vect.x)*0.5f;
							float fY = pos1.y + (ratio*vect.y)*0.5f; 

							pos1.x = checked_range_cast<short>(fX);
							pos1.y = checked_range_cast<short>(fY);


							Vec2s pos;
							pos.x=(short)(pos1.x*Xratio);	
							pos.y=(short)(pos1.y*Yratio);

							if (io == entities.player())
								AddFlare2(&pos,0.1f,1,entities[i]);
							else
								AddFlare(&pos,0.1f,1,entities[i]);


							break;
						}

						pos1 += vect;

						newtime-=(long)ti;
					}
				}
			}
		}
	}
}

void ARX_SPELLS_ClearAllSymbolDraw() {
	BOOST_FOREACH(Entity * e, entities) {
		if(e && e->symboldraw) {
			free(e->symboldraw), e->symboldraw = NULL;
		}
	}
}

static void ARX_SPELLS_AnalyseSYMBOL() {
	
	long sm = atoi(SpellMoves);
	switch(sm) {
		
		// COSUM
		case 62148  :
		case 632148 :
		case 62498  :
		case 62748  :
		case 6248   :
				SpellSymbol[CurrSpellSymbol++] = RUNE_COSUM;

				if((size_t)CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
					CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
				}

				ARX_SOUND_PlaySFX(SND_SYMB_COSUM);
			break;

		// COMUNICATUM
		case 632426 :
		case 627426 :
		case 634236 :
		case 624326 :
		case 62426  :
				SpellSymbol[CurrSpellSymbol++] = RUNE_COMUNICATUM;

				if((size_t)CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
					CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
				}

				ARX_SOUND_PlaySFX(SND_SYMB_COMUNICATUM);
			break;

		// FOLGORA
		case 9823   :
		case 9232   :
		case 983    :
		case 963    :
		case 923    :
		case 932    :
		case 93     :
				SpellSymbol[CurrSpellSymbol++] = RUNE_FOLGORA;

				if((size_t)CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
					CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
				}

				ARX_SOUND_PlaySFX(SND_SYMB_FOLGORA);
			break;

		// SPACIUM
		case 42368  :
		case 42678  :
		case 42698  :
		case 4268   :
				SpellSymbol[CurrSpellSymbol++] = RUNE_SPACIUM;

				if((size_t)CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
					CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
				}

				ARX_SOUND_PlaySFX(SND_SYMB_SPACIUM);
			break;

		// TERA
		case 9826   :
		case 92126  :
		case 9264   :
		case 9296   :
		case 926    :
				SpellSymbol[CurrSpellSymbol++] = RUNE_TERA;

				if((size_t)CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
					CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
				}

				ARX_SOUND_PlaySFX(SND_SYMB_TERA);
			break;

		// CETRIUS
		case 286   :
		case 3286  :
		case 23836 :
		case 38636 :
		case 2986  :
		case 2386  :
		case 386   :
				SpellSymbol[CurrSpellSymbol++] = RUNE_CETRIUS;

				if((size_t)CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
					CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
				}

				ARX_SOUND_PlaySFX(SND_SYMB_CETRIUS);
			break;

		// RHAA
		case 28    :
		case 2     :
				SpellSymbol[CurrSpellSymbol++] = RUNE_RHAA;

				if((size_t)CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
					CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
				}

				ARX_SOUND_PlaySFX(SND_SYMB_RHAA);
			break;

		// FRIDD
		case 98362	:
		case 8362	:
		case 8632	:
		case 8962	:
		case 862	:
				SpellSymbol[CurrSpellSymbol++] = RUNE_FRIDD;

				if((size_t)CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
					CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
				}

				ARX_SOUND_PlaySFX(SND_SYMB_FRIDD);
			break;

		// KAOM
		case 41236	:
		case 23		:
		case 236	:
		case 2369	:
		case 136	:
		case 12369	:
		case 1236	:

				if ((cur_arm>=0) && (cur_arm & 1) )
				{
					cur_arm++;					

					if (cur_arm>20)
						ApplySPArm();
				}
				else
					cur_arm=-1;

				SpellSymbol[CurrSpellSymbol++] = RUNE_KAOM;

				if((size_t)CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
					CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
				}

				ARX_SOUND_PlaySFX(SND_SYMB_KAOM);
			break;

		// STREGUM
		case 82328 :
		case 8328  :
		case 2328  :
		case 8938  :
		case 8238  :
		case 838   :
				SpellSymbol[CurrSpellSymbol++] = RUNE_STREGUM;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_STREGUM);
			break;

		// MORTE
		case 628   :
		case 621   :
		case 62    :
				SpellSymbol[CurrSpellSymbol++] = RUNE_MORTE;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_MORTE);
			break;

		// TEMPUS
		case 962686  :
		case 862686  :
		case 8626862 : 
				SpellSymbol[CurrSpellSymbol++] = RUNE_TEMPUS;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_TEMPUS);
			break;

		// MOVIS
		case 6316:
		case 61236:
		case 6146:
		case 61216:
		case 6216:
		case 6416:
		case 62126:
		case 61264:
		case 6126:
		case 6136:
		case 616: 
				SpellSymbol[CurrSpellSymbol++] = RUNE_MOVIS;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_MOVIS);
			break;

		// NHI
		case 46:
		case 4:
				SpellSymbol[CurrSpellSymbol++] = RUNE_NHI;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_NHI);
			break;

		// AAM
		case 64:
		case 6:
				SpellSymbol[CurrSpellSymbol++] = RUNE_AAM;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_AAM);
			break;
																		
		// YOK
		case 412369:
		case 2687:
		case 2698:
		case 2638:
		case 26386:
		case 2368:
		case 2689:
		case 268:
				SpellSymbol[CurrSpellSymbol++] = RUNE_YOK;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_YOK);
			break;

		// TAAR
		case 6236:
		case 6264:
		case 626:
				SpellSymbol[CurrSpellSymbol++] = RUNE_TAAR;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_TAAR);
			break;

		// MEGA
		case 82:
		case 8:

				if ((cur_arm>=0) && !(cur_arm & 1) )
				{
					cur_arm++;					
				}
				else
					cur_arm=-1;

				SpellSymbol[CurrSpellSymbol++] = RUNE_MEGA;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_MEGA);
			break;

		// VISTA
		case 3614:
		case 361:
		case 341:
		case 3212:
		case 3214:
		case 312:
		case 314:
		case 321:
		case 31:
				SpellSymbol[CurrSpellSymbol++] = RUNE_VISTA;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_VISTA);
			break;

		// VITAE
		case 698:
		case 68:
			SpellSymbol[CurrSpellSymbol++] = RUNE_VITAE;

			if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

			ARX_SOUND_PlaySFX(SND_SYMB_VITAE);
			break;

//--------------------------------------------------------------------------------------------------------------------

			// Special UW mode
		case 238:
		case 2398:
		case 23898:
		case 236987:
		case 23698:
				
			if (uw_mode_pos == 0) uw_mode_pos++;
		
			goto failed; 
		break;
		case 2382398:
		case 2829:
		case 23982398:
		case 39892398:
		case 2398938:
		case 28239898:
		case 238982398:
		case 238923898:
		case 28982398:
		case 3923989:
		case 292398:
		case 398329:
		case 38923898:
		case 2398289:
		case 289823898:
		case 2989238:
		case 29829:
		case 2389823898u:
		case 2393239:
		case 38239:
		case 2982323989u:
		case 3298232329u:
		case 239829:
		case 2898239:
		case 28982898:
		case 389389:
		case 3892389:
		case 289289:
		case 289239:
		case 239289:
		case 2989298:
		case 2392398:
		case 238929:
		case 28923898:
		case 2929:
		case 2398298:
		case 239823898:
		case 28238:
		case 2892398:
		case 28298:
		case 298289:
		case 38929:
		case 2389238989u:
		case 289298989:
		case 23892398:
		case 238239:
		case 29298:
		case 2329298:
		case 232389829:
		case 2389829:
		case 239239:
		case 282398:
		case 2398982398u:
		case 2389239:
		case 2929898:
		case 3292398:
		case 23923298:
		case 23898239:
		case 3232929:
		case 2982398:
		case 238298:
		case 3939:

			if (uw_mode_pos == 1)
			{
				ApplySPuw();
			}

			goto failed; 
		break;
		case 161:
		case 1621:
		case 1261:

			if (cur_sm==0) cur_sm++;

			if (cur_bh==0) cur_bh++;

			if (cur_bh==2) cur_bh++;

			if (cur_sos==0) cur_sos++;

			if (cur_sos == 2)
			{
				cur_sos = 0;
				ApplyCurSOS();
			}

			goto failed;
			break;
		case 83614:
		case 8361:
		case 8341:
		case 83212:
		case 83214:
		case 8312:
		case 8314:
		case 8321:
		case 831:
		case 82341:
		case 834:
		case 823:
		case 8234:
		case 8231:  

			if (cur_pom==0) cur_pom++; 

			if (cur_pnux==0) cur_pnux++;

			if (cur_pnux==2) cur_pnux++;			

			if (cur_bh == 1)
			{
				cur_bh++;
			}

			if (cur_bh == 3)
			{
				cur_bh = 0;
				EERIE_OBJECT_SetBHMode();
			}

			goto failed;				  
			break;
		break;

		case 83692:
		case 823982:
		case 83982:
		case 82369892:
		case 82392:
		case 83892:
		case 823282:
		case 8392:
		{
			if (cur_sm == 2)
			{
				cur_sm++;
				ApplySPBow();
			}

			if (cur_mx == 0)
			{
				cur_mx = 1;
			}

			if (cur_mr == 0)
			{
				cur_mr = 1;
			}

			if (cur_pom == 2)
			{
				cur_pom++;
				ApplySPWep();
			}

				goto failed;
			}
			break;			
		case 98324:
		case 92324:
		case 89324:
		case 9324:
		case 9892324:
		case 9234:
		case 934:
		{
			if (cur_mr == 1)
			{
				cur_mr = 2;
				MakeCoolFx(&player.pos);
			}

			if (cur_mx == 1)
			{
				cur_mx = 2;
				MakeCoolFx(&player.pos);
			}

			if (cur_rf == 1)
			{
				cur_rf = 2;
				MakeCoolFx(&player.pos);
			}

				if (cur_sm==1) cur_sm++;

				goto failed;
			}
			break;
		case 3249:
		case 2349:
		case 323489:
		case 23249:
		case 3489:
		case 32498:
		case 349:
		{
			if (cur_mx == 2)
			{
				cur_mx = 3;
				ApplySPMax();
			}

				goto failed;				
			}
		break;

		case 26:
		{
			if (cur_pnux == 1)
			{
				cur_pnux++;
				   }

			if (cur_pnux == 3)
		{
				cur_pnux++;
				ApplyCurPNux();
			}

			goto failed;
		}
		break;
		case 9232187:
		case 93187:
		case 9234187:
		case 831878:
		case 923187:
		case 932187:
		case 93217:
		case 9317:
		{
			if (cur_pom==1) cur_pom++; 

			if (cur_sos==1) cur_sos++;

			goto failed;
		}
		break;
		case 82313:
		case 8343:
		case 82343:
		case 83413:
		case 8313:
		{
			if (cur_mr == 2)
			{
				cur_mr = 3;
				MakeCoolFx(&player.pos);
				ApplyCurMr();
			}

			if (cur_rf == 0)
			{
				cur_rf = 1;
			}

			goto failed;
			break;
		}
		case 86:

			if (cur_rf == 2)
		{
				cur_rf = 3;
				MakeCoolFx(&player.pos);
				ApplySPRf();
		}

			goto failed;
			break;

		case 626262: 
		{
			passwall++;

			if (passwall==3)
			{
				passwall=0;
				ApplyPasswall(); 
			}
		}
		break;
		case 828282: 
		{
			player.skin++;

			if ((player.skin==4) && (rnd()<0.9f))
				player.skin++;

			if (player.skin>5)
				player.skin=0;

			ARX_EQUIPMENT_RecreatePlayerMesh();
			 goto failed; 
		}
		break;

		default:
		{
		failed:
			;
			std::string tex;

			if (SpellMoves.length()>=127)
				SpellMoves.resize(127);

			LAST_FAILED_SEQUENCE = SpellMoves;

			LogDebug("Unknown Symbol - " + SpellMoves);
		}
	}

	bPrecastSpell = false;

	// wanna precast?
	if (GInput->actionPressed(CONTROLS_CUST_STEALTHMODE))
	{
		bPrecastSpell = true;
	}
}

struct SpellDefinition {
	SpellDefinition * next[RUNE_COUNT];
	Spell spell;
	SpellDefinition() : spell(SPELL_NONE) {
		for(size_t i = 0; i < RUNE_COUNT; i++) {
			next[i] = NULL;
		}
	}

	~SpellDefinition() {
		for(size_t i = 0; i < RUNE_COUNT; i++) {
			if(next[i]) {
				delete next[i];
			}
		}
	}
};

static SpellDefinition definedSpells;
typedef std::map<string, Spell> SpellNames;
static SpellNames spellNames;

static void addSpell(const Rune symbols[MAX_SPELL_SYMBOLS], Spell spell, const string & name) {
	
	typedef std::pair<SpellNames::const_iterator, bool> Res;
	Res res = spellNames.insert(std::make_pair(name, spell));
	if(!res.second) {
		LogWarning << "duplicate spell name: " + name;
	}
	
	if(symbols[0] == RUNE_NONE) {
		return;
	}
	
	SpellDefinition * def = &definedSpells;
	
	for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
		if(symbols[i] == RUNE_NONE) {
			break;
		}
		arx_assert(symbols[i] >= 0 && (size_t)symbols[i] < RUNE_COUNT);
		if(def->next[symbols[i]] == NULL) {
			def->next[symbols[i]] = new SpellDefinition();
		}
		def = def->next[symbols[i]];
	}
	
	arx_assert(def->spell == SPELL_NONE);
	
	def->spell = spell;
}

static Spell getSpell(const Rune symbols[MAX_SPELL_SYMBOLS]) {
	
	const SpellDefinition * def = &definedSpells;
	
	for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
		if(symbols[i] == RUNE_NONE) {
			break;
		}
		arx_assert(symbols[i] >= 0 && (size_t)symbols[i] < RUNE_COUNT);
		if(def->next[symbols[i]] == NULL) {
			return SPELL_NONE;
		}
		def = def->next[symbols[i]];
	}
	
	return def->spell;
}

Spell GetSpellId(const string & spell) {
	
	SpellNames::const_iterator it = spellNames.find(spell);
	
	return (it == spellNames.end()) ? SPELL_NONE : it->second;
}

struct RawSpellDefinition {
	Rune symbols[MAX_SPELL_SYMBOLS];
	Spell spell;
	std::string name;
};

// TODO move to external file
static const RawSpellDefinition allSpells[] = {
	{{RUNE_RHAA, RUNE_STREGUM, RUNE_VITAE, RUNE_NONE}, SPELL_CURSE, "curse"}, // level 4
	{{RUNE_RHAA, RUNE_TEMPUS, RUNE_NONE}, SPELL_FREEZE_TIME, "freeze_time"}, // level 10
	{{RUNE_RHAA, RUNE_KAOM, RUNE_NONE}, SPELL_LOWER_ARMOR, "lower_armor"}, // level 2
	{{RUNE_RHAA, RUNE_MOVIS, RUNE_NONE}, SPELL_SLOW_DOWN, "slowdown"}, // level 6
	{{RUNE_RHAA, RUNE_VITAE, RUNE_NONE}, SPELL_HARM, "harm"}, // level 2
	{{RUNE_RHAA, RUNE_VISTA, RUNE_NONE}, SPELL_CONFUSE, "confuse"}, // level 7
	{{RUNE_MEGA, RUNE_NHI, RUNE_MOVIS, RUNE_NONE}, SPELL_MASS_PARALYSE, "mass_paralyse"}, // level 9
	{{RUNE_MEGA, RUNE_KAOM, RUNE_NONE}, SPELL_ARMOR, "armor"}, // level 2
	{{RUNE_MEGA, RUNE_VISTA, RUNE_NONE}, SPELL_MAGIC_SIGHT, "magic_sight"}, // level 1
	{{RUNE_MEGA, RUNE_VITAE, RUNE_NONE}, SPELL_HEAL, "heal"}, // level 2
	{{RUNE_MEGA, RUNE_MOVIS, RUNE_NONE}, SPELL_SPEED, "speed"}, // level 3
	{{RUNE_MEGA, RUNE_STREGUM, RUNE_VITAE, RUNE_NONE}, SPELL_BLESS, "bless"}, // level 4
	{{RUNE_MEGA, RUNE_STREGUM, RUNE_COSUM, RUNE_NONE}, SPELL_ENCHANT_WEAPON, "enchant_weapon"}, // level 8
	{{RUNE_MEGA, RUNE_AAM, RUNE_MEGA, RUNE_YOK, RUNE_NONE}, SPELL_MASS_INCINERATE, "mass_incinerate"}, // level 10
	{{RUNE_MEGA, RUNE_SPACIUM, RUNE_NONE}, SPELL_ACTIVATE_PORTAL, "activate_portal"}, // level ?
	{{RUNE_MEGA, RUNE_SPACIUM, RUNE_MOVIS, RUNE_NONE}, SPELL_LEVITATE, "levitate"}, // level 5
	{{RUNE_NHI, RUNE_MOVIS, RUNE_NONE}, SPELL_PARALYSE, "paralyse"}, // level 6
	{{RUNE_NHI, RUNE_CETRIUS, RUNE_NONE}, SPELL_CURE_POISON, "cure_poison"}, // level 5
	{{RUNE_NHI, RUNE_YOK, RUNE_NONE}, SPELL_DOUSE, "douse"}, // level 1
	{{RUNE_NHI, RUNE_STREGUM, RUNE_VISTA, RUNE_NONE}, SPELL_DISPELL_ILLUSION, "dispell_illusion"}, // level 3
	{{RUNE_NHI, RUNE_STREGUM, RUNE_SPACIUM, RUNE_NONE}, SPELL_NEGATE_MAGIC, "negate_magic"}, // level 9
	{{RUNE_NHI, RUNE_SPACIUM, RUNE_NONE}, SPELL_DISPELL_FIELD, "dispell_field"}, // level 4
	{{RUNE_NHI, RUNE_MORTE, RUNE_COSUM, RUNE_NONE}, SPELL_DISARM_TRAP, "disarm_trap"}, // level 6
	{{RUNE_NHI, RUNE_VISTA, RUNE_NONE}, SPELL_INVISIBILITY, "invisibility"}, // level ?
	{{RUNE_VISTA, RUNE_MOVIS, RUNE_NONE}, SPELL_FLYING_EYE, "flying_eye"}, // level 7
	{{RUNE_MORTE, RUNE_KAOM, RUNE_NONE}, SPELL_REPEL_UNDEAD, "repel_undead"}, // level 5
	{{RUNE_MORTE, RUNE_COSUM, RUNE_VISTA, RUNE_NONE}, SPELL_DETECT_TRAP, "detect_trap"}, // level 2
	{{RUNE_MOVIS, RUNE_COMUNICATUM, RUNE_NONE}, SPELL_CONTROL_TARGET, "control"}, // level 10
	{{RUNE_STREGUM, RUNE_MOVIS, RUNE_NONE}, SPELL_MANA_DRAIN, "mana_drain"}, // level 8
	{{RUNE_AAM, RUNE_MEGA, RUNE_YOK, RUNE_NONE}, SPELL_INCINERATE, "incinerate"}, // level 9
	{{RUNE_AAM, RUNE_MEGA, RUNE_MORTE, RUNE_NONE}, SPELL_EXPLOSION, "explosion"}, // level 8
	{{RUNE_AAM, RUNE_KAOM, RUNE_SPACIUM, RUNE_NONE}, SPELL_CREATE_FIELD, "create_field"}, // level 6
	{{RUNE_AAM, RUNE_MORTE, RUNE_VITAE, RUNE_NONE}, SPELL_RISE_DEAD, "raise_dead"}, // level 6
	{{RUNE_AAM, RUNE_MORTE, RUNE_COSUM, RUNE_NONE}, SPELL_RUNE_OF_GUARDING, "rune_of_guarding"}, // level 5
	{{RUNE_AAM, RUNE_VITAE, RUNE_TERA, RUNE_NONE}, SPELL_SUMMON_CREATURE, "summon_creature"}, // level 9
	{{RUNE_AAM, RUNE_VITAE, RUNE_COSUM, RUNE_NONE}, SPELL_CREATE_FOOD, "create_food"}, // level 3
	{{RUNE_AAM, RUNE_FOLGORA, RUNE_TAAR, RUNE_NONE}, SPELL_LIGHTNING_STRIKE, "lightning_strike"}, // level 7
	{{RUNE_AAM, RUNE_FOLGORA, RUNE_SPACIUM, RUNE_NONE}, SPELL_MASS_LIGHTNING_STRIKE, "mass_lightning_strike"}, // level 10
	{{RUNE_AAM, RUNE_YOK, RUNE_NONE}, SPELL_IGNIT, "ignit"}, // level 1
	{{RUNE_AAM, RUNE_YOK, RUNE_SPACIUM, RUNE_NONE}, SPELL_FIRE_FIELD, "fire_field"}, // level 7
	{{RUNE_AAM, RUNE_YOK, RUNE_TAAR, RUNE_NONE}, SPELL_FIREBALL, "fireball"}, // level 3
	{{RUNE_AAM, RUNE_FRIDD, RUNE_SPACIUM, RUNE_NONE}, SPELL_ICE_FIELD, "ice_field"}, // level 7
	{{RUNE_AAM, RUNE_FRIDD, RUNE_TAAR, RUNE_NONE}, SPELL_ICE_PROJECTILE, "ice_projectile"}, // level 3
	{{RUNE_AAM, RUNE_CETRIUS, RUNE_TAAR, RUNE_NONE}, SPELL_POISON_PROJECTILE, "poison_projectile"}, // level 5
	{{RUNE_AAM, RUNE_TAAR, RUNE_NONE}, SPELL_MAGIC_MISSILE, "magic_missile"}, // level 1
	{{RUNE_YOK, RUNE_KAOM, RUNE_NONE}, SPELL_FIRE_PROTECTION, "fire_protection"}, // level 4
	{{RUNE_FRIDD, RUNE_KAOM, RUNE_NONE}, SPELL_COLD_PROTECTION, "cold_protection"}, // level 4
	{{RUNE_VITAE, RUNE_MOVIS, RUNE_NONE}, SPELL_LIFE_DRAIN, "life_drain"}, // level 8
	{{RUNE_SPACIUM, RUNE_COMUNICATUM, RUNE_NONE}, SPELL_TELEKINESIS, "telekinesis"}, // level 4
	{{RUNE_NONE}, SPELL_FAKE_SUMMON, "fake_summon"}
};

//! Plays the sound of Fizzling spell
static void ARX_SPELLS_Fizzle(long num) {
	if(num < 0) {
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE); // player fizzle
	} else {
		spells[num].tolive = 0;
		
		if(spells[num].caster >= 0) {
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &spells[num].caster_pos);
		}
	}
}

static bool ARX_SPELLS_AnalyseSPELL() {
	
	long caster = 0; // Local Player
	SpellcastFlags flags = 0;
	
	if(GInput->actionPressed(CONTROLS_CUST_STEALTHMODE) || bPrecastSpell) {
		flags |= SPELLCAST_FLAG_PRECAST;
	}
	
	bPrecastSpell = false;
	
	Spell spell;
	
	if(SpellSymbol[0] == RUNE_MEGA && SpellSymbol[1] == RUNE_MEGA
	   && SpellSymbol[2] == RUNE_MEGA && SpellSymbol[3] == RUNE_AAM
	   && SpellSymbol[4] == RUNE_VITAE && SpellSymbol[5] == RUNE_TERA) {
		cur_mega = 10;
		spell = SPELL_SUMMON_CREATURE;
	} else {
		spell = getSpell(SpellSymbol);
	}
	
	if(spell == SPELL_NONE) {
		
		ARX_SPELLS_Fizzle(-1);
		
		if(player.SpellToMemorize.bSpell) {
			CurrSpellSymbol = 0;
			player.SpellToMemorize.bSpell = false;
		}
		
		return false;
	}
	
	return ARX_SPELLS_Launch(spell, caster, flags);
	
}


bool No_MagicAllowed()
{
	ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
	return false;
}
extern long PLAYER_PARALYSED;


static void ARX_SPELLS_Analyse() {
	
	unsigned char dirs[MAX_POINTS];
	unsigned char lastdir = 255;
	long cdir = 0;

	for(long i = 1; i < CurrPoint ; i++) {
		
		Vec2s d = plist[i-1] - plist[i];
		
		if(d.lengthSqr() > 100) {
			
			float a = (float)abs(d.x);
			float b = (float)abs(d.y);
			
			if(b != 0.f && a / b > 0.4f && a / b < 2.5f) {
				// Diagonal movemement.
				
				if(d.x < 0 && d.y < 0) {
					if(lastdir != ADOWNRIGHT) {
						lastdir = dirs[cdir++] = ADOWNRIGHT;
					}
				} else if(d.x > 0 && d.y < 0) {
					if(lastdir != ADOWNLEFT) {
						lastdir = dirs[cdir++] = ADOWNLEFT;
					}
				} else if(d.x < 0 && d.y > 0) {
					if(lastdir != AUPRIGHT) {
						lastdir = dirs[cdir++] = AUPRIGHT;
					}
				} else if(d.x > 0 && d.y > 0) {
					if(lastdir != AUPLEFT) {
						lastdir = dirs[cdir++] = AUPLEFT;
					}
				}
				
			} else if(a > b) {
				// Horizontal movement.
				
				if(d.x < 0) {
					if(lastdir != ARIGHT) {
						lastdir = dirs[cdir++] = ARIGHT;
					}
				} else {
					if(lastdir != ALEFT) {
						lastdir = dirs[cdir++] = ALEFT;
					}
				}
				
			} else {
				// Vertical movement.
				
				if(d.y < 0) {
					if(lastdir != ADOWN) {
						lastdir = dirs[cdir++] = ADOWN;
					}
				} else {
					if(lastdir != AUP) {
						lastdir = dirs[cdir++] = AUP;
					}
				}
			}
		}
	}

	SpellMoves.clear();
	
	if ( cdir > 0 )
	{

		for (long i = 0 ; i < cdir ; i++ )
		{
			switch ( dirs[i] )
			{
				case AUP:
					SpellMoves += "8"; //uses PAD values
					break;

				case ADOWN:
					SpellMoves += "2";
					break;

				case ALEFT:
					SpellMoves += "4";
					break;

				case ARIGHT:
					SpellMoves += "6";
					break;

				case AUPRIGHT:
					SpellMoves += "9";
					break;

				case ADOWNRIGHT:
					SpellMoves += "3";
					break;

				case AUPLEFT:
					SpellMoves += "7";
					break;

				case ADOWNLEFT:
					SpellMoves += "1";
					break;
			}
		}
	}
}

void ARX_SPELLS_ManageMagic()
{
	if (ARXmenu.currentmode!=AMCM_OFF)
		return;

	Entity * io=entities.player();

	if (!io) return;

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

	if ((!(player.Current_Movement & PLAYER_CROUCH)) && (!BLOCK_PLAYER_CONTROLS && 
		(GInput->actionPressed(CONTROLS_CUST_MAGICMODE))) && (!PLAYER_PARALYSED))
	{
		
		if (player.Interface & INTER_COMBATMODE)
		{
			WILLRETURNTOCOMBATMODE=1;

			ARX_INTERFACE_Combat_Mode(0);
			bGToggleCombatModeWithKey=false;

			
			ResetAnim(&entities.player()->animlayer[1]);
			entities.player()->animlayer[1].flags&=~EA_LOOP;
		}

		if (TRUE_PLAYER_MOUSELOOK_ON)
		{
			WILLRETURNTOFREELOOK = 1;
			TRUE_PLAYER_MOUSELOOK_ON = false;
		}

		if (player.doingmagic!=2)
		{
			player.doingmagic=2;

			if (entities.player()->anims[ANIM_CAST_START])
			{
				AcquireLastAnim(entities.player());
				ANIM_Set(&entities.player()->animlayer[1],entities.player()->anims[ANIM_CAST_START]);
				MAGICMODE = 1;
			}
		}
		
		if (snip >= 2)
		{	
			if ((!EERIEMouseButton & 1) && (ARX_FLARES_broken==0)) // TODO should this be !(EERIEMouseButton & 1)?
			{
				ARX_FLARES_broken=2;
				PIPOrgb++;

				if (PIPOrgb>2) PIPOrgb=0;			
			}
			
			if(EERIEMouseButton & 1) {
				
				Vec2s pos = DANAEMouse;
				if(TRUE_PLAYER_MOUSELOOK_ON) {
					pos = MemoMouse;
				}
				
				Vec2s pos2 = Lm;
				
				if (!ARX_FLARES_broken) FlareLine(&pos2,&pos);

				if (rnd()>0.6) AddFlare(&pos,1.f,-1);
				else AddFlare(&pos,1.f,3);
				
				OPIPOrgb = PIPOrgb;
				
				Lm = DANAEMouse;
				if(TRUE_PLAYER_MOUSELOOK_ON) {
					Lm = MemoMouse;
				}
				
				ARX_FLARES_broken=0;
				
				if (!ARX_SOUND_IsPlaying(SND_MAGIC_DRAW))
					ARX_SOUND_PlaySFX(SND_MAGIC_DRAW, NULL, 1.0F, ARX_SOUND_PLAY_LOOPED);
			}
			else
			{
				ARX_SOUND_Stop(SND_MAGIC_DRAW);				
			}
			
			snip=0;
		}
	}
	else
	{
		ARX_FLARES_broken=1;
		PIPOrgb++;

		if (PIPOrgb>2) PIPOrgb=0;

		if (player.doingmagic!=0)//==2) 
		{
			player.doingmagic=0;//1

			if (entities.player()->anims[ANIM_CAST_END])
			{
				AcquireLastAnim(entities.player());
				ANIM_Set(&entities.player()->animlayer[1],entities.player()->anims[ANIM_CAST_END]);
			}
			
			ARX_FLARES_broken=3;
		}
	}
	

	if (ARX_FLARES_broken==3)
	{
		cur_arm=0;
		cur_mega=0;
		passwall=0;

		if (cur_mr!=3)
			cur_mr=0;

		if (cur_mx!=3)
			cur_mx=0;

		if (cur_rf!=3)
			cur_rf=0;

		if (cur_pom!=3)
			cur_pom=0;

		if (cur_pnux<3)
			cur_pnux=0;

		if (cur_sm<3)
			cur_sm=0;

		cur_bh=0;
		cur_sos=0;

		if (CurrSpellSymbol != 0)
		{
			if (!ARX_SPELLS_AnalyseSPELL())
			{
				if (entities.player()->anims[ANIM_CAST])
				{
					AcquireLastAnim(entities.player());
					ANIM_Set(&entities.player()->animlayer[1],entities.player()->anims[ANIM_CAST]);
				}
			}
		}

		ARX_FLARES_broken=1;

		if (WILLRETURNTOCOMBATMODE)
		{
			player.Interface|=INTER_COMBATMODE;
			player.Interface|=INTER_NO_STRIKE;

			ARX_EQUIPMENT_LaunchPlayerReadyWeapon();
			player.doingmagic=0;
			WILLRETURNTOCOMBATMODE=0;

			TRUE_PLAYER_MOUSELOOK_ON = true;
			bRenderInCursorMode=false;
		}

		if (WILLRETURNTOFREELOOK)
		{
			TRUE_PLAYER_MOUSELOOK_ON = true;
			WILLRETURNTOFREELOOK = 0;
		}

		ARX_SPELLS_ResetRecognition();
	}
	else if (ARX_FLARES_broken==2)
	{
		ARX_SPELLS_Analyse();

		if (!SpellMoves.empty()) 
		 ARX_SPELLS_AnalyseSYMBOL();
	
		ARX_FLARES_broken = 1;
	}
}

/*!
 * Plays the sound of Fizzling spell plus "NO MANA" speech
 */
static void ARX_SPELLS_FizzleNoMana(long num) {
	if(num < 0) {
		return;
	}
	if(spells[num].caster >= 0) {
		spells[num].tolive = 0;
		ARX_SPELLS_Fizzle(num);
	}
}

long CanPayMana(long num, float cost, bool _bSound = true) {
	
	if (num<0) return 0;

	if (spells[num].flags & SPELLCAST_FLAG_NOMANA) return 1;

	if (spells[num].caster==0) 
	{
		if (player.mana<cost)
		{
			ARX_SPELLS_FizzleNoMana(num);

			if(_bSound) {
				ARX_SPEECH_Add(getLocalised("player_cantcast"));
				ARX_SPEECH_AddSpeech(entities.player(), "player_cantcast", ANIM_TALK_NEUTRAL);
			}

			return 0;
		}

		player.mana -= cost;
		return 1;
	}
	else if(ValidIONum(spells[num].caster)) {
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

//-----------------------------------------------------------------------------
// Resets Spell Recognition
void ARX_SPELLS_ResetRecognition() {
	
	for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
		SpellSymbol[i] = RUNE_NONE;
	}
	
	for(size_t i = 0; i < 6; i++) {
		player.SpellToMemorize.iSpellSymbols[i] = RUNE_NONE;
	}
	
	CurrSpellSymbol = 0;
}

// Adds a 2D point to currently drawn spell symbol
void ARX_SPELLS_AddPoint(const Vec2s & pos) {
	plist[CurrPoint] = pos;
	CurrPoint++;
	if(CurrPoint >= MAX_POINTS) {
		CurrPoint = MAX_POINTS - 1;
	}
}

long TemporaryGetSpellTarget(const Vec3f * from) {
	
	float mindist = std::numeric_limits<float>::max();
	long found = 0;
	for(size_t i = 1; i < entities.size(); i++) {
		if(entities[i] && entities[i]->ioflags & IO_NPC) {
			float dist = distSqr(*from, entities[i]->pos);
			if(dist < mindist) {
				found = i;
				mindist = dist;
			}
		}
	}
	
	return found;
}

static void ARX_SPEELS_GetMaxRect(const char *_pcName)
{
	char tcTxt[32];
	int iMinX,iMinY,iMaxX,iMaxY;
	long iSizeX,iSizeY;

	ARX_SPELLS_RequestSymbolDraw3(_pcName,tcTxt);
	ReCenterSequence(tcTxt,iMinX,iMinY,iMaxX,iMaxY);
	iSizeX=iMaxX-iMinX;
	iSizeY=iMaxY-iMinY;
	lMaxSymbolDrawSizeX=std::max(iSizeX,lMaxSymbolDrawSizeX);
	lMaxSymbolDrawSizeY=std::max(iSizeY,lMaxSymbolDrawSizeY);
}
//-----------------------------------------------------------------------------
// Initializes Spell engine (Called once at DANAE startup)
void ARX_SPELLS_Init_Rects() {
	lMaxSymbolDrawSizeX = std::numeric_limits<long>::min();
	lMaxSymbolDrawSizeY = std::numeric_limits<long>::min();

	ARX_SPEELS_GetMaxRect("aam");
	ARX_SPEELS_GetMaxRect("cetrius");
	ARX_SPEELS_GetMaxRect("comunicatum");
	ARX_SPEELS_GetMaxRect("cosum");
	ARX_SPEELS_GetMaxRect("folgora");
	ARX_SPEELS_GetMaxRect("fridd");
	ARX_SPEELS_GetMaxRect("kaom");
	ARX_SPEELS_GetMaxRect("mega");
	ARX_SPEELS_GetMaxRect("morte");
	ARX_SPEELS_GetMaxRect("movis");
	ARX_SPEELS_GetMaxRect("nhi");
	ARX_SPEELS_GetMaxRect("rhaa");
	ARX_SPEELS_GetMaxRect("spacium");
	ARX_SPEELS_GetMaxRect("stregum");
	ARX_SPEELS_GetMaxRect("taar");
	ARX_SPEELS_GetMaxRect("tempus");
	ARX_SPEELS_GetMaxRect("tera");
	ARX_SPEELS_GetMaxRect("vista");
	ARX_SPEELS_GetMaxRect("vitae");
	ARX_SPEELS_GetMaxRect("yok");
	ARX_SPEELS_GetMaxRect("akbaa");
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_Init() {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		spells[i].tolive = 0;
		spells[i].exist = false;
		spells[i].pSpellFx = NULL;
	}
	
	for(size_t i = 0; i < ARRAY_SIZE(allSpells); i++) {
		addSpell(allSpells[i].symbols, allSpells[i].spell, allSpells[i].name);
	}
	
}

// Clears All Spells.
void ARX_SPELLS_ClearAll() {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].exist) {
			spells[i].tolive = 0;
			spells[i].exist = false;
			
			if(spells[i].pSpellFx) {
				delete spells[i].pSpellFx;
				spells[i].pSpellFx = NULL;
			}
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
			spells[i].longinfo = spells[i].longinfo2 = -1;
			spells[i].misc = NULL;
			return i;
		}
	}
	
	return -1;
}

long ARX_SPELLS_GetInstance(Spell typ) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].exist && spells[i].type == typ) {
			return i;
		}
	}
	
	return -1;
}

// Checks for an existing instance of this spelltype
bool ARX_SPELLS_ExistAnyInstance(Spell typ) {
	return (ARX_SPELLS_GetInstance(typ) != -1);
}

long ARX_SPELLS_GetInstanceForThisCaster(Spell typ, long caster) {
	
	for(size_t i = 0; i < MAX_SPELLS; i++) {
		if(spells[i].exist && spells[i].type == typ && spells[i].caster == caster) {
			return i;
		}
	}
	
	return -1;
}

static bool ARX_SPELLS_ExistAnyInstanceForThisCaster(Spell typ, long caster) {
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

PRECAST_STRUCT Precast[MAX_PRECAST];

void ARX_SPELLS_Precast_Reset() {
	for(size_t i = 0; i < MAX_PRECAST; i++) {
		Precast[i].typ = SPELL_NONE;
	}
}

void ARX_SPELLS_Precast_Add(Spell typ, long _level, SpellcastFlags flags, long duration) {
	
	long found = -1;
	
	for(size_t i = 0; i < MAX_PRECAST; i++) {
		if(Precast[i].typ == SPELL_NONE) {
			found = i;
			break;
		}
	}
	
	if(found == -1) {
		for(size_t i = 1; i < MAX_PRECAST; i++) {
			memcpy(&Precast[i - 1], &Precast[i], sizeof(PRECAST_STRUCT));
		}
		
		found = MAX_PRECAST - 1;
	}
	
	Precast[found].typ = typ;
	Precast[found].level = _level;
	Precast[found].launch_time = 0;
	Precast[found].flags = flags;
	Precast[found].duration = duration;
}

unsigned long LAST_PRECAST_TIME=0;
long PrecastCheckCanPayMana(long num, float cost, bool _bSound = true)
{
	if (num<0) return 0;

	if (Precast[num].flags & SPELLCAST_FLAG_NOMANA) return 1;

		if (player.mana>=cost)
		{
			return 1;
		}
	
	ARX_SPELLS_FizzleNoMana(num);

	if(_bSound) {
		ARX_SPEECH_Add(getLocalised("player_cantcast"));
		ARX_SPEECH_AddSpeech(entities.player(), "player_cantcast", ANIM_TALK_NEUTRAL);
	}

	return 0;
}

void ARX_SPELLS_Precast_Launch(long num) {
	
	if (float(arxtime) >= LAST_PRECAST_TIME+1000)
	{
		Spell iNumSpells=Precast[num].typ;
		float cost=ARX_SPELLS_GetManaCost(iNumSpells,-1);

		if(		(iNumSpells != SPELL_NONE)
			&&	(!PrecastCheckCanPayMana(num,cost)	)  )
			return;

		LAST_PRECAST_TIME = (unsigned long)(arxtime);

		if ((Precast[num].typ != SPELL_NONE) && (Precast[num].launch_time==0))
		{
			Precast[num].launch_time = (unsigned long)(arxtime);
			ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FIELD);
		}
	}
}
void ARX_SPELLS_Precast_Check()
{
	for (size_t i = 0; i < MAX_PRECAST; i++)
	{
		if ((Precast[i].typ != SPELL_NONE) && (Precast[i].launch_time>0) &&(float(arxtime) >= Precast[i].launch_time))
		{
			ANIM_USE *ause1 = &entities.player()->animlayer[1];
			
			if (player.Interface & INTER_COMBATMODE)
			{
				WILLRETURNTOCOMBATMODE=1;
				ARX_INTERFACE_Combat_Mode(0);
				bGToggleCombatModeWithKey=false;
				ResetAnim(&entities.player()->animlayer[1]);
				entities.player()->animlayer[1].flags&=~EA_LOOP;
			}

			if ((ause1->cur_anim) && (ause1->cur_anim==entities.player()->anims[ANIM_CAST]))
			{
				if (ause1->ctime>ause1->cur_anim->anims[ause1->altidx_cur]->anim_time-550)
				{
					ARX_SPELLS_Launch(	Precast[i].typ,
										0,
										Precast[i].flags | SPELLCAST_FLAG_LAUNCHPRECAST, 
										Precast[i].level, 
										-1, 
										Precast[i].duration);
					Precast[i].typ = SPELL_NONE;

					for (size_t li=i; li < MAX_PRECAST - 1; li++)
					{
						if (Precast[li + 1].typ != SPELL_NONE)
						{
							memcpy(&Precast[li], &Precast[li + 1], sizeof(PRECAST_STRUCT));
							Precast[li + 1].typ = SPELL_NONE;
						}
					}
				}
			} else {
				ANIM_USE * ause1 = &entities.player()->animlayer[1];
				AcquireLastAnim(entities.player());
				FinishAnim(entities.player(), ause1->cur_anim);
				ANIM_Set(ause1, entities.player()->anims[ANIM_CAST]);	
			}
		}
	}
}

struct TARGETING_SPELL {
	Spell typ;
	long source;
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
		ARX_SPELLS_Launch(t_spell.typ, t_spell.source, t_spell.flags, t_spell.level, io->index(), t_spell.duration);
	}
}

static float ARX_SPELLS_GetManaCost(Spell _lNumSpell,long lNumSpellTab) {
	float Player_Magic_Level;
	Player_Magic_Level = (float) player.Full_Skill_Casting + player.Full_Attribute_Mind;
	Player_Magic_Level= std::max(1.0f,Player_Magic_Level*( 1.0f / 10 ));
	Player_Magic_Level= std::min(10.0f,Player_Magic_Level);

	switch (_lNumSpell) 
	{
	//-----------------------------
	case SPELL_MAGIC_SIGHT:
		return 0.3f;
		break;
	case SPELL_MAGIC_MISSILE:

		if (lNumSpellTab<0)
			return 	Player_Magic_Level;

		return spells[lNumSpellTab].caster_level;
		break;
	case SPELL_IGNIT:
		return 1.f;
		break;
	case SPELL_DOUSE:
		return 1.f;
		break;
	case SPELL_ACTIVATE_PORTAL:
		return 2.f;
		break;
	//-----------------------------
	case SPELL_HEAL:
		return 4.f;
		break;
	case SPELL_DETECT_TRAP:
		return 0.03f;
		break;
	case SPELL_ARMOR:
		return 0.01f;
		break;
	case SPELL_LOWER_ARMOR:
		return 0.01f;
		break;
	case SPELL_HARM:
		return 0.4f;
		break;
	//-----------------------------
	case SPELL_SPEED:            
		return 0.01f;
		break;
	case SPELL_DISPELL_ILLUSION: 
		return 7.f;
		break;
	case SPELL_FIREBALL:

		if (lNumSpellTab<0)
			return 	Player_Magic_Level*3.f;

		return 3.f*spells[lNumSpellTab].caster_level;
		break;
	case SPELL_CREATE_FOOD:      
		return 5.f;
		break;
	case SPELL_ICE_PROJECTILE:   

		if (lNumSpellTab<0)
			return 	Player_Magic_Level*1.5f;

		return 1.5f*spells[lNumSpellTab].caster_level;
		break;
	//----------------------------
	case SPELL_BLESS:            
		return 0.01f;
		break;
	case SPELL_DISPELL_FIELD:    
		return 7.f;
		break;
	case SPELL_FIRE_PROTECTION:  
		return 1.f;
		break;
	case SPELL_TELEKINESIS:      
		return 0.001f;
		break;
	case SPELL_CURSE:            
		return 0.001f;
		break;
	case SPELL_COLD_PROTECTION:  
		return 1.f;
		break;
	//-----------------------------
	case SPELL_RUNE_OF_GUARDING: 
		return 9.f;
		break;
	case SPELL_LEVITATE:         
		return 1.f;
		break;
	case SPELL_CURE_POISON:      
		return 10.f;
		break;
	case SPELL_REPEL_UNDEAD:     
		return 18.f;
		break;
	case SPELL_POISON_PROJECTILE:

		if (lNumSpellTab<0)
			return 	Player_Magic_Level*2.f;

		return 2.f*spells[lNumSpellTab].caster_level;
		break;
	//-----------------------------
	case SPELL_RISE_DEAD:        
		return 12.f;
		break;
	case SPELL_PARALYSE:         

		if (lNumSpellTab<0)
			return 	Player_Magic_Level*3.f;

		return 3.f*spells[lNumSpellTab].caster_level;
		break;
	case SPELL_CREATE_FIELD:     
		return 1.2f;
		break;
	case SPELL_DISARM_TRAP:      
		return 15.f;
		break;
	case SPELL_SLOW_DOWN:        
		return 1.2f;
		break;
	//-----------------------------
	case SPELL_FLYING_EYE:       
		return 4.f;
		break;
	case SPELL_FIRE_FIELD:       
		return 15.f;
		break;
	case SPELL_ICE_FIELD:        
		return 15.f;
		break;
	case SPELL_LIGHTNING_STRIKE: 

		if (lNumSpellTab<0)
			return 	Player_Magic_Level*6.f;

		return 6.f*spells[lNumSpellTab].caster_level;
		break;
	case SPELL_CONFUSE:          

		if (lNumSpellTab<0)
			return 	Player_Magic_Level*0.1f;

		return 0.1f*spells[lNumSpellTab].caster_level;
		break;
	//-----------------------------
	case SPELL_INVISIBILITY:     
		return 3.f;
		break;
	case SPELL_MANA_DRAIN:       
		return 0.4f;
		break;
	case SPELL_EXPLOSION:        
		return 45.f;
		break;
	case SPELL_ENCHANT_WEAPON:   
		return 35.f;
		break;
	case SPELL_LIFE_DRAIN:       
		return 3.f;//0.9f; //0.4f;
		break;
	//-----------------------------
	case SPELL_SUMMON_CREATURE:  

		if (lNumSpellTab<0)
		{
			if(Player_Magic_Level>=9)
			return 	80.f;
		}

		if(spells[lNumSpellTab].caster_level>=9)
			return 80.f;

		return 20.f;
		break;
	case SPELL_NEGATE_MAGIC:     
		return 2.f;
		break;
	case SPELL_INCINERATE:       
		return 40.f;
		break;
	case SPELL_MASS_PARALYSE:    

		if (lNumSpellTab<0)
			return 	Player_Magic_Level*3.f;

		return 3.f*spells[lNumSpellTab].caster_level;
		break;
	//----------------------------
	case SPELL_MASS_LIGHTNING_STRIKE:

		if (lNumSpellTab<0)
			return 	Player_Magic_Level*8.f;

		return 8.f*spells[lNumSpellTab].caster_level;
		break;
	case SPELL_CONTROL_TARGET:       
		return 40.f;
		break;
	case SPELL_FREEZE_TIME:          
		return 60.f;
		break;
	case SPELL_MASS_INCINERATE:      
		return 160.f;
		break;
	case SPELL_FAKE_SUMMON:			

		if (lNumSpellTab<0)
		{
			if(Player_Magic_Level>=9)
			return 	80.f;
		}

		if(spells[lNumSpellTab].caster_level>=9)
			return 80.f;

		return 20.f;
		break;
	//-----------------------------
	case SPELL_TELEPORT:	
		return 10.f;
		break;
	default:
		return 0.f;
	}
}

bool ARX_SPELLS_Launch(Spell typ, long source, SpellcastFlags flagss, long levell, long target, long duration) {
	
	SpellcastFlags flags = flagss;
	long level = levell;

	if ( cur_rf == 3 )
	{ 
		flags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NOMANA;	
		level += 2;
	}

	if ( sp_max ) 
	{ 
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
	
	if ( !source )
	{
		ARX_SPELLS_ResetRecognition();

		if ( player.SpellToMemorize.bSpell )
		{
			CurrSpellSymbol					= 0;
			player.SpellToMemorize.bSpell	= false;
		}

		ARX_PLAYER_ComputePlayerFullStats();

		if ( level == -1 )
		{
			Player_Magic_Level = (float) player.Full_Skill_Casting + player.Full_Attribute_Mind;
			Player_Magic_Level = std::max( 1.0f, Player_Magic_Level * ( 1.0f / 10 ) );
			Player_Magic_Level = std::min( 10.0f, Player_Magic_Level );
		}
		else 
		{
			Player_Magic_Level = static_cast<float>(level);
		}
	}

	arx_assert( !( source && (flags & SPELLCAST_FLAG_PRECAST) ) );


	if ( flags & SPELLCAST_FLAG_PRECAST )
	{
		int l = level;

		if(l <= 0) {
			l = checked_range_cast<int>(Player_Magic_Level);
		}

		SpellcastFlags flgs=flags;
		flgs&=~SPELLCAST_FLAG_PRECAST;
		ARX_SPELLS_Precast_Add( typ, l, flgs, duration);
		return true;
	}

	if ( flags & SPELLCAST_FLAG_NOMANA )
	{
		Player_Magic_Level = static_cast<float>(level);
	}

	static TextureContainer * tc4 = TextureContainer::Load("graph/particles/smoke");


	if ((target < 0)
		&&	( source == 0 )	)
	switch ( typ )
	{
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
			t_spell.source					= source;
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
			t_spell.source					= source;
			t_spell.flags					= flags;
			t_spell.level					= level;
			t_spell.target					= target;
			t_spell.duration				= duration;
			return false;
		}	
		break;
		case SPELL_CONTROL_TARGET:
		{
			long		tcount = 0;

			if ( !ValidIONum( source ) )
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

			if ( tcount == 0 ) 
			{
				ARX_SOUND_PlaySFX( SND_MAGIC_FIZZLE, &cpos );
				return false;
			}

			ARX_SOUND_PlaySpeech("player_follower_attack");
				LOOKING_FOR_SPELL_TARGET_TIME	= (unsigned long)(arxtime);
			LOOKING_FOR_SPELL_TARGET		= 1;
			t_spell.typ						= typ;
			t_spell.source					= source;
			t_spell.flags					= flags;
			t_spell.level					= level;
			t_spell.target					= target;
			t_spell.duration				= duration;
			return false;
		}
		break;
		default: break;
	}

	if ( source == 0 )
	{
		ARX_SPELLS_CancelSpellTarget();
	}


	// Try to create a new spell instance
	long i = ARX_SPELLS_GetFree();

	if ( i < 0 )
	{
		return false;
	}
	
	if(ValidIONum(source) && spellicons[typ].bAudibleAtStart) {
		ARX_NPC_SpawnAudibleSound(&entities[source]->pos, entities[source]);
	}
	
	spells[i].caster = source;	// Caster...
	spells[i].target = target;	// No target if <0

	if ( target < 0 )
		spells[i].target = TemporaryGetSpellTarget( &entities[spells[i].caster]->pos );

	// Create hand position if a hand is defined
	if ( spells[i].caster == 0 ) 
	{
		spells[i].hand_group = entities[spells[i].caster]->obj->fastaccess.primary_attach;
	}
	else 
	{
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
		spells[i].caster_level = level < 1 ? 1 : level > 10 ? 10 : static_cast<float>(level);
		spells[i].caster_pos = entities[source]->pos;
	}

	if(flags & SPELLCAST_FLAG_LAUNCHPRECAST) {
		spells[i].caster_level = static_cast<float>(level);
	}

	// Checks target TODO if ( target < 0 ) is already handled above!
	if (target<0) // no target... targeted by sight
	{
		if (source==0) // no target... player spell targeted by sight
		{
			spells[i].target_pos.x=player.pos.x-EEsin(radians(player.angle.b))*60.f;
			spells[i].target_pos.y=player.pos.y+EEsin(radians(player.angle.a))*60.f;
			spells[i].target_pos.z=player.pos.z+EEcos(radians(player.angle.b))*60.f;
		}
		else
		{
			// TODO entities[target] with target < 0 ??? - uh oh!
			spells[i].target_pos.x=entities[target]->pos.x-EEsin(radians(entities[target]->angle.b))*60.f;
			spells[i].target_pos.y=entities[target]->pos.y-120.f;
			spells[i].target_pos.z=entities[target]->pos.z+EEcos(radians(entities[target]->angle.b))*60.f;
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
	
	
	// Check spell-specific preconditions
	switch(typ) {
		
		case SPELL_MAGIC_SIGHT: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster)) {
				return false;
			}
			break;
		}
		
		case SPELL_HEAL: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster)) {
				return false;
			}
			break;
		}
		
		case SPELL_BLESS: {
			if(ARX_SPELLS_ExistAnyInstance(typ)) {
				return false;
			}
			break;
		}
		
		case SPELL_TELEKINESIS: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster)) {
				return false;
			}
			break;
		}
		
		case SPELL_FLYING_EYE: {
			if(eyeball.exist) {
				return false;
			}
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster)) {
				return false;
			}
			break;
		}
		
		case SPELL_INVISIBILITY: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster)) {
				return false;
			}
			break;
		}
		
		case SPELL_MANA_DRAIN: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster)) {
				return false;
			}
			break;
		}
		
		case SPELL_LIFE_DRAIN: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster)) {
				return false;
			}
			break;
		}
		
		case SPELL_CONTROL_TARGET: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster)) {
				return false;
			}
			break;
		}
		
		case SPELL_FREEZE_TIME: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster)) {
				return false;
			}
			break;
		}
		
		case SPELL_TELEPORT: {
			if(ARX_SPELLS_ExistAnyInstanceForThisCaster(typ, spells[i].caster)) {
				return false;
			}
			break;
		}
		
		default: break; // no preconditions to check
	}
	
	if(!CanPayMana(i, ARX_SPELLS_GetManaCost(typ, i))) {
		return false;
	}
	
	if(!GLOBAL_MAGIC_MODE) {
		return No_MagicAllowed();
	}
	
	bool notifyAll = true;
	
	switch(typ) {
		
		case SPELL_NONE: return true;
		
		// level 1 spells
		
		case SPELL_MAGIC_SIGHT: {
			
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
			
			break;
		}
		
		case SPELL_MAGIC_MISSILE: {
			
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
			
			break;
		}
		
		case SPELL_IGNIT: {
			
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
			
			long id = GetFreeDynLight();
			if(id != -1) {
				DynLight[id].exist     = 1;
				DynLight[id].intensity = 1.8f;
				DynLight[id].fallend   = 450.f;
				DynLight[id].fallstart = 380.f;
				DynLight[id].rgb       = Color3f(1.f, 0.75f, 0.5f);
				DynLight[id].pos       = target;
				DynLight[id].duration  = 300;
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
			
			break;
		}
		
		case SPELL_DOUSE: {
			
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
			
			if(CURRENT_TORCH
			   && closerThan(target, player.pos, effect->GetPerimetre())) {
				ARX_PLAYER_ClickedOnTorch(CURRENT_TORCH);
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
			
			break;
		}
		
		case SPELL_ACTIVATE_PORTAL: {
			
			ARX_SOUND_PlayInterface(SND_SPELL_ACTIVATE_PORTAL);
			spells[i].exist = true;
			spells[i].tolive = 20;
			
			break;
		}
		
		// level 2 spells
		
		case SPELL_HEAL: {
			
			if(!(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
				ARX_SOUND_PlaySFX(SND_SPELL_HEALING, &spells[i].caster_pos);
			}
			
			spells[i].exist = true;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.4f * spells[i].caster_level;
			spells[i].tolive = (duration > -1) ? duration : 3500;
			
			CHeal * effect = new CHeal();
			effect->spellinstance = i;
			effect->Create();
			effect->SetDuration(spells[i].tolive);
			
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			break;
		}
		
		case SPELL_DETECT_TRAP: {
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			if(spells[i].caster == 0 && !(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
				ARX_SOUND_PlayInterface(SND_SPELL_DETECT_TRAP);
			}
			
			spells[i].snd_loop = SND_SPELL_DETECT_TRAP_LOOP;
			if(spells[i].caster == 0 && !(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
				ARX_SOUND_PlaySFX(spells[i].snd_loop, &spells[i].caster_pos, 1.f,
				                  ARX_SOUND_PLAY_LOOPED);
			}
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 60000;
			spells[i].fManaCostPerSecond = 0.4f;
			spells[i].bDuration = true;
			
			break;
		}
		
		case SPELL_ARMOR: {
			
			long idx = ARX_SPELLS_GetSpellOn(entities[spells[i].target], typ);
			if(idx >= 0) {
				spells[idx].tolive = 0;
			}
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR,
			                                                   spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			if(spells[i].caster == 0) {
				spells[i].target = spells[i].caster;
			}
			
			if(!(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
				ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_START, &spells[i].target_pos);
			}
			
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_LOOP,
			                                       &spells[i].caster_pos, 1.f,
			                                       ARX_SOUND_PLAY_LOOPED);
			
			spells[i].exist = true;
			spells[i].tolive = (spells[i].caster == 0) ? 20000000 : 20000;
			if(duration > -1) {
				spells[i].tolive = duration;
			}
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.2f * spells[i].caster_level;
				
			CArmor * effect = new CArmor();
			effect->spellinstance = i;
			effect->Create(spells[i].tolive);
			
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			break;
		}
		
		case SPELL_LOWER_ARMOR: {
			
			long idx = ARX_SPELLS_GetSpellOn(entities[spells[i].target], typ);
			if(idx >= 0) {
				spells[idx].tolive = 0;
			}
			
			if(spells[i].target < 0) {
				return false;
			}
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR,
			                                                   spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			if(!(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
				ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_START, &spells[i].caster_pos);
			}
			
			spells[i].exist = true;
			spells[i].tolive = (spells[i].caster == 0) ? 20000000 : 20000;
			if(duration > -1) {
				spells[i].tolive = duration;
			}
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.2f * spells[i].caster_level;
			
			CLowerArmor * effect = new CLowerArmor();
			effect->spellinstance = i;
			effect->Create(spells[i].tolive);
			
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			break;
		}
		
		case SPELL_HARM: {
			
			if(!(spells[i].flags & SPELLCAST_FLAG_NOSOUND)) {
				ARX_SOUND_PlaySFX(SND_SPELL_HARM, &spells[i].caster_pos);
			}
			
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD,
			                                       &spells[i].caster_pos, 1.f,
			                                       ARX_SOUND_PLAY_LOOPED);
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LIFE_DRAIN,
			                                                   spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_MANA_DRAIN,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			spells[i].exist = true;
			spells[i].tolive = (duration >-1) ? duration : 6000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.4f;

			spells[i].longinfo = ARX_DAMAGES_GetFree();
			if(spells[i].longinfo != -1) {
				damages[spells[i].longinfo].radius = 150.f;
				damages[spells[i].longinfo].damages = 4.f;
				damages[spells[i].longinfo].area = DAMAGE_FULL;
				damages[spells[i].longinfo].duration = 100000000;
				damages[spells[i].longinfo].source = spells[i].caster;
				damages[spells[i].longinfo].flags = DAMAGE_FLAG_DONT_HURT_SOURCE
				                                    | DAMAGE_FLAG_FOLLOW_SOURCE
				                                    | DAMAGE_FLAG_ADD_VISUAL_FX;
				damages[spells[i].longinfo].type = DAMAGE_TYPE_FAKEFIRE
				                                   | DAMAGE_TYPE_MAGICAL;
				damages[spells[i].longinfo].exist = true;
			}
			
			spells[i].longinfo2 = GetFreeDynLight();
			if(spells[i].longinfo2 != -1) {
				long id = spells[i].longinfo2;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 2.3f;
				DynLight[id].fallend = 700.f;
				DynLight[id].fallstart = 500.f;
				DynLight[id].rgb = Color3f::red;
				DynLight[id].pos = spells[i].caster_pos;
			}
			
			break;
		}
		
		// level 3 spells
		
		case SPELL_SPEED: {
			
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 2.f;
			
			ARX_SOUND_PlaySFX(SND_SPELL_SPEED_START, &spells[i].caster_pos);
			
			if(spells[i].caster == 0) {
				spells[i].target = spells[i].caster;
				spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_SPEED_LOOP,
				                                       &spells[i].caster_pos, 1.f,
				                                       ARX_SOUND_PLAY_LOOPED);
			}
			
			spells[i].exist = true;
			spells[i].tolive = (spells[i].caster == 0) ? 200000000 : 20000;
			if(duration > -1) {
				spells[i].tolive = duration;
			}
			
			CSpeed * effect = new CSpeed();
			effect->spellinstance = i;
			effect->Create(spells[i].target, spells[i].tolive);
			
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			if(spells[i].caster >= 0 && spells[i].target < long(entities.size())) {
				Entity * t = entities[spells[i].target];
				if(t) {
					t->speed_modif += spells[i].caster_level * 0.1f;
				}
			}
			
			break;
		}
		
		case SPELL_DISPELL_ILLUSION: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_DISPELL_ILLUSION);
			spells[i].exist = true;
			spells[i].tolive = 1000;
			
			for(size_t n = 0; n < MAX_SPELLS; n++) {
				
				if(!spells[n].exist || spells[n].target == spells[i].caster) {
					continue;
				}
				
				if(spells[n].caster_level > spells[i].caster_level) {
					continue;
				}
				
				if(spells[n].type == SPELL_INVISIBILITY) {
					if(ValidIONum(spells[n].target) && ValidIONum(spells[i].caster)) {
						if(closerThan(entities[spells[n].target]->pos,
						   entities[spells[i].caster]->pos, 1000.f)) {
							spells[n].tolive = 0;
						}
					}
				}
			}
			
			break;
		}
		
		case SPELL_FIREBALL: {
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 20000; // TODO probbaly never read
			
			CFireBall * effect = new CFireBall();
			effect->spellinstance = i;
			
			if(spells[i].caster != 0) {
				spells[i].hand_group = -1;
			}
			
			Vec3f target;
			if(spells[i].hand_group >= 0) {
				target = spells[i].hand_pos;
			} else {
				target = spells[i].caster_pos;
				if(ValidIONum(spells[i].caster)) {
					Entity * c = entities[spells[i].caster];
					if(c->ioflags & IO_NPC) {
						target.x -= EEsin(radians(c->angle.b)) * 30.f;
						target.y -= 80.f;
						target.z += EEcos(radians(c->angle.b)) * 30.f;
					}
				}
			}
			
			effect->SetDuration(6000ul);
			
			float anglea = 0, angleb;
			if(spells[i].caster == 0) {
				anglea = player.angle.a, angleb = player.angle.b;
			} else {
				
				Vec3f start = entities[spells[i].caster]->pos;
				if(ValidIONum(spells[i].caster)
				   && (entities[spells[i].caster]->ioflags & IO_NPC)) {
					start.y -= 80.f;
				}
				
				Entity * _io = entities[spells[i].caster];
				if(ValidIONum(_io->targetinfo)) {
					const Vec3f & end = entities[_io->targetinfo]->pos;
					float d = dist(Vec2f(end.x, end.z), Vec2f(start.x, start.z));
					anglea = degrees(getAngle(start.y, start.z, end.y, end.z + d));
				}
				
				angleb = entities[spells[i].caster]->angle.b;
			}
			
			effect->Create(target, MAKEANGLE(angleb), anglea, spells[i].caster_level);
			
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_LAUNCH, &spells[i].caster_pos);
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND,
			                                       &spells[i].caster_pos, 1.f,
			                                       ARX_SOUND_PLAY_LOOPED);
			
			break;
		}
		
		case SPELL_CREATE_FOOD: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FOOD, &spells[i].caster_pos);
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 3500;
			
			if(spells[i].caster == 0 || spells[i].target == 0) {
				player.hunger = 100;
			}
			
			CCreateFood * effect = new CCreateFood();
			effect->spellinstance = i;
			effect->Create();
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			break;
		}
		
		case SPELL_ICE_PROJECTILE: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_ICE_PROJECTILE_LAUNCH, &spells[i].caster_pos);
			spells[i].exist = true;
			spells[i].tolive = 4200;
			
			CIceProjectile * effect = new CIceProjectile();
			effect->spellinstance = i;
			
			Vec3f target;
			float angleb;
			if(spells[i].caster == 0) {
				target = player.pos + Vec3f(0.f, 160.f, 0.f);
				angleb = player.angle.b;
			} else {
				target = entities[spells[i].caster]->pos;
				angleb = entities[spells[i].caster]->angle.b;
			}
			angleb = MAKEANGLE(angleb);
			target.x -= EEsin(radians(angleb)) * 150.0f;
			target.z += EEcos(radians(angleb)) * 150.0f;
			effect->Create(target, angleb, spells[i].caster_level);
			
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			break;
		}
		
		// level 4 spells
		
		case SPELL_BLESS: {
			
			if(spells[i].caster == 0) {
				spells[i].target = 0;
			}
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].target);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_BLESS);
			spells[i].exist = true;
			// TODO this tolive value is probably never read
			spells[i].tolive = (duration > -1) ? duration : 2000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.5f * spells[i].caster_level * 0.6666f;
			
			CBless * effect = new CBless();
			effect->spellinstance = i;
			Vec3f target = entities[spells[i].caster]->pos;
			effect->Create(target, MAKEANGLE(player.angle.b));
			effect->SetDuration(20000);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			break;
		}
		
		case SPELL_DISPELL_FIELD: {
			
			spells[i].tolive = 10;
			
			long valid = 0, dispelled = 0;

			for(size_t n = 0; n < MAX_SPELLS; n++) {
				
				if(!spells[n].exist || !spells[n].pSpellFx) {
					continue;
				}
				
				bool cancel = false;
				Vec3f pos;
				
				switch(spells[n].type) {
					
					case SPELL_CREATE_FIELD: {
						if(spells[i].caster != 0 || spells[n].caster == 0) {
							pos = static_cast<CCreateField *>(spells[n].pSpellFx)->eSrc;
							cancel = true;
						}
						break;
					}
					
					case SPELL_FIRE_FIELD: {
						pos = static_cast<CFireField *>(spells[n].pSpellFx)->pos;
						cancel = true;
						break;
					}
					
					case SPELL_ICE_FIELD: {
						pos = static_cast<CIceField *>(spells[n].pSpellFx)->eSrc;
						cancel = true;
						break;
					}
					
					default: break;
				}
				
				Entity * caster = entities[spells[i].caster];
				if(cancel && closerThan(pos, caster->pos, 400.f)) {
					valid++;
					if(spells[n].caster_level <= spells[i].caster_level) {
						spells[n].tolive = 0;
						dispelled++;
					}
				}
			}
			
			if(valid > dispelled) {
				// Some fileds could not be dispelled
				ARX_SPEECH_AddSpeech(entities.player(), "player_not_skilled_enough",
				                     ANIM_TALK_NEUTRAL, ARX_SPEECH_FLAG_NOTEXT);
			}
			
			if(dispelled > 0) {
				ARX_SOUND_PlaySFX(SND_SPELL_DISPELL_FIELD);
			} else {
				ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &spells[i].caster_pos);
			}
			
			break;
		}
		
		case SPELL_FIRE_PROTECTION: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION);
			
			long idx = ARX_SPELLS_GetSpellOn(entities[spells[i].target], typ);
			if(idx >= 0) {
				spells[idx].tolive = 0;
			}
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR,
			                                                   spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 2000000; // TODO should respect user-supplied duration
			
			if(spells[i].caster == 0) {
				spells[i].target = 0;
			}
			
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.f;
			
			CFireProtection * effect = new CFireProtection();
			effect->spellinstance = i;
			effect->Create(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			break;
		}
		
		case SPELL_COLD_PROTECTION: {
			
			long idx = ARX_SPELLS_GetSpellOn(entities[spells[i].target], typ);
			if(idx >= 0) {
				spells[idx].tolive = 0;
			}
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR,
			                                                   spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_START);
			
			if(spells[i].caster == 0) {
				spells[i].target = 0;
			}
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = (duration > -1) ? duration : 2000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.f;
			
			CColdProtection * effect = new CColdProtection();
			effect->spellinstance=i;
			effect->Create(spells[i].tolive, spells[i].target);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_LOOP,
			                                       &spells[i].caster_pos, 1.f,
			                                       ARX_SOUND_PLAY_LOOPED);
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			break;
		}
		
		case SPELL_TELEKINESIS: {
			
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 6000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.9f;
			
			if(spells[i].caster == 0) {
				Project.telekinesis = 1;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_START, &spells[i].caster_pos);
			
			break;
		}
		
		case SPELL_CURSE: {
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].target);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_CURSE, &spells[i].caster_pos);
			
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 2000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.5f * spells[i].caster_level;
			
			CCurse * effect = new CCurse();
			effect->spellinstance = i;
			
			Vec3f target = spells[i].target_pos;
			if(spells[i].target == 0) {
				target.y -= 200.f;
			} else if(spells[i].target > 0 && entities[spells[i].target]) {
				target.y += entities[spells[i].target]->physics.cyl.height - 50.f;
			}
			
			effect->Create(target, MAKEANGLE(player.angle.b));
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			break;
		}
		
		// level 5 spells
		
		case SPELL_RUNE_OF_GUARDING: {
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_RUNE_OF_GUARDING);
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 99999999;
			
			CRuneOfGuarding * effect = new CRuneOfGuarding();
			effect->spellinstance = i;
			effect->Create(entities[spells[i].caster]->pos, 0);
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			break;
		}
		
		case SPELL_LEVITATE: {
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_START);
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 2000000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.f;
			
			CLevitate * effect = new CLevitate();
			effect->spellinstance = i;
			
			Vec3f target;
			if(spells[i].caster == 0 || spells[i].target == 0) {
				target = player.pos + Vec3f(0.f, 150.f, 0.f);
				spells[i].target = 0;
				spells[i].tolive = 200000000;
				player.levitate = 1;
			} else {
				target = entities[spells[i].target]->pos;
			}
			
			effect->Create(16, 50.f, 100.f, 80.f, &target, spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			break;
		}
		
		case SPELL_CURE_POISON: {
			
			if(spells[i].caster == 0) {
				spells[i].target = 0;
			}
			
			float cure = spells[i].caster_level * 10;
			if(spells[i].target == 0) {
				player.poison -= std::min(player.poison, cure);
				ARX_SOUND_PlaySFX(SND_SPELL_CURE_POISON);
			} else if (ValidIONum(spells[i].target)) {
				Entity * io = entities[spells[i].target];
				if(io->ioflags & IO_NPC) {
					io->_npcdata->poisonned -= std::min(io->_npcdata->poisonned, cure);
				}
				ARX_SOUND_PlaySFX(SND_SPELL_CURE_POISON, &io->pos);
			}
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 3500;
			
			CCurePoison * effect = new CCurePoison();
			effect->spellinstance = i;
			effect->Create();
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			break;
		}
		
		case SPELL_REPEL_UNDEAD: {
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_REPEL_UNDEAD, spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			if(spells[i].caster == 0) {
				spells[i].target = 0;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_REPEL_UNDEAD);
			if(spells[i].target == 0) {
				spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_REPEL_UNDEAD_LOOP,
				                                       &spells[i].caster_pos, 1.f,
				                                       ARX_SOUND_PLAY_LOOPED);
			}
			
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 20000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.f;
			
			CRepelUndead * effect = new CRepelUndead();
			effect->spellinstance = i;
			effect->Create(player.pos, MAKEANGLE(player.angle.b));
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			break;
		}
		
		case SPELL_POISON_PROJECTILE: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_POISON_PROJECTILE_LAUNCH,
			                  &spells[i].caster_pos);
			
			spells[i].exist = true;
			spells[i].tolive = 900000000; // TODO probably never read
			
			long level = std::max(long(spells[i].caster_level), 1l);
			CMultiPoisonProjectile * effect = new CMultiPoisonProjectile(level);
			effect->spellinstance = i;
			effect->SetDuration(8000ul);
			float ang;
			if(spells[i].caster == 0) {
				ang = player.angle.b;
			} else {
				ang = entities[spells[i].caster]->angle.b;
			}
			effect->Create(Vec3f::ZERO, MAKEANGLE(ang));
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			break;
		}
		
		// level 6 spells
		
		case SPELL_RISE_DEAD: {
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			float beta;
			Vec3f target;
			bool displace = true;
			if(spells[i].caster == 0) {
				target = player.basePosition();
				beta = MAKEANGLE(player.angle.b);
			} else {
				target = entities[spells[i].caster]->pos;
				beta = MAKEANGLE(entities[spells[i].caster]->angle.b);
				displace = (entities[spells[i].caster]->ioflags & IO_NPC) == IO_NPC;
			}
			if(displace) {
				target.x -= EEsin(radians(beta)) * 300.f;
				target.z += EEcos(radians(beta)) * 300.f;
			}
			if(!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target)) {
				ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
				return false;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_RAISE_DEAD, &spells[i].caster_pos);
			
			spells[i].target_pos = target;
			spells[i].exist = true;
			// TODO this tolive value is probably never read
			spells[i].tolive = (duration > -1) ? duration : 2000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.2f;
			spells[i].longinfo = -1;
			
			CRiseDead * effect = new CRiseDead();
			effect->spellinstance = i;
			effect->Create(target, beta);
			effect->SetDuration(2000, 500, 1800);
			effect->SetColorBorder(0.5, 0.5, 0.5);
			effect->SetColorRays1(0.5, 0.5, 0.5);
			effect->SetColorRays2(1, 0, 0);
			
			if(effect->lLightId == -1) {
				effect->lLightId = GetFreeDynLight();
			}
			if(effect->lLightId != -1) {
				long id = effect->lLightId;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 1.3f;
				DynLight[id].fallend = 450.f;
				DynLight[id].fallstart = 380.f;
				DynLight[id].rgb = Color3f::black;
				DynLight[id].pos = target - Vec3f(0.f, 100.f, 0.f);
				DynLight[id].duration = 200;
				DynLight[id].time_creation = (unsigned long)(arxtime);
			}
			
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			break;
		}
		
		case SPELL_PARALYSE: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE, &spells[i].caster_pos);
			
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 5000;
			
			float resist_magic = 0.f;
			if(spells[i].target == 0 && spells[i].caster_level <= player.level) {
				resist_magic = player.resist_magic;
			} else if(entities[spells[i].target]->ioflags & IO_NPC) {
				resist_magic = entities[spells[i].target]->_npcdata->resist_magic;
			}
			if(rnd() * 100.f < resist_magic) {
				float mul = max(0.5f, 1.f - (resist_magic * 0.005f));
				spells[i].tolive = long(spells[i].tolive * mul);
			}
			
			entities[spells[i].target]->ioflags |= IO_FREEZESCRIPT;
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			ARX_NPC_Kill_Spell_Launch(entities[spells[i].target]);
			
			break;
		}
		
		case SPELL_CREATE_FIELD: {
			
			spells[i].exist = true;
			
			unsigned long start = (unsigned long)(arxtime);
			if(flags & SPELLCAST_FLAG_RESTORE) {
				start -= std::min(start, 4000ul);
			}
			spells[i].lastupdate = spells[i].timcreation = start;
			
			spells[i].tolive = (duration > -1) ? duration : 800000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.2f;
			
			Vec3f target;
			float beta;
			bool displace = false;
			if(spells[i].caster == 0) {
				target = entities.player()->pos;
				beta = player.angle.b;
				displace = true;
			} else {
				if(ValidIONum(spells[i].caster)) {
					Entity * io = entities[spells[i].caster];
					target = io->pos;
					beta = io->angle.b;
					displace = (io->ioflags & IO_NPC) == IO_NPC;
				} else {
					ARX_DEAD_CODE();
				}
			}
			if(displace) {
				target.x -= EEsin(radians(MAKEANGLE(beta))) * 250.f;
				target.z += EEcos(radians(MAKEANGLE(beta))) * 250.f;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FIELD, &target);
			
			CCreateField * effect  = new CCreateField();
			effect->spellinstance = i;
			
			res::path cls = "graph/obj3d/interactive/fix_inter/blue_cube/blue_cube";
			Entity * io = AddFix(cls, -1, IO_IMMEDIATELOAD);
			if(io) {
				
				ARX_INTERACTIVE_HideGore(io);
				RestoreInitialIOStatusOfIO(io);
				spells[i].longinfo = io->index();
				io->scriptload = 1;
				io->ioflags |= IO_NOSAVE | IO_FIELD;
				io->initpos = io->pos = target;
				SendInitScriptEvent(io);
				
				effect->Create(target, 0);
				effect->SetDuration(spells[i].tolive);
				effect->lLightId = GetFreeDynLight();
				
				if(effect->lLightId != -1) {
					long id = effect->lLightId;
					DynLight[id].exist = 1;
					DynLight[id].intensity = 0.7f + 2.3f;
					DynLight[id].fallend = 500.f;
					DynLight[id].fallstart = 400.f;
					DynLight[id].rgb = Color3f(0.8f, 0.0f, 1.0f);
					DynLight[id].pos = effect->eSrc - Vec3f(0.f, 150.f, 0.f);
				}
				
				spells[i].pSpellFx = effect;
				spells[i].tolive = effect->GetDuration();
				
				if(flags & SPELLCAST_FLAG_RESTORE) {
					effect->Update(4000);
				}
				
			} else {
				spells[i].tolive = 0;
			}
			
			break;
		}
		
		case SPELL_DISARM_TRAP: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_DISARM_TRAP);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 1;
			
			CDisarmTrap * effect = new CDisarmTrap();
			effect->spellinstance = i;
			
			effect->Create(player.pos, MAKEANGLE(player.angle.b));
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			EERIE_SPHERE sphere;
			sphere.origin = player.pos;
			sphere.radius = 400.f;
			
			for(size_t n = 0; n < MAX_SPELLS; n++) {
				
				if(!spells[n].exist || spells[n].type != SPELL_RUNE_OF_GUARDING) {
					continue;
				}
				
				if(!spells[n].pSpellFx) {
					continue;
				}
				
				CSpellFx * effect = spells[n].pSpellFx;
				if(sphere.contains(static_cast<CRuneOfGuarding *>(effect)->eSrc)) {
					spells[n].caster_level -= spells[i].caster_level;
					if(spells[n].caster_level <= 0) {
						spells[n].tolive = 0;
					}
				}
			}
			
			break;
		}
		
		case SPELL_SLOW_DOWN: {
			
			long target = spells[i].target;
			
			Entity * io = entities[target];
			for(int il = 0; il < io->nb_spells_on; il++) {
				if(spells[io->spells_on[il]].type == SPELL_SLOW_DOWN) {
					spells[i].exist = false;
					return false;
				}
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_SLOW_DOWN, &spells[i].caster_pos);
			
			spells[i].exist = true;
			spells[i].tolive = (spells[i].caster == 0) ? 10000000 : 10000;
			if(duration > -1) {
				spells[i].tolive=duration;
			}
			spells[i].pSpellFx = NULL;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.2f;
			
			CSlowDown * effect = new CSlowDown();
			effect->spellinstance = i;
			effect->Create(spells[i].target_pos, MAKEANGLE(player.angle.b));
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SPELLS_AddSpellOn(target, i);
			
			if(ValidIONum(target)) {
				entities[target]->speed_modif -= spells[i].caster_level * 0.05f;
			}
			
			break;
		}
		
		// level 7 spells
		
		case SPELL_FLYING_EYE: {
			
			if(spells[i].caster == 0) {
				spells[i].target = 0;
			}
			
			if(spells[i].target != 0) {
				return false;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_EYEBALL_IN);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 1000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 3.2f;
			eyeball.exist = 1;
			float angleb = MAKEANGLE(player.angle.b);
			eyeball.pos.x = player.pos.x - EEsin(radians(angleb)) * 200.f;
			eyeball.pos.y = player.pos.y + 50.f;
			eyeball.pos.z = player.pos.z + EEcos(radians(angleb)) * 200.f;
			eyeball.angle = player.angle;
			
			for(long n = 0; n < 12; n++) {
				
				PARTICLE_DEF * pd = createParticle();
				if(!pd) {
					continue;
				}
				
				pd->ov = eyeball.pos + randomVec(-5.f, 5.f);
				pd->move = randomVec(-2.f, 2.f);
				pd->siz = 28.f;
				pd->tolive = Random::get(2000, 6000);
				pd->scale = Vec3f::repeat(12.f);
				pd->tc = tc4;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION
				              | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
			TRUE_PLAYER_MOUSELOOK_ON = true;
			SLID_START = float(arxtime);
			bOldLookToggle = config.input.mouseLookToggle;
			config.input.mouseLookToggle = true;
			
			break;
		}
		
		case SPELL_FIRE_FIELD: {
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_FIELD_START);
			
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 100000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 2.8f;
			spells[i].longinfo2 = -1;
			
			CFireField * effect = new CFireField();
			effect->spellinstance = i;
			
			Vec3f target;
			float beta;
			float displace = false;
			if(spells[i].caster == 0) {
				target = player.basePosition();
				beta = player.angle.b;
				displace = true;
			} else {
				if(ValidIONum(spells[i].caster)) {
					Entity * io = entities[spells[i].caster];
					target = io->pos;
					beta = io->angle.b;
					displace = (io->ioflags & IO_NPC);
				} else {
					ARX_DEAD_CODE();
				}
			}
			if(displace) {
				target.x -= EEsin(radians(MAKEANGLE(beta))) * 250.f;
				target.z += EEcos(radians(MAKEANGLE(beta))) * 250.f;
			}
			
			spells[i].longinfo = ARX_DAMAGES_GetFree();
			if(spells[i].longinfo != -1) {
				damages[spells[i].longinfo].radius = 150.f;
				damages[spells[i].longinfo].damages = 10.f;
				damages[spells[i].longinfo].area = DAMAGE_FULL;
				damages[spells[i].longinfo].duration = 100000000;
				damages[spells[i].longinfo].source = spells[i].caster;
				damages[spells[i].longinfo].flags = 0;
				damages[spells[i].longinfo].type = DAMAGE_TYPE_MAGICAL
				                                   | DAMAGE_TYPE_FIRE
				                                   | DAMAGE_TYPE_FIELD;
				damages[spells[i].longinfo].exist = true;
				damages[spells[i].longinfo].pos = target;
			}
			
			effect->Create(200.f, &target, spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_FIELD_LOOP,
			                                       &target, 1.f,
			                                       ARX_SOUND_PLAY_LOOPED);
			
			break;
		}
		
		case SPELL_ICE_FIELD: {
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(typ, spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
				
			ARX_SOUND_PlaySFX(SND_SPELL_ICE_FIELD);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = (duration > -1) ? duration : 100000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 2.8f;
			spells[i].longinfo2 =-1;
			
			CIceField * effect = new CIceField();
			effect->spellinstance = i;
			
			Vec3f target;
			float beta;
			float displace = false;
			if(spells[i].caster == 0) {
				target = player.basePosition();
				beta = player.angle.b;
				displace = true;
			} else {
				if(ValidIONum(spells[i].caster)) {
					Entity * io = entities[spells[i].caster];
					target = io->pos;
					beta = io->angle.b;
					displace = (io->ioflags & IO_NPC);
				} else {
					ARX_DEAD_CODE();
				}
			}
			if(displace) {
				target.x -= EEsin(radians(MAKEANGLE(beta))) * 250.f;
				target.z += EEcos(radians(MAKEANGLE(beta))) * 250.f;
			}
			
			spells[i].longinfo = ARX_DAMAGES_GetFree();
			if(spells[i].longinfo != -1) {
				damages[spells[i].longinfo].radius = 150.f;
				damages[spells[i].longinfo].damages = 10.f;
				damages[spells[i].longinfo].area = DAMAGE_FULL;
				damages[spells[i].longinfo].duration = 100000000;
				damages[spells[i].longinfo].source = spells[i].caster;
				damages[spells[i].longinfo].flags = 0;
				damages[spells[i].longinfo].type = DAMAGE_TYPE_MAGICAL
				                                   | DAMAGE_TYPE_COLD
				                                   | DAMAGE_TYPE_FIELD;
				damages[spells[i].longinfo].exist = true;
				damages[spells[i].longinfo].pos = target;
			}
			
			effect->Create(target, MAKEANGLE(player.angle.b));
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			break;
		}
		
		case SPELL_LIGHTNING_STRIKE: {
			
			spells[i].exist = true;
			
			CLightning * effect = new CLightning();
			effect->spellinstance = i;
			Vec3f target(0.f, 0.f, -500.f);
			effect->Create(Vec3f::ZERO, target, MAKEANGLE(player.angle.b));
			effect->SetDuration(long(500 * spells[i].caster_level));
			effect->lSrc = 0;
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_START, &spells[i].caster_pos);
			
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_LOOP,
			                                       &spells[i].caster_pos, 1.f,
			                                       ARX_SOUND_PLAY_LOOPED);
			
			break;
		}
		
		case SPELL_CONFUSE: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_CONFUSE);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.5f;
			if(duration > -1) {
				spells[i].tolive = duration;
			} else {
				// TODO what then?
			}
			
			CConfuse * effect = new CConfuse();
			effect->spellinstance = i;
			effect->Create(player.pos, MAKEANGLE(player.angle.b));
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			notifyAll = false; // TODO inconsistent use of the SM_SPELLCAST event
			
			break;
		}
		
		// level 8 spells
		
		case SPELL_INVISIBILITY: {
			
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 6000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 3.f;
			
			if(spells[i].caster == 0) {
				spells[i].target = 0;
			}

			entities[spells[i].target]->gameFlags |= GFLAG_INVISIBILITY;
			entities[spells[i].target]->invisibility = 0.f;
			
			ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_START, &spells[i].caster_pos);
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			break;
		}
		
		case SPELL_MANA_DRAIN: {
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LIFE_DRAIN,
			                                                   spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_HARM,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 6000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 2.f;
			
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD,
			                                       &spells[i].caster_pos, 1.2f,
			                                       ARX_SOUND_PLAY_LOOPED);
			
			spells[i].longinfo = ARX_DAMAGES_GetFree();
			if(spells[i].longinfo != -1) {
				damages[spells[i].longinfo].radius = 150.f;
				damages[spells[i].longinfo].damages = 8.f;
				damages[spells[i].longinfo].area = DAMAGE_FULL;
				damages[spells[i].longinfo].duration = 100000000;
				damages[spells[i].longinfo].source = spells[i].caster;
				damages[spells[i].longinfo].flags = DAMAGE_FLAG_DONT_HURT_SOURCE
				                                    | DAMAGE_FLAG_FOLLOW_SOURCE
				                                    | DAMAGE_FLAG_ADD_VISUAL_FX;
				damages[spells[i].longinfo].type = DAMAGE_TYPE_FAKEFIRE
				                                   | DAMAGE_TYPE_MAGICAL
				                                   | DAMAGE_TYPE_DRAIN_MANA;
				damages[spells[i].longinfo].exist = true;
			}
			
			spells[i].longinfo2 = GetFreeDynLight();
			if(spells[i].longinfo2 != -1) {
				long id = spells[i].longinfo2;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 2.3f;
				DynLight[id].fallend = 700.f;
				DynLight[id].fallstart = 500.f;
				DynLight[id].rgb = Color3f::blue;
				DynLight[id].pos = spells[i].caster_pos;
				DynLight[id].duration=900;
			}
			
			break;
		}
		
		case SPELL_EXPLOSION: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_EXPLOSION);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 2000;
			
			Vec3f target = entities[spells[i].caster]->pos;
			if(spells[i].caster == 0) {
				target.y += 60.f;
			} else {
				target.y -= 60.f;
			}
			
			spells[i].longinfo = ARX_DAMAGES_GetFree();
			if(spells[i].longinfo != -1) {
				damages[spells[i].longinfo].radius = 350.f;
				damages[spells[i].longinfo].damages = 10.f;
				damages[spells[i].longinfo].area = DAMAGE_AREA; 
				damages[spells[i].longinfo].duration = spells[i].tolive;
				damages[spells[i].longinfo].source = spells[i].caster;
				damages[spells[i].longinfo].flags = DAMAGE_FLAG_DONT_HURT_SOURCE
				                                    | DAMAGE_FLAG_FOLLOW_SOURCE
				                                    | DAMAGE_FLAG_ADD_VISUAL_FX;
				damages[spells[i].longinfo].type = DAMAGE_TYPE_FAKEFIRE
				                                    | DAMAGE_TYPE_MAGICAL;
				damages[spells[i].longinfo].exist = true;
				damages[spells[i].longinfo].pos = target;
			}
			
			spells[i].longinfo2 = GetFreeDynLight();
			if(spells[i].longinfo2 != -1) {
				long id = spells[i].longinfo2;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 2.3f;
				DynLight[id].fallend = 700.f;
				DynLight[id].fallstart = 500.f;
				DynLight[id].rgb.r = 0.1f + rnd() * (1.f / 3);
				DynLight[id].rgb.g = 0.1f + rnd() * (1.f / 3);
				DynLight[id].rgb.b = 0.8f + rnd() * (1.f / 5);
				DynLight[id].pos = target;
				DynLight[id].duration = 200;
			}
			
			AddQuakeFX(300, 2000, 400, 1);
			
			for(long i_angle = 0 ; i_angle < 360 ; i_angle += 12) {
				for(long j = -100 ; j < 100 ; j += 50) {
					float rr = radians(float(i_angle));
					Vec3f pos(target.x - EEsin(rr) * 360.f, target.y,
					          target.z + EEcos(rr) * 360.f);
					Vec3f dir = Vec3f(pos.x - target.x, 0.f,
					                  pos.z - target.z).getNormalized() * 60.f;
					Color3f rgb(0.1f + rnd() * (1.f/3), 0.1f + rnd() * (1.f/3),
					            0.8f + rnd() * (1.f/5));
					Vec3f posi = target + Vec3f(0.f, j * 2, 0.f);
					LaunchFireballBoom(&posi, 16, &dir, &rgb);
				}
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND);
			
			break;
		}
		
		case SPELL_ENCHANT_WEAPON: {
			
			spells[i].exist = true;
			spells[i].tolive = 20;
			
			notifyAll = false; // TODO inconsistent use of the SM_SPELLCAST event
			
			break;
		}
		
		case SPELL_LIFE_DRAIN: {
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_HARM,
			                                                   spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_MANA_DRAIN,
			                                              spells[i].caster);
			if(iCancel > -1) {
				spells[iCancel].tolive = 0;
			}
			
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 6000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 12.f;
			
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD,
			                                       &spells[i].caster_pos, 0.8f,
			                                       ARX_SOUND_PLAY_LOOPED);
			
			spells[i].longinfo = ARX_DAMAGES_GetFree();
			if(spells[i].longinfo != -1) {
				long id = spells[i].longinfo;
				damages[id].radius = 150.f;
				damages[id].damages = spells[i].caster_level * 0.08f;
				damages[id].area = DAMAGE_AREA;
				damages[id].duration = 100000000;
				damages[id].source = spells[i].caster;
				damages[id].flags = DAMAGE_FLAG_DONT_HURT_SOURCE
				                    | DAMAGE_FLAG_FOLLOW_SOURCE
				                    | DAMAGE_FLAG_ADD_VISUAL_FX;
				damages[id].type = DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL
				                   | DAMAGE_TYPE_DRAIN_LIFE;
				damages[id].exist = true;
			}
			
			spells[i].longinfo2 = GetFreeDynLight();
			if(spells[i].longinfo2 != -1) {
				long id = spells[i].longinfo2;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 2.3f;
				DynLight[id].fallend = 700.f;
				DynLight[id].fallstart = 500.f;
				DynLight[id].rgb = Color3f::red;
				DynLight[id].pos = spells[i].caster_pos;
				DynLight[id].duration = 900;
			}
			
			break;
		}
		
		// level 9 spells
		
		case SPELL_SUMMON_CREATURE: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.9f;
			spells[i].longinfo = 0;
			spells[i].longinfo2 = 0;
			spells[i].tolive = (duration > -1) ? duration : 2000000;
			
			Vec3f target;
			float beta;
			bool displace = false;
			if(spells[i].caster == 0) {
				target = player.basePosition();
				beta = player.angle.b;
				displace = true;
			} else {
				target = entities[spells[i].caster]->pos;
				beta = entities[spells[i].caster]->angle.b;
				displace = (entities[spells[i].caster]->ioflags & IO_NPC) == IO_NPC;
			}
			if(displace) {
				target.x -= EEsin(radians(MAKEANGLE(beta))) * 300.f;
				target.z += EEcos(radians(MAKEANGLE(beta))) * 300.f;
			}
			
			if(!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target)) {
				spells[i].exist = false;
				ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
				return false;
			}
			
			spells[i].fdata = (spells[i].caster == 0 && cur_mega == 10) ? 1.f : 0.f;
			spells[i].target_pos = target;
			
			CSummonCreature * effect = new CSummonCreature();
			effect->spellinstance = i;
			effect->Create(target, MAKEANGLE(player.angle.b));
			effect->SetDuration(2000, 500, 1500);
			effect->SetColorBorder(Color3f::red);
			effect->SetColorRays1(Color3f::red);
			effect->SetColorRays2(Color3f::yellow * .5f);
			
			effect->lLightId = GetFreeDynLight();
			if(effect->lLightId > -1) {
				long id = effect->lLightId;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 0.3f;
				DynLight[id].fallend = 500.f;
				DynLight[id].fallstart = 400.f;
				DynLight[id].rgb = Color3f::red;
				DynLight[id].pos = effect->eSrc;
			}
			
			spells[i].pSpellFx = effect;
			
			break;
		}
		
		case SPELL_FAKE_SUMMON: {
			
			if(spells[i].caster <= 0 || !ValidIONum(spells[i].target)) {
				return false;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.9f;
			spells[i].tolive = 4000;
			
			Vec3f target = entities[spells[i].target]->pos;
			if(spells[i].target != 0) {
				target.y += player.baseHeight();
			}
			spells[i].target_pos = target;
			
			CSummonCreature * effect = new CSummonCreature();
			effect->spellinstance = i;
			effect->Create(target, MAKEANGLE(player.angle.b));
			effect->SetDuration(2000, 500, 1500);
			effect->SetColorBorder(Color3f::red);
			effect->SetColorRays1(Color3f::red);
			effect->SetColorRays2(Color3f::yellow * .5f);
			
			effect->lLightId = GetFreeDynLight();
			
			if(effect->lLightId > -1) {
				long id = effect->lLightId;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 0.3f;
				DynLight[id].fallend = 500.f;
				DynLight[id].fallstart = 400.f;
				DynLight[id].rgb = Color3f::red;
				DynLight[id].pos = effect->eSrc;
			}
			
			spells[i].pSpellFx = effect;
			
			break;
		}
		
		case SPELL_NEGATE_MAGIC: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_NEGATE_MAGIC);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 2.f;
			spells[i].tolive = (duration > -1) ? duration : 1000000;
			
			CNegateMagic * effect = new CNegateMagic();
			effect->spellinstance = i;
			effect->Create(player.pos, MAKEANGLE(player.angle.b));
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			if(spells[i].caster == 0) {
				spells[i].target = 0;
			}
			
			if(ValidIONum(spells[i].target)) {
				LaunchAntiMagicField(i);
			}
			
			break;
		}
		
		case SPELL_INCINERATE: {
			
			Entity * tio = entities[spells[i].target];
			if((tio->ioflags & IO_NPC) && tio->_npcdata->life <= 0.f) {
				return false;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE);
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_FIREPLACE,
			                                       &spells[i].caster_pos, 1.f,
			                                       ARX_SOUND_PLAY_LOOPED);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 20000;
			
			tio->sfx_flag |= SFX_TYPE_YLSIDE_DEATH | SFX_TYPE_INCINERATE;
			tio->sfx_time = (unsigned long)(arxtime);
			
			ARX_SPELLS_AddSpellOn(spells[i].target, i);
			
			break;
		}
		
		case SPELL_MASS_PARALYSE: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_MASS_PARALYSE);
			
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 10000;
			spells[i].longinfo2 = 0;
			
			for(size_t ii = 0; ii < entities.size(); ii++) {
				
				Entity * tio = entities[ii];
				if(long(ii) == spells[i].caster || !tio || !(tio->ioflags & IO_NPC)) {
					continue;
				}
				
				if(tio->show != SHOW_FLAG_IN_SCENE) {
					continue;
				}
				
				if(tio->ioflags & IO_FREEZESCRIPT) {
					continue;
				}
				
				if(fartherThan(tio->pos, entities[spells[i].caster]->pos, 500.f)) {
					continue;
				}
				
				tio->ioflags |= IO_FREEZESCRIPT;
				
				ARX_NPC_Kill_Spell_Launch(tio);
				ARX_SPELLS_AddSpellOn(ii, i);
				
				spells[i].longinfo2++;
				spells[i].misc = realloc(spells[i].misc,
				                         sizeof(long) * spells[i].longinfo2);
				long * ptr = (long *)spells[i].misc;
				ptr[spells[i].longinfo2 - 1] = ii;
			}
			
			break;
		}
		
		// level 10 spells
		
		case SPELL_MASS_LIGHTNING_STRIKE: {
			
			for(size_t ii = 0; ii < MAX_SPELLS; ii++) {
				if(spells[ii].exist && spells[ii].type == typ) {
					if(spells[ii].longinfo != -1) {
						DynLight[spells[ii].longinfo].exist = 0;
					}
					spells[ii].longinfo = -1;
					spells[ii].tolive = 0;
				}
			}
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 5000; // TODO probably never read
			spells[i].siz = 0;
			
			spells[i].longinfo = GetFreeDynLight();
			if(spells[i].longinfo != -1) {
				long id = spells[i].longinfo;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 1.8f;
				DynLight[id].fallend = 450.f;
				DynLight[id].fallstart = 380.f;
				DynLight[id].rgb = Color3f(1.f, 0.75f, 0.75f);
				DynLight[id].pos = spells[i].vsource;
			}
			
			long count = std::max(long(spells[i].caster_level), 1l);
			CMassLightning * effect = new CMassLightning(count);
			effect->spellinstance=i;
			
			Vec3f target;
			float beta;
			if(spells[i].caster == 0) {
				target = player.pos + Vec3f(0.f, 150.f, 0.f);
				beta = player.angle.b;
			} else {
				Entity * io = entities[spells[i].caster];
				target = io->pos + Vec3f(0.f, -20.f, 0.f);
				beta = io->angle.b;
			}
			target.x -= EEsin(radians(MAKEANGLE(beta))) * 500.f;
			target.z += EEcos(radians(MAKEANGLE(beta))) * 500.f;
			
			effect->SetDuration(long(500 * spells[i].caster_level));
			effect->Create(target, MAKEANGLE(player.angle.b));
			spells[i].pSpellFx = effect;
			spells[i].tolive = effect->GetDuration();
			
			ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_START);
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_LOOP, &target,
			                                       1.f, ARX_SOUND_PLAY_LOOPED);
			
			// Draws White Flash on Screen
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			EERIEDrawBitmap(0.f, 0.f, float(DANAESIZX), float(DANAESIZY), 0.00009f,
			                NULL, Color::white);
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);
			
			break;
		}
		
		case SPELL_CONTROL_TARGET: {
			
			if(!ValidIONum(spells[i].target)) {
				return false;
			}
			
			long tcount = 0;
			for(size_t ii = 1; ii < entities.size(); ii++) {
				
				Entity * ioo = entities[ii];
				if(!ioo || !(ioo->ioflags & IO_NPC)) {
					continue;
				}
				
				if(ioo->_npcdata->life <= 0.f || ioo->show != SHOW_FLAG_IN_SCENE) {
					continue;
				}
				
				if(ioo->groups.find("demon") == ioo->groups.end()) {
					continue;
				}
				
				if(closerThan(ioo->pos, spells[i].caster_pos, 900.f)) {
					tcount++;
					long n = spells[i].caster_level;
					std::string str = entities[spells[i].target]->long_name() + ' '
					                  + itoa(n);
					SendIOScriptEvent(ioo, SM_NULL, str, "npc_control");
				}
			}
			if(tcount == 0) {
				return false;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_CONTROL_TARGET);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 1000;
			
			CControlTarget * effect = new CControlTarget();
			effect->spellinstance = i;
			effect->Create(player.pos, MAKEANGLE(player.angle.b));
			effect->SetDuration(spells[i].tolive);
			spells[i].pSpellFx = effect;
			
			break;
		}
		
		case SPELL_FREEZE_TIME: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_FREEZETIME);
			
			spells[i].siz = spells[i].caster_level * 0.08f;
			GLOBAL_SLOWDOWN -= spells[i].siz;
			
			spells[i].exist = true;
			spells[i].tolive = (duration > -1) ? duration : 200000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 30.f * spells[i].siz;
			spells[i].longinfo = (long)arxtime.get_updated();
			
			break;
		}
		
		case SPELL_MASS_INCINERATE: {
			
			ARX_SOUND_PlaySFX(SND_SPELL_MASS_INCINERATE);
			
			spells[i].exist = true;
			spells[i].lastupdate = spells[i].timcreation = (unsigned long)(arxtime);
			spells[i].tolive = 20000;
			
			long nb_targets=0;
			for(size_t ii = 0; ii < entities.size(); ii++) {
				
				Entity * tio = entities[ii];
				if(long(ii) == spells[i].caster || !tio || !(tio->ioflags & IO_NPC)) {
					continue;
				}
				
				if(tio->_npcdata->life <= 0.f || tio->show != SHOW_FLAG_IN_SCENE) {
					continue;
				}
				
				if(fartherThan(tio->pos, entities[spells[i].caster]->pos, 500.f)) {
					continue;
				}
				
				tio->sfx_flag |= SFX_TYPE_YLSIDE_DEATH | SFX_TYPE_INCINERATE;
				tio->sfx_time = (unsigned long)(arxtime);
				nb_targets++;
				ARX_SPELLS_AddSpellOn(ii, i);
			}
			
			if(nb_targets) {
				spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_FIREPLACE,
				                                       &spells[i].caster_pos, 1.f,
				                                       ARX_SOUND_PLAY_LOOPED);
			} else {
				spells[i].snd_loop = -1;
			}
			
			break;
		}
		
		case SPELL_TELEPORT: {
			
			spells[i].exist = true;
			spells[i].tolive = 7000;
			
			ARX_SOUND_PlaySFX(SND_SPELL_TELEPORT, &spells[i].caster_pos);
			
			if(spells[i].caster == 0) {
				LASTTELEPORT = 0.f;
			}
			
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

//*************************************************************************************
// Used for specific Spell-End FX
//*************************************************************************************
void ARX_SPELLS_Kill(long i) {
	
	static TextureContainer * tc4=TextureContainer::Load("graph/particles/smoke");

	if (!spells[i].exist) return;

	spells[i].exist=false;

	// All Levels - Kill Light
	if (spells[i].pSpellFx && spells[i].pSpellFx->lLightId != -1)
	{
		DynLight[spells[i].pSpellFx->lLightId].duration = 500; 
		DynLight[spells[i].pSpellFx->lLightId].time_creation = (unsigned long)(arxtime);
	}

	switch(spells[i].type)
	{
		//----------------------------------------------------------------------------
		case SPELL_FIREBALL :

			if (spells[i].longinfo!=-1) 
			{
				DynLight[spells[i].longinfo].duration = 500; 
				DynLight[spells[i].longinfo].time_creation = (unsigned long)(arxtime);
			}

			spells[i].longinfo=-1;

			if (spells[i].pSpellFx)
				delete spells[i].pSpellFx;

			spells[i].pSpellFx=NULL;
		break;
		//----------------------------------------------------------------------------
		case SPELL_LIGHTNING_STRIKE :

			if (spells[i].longinfo!=-1) 
			{
				DynLight[spells[i].longinfo].duration = 200; 
				DynLight[spells[i].longinfo].time_creation = (unsigned long)(arxtime);
			}

			spells[i].longinfo=-1;
			ARX_SOUND_Stop(spells[i].snd_loop);
			ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_END);

			if (spells[i].pSpellFx)
				delete spells[i].pSpellFx;

			spells[i].pSpellFx=NULL;
		break;
		//----------------------------------------------------------------------------
		case SPELL_MASS_LIGHTNING_STRIKE :

			if (spells[i].longinfo!=-1) 
			{
				DynLight[spells[i].longinfo].duration = 200; 
				DynLight[spells[i].longinfo].time_creation = (unsigned long)(arxtime);
			}

			spells[i].longinfo=-1;
			ARX_SOUND_Stop(spells[i].snd_loop);
			ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_END);

			if (spells[i].pSpellFx)
				delete spells[i].pSpellFx;

			spells[i].pSpellFx=NULL;
		break;
		case SPELL_REPEL_UNDEAD:

			if (spells[i].longinfo!=-1) 
			{
				DynLight[spells[i].longinfo].duration = 200; 
				DynLight[spells[i].longinfo].time_creation = (unsigned long)(arxtime);
			}

			spells[i].longinfo=-1;
			ARX_SOUND_Stop(spells[i].snd_loop);

			if (spells[i].pSpellFx)
				delete spells[i].pSpellFx;

			spells[i].pSpellFx=NULL;
		break;
		//----------------------------------------------------------------------------
		case SPELL_HARM :
		case SPELL_LIFE_DRAIN :
		case SPELL_MANA_DRAIN :

			if (spells[i].longinfo!=-1) damages[spells[i].longinfo].exist=false;	

			if (spells[i].longinfo2!=-1) 
			{
				DynLight[spells[i].longinfo2].time_creation = (unsigned long)(arxtime);
				DynLight[spells[i].longinfo2].duration = 600; 
			}

			ARX_SOUND_Stop(spells[i].snd_loop);
			break;
		
		case SPELL_FLYING_EYE : {
			
			ARX_SOUND_PlaySFX(SND_SPELL_EYEBALL_OUT);
			eyeball.exist = -100;
			
			for(long n = 0; n < 12; n++) {
				
				PARTICLE_DEF * pd = createParticle();
				if(!pd) {
					break;
				}
				
				pd->ov = eyeball.pos + randomVec(-5.f, 5.f);
				pd->move = randomVec(-2.f, 2.f);
				pd->siz = 28.f;
				pd->tolive = Random::get(2000, 6000);
				pd->scale = Vec3f::repeat(12.f);
				pd->timcreation = spells[i].lastupdate;
				pd->tc = tc4;
				pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
				pd->fparam = 0.0000001f;
				pd->rgb = Color3f(0.7f, 0.7f, 1.f);
			}
			
			config.input.mouseLookToggle = bOldLookToggle; 
			
			break;
		}
		
		//----------------------------------------------------------------------------
		// Level 06	
		//---------------------------------------LEVEL1
		case SPELL_IGNIT:

			if (spells[i].pSpellFx)
				delete spells[i].pSpellFx;

			spells[i].pSpellFx=NULL;
		break;
		case SPELL_DOUSE:

			if (spells[i].pSpellFx)
				delete spells[i].pSpellFx;

			spells[i].pSpellFx=NULL;
		break;
		//----------------------------------------------------------------------------
		case SPELL_PARALYSE:
			ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE_END);
			break;

		//---------------------------------------------------------------------
		// Level 7
		case SPELL_FIRE_FIELD:
			{
				ARX_SOUND_Stop(spells[i].snd_loop);
				ARX_SOUND_PlaySFX(SND_SPELL_FIRE_FIELD_END);
		}
		break;
		case SPELL_LOWER_ARMOR:
		break;

		//----------------------------------------------------------------------------
		case SPELL_EXPLOSION:
		{
			if (spells[i].pSpellFx)
				delete spells[i].pSpellFx;

			spells[i].pSpellFx=NULL;
			break;
		}

		//----------------------------------------------------------------------------
		case SPELL_MASS_PARALYSE:

			ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE_END);
			break;
		//----------------------------------------------------------------------------
		case SPELL_SUMMON_CREATURE:

			if (spells[i].pSpellFx->lLightId > -1)
			{
				long id = spells[i].pSpellFx->lLightId;
				DynLight[id].exist = 0;
				spells[i].pSpellFx->lLightId=-1;
			}

			if (ValidIONum(spells[i].longinfo2) && (spells[i].longinfo2!=0))
			{
				if (	(entities[spells[i].longinfo2]->scriptload)
					&&	(entities[spells[i].longinfo2]->ioflags & IO_NOSAVE)	)
				{
					AddRandomSmoke(entities[spells[i].longinfo2],100);
					Vec3f posi;
					posi = entities[spells[i].longinfo2]->pos;
					posi.y-=100.f;
					MakeCoolFx(&posi);
					long nn=GetFreeDynLight();

					if(nn >= 0) {
						DynLight[nn].exist=1;
						DynLight[nn].intensity = 0.7f + 2.f*rnd();
						DynLight[nn].fallend = 600.f;
						DynLight[nn].fallstart = 400.f;
						DynLight[nn].rgb = Color3f(1.0f, 0.8f, 0.0f);
						DynLight[nn].pos = posi;
						DynLight[nn].duration=600;
					}

					ARX_INTERACTIVE_DestroyIO(entities[spells[i].longinfo2]);
				}
			}

			spells[i].longinfo2=0;
		break;
		case SPELL_FAKE_SUMMON:

			if (spells[i].pSpellFx->lLightId > -1)
			{
				long id = spells[i].pSpellFx->lLightId;
				DynLight[id].exist = 0;
				spells[i].pSpellFx->lLightId=-1;
			}

		break;
		default:

			if (spells[i].pSpellFx)
				delete spells[i].pSpellFx;

			spells[i].pSpellFx=NULL;
			break;
	}

	if (spells[i].pSpellFx)
		delete spells[i].pSpellFx;

	spells[i].pSpellFx=NULL;
}


EYEBALL_DEF eyeball;

Anglef cabalangle;
Vec3f cabalpos;
Vec3f cabalscale;
Color3f cabalcolor;


float ARX_SPELLS_ApplyFireProtection(Entity * io,float damages)
{
	if (io)
	{
		long idx=ARX_SPELLS_GetSpellOn(io,SPELL_FIRE_PROTECTION);

		if (idx>=0)
		{
			float modif=1.f-((float)spells[idx].caster_level*( 1.0f / 10 ));

			if (modif>1.f) modif=1.f;
			else if (modif<0.f) modif=0.f;

			damages*=modif;
		}

		if (io->ioflags & IO_NPC)
		{
			damages-=io->_npcdata->resist_fire*( 1.0f / 100 )*damages;

			if (damages<0.f) damages=0.f;
		}
	}

	return damages;
}
float ARX_SPELLS_ApplyColdProtection(Entity * io,float damages)
{
	long idx=ARX_SPELLS_GetSpellOn(io,SPELL_COLD_PROTECTION);

	if (idx>=0)
	{
		float modif=1.f-((float)spells[idx].caster_level*( 1.0f / 10 ));

		if (modif>1.f) modif=1.f;
		else if (modif<0.f) modif=0.f;

		damages*=modif;
	}

	return damages;
}

//*************************************************************************************
// Updates all currently working spells.
//*************************************************************************************
void ARX_SPELLS_Update()
{
	
	unsigned long tim;
	long framediff,framediff3;

	ucFlick++;

	tim = (unsigned long)(arxtime);

	for(size_t i = 0; i < MAX_SPELLS; i++) {

		if (!GLOBAL_MAGIC_MODE) spells[i].tolive=0;

		if (spells[i].exist) 
		{
			if (spells[i].bDuration && !CanPayMana(i,spells[i].fManaCostPerSecond * (float)FrameDiff * ( 1.0f / 1000 ), false))
				ARX_SPELLS_Fizzle(i);

			framediff=spells[i].timcreation+spells[i].tolive-tim;
			framediff3=tim-spells[i].lastupdate;

			if (framediff<0) 
			{
				SPELLEND_Notify(i);

				switch (spells[i].type)
				{
				//----------------------------------------------------------------------------
				case SPELL_TELEPORT:
					ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &spells[i].caster_pos);
				break;
				//----------------------------------------------------------------------------
				
				//****************************************************************************
				// LEVEL 1 SPELLS -----------------------------------------------------------------------------
				//----------------------------------------------------------------------------
				case SPELL_MAGIC_SIGHT:

					if (spells[i].caster==0)
					{
						Project.improve=0;
						ARX_SOUND_Stop(spells[i].snd_loop);
					}

					ARX_SOUND_PlaySFX(SND_SPELL_VISION_START, &spells[i].caster_pos);					
				break;
				//-----------------------------------------------------------------------------
				case SPELL_MAGIC_MISSILE:
				{
					if (spells[i].longinfo!=-1) 		
						DynLight[spells[i].longinfo].exist=0;							
				}
				break;
				//----------------------------------------------------------------------------
				case SPELL_IGNIT:
				{
					CIgnit *pIgnit;
					pIgnit=(CIgnit *)spells[i].pSpellFx;
					pIgnit->Action(1);					
				}
				break;
				//----------------------------------------------------------------------------
				case SPELL_DOUSE:
				{
					CDoze *pDoze;
					pDoze=(CDoze *)spells[i].pSpellFx;
					pDoze->Action(0);					
				}
				break;
				//----------------------------------------------------------------------------------
				//**********************************************************************************
				// LEVEL 2 -------------------------------------------------------------------------
				//----------------------------------------------------------------------------------				
				case SPELL_DETECT_TRAP:

					if (spells[i].caster==0)
					{
						Project.improve=0;
						ARX_SOUND_Stop(spells[i].snd_loop);
					}					

				break;					
				//----------------------------------------------------------------------------
				case SPELL_ARMOR:
				{
					ARX_SOUND_Stop(spells[i].snd_loop);
					ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_END, &spells[i].caster_pos);					
					Entity * io=entities[spells[i].target];

					if (spells[i].longinfo)
					{
						io->halo.flags&=~HALO_ACTIVE;
						ARX_HALO_SetToNative(io);
					}

					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
				}
				break;
				//----------------------------------------------------------------------------
				case SPELL_LOWER_ARMOR:
				{
					Entity * io=entities[spells[i].target];

					if (spells[i].longinfo)
					{
						io->halo.flags&=~HALO_ACTIVE;
						ARX_HALO_SetToNative(io);
					}

					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);					
				}
				break;					
				//----------------------------------------------------------------------------------
				//**********************************************************************************
				// LEVEL 3 -------------------------------------------------------------------------
				//----------------------------------------------------------------------------------
				case SPELL_SPEED:						

					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);

					if(spells[i].target >= 0 && spells[i].target < long(entities.size()))
					{
						if (entities[spells[i].target])
							entities[spells[i].target]->speed_modif-=spells[i].caster_level*( 1.0f / 10 );
					}

					if (spells[i].caster == 0) ARX_SOUND_Stop(spells[i].snd_loop);

					ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_END, &spells[i].caster_pos);
				break;
				//----------------------------------------------------------------------------------
				case SPELL_FIREBALL:
					ARX_SOUND_Stop(spells[i].snd_loop);					
				break;
				//----------------------------------------------------------------------------------
				//**********************************************************************************
				// LEVEL 4 -------------------------------------------------------------------------
				//----------------------------------------------------------------------------------
				case SPELL_BLESS:
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
				break;
				case SPELL_CURSE:
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
				break;
				//----------------------------------------------------------------------------
				case SPELL_TELEKINESIS:						

					if (spells[i].caster==0)
						Project.telekinesis=0;

					ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_END, &spells[i].caster_pos);					
				break;
				case SPELL_FIRE_PROTECTION:
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);;

					if (ValidIONum(spells[i].target))
						ARX_HALO_SetToNative(entities[spells[i].target]);

				break;
				case SPELL_COLD_PROTECTION:
					ARX_SOUND_Stop(spells[i].snd_loop);
					ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_END);
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);;

					if (ValidIONum(spells[i].target))
						ARX_HALO_SetToNative(entities[spells[i].target]);

				break;
				//----------------------------------------------------------------------------------
				//**********************************************************************************
				// LEVEL 5 -------------------------------------------------------------------------
				//----------------------------------------------------------------------------------
				//----------------------------------------------------------------------------
				case SPELL_LEVITATE:
				{
 
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);

					if (spells[i].target==0)
						player.levitate=0;
				}
				break;
				//----------------------------------------------------------------------------------
				//**********************************************************************************
				// LEVEL 6 SPELLS ------------------------------------------------------------------
				//----------------------------------------------------------------------------------
				case SPELL_PARALYSE:
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
					entities[spells[i].target]->ioflags&=~IO_FREEZESCRIPT;
				break;
				//----------------------------------------------------------------------------------
				case SPELL_RISE_DEAD:
					ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].caster_pos);

					if (ValidIONum(spells[i].longinfo) && spells[i].longinfo!=0)
					{				
						if (	(entities[spells[i].longinfo]->scriptload)
							&&	(entities[spells[i].longinfo]->ioflags & IO_NOSAVE)	)
						{
							AddRandomSmoke(entities[spells[i].longinfo],100);
							Vec3f posi = entities[spells[i].longinfo]->pos;
							posi.y-=100.f;
							MakeCoolFx(&posi);
							long nn=GetFreeDynLight();

							if(nn >= 0) {
								DynLight[nn].exist=1;
								DynLight[nn].intensity = 0.7f + 2.f*rnd();
								DynLight[nn].fallend = 600.f;
								DynLight[nn].fallstart = 400.f;
								DynLight[nn].rgb = Color3f(1.0f, 0.8f, 0.f);
								DynLight[nn].pos = posi;
								DynLight[nn].duration = 600;
							}

							ARX_INTERACTIVE_DestroyIO(entities[spells[i].longinfo]);
						}
					}					

				break;
				case SPELL_CREATE_FIELD:
					CCreateField * pCreateField;
					pCreateField = (CCreateField *) spells[i].pSpellFx;

					if (	(pCreateField)
						&&	(pCreateField->lLightId != -1)	)
					{
						long id=pCreateField->lLightId;
						DynLight[id].duration=800;
					}

					if(ValidIONum(spells[i].longinfo)) {
						delete entities[spells[i].longinfo];
					}

				break;
				//----------------------------------------------------------------------------
				case SPELL_SLOW_DOWN:
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);

					if(spells[i].target >= 0 && spells[i].target < long(entities.size()))
					{
						if (entities[spells[i].target])
							entities[spells[i].target]->speed_modif+=spells[i].caster_level*( 1.0f / 20 );
					}

				break;				
				//----------------------------------------------------------------------------------
				//**********************************************************************************
				// LEVEL 7 -------------------------------------------------------------------------
				//----------------------------------------------------------------------------------				
				case SPELL_ICE_FIELD:

					if (spells[i].longinfo!=-1)
						damages[spells[i].longinfo].exist=false;					

				break;
				case SPELL_FIRE_FIELD:

					if (spells[i].longinfo!=-1)
						damages[spells[i].longinfo].exist=false;					

				break;
				//----------------------------------------------------------------------------
				case SPELL_LIGHTNING_STRIKE:					
					ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].caster_pos);					
				break;
				//----------------------------------------------------------------------------
				case SPELL_FLYING_EYE:					
					ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &entities[spells[i].caster]->pos);
				break;
				case SPELL_CONFUSE:
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
				break;
				//----------------------------------------------------------------------------------
				//**********************************************************************************
				// LEVEL 8 -------------------------------------------------------------------------
				//----------------------------------------------------------------------------------
				case SPELL_EXPLOSION:					
				break;
				//----------------------------------------------------------------------------
				case SPELL_INVISIBILITY:	
					{
						if (ValidIONum(spells[i].target))
						{
							entities[spells[i].target]->gameFlags&=~GFLAG_INVISIBILITY;											
							ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_END, &entities[spells[i].target]->pos);					
							ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
						}
					}
				break;
				//----------------------------------------------------------------------------------
				//**********************************************************************************
				// LEVEL 9 -------------------------------------------------------------------------
				//----------------------------------------------------------------------------------
				//----------------------------------------------------------------------------
				case SPELL_MASS_PARALYSE:
					{
						long * ptr=(long *)spells[i].misc;

						for (long in=0;in<spells[i].longinfo2;in++)
						{
							
							if (ValidIONum(ptr[in]))
							{
								ARX_SPELLS_RemoveSpellOn(ptr[in],i);
								entities[ptr[in]]->ioflags&=~IO_FREEZESCRIPT;											
							}
						}

						if (ptr) free(spells[i].misc);

						spells[i].misc=NULL;
					}
				break;
				case SPELL_SUMMON_CREATURE :
						ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].caster_pos);						

						if (spells[i].pSpellFx->lLightId > -1)
					{
						long id = spells[i].pSpellFx->lLightId;
						DynLight[id].exist = 0;
						spells[i].pSpellFx->lLightId=-1;
					}

						// need to killio
				break;
				case SPELL_FAKE_SUMMON :
					ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].caster_pos);						

					if (spells[i].pSpellFx->lLightId > -1)
					{
						long id = spells[i].pSpellFx->lLightId;
						DynLight[id].exist = 0;
						spells[i].pSpellFx->lLightId=-1;
					}

				break;
				case SPELL_INCINERATE:
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);					
					ARX_SOUND_Stop(spells[i].snd_loop);
				break;
				//----------------------------------------------------------------------------------
				//**********************************************************************************
				// LEVEL 10 ------------------------------------------------------------------------
				//----------------------------------------------------------------------------------
				case SPELL_FREEZE_TIME:
						GLOBAL_SLOWDOWN += spells[i].siz;
					ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_END, &spells[i].caster_pos);					
				break;
				case SPELL_MASS_INCINERATE:
					ARX_SPELLS_RemoveMultiSpellOn(i);
					ARX_SOUND_Stop(spells[i].snd_loop);
				break;
				default: break;
				//----------------------------------------------------------------------------------
			}				

			ARX_SPELLS_Kill(i);
			continue;			
		}
			
		//******************************************************************************************
		//******************************************************************************************
		//******************************************************************************************
		//******************************************************************************************
		//******************************************************************************************
		//******************************************************************************************

	if (spells[i].exist) 
		switch (spells[i].type)
		{
			case SPELL_DISPELL_FIELD: break;
			case SPELL_NONE: break;
			//**************************************************************************************
			// LEVEL 1 -----------------------------------------------------------------------------
			case SPELL_MAGIC_MISSILE:
				{
					CSpellFx *pCSpellFX = spells[i].pSpellFx;

					if (pCSpellFX)
					{
						CMultiMagicMissile *pMMM = (CMultiMagicMissile *) pCSpellFX;
							pMMM->CheckCollision();
						
						// Update
						pCSpellFX->Update(FrameDiff);

						if (pCSpellFX->Render()==-1)
							spells[i].tolive=0;
					}
				}
			break;
			//---------------------------------------------------------------------------------------
			case SPELL_IGNIT:
			case SPELL_DOUSE:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
				}
			} 
			break;
			//---------------------------------------------------------------------------------------
			case SPELL_ACTIVATE_PORTAL:
			{
			} 
			break;
			//---------------------------------------------------------------------------------------
			//***************************************************************************************	
			// LEVEL 2 -----------------------------------------------------------------------------
			case SPELL_HEAL: // guérit les ennemis collés
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
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

						if (long(ii) == spells[i].caster) dist=0;
						else dist=fdist(ch->eSrc, entities[ii]->pos);

						if (dist<300.f)
						{
							float gain=((rnd()*1.6f+0.8f)*spells[i].caster_level)*(300.f-dist)*( 1.0f / 300 )*framedelay*( 1.0f / 1000 );

							if (ii==0) 
							{
								if (!BLOCK_PLAYER_CONTROLS)
									player.life=std::min(player.life+gain,player.Full_maxlife);									
							}
							else entities[ii]->_npcdata->life=std::min(entities[ii]->_npcdata->life+gain,entities[ii]->_npcdata->maxlife);
						}
					}
				}
			}
			break;
			//------------------------------------------------------------------------------------
			case SPELL_DETECT_TRAP:				
			{
				if (spells[i].caster == 0)
				{
					ARX_SOUND_RefreshPosition(spells[i].snd_loop);						
				}

				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render();
				}
			} 
			break;
			//------------------------------------------------------------------------------------
			case SPELL_ARMOR:
			case SPELL_LOWER_ARMOR:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render();
				}
			} 
			break;
			//--------------------------------------------------------------------------------------
			case SPELL_HARM:
			{						
				if ( (cabal!=NULL) )
				{
					float refpos;
					float scaley;

					if (spells[i].caster==0) scaley=90.f;
					else scaley=EEfabs(entities[spells[i].caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;

 
					float mov=EEsin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;

					if (spells[i].caster==0)
					{
								cabalpos.x = player.pos.x; 
						cabalpos.y=player.pos.y+60.f-mov;
								cabalpos.z = player.pos.z; 
						refpos=player.pos.y+60.f;							
					}
					else
					{							
								cabalpos.x = entities[spells[i].caster]->pos.x; 
						cabalpos.y=entities[spells[i].caster]->pos.y-scaley-mov;
								cabalpos.z = entities[spells[i].caster]->pos.z; 
						refpos=entities[spells[i].caster]->pos.y-scaley;							
					}

					float Es=EEsin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + radians(scaley));

					if (spells[i].longinfo2!=-1)
					{
						DynLight[spells[i].longinfo2].pos.x = cabalpos.x;
						DynLight[spells[i].longinfo2].pos.y = refpos;
						DynLight[spells[i].longinfo2].pos.z = cabalpos.z; 
						DynLight[spells[i].longinfo2].rgb.r=rnd()*0.2f+0.8f;
						DynLight[spells[i].longinfo2].rgb.g=rnd()*0.2f+0.6f;
						DynLight[spells[i].longinfo2].fallstart=Es*1.5f;
					}

					GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					GRenderer->SetRenderState(Renderer::DepthWrite, false);
					cabalangle.b=spells[i].fdata+(float)FrameDiff*0.1f;
					spells[i].fdata=cabalangle.b;

							cabalangle.g = 0.f; 
							cabalcolor.r = 0.8f;
							cabalcolor.g = 0.4f;
							cabalcolor.b = 0.f;
					cabalscale.z=cabalscale.y=cabalscale.x=Es;				
					DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
					mov=EEsin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
					cabalpos.y=refpos-mov;						
							cabalcolor.b = 0.f;
							cabalcolor.g = 3.f;
							cabalcolor.r = 0.5f;
					DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
					mov=EEsin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
					cabalpos.y=refpos-mov;
							cabalcolor.b = 0.f;
							cabalcolor.g = 0.1f;
							cabalcolor.r = 0.25f;
					DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
					mov=EEsin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
					cabalpos.y=refpos-mov;
							cabalcolor.b = 0.f;
							cabalcolor.g = 0.1f;
							cabalcolor.r = 0.15f;
					DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);		
					GRenderer->SetRenderState(Renderer::DepthWrite, true);	
				}
			}
			break;				
			//--------------------------------------------------------------------------------------
			//**************************************************************************************
			// LEVEL 3 SPELLS -----------------------------------------------------------------------------
			case SPELL_FIREBALL:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					CFireBall *pCF = (CFireBall*) pCSpellFX;
						
					if (spells[i].longinfo==-1) spells[i].longinfo=GetFreeDynLight();

					if (spells[i].longinfo!=-1)
					{
						long id=spells[i].longinfo;
						EERIE_LIGHT * el=&DynLight[id];
						el->exist=1;
						el->pos = pCF->eCurPos;
						el->intensity = 2.2f;
						el->fallend = 500.f;
						el->fallstart = 400.f;
						el->rgb.r = 1.0f-rnd()*0.3f;
						el->rgb.g = 0.6f-rnd()*0.1f;
						el->rgb.b = 0.3f-rnd()*0.1f;
					}

					EERIE_SPHERE sphere;
					sphere.origin = pCF->eCurPos;
					sphere.radius=std::max(spells[i].caster_level*2.f,12.f);
					#define MIN_TIME_FIREBALL 2000 

					if (pCF->pPSFire.iParticleNbMax)
					{
						if (pCF->ulCurrentTime > MIN_TIME_FIREBALL)
						{
							SpawnFireballTail(&pCF->eCurPos,&pCF->eMove,(float)spells[i].caster_level,0);
						}
						else
						{
							if (rnd()<0.9f)
							{
								Vec3f move(0, 0, 0);
								float dd=(float)pCF->ulCurrentTime / (float)MIN_TIME_FIREBALL*10;

								if (dd>spells[i].caster_level) dd=spells[i].caster_level;

								if (dd<1) dd=1;

								SpawnFireballTail(&pCF->eCurPos,&move,(float)dd,1);
							}
						}
					}

					if (pCF->bExplo == false)
					if (CheckAnythingInSphere(&sphere,spells[i].caster,CAS_NO_SAME_GROUP))
					{
						ARX_BOOMS_Add(&pCF->eCurPos);
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
						ARX_NPC_SpawnAudibleSound(&sphere.origin, entities[spells[i].caster]);
								}

					pCSpellFX->Update(FrameDiff);
					ARX_SOUND_RefreshPosition(spells[i].snd_loop, &pCF->eCurPos);
				}
			}
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_SPEED:

			if (spells[i].pSpellFx)
			{
				spells[i].pSpellFx->Update(FrameDiff);
				spells[i].pSpellFx->Render();

				if (spells[i].caster == 0) ARX_SOUND_RefreshPosition(spells[i].snd_loop);
			}

			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_CREATE_FOOD:
			case SPELL_ICE_PROJECTILE:
			case SPELL_DISPELL_ILLUSION:

			if (spells[i].pSpellFx)
			{
				spells[i].pSpellFx->Update(FrameDiff);
				spells[i].pSpellFx->Render();
			}

			break;
			//-----------------------------------------------------------------------------------------
			//*****************************************************************************************
			// LEVEL 4 SPELLS -----------------------------------------------------------------------------
			case SPELL_BLESS:
			{
				if (spells[i].pSpellFx)
				{
					CBless * pBless=(CBless *)spells[i].pSpellFx;

					if (pBless)
					{
						if (ValidIONum(spells[i].target))
						{
							pBless->eSrc = entities[spells[i].target]->pos;
							Anglef angle = Anglef::ZERO;

							if (spells[i].target==0)
								angle.b=player.angle.b;	
							else 
								angle.b=entities[spells[i].target]->angle.b;

							pBless->Set_Angle(angle);
						}
					}

					spells[i].pSpellFx->Update(FrameDiff);
					spells[i].pSpellFx->Render();
				}
			}
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_CURSE:

			if (spells[i].pSpellFx)
			{
				CCurse * curse=(CCurse *)spells[i].pSpellFx;
				Vec3f target(0, 0, 0);
					
				if ((spells[i].target>=0) && (entities[spells[i].target]))
				{
					target = entities[spells[i].target]->pos;

					if (spells[i].target==0) target.y-=200.f;
					else target.y+=entities[spells[i].target]->physics.cyl.height-30.f;
				}
				
				curse->Update(checked_range_cast<unsigned long>(FrameDiff));
				
				curse->eTarget = target;
				curse->Render();
				GRenderer->SetCulling(Renderer::CullNone);
			}

			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_FIRE_PROTECTION:
				spells[i].pSpellFx->Update(FrameDiff);
				spells[i].pSpellFx->Render();
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_COLD_PROTECTION:
				spells[i].pSpellFx->Update(FrameDiff);
				spells[i].pSpellFx->Render();
			break;
			//-----------------------------------------------------------------------------------------
			//*****************************************************************************************				
			// LEVEL 5 SPELLS -----------------------------------------------------------------------------
			//-----------------------------------------------------------------------------------------
			case SPELL_CURE_POISON:
			{
				if (spells[i].pSpellFx)
				{
					spells[i].pSpellFx->Update(FrameDiff);
					spells[i].pSpellFx->Render();					
				}
			}
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_RUNE_OF_GUARDING:
			{
				if (spells[i].pSpellFx)
				{
					spells[i].pSpellFx->Update(FrameDiff);
					spells[i].pSpellFx->Render();
					CRuneOfGuarding * pCRG=(CRuneOfGuarding *)spells[i].pSpellFx;

					if (pCRG)
					{
						EERIE_SPHERE sphere;
						sphere.origin = pCRG->eSrc;
						sphere.radius=std::max(spells[i].caster_level*15.f,50.f);

						if (CheckAnythingInSphere(&sphere,spells[i].caster,CAS_NO_SAME_GROUP | CAS_NO_BACKGROUND_COL | CAS_NO_ITEM_COL| CAS_NO_FIX_COL | CAS_NO_DEAD_COL))
						{
							ARX_BOOMS_Add(&pCRG->eSrc);
							LaunchFireballBoom(&pCRG->eSrc,(float)spells[i].caster_level);
							DoSphericDamage(&pCRG->eSrc,4.f*spells[i].caster_level,30.f*spells[i].caster_level,DAMAGE_AREA,DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL,spells[i].caster);
							spells[i].tolive=0;
							ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &sphere.origin);
						}
					}
				}
			}
			break;
			case SPELL_REPEL_UNDEAD:
			{
				if (spells[i].pSpellFx)
				{
					spells[i].pSpellFx->Update(FrameDiff);
					spells[i].pSpellFx->Render();					

					if (spells[i].target == 0)
						ARX_SOUND_RefreshPosition(spells[i].snd_loop);
				}
			}
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_POISON_PROJECTILE:

			if (spells[i].pSpellFx)
			{
				spells[i].pSpellFx->Update(FrameDiff);
				spells[i].pSpellFx->Render();
						}

			break;
			//-----------------------------------------------------------------------------------------	
			case SPELL_LEVITATE:
			{
				CLevitate *pLevitate=(CLevitate *)spells[i].pSpellFx;
				Vec3f target;

				if (spells[i].target==0)
				{
					target.x=player.pos.x;
					target.y=player.pos.y+150.f;
					target.z=player.pos.z;
					player.levitate=1;
				}
				else
				{
					target.x=entities[spells[i].caster]->pos.x;
							target.y = entities[spells[i].caster]->pos.y; 
					target.z=entities[spells[i].caster]->pos.z;
				}

				pLevitate->ChangePos(&target);
					
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render();
					GRenderer->SetCulling(Renderer::CullNone);
				}
			}
			break;
			//-----------------------------------------------------------------------------------------
			//*****************************************************************************************
			// LEVEL 6 SPELLS -----------------------------------------------------------------------------
			case SPELL_RISE_DEAD:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{				
					if (spells[i].longinfo==-2) 
					{
						pCSpellFX->lLightId=-1;
						break;
					}

					spells[i].tolive+=200;
				
					pCSpellFX->Update(FrameDiff);
							pCSpellFX->Render();

					if (pCSpellFX->lLightId > -1)
					{
						long id=pCSpellFX->lLightId;
						DynLight[id].exist=1;
								DynLight[id].intensity = 0.7f + 2.3f;
						DynLight[id].fallend = 500.f;
						DynLight[id].fallstart = 400.f;
						DynLight[id].rgb.r = 0.8f;
						DynLight[id].rgb.g = 0.2f;
						DynLight[id].rgb.b = 0.2f;
						DynLight[id].duration=800;
								DynLight[id].time_creation = (unsigned long)(arxtime);
					}

					unsigned long tim=pCSpellFX->getCurrentTime();

					if ((tim>3000) && (spells[i].longinfo==-1))
					{
						ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].caster_pos);
						CRiseDead *prise;
						prise= (CRiseDead *)spells[i].pSpellFx;

						if (prise)
						{	
							EERIE_CYLINDER phys;
							phys.height=-200;
							phys.radius=50;
							phys.origin=spells[i].target_pos;
	
									float anything = CheckAnythingInCylinder(&phys, NULL, CFLAG_JUST_TEST);

							if(EEfabs(anything) < 30) {
								
								const char * cls =
									"graph/obj3d/interactive/npc/undead_base/undead_base";
								Entity * io = AddNPC(cls, -1, IO_IMMEDIATELOAD);
								
								if(io) {
									
									ARX_INTERACTIVE_HideGore(io);
									RestoreInitialIOStatusOfIO(io);
									
									long lSpellsCaster = spells[i].caster;
									io->summoner = checked_range_cast<short>(lSpellsCaster);
									
									io->ioflags|=IO_NOSAVE;
									spells[i].longinfo = io->index();
									io->scriptload=1;
									
									ARX_INTERACTIVE_Teleport(io,&phys.origin,0);
									SendInitScriptEvent(io);

									if(ValidIONum(spells[i].caster)) {
										EVENT_SENDER = entities[spells[i].caster];
									} else {
										EVENT_SENDER = NULL;
									}

									SendIOScriptEvent(io,SM_SUMMONED);
										
									Vec3f pos;
										{
											pos.x=prise->eSrc.x+rnd()*100.f-50.f;
											pos.y=prise->eSrc.y+100+rnd()*100.f-50.f;
											pos.z=prise->eSrc.z+rnd()*100.f-50.f;
											MakeCoolFx(&pos);
										}
									}

									pCSpellFX->lLightId=-1;
								}
								else
								{
									ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
									spells[i].longinfo=-2;
									spells[i].tolive=0;
								}
							}
							
						} else if(!arxtime.is_paused() && tim < 4000) {
						  if(rnd() > 0.95f) {
								CRiseDead *pRD = (CRiseDead*)pCSpellFX;
								Vec3f pos = pRD->eSrc;
								MakeCoolFx(&pos);
							}
						}
						
					}
			}
			break;
								
			//-----------------------------------------------------------------------------------------
			case SPELL_SLOW_DOWN:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render();
				}					
			}
			break;
			case SPELL_DISARM_TRAP:
			{
			}
			break;
			case SPELL_PARALYSE:
			break;
			case SPELL_CREATE_FIELD:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{				
					if (ValidIONum(spells[i].longinfo))
					{
						Entity * io=entities[spells[i].longinfo];
						CCreateField * ccf=(CCreateField *)pCSpellFX;
						io->pos = ccf->eSrc;

						if (IsAnyNPCInPlatform(io))
						{
							spells[i].tolive=0;
						}
					
						pCSpellFX->Update(FrameDiff);

						if (VisibleSphere(ccf->eSrc.x,ccf->eSrc.y-120.f,ccf->eSrc.z,400.f))					
							pCSpellFX->Render();
					}
				}					
			}
			break;

			//-----------------------------------------------------------------------------------------
			//*****************************************************************************************
			// LEVEL 7 SPELLS -----------------------------------------------------------------------------
			case SPELL_CONFUSE:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render();
				}
			}
			break;
			
			case SPELL_FIRE_FIELD: {
				
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					CFireField *pf = (CFireField *) pCSpellFX;
					pCSpellFX->Update(FrameDiff);
					
					if (spells[i].longinfo2==-1)
						spells[i].longinfo2=GetFreeDynLight();

					if (spells[i].longinfo2!=-1)
					{
						EERIE_LIGHT * el=&DynLight[spells[i].longinfo2];						
						
						el->pos.x = pf->pos.x;
						el->pos.y = pf->pos.y-120.f;
						el->pos.z = pf->pos.z;
						el->exist = 1;
						el->intensity = 4.6f;
						el->fallstart = 150.f+rnd()*30.f;
						el->fallend   = 290.f+rnd()*30.f;
						el->rgb.r = 1.f-rnd()*( 1.0f / 10 );
						el->rgb.g = 0.8f;
						el->rgb.b = 0.6f;
						el->duration = 600;
						el->extras=0;
					}
					
					if(VisibleSphere(pf->pos.x, pf->pos.y - 120.f, pf->pos.z, 350.f)) {
						
						pCSpellFX->Render();
						float fDiff = FrameDiff / 8.f;
						int nTime = checked_range_cast<int>(fDiff);
						
						for(long nn=0;nn<=nTime+1;nn++) {
							
							PARTICLE_DEF * pd = createParticle();
							if(!pd) {
								break;
							}
							
							float t = rnd() * (PI * 2.f) - PI;
							float ts = EEsin(t);
							float tc = EEcos(t);
							pd->ov = pf->pos + Vec3f(120.f * ts, 15.f * ts, 120.f * tc) * randomVec();
							pd->move = Vec3f(2.f - 4.f * rnd(), 1.f - 8.f * rnd(), 2.f - 4.f * rnd());
							pd->siz = 7.f;
							pd->tolive = Random::get(500, 1500);
							pd->tc = fire2;
							pd->special = ROTATING | MODULATE_ROTATION | FIRE_TO_SMOKE;
							pd->fparam = 0.1f - rnd() * 0.2f;
							pd->scale = Vec3f::repeat(-8.f);
							
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
			
			case SPELL_ICE_FIELD:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					
					CIceField *pf = (CIceField *) pCSpellFX;

					if (spells[i].longinfo2==-1)
						spells[i].longinfo2=GetFreeDynLight();

					if (spells[i].longinfo2!=-1)
					{
						EERIE_LIGHT * el=&DynLight[spells[i].longinfo2];						
						
						el->pos.x = pf->eSrc.x;
						el->pos.y = pf->eSrc.y-120.f;
						el->pos.z = pf->eSrc.z;
						el->exist = 1;
						el->intensity = 4.6f;
						el->fallstart = 150.f+rnd()*30.f;
						el->fallend   = 290.f+rnd()*30.f;
						el->rgb.r = 0.76f;
						el->rgb.g = 0.76f;
						el->rgb.b = 1.0f-rnd()*( 1.0f / 10 );
						el->duration = 600;
						el->extras=0;						
					}

 

					if (VisibleSphere(pf->eSrc.x,pf->eSrc.y-120.f,pf->eSrc.z,350.f))
					{
						pCSpellFX->Render();
					}
				}

				GRenderer->SetCulling(Renderer::CullNone);
			}
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_LIGHTNING_STRIKE:
				{
					CSpellFx *pCSpellFX = spells[i].pSpellFx;

					if (pCSpellFX)
					{
						pCSpellFX->Update(FrameDiff);
						pCSpellFX->Render();
					}
						}
			break;
			//-----------------------------------------------------------------------------------------
			//*****************************************************************************************
			// LEVEL 8 SPELLS -----------------------------------------------------------------------------
			case SPELL_ENCHANT_WEAPON:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render();
				}
			}
			case SPELL_EXPLOSION:
			{
				if (spells[i].longinfo2 == -1)
					spells[i].longinfo2=GetFreeDynLight();

				if (spells[i].longinfo2 != -1)
				{
					long id = spells[i].longinfo2;
					DynLight[id].rgb.r = 0.1f+rnd()*( 1.0f / 3 );;
					DynLight[id].rgb.g = 0.1f+rnd()*( 1.0f / 3 );;
					DynLight[id].rgb.b = 0.8f+rnd()*( 1.0f / 5 );;
					DynLight[id].duration=200;
				
					float rr,r2;
					Vec3f pos;
					
					float choice = rnd();
					if(choice > .8f) {
						long lvl = Random::get(9, 13);
						rr=radians(rnd()*360.f);
						r2=radians(rnd()*360.f);
						pos.x=DynLight[id].pos.x-EEsin(rr)*260;
						pos.y=DynLight[id].pos.y-EEsin(r2)*260;
						pos.z=DynLight[id].pos.z+EEcos(rr)*260;
						Color3f rgb(0.1f + rnd()*(1.f/3), 0.1f + rnd()*(1.0f/3), 0.8f + rnd()*(1.0f/5));
						LaunchFireballBoom(&pos, static_cast<float>(lvl), NULL, &rgb);
					} else if(choice > .6f) {
						rr=radians(rnd()*360.f);
						r2=radians(rnd()*360.f);
						pos.x=DynLight[id].pos.x-EEsin(rr)*260;
						pos.y=DynLight[id].pos.y-EEsin(r2)*260;
						pos.z=DynLight[id].pos.z+EEcos(rr)*260;
						MakeCoolFx(&pos);
					} else if(choice > 0.4f) {
						rr=radians(rnd()*360.f);
						r2=radians(rnd()*360.f);
						pos.x=DynLight[id].pos.x-EEsin(rr)*160;
						pos.y=DynLight[id].pos.y-EEsin(r2)*160;
						pos.z=DynLight[id].pos.z+EEcos(rr)*160;
						ARX_PARTICLES_Add_Smoke(&pos, 2, 20); // flag 1 = randomize pos
					}
				}
			}
			break;
			//-----------------------------------------------------------------------------------------
			//*****************************************************************************************	
			// LEVEL 9 SPELLS -------------------------------------------------------------------------
			//-----------------------------------------------------------------------------------------
			case SPELL_SUMMON_CREATURE:
			{
 

				if (!arxtime.is_paused())
				{

					if (float(arxtime)-(float)spells[i].timcreation<=4000)
					{
						if (rnd()>0.7f) 
						{
							CSummonCreature * pSummon = (CSummonCreature *)spells[i].pSpellFx;
							if(pSummon) {
								Vec3f pos = pSummon->eSrc;
								MakeCoolFx(&pos);
							}
						}

						CSpellFx *pCSpellFX = spells[i].pSpellFx;

						if (pCSpellFX)
						{
							pCSpellFX->Update(FrameDiff);
							pCSpellFX->Render();
						}	

						spells[i].longinfo=1;
						spells[i].longinfo2=-1;
					}
					else if (spells[i].longinfo)
					{
						if (spells[i].pSpellFx->lLightId > -1)
						{
							long id = spells[i].pSpellFx->lLightId;
							DynLight[id].exist = 0;
							spells[i].pSpellFx->lLightId=-1;
						}

						spells[i].longinfo=0;
						ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].caster_pos);
						CSummonCreature *pSummon;
						pSummon= (CSummonCreature *)spells[i].pSpellFx;

						if (pSummon)
						{			
							EERIE_CYLINDER phys;
							phys.height=-200;
							phys.radius=50;
							phys.origin=spells[i].target_pos;
									float anything = CheckAnythingInCylinder(&phys, NULL, CFLAG_JUST_TEST);

							if (EEfabs(anything)<30)
							{
							
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

								if (tokeep<0)
								{
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
								
								for (long j=0;j<3;j++)
								{
									pos.x=pSummon->eSrc.x+rnd()*100.f-50.f;
									pos.y=pSummon->eSrc.y+100+rnd()*100.f-50.f;
									pos.z=pSummon->eSrc.z+rnd()*100.f-50.f;
									MakeCoolFx(&pos);
								}

								if (tokeep==1)	spells[i].longinfo2 = io->index();
								else spells[i].longinfo2=-1;
							}
							}
					}
					}
					else if (spells[i].longinfo2<=0)
					{
						spells[i].tolive=0;
					}
				}
			}
			break;
			case SPELL_FAKE_SUMMON:
			{
 

					if (!arxtime.is_paused())
						if(rnd() > 0.7f) {
							CSummonCreature * pSummon = (CSummonCreature *)spells[i].pSpellFx;
							if(pSummon) {
								Vec3f pos = pSummon->eSrc;
								MakeCoolFx(&pos);
							}
						}

				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render();
				}					
			}
			break;
			//-----------------------------------------------------------------------------------------	
			
			case SPELL_INCINERATE:
			{
				if (ValidIONum(spells[i].caster))
				{
					ARX_SOUND_RefreshPosition(spells[i].snd_loop, &entities[spells[i].caster]->pos);
				}
			}
			break;
			case SPELL_NEGATE_MAGIC:
			{
				if (ValidIONum(spells[i].target))
					LaunchAntiMagicField(i);

				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render();
				}
			}
			break;
			case SPELL_MASS_PARALYSE:
			break;
			//*******************************************************************************************	
			// LEVEL 10 SPELLS -----------------------------------------------------------------------------
			//-----------------------------------------------------------------------------------------
			case SPELL_FREEZE_TIME:
			{
			}
			break;
			//-----------------------------------------------------------------------------------------				
			case SPELL_CONTROL_TARGET:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render();
				}
			}
			break;
			case SPELL_MASS_INCINERATE:
			{
				if (ValidIONum(spells[i].caster))
				{
					ARX_SOUND_RefreshPosition(spells[i].snd_loop, &entities[spells[i].caster]->pos);
				}
			}
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_MASS_LIGHTNING_STRIKE:
			{
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
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
				ARX_SOUND_RefreshPosition(spells[i].snd_loop, &position);
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
				
				if ((_gct>spells[i].tolive-1800) && (spells[i].siz==0))
				{
					spells[i].siz=1;
					ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, NULL, 0.8F + 0.4F * rnd());
				}

				if (spells[i].longinfo!=-1)
				{
					float fxx;

					if (_fx>0.2f)  fxx=1.f;
					else fxx=_fx*5.f;

					DynLight[spells[i].longinfo].intensity=1.3f+rnd()*1.f;
					DynLight[spells[i].longinfo].fallend=850.f;
					DynLight[spells[i].longinfo].fallstart=500.f;
					DynLight[spells[i].longinfo].rgb = Color3f::red * fxx;
				}
			}
		break;
		//-----------------------------------------------------------------------------------------				
		case SPELL_TELEPORT:
				{
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
				}
				break;				
				//-----------------------------------------------------------------------------------------
				case SPELL_MAGIC_SIGHT:

					if (spells[i].caster == 0)
					{
						ARX_SOUND_RefreshPosition(spells[i].snd_loop);

						if (subj.focal>IMPROVED_FOCAL) subj.focal-=DEC_FOCAL;
					}

				break;
				//-----------------------------------------------------------------------------------------
				case SPELL_TELEKINESIS:
				break;
				//-----------------------------------------------------------------------------------------
				case SPELL_INVISIBILITY:

					if (spells[i].target!=0)
					{
						if (!(entities[spells[i].target]->gameFlags & GFLAG_INVISIBILITY))
						{
							ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
							ARX_SPELLS_Fizzle(i);
						}
					}
				break;				
				//-----------------------------------------------------------------------------------------
				case SPELL_MANA_DRAIN:
					{
						
					if ( (cabal!=NULL) )
					{
						float refpos;
						float scaley;

						if (spells[i].caster==0) scaley=90.f;
						else scaley=EEfabs(entities[spells[i].caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;

						float mov1=EEsin((float)arxtime.get_last_frame_time()*( 1.0f / 800 ))*scaley;
						float mov=EEsin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;

						if ((mov1<scaley-10.f) && (mov>scaley-10.f)) ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD, &spells[i].caster_pos, 0.4F);

						if ((mov1>-scaley+10.f) && (mov<-scaley+10.f)) ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD, &spells[i].caster_pos, 0.4F);

						if (spells[i].caster==0)
						{
								cabalpos.x = player.pos.x; 
							cabalpos.y=player.pos.y+60.f-mov;
								cabalpos.z = player.pos.z; 
							refpos=player.pos.y+60.f;							
						}
						else
						{							
								cabalpos.x = entities[spells[i].caster]->pos.x; 
							cabalpos.y=entities[spells[i].caster]->pos.y-scaley-mov;
								cabalpos.z = entities[spells[i].caster]->pos.z; 
							refpos=entities[spells[i].caster]->pos.y-scaley;							
						}

						float Es=EEsin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + radians(scaley));

						if (spells[i].longinfo2!=-1)
						{
							DynLight[spells[i].longinfo2].pos.x = cabalpos.x;
							DynLight[spells[i].longinfo2].pos.y = refpos;
							DynLight[spells[i].longinfo2].pos.z = cabalpos.z;
							DynLight[spells[i].longinfo2].rgb.b=rnd()*0.2f+0.8f;
							DynLight[spells[i].longinfo2].fallstart=Es*1.5f;
						}

						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						GRenderer->SetRenderState(Renderer::DepthWrite, false);
						cabalangle.b=spells[i].fdata+(float)FrameDiff*0.1f;
						spells[i].fdata=cabalangle.b;

							cabalangle.g = 0.f; 
						
							cabalcolor.r = cabalcolor.g = 0.4f;
							cabalcolor.b = 0.8f;
						
						cabalscale.z=cabalscale.y=cabalscale.x=Es;				
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos-mov;						
							cabalcolor.r = cabalcolor.g = 0.2f;
							cabalcolor.b = 0.5f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos-mov;
							cabalcolor.r = cabalcolor.g = 0.1f;
							cabalcolor.b = 0.25f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos-mov;
							cabalcolor.r = cabalcolor.g = 0.f;
							cabalcolor.b = 0.15f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						cabalangle.b=-cabalangle.b;
						cabalpos.y=refpos-mov;
						cabalscale.x=cabalscale.y=cabalscale.z=Es;
							cabalcolor.r = cabalcolor.g = 0.f;
							cabalcolor.b = 0.15f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()+30.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.r = cabalcolor.g = 0.1f;
							cabalcolor.b = 0.25f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()+60.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.r = cabalcolor.g = 0.2f;
							cabalcolor.b = 0.5f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()+120.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.r = cabalcolor.g = 0.4f;
							cabalcolor.b = 0.8f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						cabalangle.b=-cabalangle.b;
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);		
						GRenderer->SetRenderState(Renderer::DepthWrite, true);	

						ARX_SOUND_RefreshPosition(spells[i].snd_loop, &cabalpos);
					}
					}
				break;				
				//-----------------------------------------------------------------------------------------
				case SPELL_LIFE_DRAIN:

					{
					if ( (cabal!=NULL) )
					{
						float refpos;
						float scaley;

						if (spells[i].caster==0) scaley=90.f;
						else scaley=EEfabs(entities[spells[i].caster]->physics.cyl.height*( 1.0f / 2 ))+30.f;

 
						float mov=EEsin((float)arxtime.get_frame_time()*( 1.0f / 800 ))*scaley;

						if (spells[i].caster==0)
						{
								cabalpos.x = player.pos.x; 
							cabalpos.y=player.pos.y+60.f-mov;
								cabalpos.z = player.pos.z; 
							refpos=player.pos.y+60.f;							
						}
						else
						{							
								cabalpos.x = entities[spells[i].caster]->pos.x; 
							cabalpos.y=entities[spells[i].caster]->pos.y-scaley-mov;
								cabalpos.z = entities[spells[i].caster]->pos.z; 
							refpos=entities[spells[i].caster]->pos.y-scaley;							
						}

						float Es=EEsin((float)arxtime.get_frame_time()*( 1.0f / 800 ) + radians(scaley));

						if (spells[i].longinfo2!=-1)
						{
							DynLight[spells[i].longinfo2].pos.x = cabalpos.x;
							DynLight[spells[i].longinfo2].pos.y = refpos;
							DynLight[spells[i].longinfo2].pos.z = cabalpos.z; 
							DynLight[spells[i].longinfo2].rgb.r=rnd()*0.2f+0.8f;
							DynLight[spells[i].longinfo2].fallstart=Es*1.5f;
						}

						GRenderer->SetCulling(Renderer::CullNone);
						GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
						GRenderer->SetRenderState(Renderer::AlphaBlending, true);
						GRenderer->SetRenderState(Renderer::DepthWrite, false);
						cabalangle.b=spells[i].fdata+(float)FrameDiff*0.1f;
						spells[i].fdata=cabalangle.b;
							cabalangle.g = 0.f;
							cabalcolor.r = 0.8f;
							cabalcolor.g = 0.f;
							cabalcolor.b = 0.f;
						cabalscale.z=cabalscale.y=cabalscale.x=Es;				
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()-30.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos-mov;						
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.5f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()-60.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos-mov;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.25f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()-120.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos-mov;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.15f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						cabalangle.b=-cabalangle.b;
						cabalpos.y=refpos-mov;
						cabalscale.x=cabalscale.y=cabalscale.z=Es;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.15f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()+30.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.25f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()+60.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.5f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(arxtime.get_frame_time()+120.f)*( 1.0f / 800 ))*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.8f;
						DrawEERIEObjEx(cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						cabalangle.b=-cabalangle.b;
						GRenderer->SetRenderState(Renderer::AlphaBlending, false);		
						GRenderer->SetRenderState(Renderer::DepthWrite, true);	

						ARX_SOUND_RefreshPosition(spells[i].snd_loop, &cabalpos);
						}
					}
				break;
				
				case SPELL_FLYING_EYE: {
					
						eyeball.floating = EEsin(spells[i].lastupdate-spells[i].timcreation * 0.001f);
						eyeball.floating *= 10.f;
						
						if(spells[i].lastupdate-spells[i].timcreation <= 3000) {
							eyeball.exist = spells[i].lastupdate - spells[i].timcreation * (1.0f / 30);
							eyeball.size = Vec3f::repeat(1.f - float(eyeball.exist) * 0.01f);
							eyeball.angle.b += framediff3 * 0.6f;
						} else {
							eyeball.exist = 2;
						}
						
						spells[i].lastupdate=tim;
					break;
				}
				
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void TryToCastSpell(Entity * io, Spell spellid, long level, long target, SpellcastFlags flags, long duration)
{
	if (!io || io->spellcast_data.castingspell != SPELL_NONE) return;

	if (!(flags & SPELLCAST_FLAG_NOMANA)
			&& (io->ioflags & IO_NPC) && (io->_npcdata->mana<=0.f))
		return;

	unsigned long i(0);

	for (; i < SPELL_COUNT; i++)
		if (spellicons[i].spellid == spellid) break;

	if ( i >= SPELL_COUNT) return; // not an existing spell...

	for (unsigned long j(0); j < 4; j++) io->spellcast_data.symb[j] = RUNE_NONE;

	// checks for symbol drawing...
	if (!(flags & SPELLCAST_FLAG_NOANIM) && io->ioflags & IO_NPC)
	{
		ANIM_USE *ause1 = &io->animlayer[1];

		AcquireLastAnim(io);
		FinishAnim(io, ause1->cur_anim);
		ANIM_Set(ause1, io->anims[ANIM_CAST_START]);

		for (unsigned long j(0); j < 4; j++)
			io->spellcast_data.symb[j] = spellicons[i].symbols[j];
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

static void ApplySPWep() {
	
	if(!sp_wep) {
		
		ARX_SPSound();
		
		res::path cls = "graph/obj3d/interactive/items/weapons/sword_mx/sword_mx";
		Entity * ioo = AddItem(cls);
		if(ioo) {
			
			sp_wep = 1;
			MakeCoolFx(&player.pos);
			MakeCoolFx(&player.pos);
			ioo->scriptload = 1;
			SendInitScriptEvent(ioo);
			
			giveToPlayer(ioo);
			
			MakeSpCol();
			strcpy(sp_max_ch,"!!!_Grosbillite_!!!");
			sp_max_nb=strlen(sp_max_ch);
			sp_max_start=arxtime.get_updated();
		}
	}
}

void MakeSpCol() {
	
	ARX_SPSound();
	
	for(long i = 0; i < 64; i++) {
		sp_max_y[i] = 0;
	}
	
	sp_max_col[0] = Color::fromRGBA(0x00FF0000);
	sp_max_col[1] = Color::fromRGBA(0x0000FF00);
	sp_max_col[2] = Color::fromRGBA(0x000000FF);
	
	sp_max_col[3] = Color::fromRGBA(0x00FFFF00);
	sp_max_col[4] = Color::fromRGBA(0x00FF00FF);
	sp_max_col[5] = Color::fromRGBA(0x0000FFFF);
	
	for(size_t i = 6; i < 24; i++) {
		sp_max_col[i] = sp_max_col[i - 6];
	}
	
	for(size_t i = 24; i < 27; i++) {
		sp_max_col[i] = sp_max_col[i - 3];
	}
	
	for(size_t i = 27; i < 33; i++) {
		sp_max_col[i] = sp_max_col[i - 9];
	}
	
}

static void ApplyCurSOS() {
	MakeSpCol();
	ARX_MINIMAP_Reveal();
	strcpy(sp_max_ch,"!!!_Temple of Elemental Lavis_!!!");
	sp_max_nb=strlen(sp_max_ch);
	sp_max_start=arxtime.get_updated();
}

static void ApplySPBow() {
	
	ARX_SPSound();
	
	const char * cls = "graph/obj3d/interactive/items/weapons/bow_mx/bow_mx";
	Entity * ioo = AddItem(cls);
	if(ioo) {
		
		MakeCoolFx(&player.pos);
		MakeCoolFx(&player.pos);
		
		ioo->scriptload = 1;
		SendInitScriptEvent(ioo);
		
		giveToPlayer(ioo);
		
		MakeSpCol();
		strcpy(sp_max_ch,"!!!_Bow to Samy & Anne_!!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
	}
}

static void ApplySPArm() {
	ARX_SPSound();
	
	res::path cls;
	switch (sp_arm) {
		case 0:
			cls = "graph/obj3d/interactive/items/armor/helmet_plate_cm/helmet_plate_cm";
		break;
		case 1:
			cls = "graph/obj3d/interactive/items/armor/legging_plate_cm/legging_plate_cm";
		break;
		case 2:
			cls = "graph/obj3d/interactive/items/armor/chest_plate_cm/chest_plate_cm";
		break;
		default:
			return;
		break;
	}
	
	Entity * ioo = AddItem(cls);
	if(ioo) {
		
		sp_wep = 1;
		MakeCoolFx(&player.pos);
		MakeCoolFx(&player.pos);
		ioo->scriptload = 1;
		SendInitScriptEvent(ioo);
		
		giveToPlayer(ioo);
		
		MakeSpCol();
		strcpy(sp_max_ch,"!! Toi aussi cherches les Cheats !!");

		switch (sp_arm)
		{
		case 0:
			strcpy(sp_max_ch,"------ZoliChapo------");
		break;
		case 1:
			strcpy(sp_max_ch,"-----TiteBottine-----");
		break;
		case 2:
			strcpy(sp_max_ch,"-----Roooo-La-La-----");
		break;
		default:
			return;
		break;
		}

		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
	}

	sp_arm++;
}

long SPECIAL_PNUX;
static void ApplyCurPNux() {
	
	MakeSpCol();
	strcpy(sp_max_ch,"! PhilNux & Gluonne !");
	sp_max_nb=strlen(sp_max_ch);
	
	SPECIAL_PNUX = (SPECIAL_PNUX + 1) % 3;
	
	// TODO-RENDERING: Create a post-processing effect for that cheat... see original source...
	
	cur_pnux=0;
	sp_max_start=arxtime.get_updated();
}

static void ApplyPasswall() {
	MakeSpCol();
	strcpy(sp_max_ch,"!!! PassWall !!!");
	sp_max_nb=strlen(sp_max_ch);
	sp_max_start=arxtime.get_updated();

	if (USE_PLAYERCOLLISIONS)
		USE_PLAYERCOLLISIONS=0;
	else
		USE_PLAYERCOLLISIONS=1;
}

static void ApplySPRf() {
	if(cur_rf == 3) {
		MakeSpCol();
		strcpy(sp_max_ch,"!!! RaFMode !!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
	}
}

static void ApplyCurMr() {
	if(cur_mr == 3) {
		MakeSpCol();
		strcpy(sp_max_ch,"!!! Marianna !!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
	}
}

static void ApplySPuw() {
	uw_mode_pos=0;
	uw_mode=~uw_mode;
	ARX_SOUND_PlayCinematic("menestrel_uw2", true);
	MakeCoolFx(&player.pos);
	if(uw_mode) {
		MakeSpCol();
		strcpy(sp_max_ch,"~-__-~~-__.U.W.__-~~-__-~");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();
	}
}

static void ApplySPMax() {
	
	MakeCoolFx(&player.pos);
	sp_max=~sp_max;

	if (sp_max)
	{
		MakeSpCol();
		strcpy(sp_max_ch,"!!!_FaNt0mAc1e_!!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=arxtime.get_updated();

			player.skin=4;

			ARX_EQUIPMENT_RecreatePlayerMesh();
		
		ARX_PLAYER_Rune_Add_All();
		std::string text = "!!!!!!! FanTomAciE !!!!!!!";
		ARX_SPEECH_Add(text);
		player.Attribute_Redistribute+=10;
		player.Skill_Redistribute+=50;
		player.level=std::max((int)player.level,10);
		player.xp=GetXPforLevel(10);
	}
	else
	{
		TextureContainer * tcm=TextureContainer::Load("graph/obj3d/textures/npc_human_cm_hero_head");

		if (tcm)
		{
			delete tcm;
			player.heads[0]=TextureContainer::Load("graph/obj3d/textures/npc_human_base_hero_head");
			player.heads[1]=TextureContainer::Load("graph/obj3d/textures/npc_human_base_hero2_head");
			player.heads[2]=TextureContainer::Load("graph/obj3d/textures/npc_human_base_hero3_head");
			ARX_EQUIPMENT_RecreatePlayerMesh();
		}
	}	
}
