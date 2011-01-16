/*
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
///////////////////////////////////////////////////////////////////////////////
//
// ARX_Spells.cpp
// ARX Spells Management & Projectiles
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//
///////////////////////////////////////////////////////////////////////////////
#include <ARX_Spells.h>

#include <HERMESMain.h>
#include <EERIEDraw.h>
#include <EERIEMath.h>
#include <EERIELight.h>
#include <EERIEObject.h>

#include "danae.h"
#include <ARX_Collisions.h>
#include <ARX_Damages.h>
#include <ARX_Equipment.h>
#include <ARX_Fogs.h>
#include <ARX_Input.h>
#include <ARX_Interface.h>
#include <ARX_NPC.h>
#include "ARX_Menu2.h"
#include "ARX_Minimap.h"
#include "ARX_Scene.h"
#include <ARX_Particles.h>
#include <ARX_Sound.h>
#include "ARX_SpellFX.h"
#include <ARX_Speech.h>
#include <ARX_Menu.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
void MakeSpCol();
extern long WILLRETURNTOCOMBATMODE;
extern CMenuConfig *pMenuConfig;
extern long TRUE_PLAYER_MOUSELOOK_ON;
long passwall=0;
long WILLRETURNTOFREELOOK = 0;
long GLOBAL_MAGIC_MODE=1;
bool bPrecastSpell = false;
extern char LAST_FAILED_SEQUENCE[128];
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
void ApplyPasswall();
void ApplySPArm();
void ApplySPuw();
void ApplySPRf();
void ApplySPMax();
void ApplySPWep();
void ApplySPBow();
void ApplyCurPNux();
void ApplyCurMr();
void ApplyCurSOS(); 
 
 
extern long FistParticles;
extern long ParticleCount;
extern long sp_max;
short uw_mode=0;
short uw_mode_pos=0;
extern long MAGICMODE;
extern INTERACTIVE_OBJ * CURRENT_TORCH;
extern float GLOBAL_SLOWDOWN;
extern long BH_MODE;
extern void EERIE_OBJECT_MakeBH(EERIE_3DOBJ * obj);
extern void ARX_SPSound();
extern float sp_max_y[64];
extern COLORREF sp_max_col[64];
extern char	sp_max_ch[64];
extern long sp_max_nb;
long cur_mega=0;
float sp_max_start = 0;
long sp_wep=0;

extern long INTERNATIONAL_MODE;
extern bool bRenderInCursorMode;

bool bOldLookToggle;
extern float SLID_START;

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
		sp_max_start=ARX_TIME_Get();
			}
}
typedef struct
{
	short SlotDir; 
} Scan;

typedef struct
{
	unsigned long	starttime;
	EERIE_3D		lastpos;
	short			lasttim;
	short			duration;
	short			symbol;
	char			sequence[32];
	char			cPosStartX;
	char			cPosStartY;
} SYMBOL_DRAW;
extern long NO_TEXT_AT_ALL;

extern bool FrustrumsClipSphere(EERIE_FRUSTRUM_DATA * frustrums,EERIE_SPHERE * sphere);
extern bool bGToggleCombatModeWithKey;
void ARX_INTERFACE_Combat_Mode(long i);

float ARX_SPELLS_GetManaCost(long _lNumSpell,long _lNumSpellTab);
//-----------------------------------------------------------------------------
///////////////Spell Interpretation
SPELL spells[MAX_SPELLS];
short ARX_FLARES_broken(1);
long CurrSlot(1);
long LastSlot(0);
long CurrPoint(0);
long cur_mx=0;
long cur_pnux=0;
long cur_pom=0;
long cur_rf=0;
long cur_mr=0;
long cur_sm=0;
long cur_bh=0;

float TELEPORT(0.0F);
float LASTTELEPORT(0.0F);
long snip=0;
EERIE_S2D Lm;

static const long MAX_POINTS(200);
static EERIE_S2D plist[MAX_POINTS];
static char SpellMoves[MAX_POINTS];
long SpellSymbol[MAX_SPELL_SYMBOLS];
long CurrSpellSymbol=0;

Scan spell[MAX_SLOT + 1];

long lMaxSymbolDrawSizeX;
long lMaxSymbolDrawSizeY;

unsigned char ucFlick=0;

BOOL GetSpellPosition(EERIE_3D * pos,long i)
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
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
			}

		break;
		//------------------------------------------------------------------------------------------------
		case SPELL_LOWER_ARMOR:// Launching LOWER_ARMOR

			if (ValidIONum(spells[i].target))
			{
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
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
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
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
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
			}

		break;
		//-----------------------------------------------------------------------------------------------
		case SPELL_DISPELL_FIELD:// Launching DISPELL_FIELD
		break;
		//-----------------------------------------------------------------------------------------------
		case SPELL_FIRE_PROTECTION:// Launching FIRE_PROTECTION

			if (ValidIONum(spells[i].target))
			{
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
			}

		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_COLD_PROTECTION:// Launching COLD_PROTECTION

			if (ValidIONum(spells[i].target))
			{
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
			}

		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_TELEKINESIS:// Launching TELEKINESIS
		break;
		//-----------------------------------------------------------------------------------------------
		case SPELL_CURSE:// Launching CURSE

			if (ValidIONum(spells[i].target))
			{
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
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
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
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
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
			}

		break;
		//****************************************************************************************
		// LEVEL 7 SPELLS -----------------------------------------------------------------------------
		case SPELL_FLYING_EYE:
		{	
			Vector_Copy(pos,&eyeball.pos);
			return TRUE;		
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_FIRE_FIELD:
			CSpellFx *pCSpellFX;
			pCSpellFX = spells[i].pSpellFx;

			if (pCSpellFX)
			{
				CFireField *pFireField = (CFireField *) pCSpellFX;
					
				Vector_Copy(pos,&pFireField->pos);
				return TRUE;
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
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
			}

		break;
		//*********************************************************************************
		// LEVEL 8 SPELLS -----------------------------------------------------------------------------
		case SPELL_INVISIBILITY:

			if (ValidIONum(spells[i].target))
			{
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
			}

		break;
		//----------------------------------------------------------------------------
		case SPELL_MANA_DRAIN:				

			if (ValidIONum(spells[i].target))
			{
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
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
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
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
				Vector_Copy(pos,&inter.iobj[spells[i].target]->pos);
				return TRUE;
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

	}			

	if (ValidIONum(spells[i].caster))
	{
		Vector_Copy(pos,&inter.iobj[spells[i].caster]->pos);
		return TRUE;
	}

	return FALSE;
}

void LaunchAntiMagicField(EERIE_3D * pos,long ident)
{
	for (long n=0;n<MAX_SPELLS;n++)
	{
		if (	(spells[n].exist) 
			&&	(spells[ident].caster_level >= spells[n].caster_level)
			&&	(n!=ident)	)
		{
			EERIE_3D pos; 
			GetSpellPosition(&pos,n);

			if (EEDistance3D(&pos,&inter.iobj[spells[ident].caster]->pos)<600.f)
			{
				if (spells[n].type==SPELL_CREATE_FIELD)
				{
					if ((spells[ident].caster==0) && (spells[n].caster==0))				
						spells[n].tolive=0;
				}
				else 	spells[n].tolive=0;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_AddSpellOn(const long &caster, const long &spell)
{
	if (caster < 0 ||  spell < 0 || !inter.iobj[caster]) return;

	INTERACTIVE_OBJ *io = inter.iobj[caster];
	void *ptr;

	ptr = realloc(io->spells_on, sizeof(long) * (io->nb_spells_on + 1));

	if (!ptr) return;

	io->spells_on = (long *)ptr;
	io->spells_on[io->nb_spells_on] = spell;
	io->nb_spells_on++;
}

//-----------------------------------------------------------------------------
long ARX_SPELLS_GetSpellOn(const INTERACTIVE_OBJ *io, const long &spellid)
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

	INTERACTIVE_OBJ *io = inter.iobj[caster];

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
void ARX_SPELLS_RemoveMultiSpellOn(long spell_id)
{
	for (long i=0;i<inter.nbmax;i++)
	{
		ARX_SPELLS_RemoveSpellOn(i,spells[spell_id].type);
	}
}
//-----------------------------------------------------------------------------
void ARX_SPELLS_RemoveAllSpellsOn(INTERACTIVE_OBJ *io)
{
	free(io->spells_on), io->spells_on = NULL, io->nb_spells_on = 0;
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_RequestSymbolDraw(INTERACTIVE_OBJ *io, const char *name, const float &duration)
{
	long symb(-1);
	char sequence[32];
	int iPosX = 0;
	int iPosY = 0;

	if (!strcmp(name, "AAM"))              symb = SYMBOL_AAM,			iPosX = 0,	iPosY = 2,	strcpy(sequence, "6666");
	else if (!strcmp(name, "CETRIUS"))     symb = SYMBOL_CETRIUS,		iPosX = 0,	iPosY = 1,	strcpy(sequence, "33388886666");
	else if (!strcmp(name, "COMUNICATUM")) symb = SYMBOL_COMUNICATUM,	iPosX = 0,	iPosY = 0,	strcpy(sequence, "6666622244442226666");
	else if (!strcmp(name, "COSUM"))       symb = SYMBOL_COSUM,			iPosX = 0,	iPosY = 2,	strcpy(sequence, "66666222244448888");
	else if (!strcmp(name, "FOLGORA"))     symb = SYMBOL_FOLGORA,		iPosX = 0,	iPosY = 3,	strcpy(sequence, "99993333");
	else if (!strcmp(name, "FRIDD"))       symb = SYMBOL_FRIDD,			iPosX = 0,	iPosY = 4,	strcpy(sequence, "888886662222");
	else if (!strcmp(name, "KAOM"))        symb = SYMBOL_KAOM,			iPosX = 3,	iPosY = 0,	strcpy(sequence, "44122366");
	else if (!strcmp(name, "MEGA"))        symb = SYMBOL_MEGA,			iPosX = 2,	iPosY = 4,	strcpy(sequence, "88888");
	else if (!strcmp(name, "MORTE"))       symb = SYMBOL_MORTE,			iPosX = 0,	iPosY = 2,	strcpy(sequence, "66666222");
	else if (!strcmp(name, "MOVIS"))       symb = SYMBOL_MOVIS,			iPosX = 0,	iPosY = 0,	strcpy(sequence, "666611116666");
	else if (!strcmp(name, "NHI"))         symb = SYMBOL_NHI,			iPosX = 4,	iPosY = 2,	strcpy(sequence, "4444");
	else if (!strcmp(name, "RHAA"))        symb = SYMBOL_RHAA,			iPosX = 2,	iPosY = 0,	strcpy(sequence, "22222");
	else if (!strcmp(name, "SPACIUM"))     symb = SYMBOL_SPACIUM,		iPosX = 4,	iPosY = 0,	strcpy(sequence, "44444222266688");
	else if (!strcmp(name, "STREGUM"))     symb = SYMBOL_STREGUM,		iPosX = 0,	iPosY = 4,	strcpy(sequence, "8888833338888");
	else if (!strcmp(name, "TAAR"))        symb = SYMBOL_TAAR,			iPosX = 0,	iPosY = 1,	strcpy(sequence, "666222666");
	else if (!strcmp(name, "TEMPUS"))      symb = SYMBOL_TEMPUS,		iPosX = 0,	iPosY = 4,	strcpy(sequence, "88886662226668866");
	else if (!strcmp(name, "TERA"))        symb = SYMBOL_TERA,			iPosX = 0,	iPosY = 3,	strcpy(sequence, "99922266");
	else if (!strcmp(name, "VISTA"))       symb = SYMBOL_VISTA,			iPosX = 1,	iPosY = 0,	strcpy(sequence, "333111");
	else if (!strcmp(name, "VITAE"))       symb = SYMBOL_VITAE,			iPosX = 0,	iPosY = 2,	strcpy(sequence, "66666888");
	else if (!strcmp(name, "YOK"))         symb = SYMBOL_YOK,			iPosX = 0,	iPosY = 0,	strcpy(sequence, "222226666888");
	else if (!strcmp(name, "AKBAA"))       symb = SYMBOL_AKBAA,			iPosX = 0,	iPosY = 0,	strcpy(sequence, "22666772222");

	if (symb == -1) return;

	io->symboldraw = realloc(io->symboldraw, sizeof(SYMBOL_DRAW));

	if (!io->symboldraw) return;

	SYMBOL_DRAW *sd = (SYMBOL_DRAW *)io->symboldraw;

	sd->duration = duration < 1.0F ? 1 : (short)(long)duration;
	strcpy(sd->sequence, sequence);

	sd->starttime = ARXTimeUL();
	sd->lasttim = 0;
	sd->symbol = (short)symb;
	sd->lastpos.x = io->pos.x - EEsin(DEG2RAD(MAKEANGLE(io->angle.b - 45.0F + iPosX*2))) * 60.0F;
	sd->lastpos.y = io->pos.y - 120.0F - iPosY*5;
	sd->lastpos.z = io->pos.z + EEcos(DEG2RAD(MAKEANGLE(io->angle.b - 45.0F + iPosX*2))) * 60.0F;
	

	ARX_CHECK_CHAR(iPosX);
	ARX_CHECK_CHAR(iPosY);
	
	sd->cPosStartX = ARX_CLEAN_WARN_CAST_CHAR(iPosX);
	sd->cPosStartY = ARX_CLEAN_WARN_CAST_CHAR(iPosY); 	

	
	io->GameFlags &= ~GFLAG_INVISIBILITY;
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_RequestSymbolDraw2(INTERACTIVE_OBJ *io, const long &symb, const float &duration)
{
	char sequence[32];
	int iPosX = 0;
	int iPosY = 0;
	long _symb;

	switch (symb)
	{
		case RUNE_AAM   :
			_symb = SYMBOL_AAM;
			iPosX = 0;
			iPosY = 2;
			strcpy(sequence, "6666");
			break;
		case RUNE_CETRIUS:
			_symb = SYMBOL_CETRIUS,		iPosX = 0,	iPosY = 1,	strcpy(sequence, "33388886666");
			break;
		case RUNE_COMUNICATUM:
			_symb = SYMBOL_COMUNICATUM,	iPosX = 0,	iPosY = 0,	strcpy(sequence, "6666622244442226666");
			break;
		case RUNE_COSUM:
			_symb = SYMBOL_COSUM,		iPosX = 0,	iPosY = 2,	strcpy(sequence, "66666222244448888");
			break;
		case RUNE_FOLGORA:
			_symb = SYMBOL_FOLGORA,		iPosX = 0,	iPosY = 3,	strcpy(sequence, "99993333");
			break;
		case RUNE_FRIDD:
			_symb = SYMBOL_FRIDD,		iPosX = 0,	iPosY = 4,	strcpy(sequence, "888886662222");
			break;
		case RUNE_KAOM:
			_symb = SYMBOL_KAOM,		iPosX = 3,	iPosY = 0,	strcpy(sequence, "44122366");
			break;
		case RUNE_MEGA:
			_symb = SYMBOL_MEGA,		iPosX = 2,	iPosY = 4,	strcpy(sequence, "88888");
			break;
		case RUNE_MORTE:
			_symb = SYMBOL_MORTE,		iPosX = 0,	iPosY = 2,	strcpy(sequence, "66666222");
			break;
		case RUNE_MOVIS:
			_symb = SYMBOL_MOVIS,		iPosX = 0,	iPosY = 0,	strcpy(sequence, "666611116666");
			break;
		case RUNE_NHI:
			_symb = SYMBOL_NHI,			iPosX = 4,	iPosY = 2,	strcpy(sequence, "4444");
			break;
		case RUNE_RHAA:
			_symb = SYMBOL_RHAA,		iPosX = 2,	iPosY = 0,	strcpy(sequence, "22222");
			break;
		case RUNE_SPACIUM:
			_symb = SYMBOL_SPACIUM,		iPosX = 4,	iPosY = 0,	strcpy(sequence, "44444222266688");
			break;
		case RUNE_STREGUM:
			_symb = SYMBOL_STREGUM,		iPosX = 0,	iPosY = 4,	strcpy(sequence, "8888833338888");
			break;
		case RUNE_TAAR:
			_symb = SYMBOL_TAAR,		iPosX = 0,	iPosY = 1,	strcpy(sequence, "666222666");
			break;
		case RUNE_TEMPUS:
			_symb = SYMBOL_TEMPUS,		iPosX = 0,	iPosY = 4,	strcpy(sequence, "88886662226668866");
			break;
		case RUNE_TERA:
			_symb = SYMBOL_TERA,		iPosX = 0,	iPosY = 3,	strcpy(sequence, "99922266");
			break;
		case RUNE_VISTA:
			_symb = SYMBOL_VISTA,		iPosX = 1,	iPosY = 0,	strcpy(sequence, "333111");
			break;
		case RUNE_VITAE:
			_symb = SYMBOL_VITAE,		iPosX = 0,	iPosY = 2,	strcpy(sequence, "66666888");
			break;
		case RUNE_YOK:
			_symb = SYMBOL_YOK,			iPosX = 0,	iPosY = 0,	strcpy(sequence, "222226666888");
			break;
		default:
			return;
	}

	void *ptr;
	ptr = realloc(io->symboldraw, sizeof(SYMBOL_DRAW));

	if (!ptr) return;

	io->symboldraw = ptr;

	SYMBOL_DRAW *sd = (SYMBOL_DRAW *)io->symboldraw;
	sd->duration = duration < 1.0F ? 1 : (short)(long)duration;
	strcpy(sd->sequence, sequence);
	sd->starttime = ARXTimeUL();
	sd->lasttim = 0;
	sd->symbol = (short)_symb;
	
	sd->lastpos.x = io->pos.x - EEsin(DEG2RAD(MAKEANGLE(io->angle.b - 45.0F + iPosX*2))) * 60.0F;
	sd->lastpos.y = io->pos.y - 120.0F - iPosY*5;
	sd->lastpos.z = io->pos.z + EEcos(DEG2RAD(MAKEANGLE(io->angle.b - 45.0F + iPosX*2))) * 60.0F;
	

	ARX_CHECK_CHAR(iPosX);
	ARX_CHECK_CHAR(iPosY);

	sd->cPosStartX = ARX_CLEAN_WARN_CAST_CHAR(iPosX);
	sd->cPosStartY = ARX_CLEAN_WARN_CAST_CHAR(iPosY); 	


	io->GameFlags &= ~GFLAG_INVISIBILITY;

}

//-----------------------------------------------------------------------------
void ARX_SPELLS_RequestSymbolDraw3(const char *_pcName,char *_pcRes)
{
	if		(!strcmp(_pcName, "AAM"))		strcpy(_pcRes, "6666");
	else if (!strcmp(_pcName, "CETRIUS"))	strcpy(_pcRes, "33388886666");
	else if (!strcmp(_pcName, "COMUNICATUM")) 	strcpy(_pcRes, "6666622244442226666");
	else if (!strcmp(_pcName, "COSUM"))     strcpy(_pcRes, "66666222244448888");
	else if (!strcmp(_pcName, "FOLGORA"))   strcpy(_pcRes, "99993333");
	else if (!strcmp(_pcName, "FRIDD"))		strcpy(_pcRes, "888886662222");
	else if (!strcmp(_pcName, "KAOM"))		strcpy(_pcRes, "44122366");
	else if (!strcmp(_pcName, "MEGA"))		strcpy(_pcRes, "88888");
	else if (!strcmp(_pcName, "MORTE"))		strcpy(_pcRes, "66666222");
	else if (!strcmp(_pcName, "MOVIS"))		strcpy(_pcRes, "666611116666");
	else if (!strcmp(_pcName, "NHI"))		strcpy(_pcRes, "4444");
	else if (!strcmp(_pcName, "RHAA"))		strcpy(_pcRes, "22222");
	else if (!strcmp(_pcName, "SPACIUM"))	strcpy(_pcRes, "44444222266688");
	else if (!strcmp(_pcName, "STREGUM"))	strcpy(_pcRes, "8888833338888");
	else if (!strcmp(_pcName, "TAAR"))		strcpy(_pcRes, "666222666");
	else if (!strcmp(_pcName, "TEMPUS"))	strcpy(_pcRes, "88886662226668866");
	else if (!strcmp(_pcName, "TERA"))		strcpy(_pcRes, "99922266");
	else if (!strcmp(_pcName, "VISTA"))		strcpy(_pcRes, "333111");
	else if (!strcmp(_pcName, "VITAE"))		strcpy(_pcRes, "66666888");
	else if (!strcmp(_pcName, "YOK"))		strcpy(_pcRes, "222226666888");
	else if (!strcmp(_pcName, "AKBAA"))		strcpy(_pcRes, "22666772222");
}

#define OFFSET_X 8*2//0
#define OFFSET_Y 6*2//0

//-----------------------------------------------------------------------------
void GetSymbVector(char c,EERIE_S2D * vec)
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

//-----------------------------------------------------------------------------
BOOL MakeSpellName(char * spell, const long &num)
{
	switch (num)
	{
		// Level 1
		case SPELL_MAGIC_SIGHT           :
			strcpy(spell, "MAGIC_SIGHT");
			break;
		case SPELL_MAGIC_MISSILE         :
			strcpy(spell, "MAGIC_MISSILE");
			break;
		case SPELL_IGNIT                 :
			strcpy(spell, "IGNIT");
			break;
		case SPELL_DOUSE                 :
			strcpy(spell, "DOUSE");
			break;
		case SPELL_ACTIVATE_PORTAL       :
			strcpy(spell, "ACTIVATE_PORTAL");
			break;

		// Level 2
		case SPELL_HEAL                  :
			strcpy(spell, "HEAL");
			break;
		case SPELL_DETECT_TRAP           :
			strcpy(spell, "DETECT_TRAP");
			break;
		case SPELL_ARMOR                 :
			strcpy(spell, "ARMOR");
			break;
		case SPELL_LOWER_ARMOR           :
			strcpy(spell, "LOWER_ARMOR");
			break;
		case SPELL_HARM                  :
			strcpy(spell, "HARM");
			break;

		// Level 3
		case SPELL_SPEED                 :
			strcpy(spell, "SPEED");
			break;
		case SPELL_DISPELL_ILLUSION      :
			strcpy(spell, "DISPELL_ILLUSION");
			break;
		case SPELL_FIREBALL              :
			strcpy(spell, "FIREBALL");
			break;
		case SPELL_CREATE_FOOD           :
			strcpy(spell, "CREATE_FOOD");
			break;
		case SPELL_ICE_PROJECTILE        :
			strcpy(spell, "ICE_PROJECTILE");
			break;

		// Level 4 
		case SPELL_BLESS                 :
			strcpy(spell, "BLESS");
			break;
		case SPELL_DISPELL_FIELD         :
			strcpy(spell, "DISPELL_FIELD");
			break;
		case SPELL_FIRE_PROTECTION       :
			strcpy(spell, "FIRE_PROTECTION");
			break;
		case SPELL_TELEKINESIS           :
			strcpy(spell, "TELEKINESIS");
			break;
		case SPELL_CURSE                 :
			strcpy(spell, "CURSE");
			break;
		case SPELL_COLD_PROTECTION       :
			strcpy(spell, "COLD_PROTECTION");
			break;

		// Level 5 
		case SPELL_RUNE_OF_GUARDING      :
			strcpy(spell, "RUNE_OF_GUARDING");
			break;
		case SPELL_LEVITATE              :
			strcpy(spell, "LEVITATE");
			break;
		case SPELL_CURE_POISON           :
			strcpy(spell, "CURE_POISON");
			break;
		case SPELL_REPEL_UNDEAD          :
			strcpy(spell, "REPEL_UNDEAD");
			break;
		case SPELL_POISON_PROJECTILE     :
			strcpy(spell, "POISON_PROJECTILE");
			break;

		// Level 6 
		case SPELL_RISE_DEAD             :
			strcpy(spell, "RAISE_DEAD");
			break;
		case SPELL_PARALYSE              :
			strcpy(spell, "PARALYSE");
			break;
		case SPELL_CREATE_FIELD          :
			strcpy(spell, "CREATE_FIELD");
			break;
		case SPELL_DISARM_TRAP           :
			strcpy(spell, "DISARM_TRAP");
			break;
		case SPELL_SLOW_DOWN             :
			strcpy(spell, "SLOWDOWN");
			break;

		// Level 7  
		case SPELL_FLYING_EYE            :
			strcpy(spell, "FLYING_EYE");
			break;
		case SPELL_FIRE_FIELD            :
			strcpy(spell, "FIRE_FIELD");
			break;
		case SPELL_ICE_FIELD             :
			strcpy(spell, "ICE_FIELD");
			break;
		case SPELL_LIGHTNING_STRIKE      :
			strcpy(spell, "LIGHTNING_STRIKE");
			break;
		case SPELL_CONFUSE               :
			strcpy(spell, "CONFUSE");
			break;

		// Level 8
		case SPELL_INVISIBILITY          :
			strcpy(spell, "INVISIBILITY");
			break;
		case SPELL_MANA_DRAIN            :
			strcpy(spell, "MANA_DRAIN");
			break;
		case SPELL_EXPLOSION             :
			strcpy(spell, "EXPLOSION");
			break;
		case SPELL_ENCHANT_WEAPON        :
			strcpy(spell, "ENCHANT_WEAPON");
			break;
		case SPELL_LIFE_DRAIN            :
			strcpy(spell, "LIFE_DRAIN");
			break;

		// Level 9
		case SPELL_SUMMON_CREATURE       :
			strcpy(spell, "SUMMON_CREATURE");
			break;
		case SPELL_FAKE_SUMMON		     :
			strcpy(spell, "FAKE_SUMMON");
			break;
		case SPELL_NEGATE_MAGIC          :
			strcpy(spell, "NEGATE_MAGIC");
			break;
		case SPELL_INCINERATE            :
			strcpy(spell, "INCINERATE");
			break;
		case SPELL_MASS_PARALYSE         :
			strcpy(spell, "MASS_PARALYSE");
			break;

		// Level 10
		case SPELL_MASS_LIGHTNING_STRIKE :
			strcpy(spell, "MASS_LIGHTNING_STRIKE");
			break;
		case SPELL_CONTROL_TARGET        :
			strcpy(spell, "CONTROL");
			break;
		case SPELL_FREEZE_TIME           :
			strcpy(spell, "FREEZE_TIME");
			break;
		case SPELL_MASS_INCINERATE       :
			strcpy(spell, "MASS_INCINERATE");
			break;
		default :
			return FALSE;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
long GetSpellId(const char *spell)
{
	if (!stricmp(spell, "ACTIVATE_PORTAL"))       return SPELL_ACTIVATE_PORTAL;		

	if (!stricmp(spell, "DOUSE"))                 return SPELL_DOUSE;

	if (!stricmp(spell, "IGNIT"))                 return SPELL_IGNIT;

	if (!stricmp(spell, "MAGIC_SIGHT"))           return SPELL_MAGIC_SIGHT;

	if (!stricmp(spell, "MAGIC_MISSILE"))         return SPELL_MAGIC_MISSILE;

	// Level 2
	if (!stricmp(spell, "ARMOR"))                 return SPELL_ARMOR;

	if (!stricmp(spell, "DETECT_TRAP"))           return SPELL_DETECT_TRAP;		

	if (!stricmp(spell, "HARM"))                  return SPELL_HARM;				

	if (!stricmp(spell, "HEAL"))                  return SPELL_HEAL;		

	if (!stricmp(spell, "LOWER_ARMOR"))           return SPELL_LOWER_ARMOR;		

	// Level 3
	if (!stricmp(spell, "CREATE_FOOD"))           return SPELL_CREATE_FOOD;		

	if (!stricmp(spell, "DISPELL_ILLUSION"))      return SPELL_DISPELL_ILLUSION;		

	if (!stricmp(spell, "FIREBALL"))              return SPELL_FIREBALL;				

	if (!stricmp(spell, "ICE_PROJECTILE"))        return SPELL_ICE_PROJECTILE;		

	if (!stricmp(spell, "SPEED"))                 return SPELL_SPEED;

	// Level 4
	if (!stricmp(spell, "BLESS"))                 return SPELL_BLESS;

	if (!stricmp(spell, "COLD_PROTECTION"))       return SPELL_COLD_PROTECTION;		

	if (!stricmp(spell, "CURSE"))                 return SPELL_CURSE;		

	if (!stricmp(spell, "DISPELL_FIELD"))         return SPELL_DISPELL_FIELD;		

	if (!stricmp(spell, "FIRE_PROTECTION"))       return SPELL_FIRE_PROTECTION;		

	if (!stricmp(spell, "TELEKINESIS"))           return SPELL_TELEKINESIS;		

	// Level 5
	if (!stricmp(spell, "CURE_POISON"))           return SPELL_CURE_POISON;

	if (!stricmp(spell, "LEVITATE"))              return SPELL_LEVITATE;

	if (!stricmp(spell, "POISON_PROJECTILE"))     return SPELL_POISON_PROJECTILE;

	if (!stricmp(spell, "REPEL_UNDEAD"))          return SPELL_REPEL_UNDEAD;

	if (!stricmp(spell, "RUNE_OF_GUARDING"))      return SPELL_RUNE_OF_GUARDING;

	// Level 6
	if (!stricmp(spell, "CREATE_FIELD"))          return SPELL_CREATE_FIELD;		

	if (!stricmp(spell, "DISARM_TRAP"))           return SPELL_DISARM_TRAP;		

	if (!stricmp(spell, "PARALYSE"))              return SPELL_PARALYSE;	

	if (!stricmp(spell, "RAISE_DEAD"))            return SPELL_RISE_DEAD;

	if (!stricmp(spell, "SLOWDOWN"))              return SPELL_SLOW_DOWN;				

	// Level 7
	if (!stricmp(spell, "CONFUSE"))               return SPELL_CONFUSE;				

	if (!stricmp(spell, "FIRE_FIELD"))            return SPELL_FIRE_FIELD;		

	if (!stricmp(spell, "FLYING_EYE"))            return SPELL_FLYING_EYE;		

	if (!stricmp(spell, "ICE_FIELD"))             return SPELL_ICE_FIELD;		

	if (!stricmp(spell, "LIGHTNING_STRIKE"))      return SPELL_LIGHTNING_STRIKE;		

	// Level 8
	if (!stricmp(spell, "ENCHANT_WEAPON"))        return SPELL_ENCHANT_WEAPON;		

	if (!stricmp(spell, "EXPLOSION"))             return SPELL_EXPLOSION;		

	if (!stricmp(spell, "INVISIBILITY"))          return SPELL_INVISIBILITY;		

	if (!stricmp(spell, "LIFE_DRAIN"))            return SPELL_LIFE_DRAIN;		

	if (!stricmp(spell, "MANA_DRAIN"))            return SPELL_MANA_DRAIN;		

	// Level 9
	if (!stricmp(spell, "INCINERATE"))            return SPELL_INCINERATE;		

	if (!stricmp(spell, "MASS_PARALYSE"))         return SPELL_MASS_PARALYSE;		

	if (!stricmp(spell, "NEGATE_MAGIC"))          return SPELL_NEGATE_MAGIC;		

	if (!stricmp(spell, "SUMMON_CREATURE"))       return SPELL_SUMMON_CREATURE;		

	if (!stricmp(spell, "FAKE_SUMMON"))			  return SPELL_FAKE_SUMMON;		

	// Level 10
	if (!stricmp(spell, "CONTROL"))               return SPELL_CONTROL_TARGET;		

	if (!stricmp(spell, "FREEZE_TIME"))           return SPELL_FREEZE_TIME;		

	if (!stricmp(spell, "MASS_INCINERATE"))       return SPELL_MASS_INCINERATE;		

	if (!stricmp(spell, "MASS_LIGHTNING_STRIKE")) return SPELL_MASS_LIGHTNING_STRIKE;		

	return -1;
}

//-----------------------------------------------------------------------------
void SPELLCAST_Notify(long num)
{
	if (num < 0) return;

	if (num >= MAX_SPELLS) return;

	char spell[128];
	long source = spells[num].caster;

	if (MakeSpellName(spell,spells[num].type))
	{
		for (long i=0;i<inter.nbmax;i++) 
		{
			if (inter.iobj[i]!=NULL) 
			{
				if (source >= 0) EVENT_SENDER = inter.iobj[source];
				else EVENT_SENDER = NULL;

				char param[256];
				sprintf(param,"%s %d",spell,(long)spells[num].caster_level);
				SendIOScriptEvent(inter.iobj[i], SM_SPELLCAST, param);
			}
		}	
	}	
}

//-----------------------------------------------------------------------------
void SPELLCAST_NotifyOnlyTarget(long num)
{
	if (num < 0) return;

	if (num >= MAX_SPELLS) return;

	if(spells[num].target<0) return;

	char spell[128];
	long source = spells[num].caster;

	if (MakeSpellName(spell,spells[num].type))
	{
		if (source >= 0) EVENT_SENDER = inter.iobj[source];
		else EVENT_SENDER = NULL;

		char param[256];
		sprintf(param,"%s %d",spell,(long)spells[num].caster_level);
		SendIOScriptEvent(inter.iobj[spells[num].target], SM_SPELLCAST, param);
	}	
}

//-----------------------------------------------------------------------------
void SPELLEND_Notify(long num)
{
	if	(	(num<0) 
		||	(num>=MAX_SPELLS)	)
		return;

	char spell[128];
	long source=spells[num].caster;

	if (spells[num].type==SPELL_CONFUSE)
	{
		if (ValidIONum(source))
			EVENT_SENDER = inter.iobj[source];
		else 
			EVENT_SENDER = NULL;

		if (ValidIONum(spells[num].target))
		{			
			if (MakeSpellName(spell,spells[num].type))
			{
				char param[128];
				INTERACTIVE_OBJ * targ= inter.iobj[spells[num].target];
				sprintf(param,"%s %d",spell,(long)spells[num].caster_level);
				SendIOScriptEvent(targ,SM_SPELLEND,param);
			}
		}

		return;
	}

	// we only notify player spells end.
	if (MakeSpellName(spell,spells[num].type))
		for (long i=0;i<inter.nbmax;i++)
			if (inter.iobj[i])
			{
				char param[128];

				if (ValidIONum(source))
					EVENT_SENDER = inter.iobj[source];
				else 
					EVENT_SENDER = NULL;

				sprintf(param,"%s %d",spell,(long)spells[num].caster_level);
				SendIOScriptEvent(inter.iobj[i],SM_SPELLEND,param);
			}
}

//-----------------------------------------------------------------------------
void ReCenterSequence(char *_pcSequence,int &_iMinX,int &_iMinY,int &_iMaxX,int &_iMaxY)
{
	int iSizeX=0,iSizeY=0;
	_iMinX=_iMinY=0;
	_iMaxX=_iMaxY=0;
	int iLenght=strlen(_pcSequence);

	for(int iI=0;iI<iLenght;iI++)
	{
		EERIE_S2D es2dVector;
		GetSymbVector(_pcSequence[iI],&es2dVector);
		iSizeX+=es2dVector.x;
		iSizeY+=es2dVector.y;
		_iMinX=__min(_iMinX,iSizeX);
		_iMinY=__min(_iMinY,iSizeY);
		_iMaxX=__max(_iMaxX,iSizeX);
		_iMaxY=__max(_iMaxY,iSizeY);
	}
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_UpdateSymbolDraw(LPDIRECT3DDEVICE7 pd3dDevice)
{
	unsigned long curtime = ARXTimeUL();

	//1
	for (long i=0;i<inter.nbmax;i++)
	{
		INTERACTIVE_OBJ * io=inter.iobj[i];

		if (io) 
		{
			if (io->spellcast_data.castingspell>0)
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
							io->spellcast_data.castingspell=-1;
					}
					else tst=1;

					if ((io->spellcast_data.symb[0]!=255)  && tst )
					{
						long symb=io->spellcast_data.symb[0];

						for (long j=0;j<3;j++)
							io->spellcast_data.symb[j]=io->spellcast_data.symb[j+1];

						io->spellcast_data.symb[3]=255;
						ARX_SPELLS_RequestSymbolDraw2(io, symb, (1000-(io->spellcast_data.spell_level*60))*__max(io->speed_modif+io->basespeed,0.01f));
						io->GameFlags &=~GFLAG_INVISIBILITY;
					}
					else if (tst)// cast spell !!!
					{					
						io->GameFlags &=~GFLAG_INVISIBILITY;
						ARX_SPELLS_Launch(io->spellcast_data.castingspell,i,io->spellcast_data.spell_flags,io->spellcast_data.spell_level,io->spellcast_data.target,io->spellcast_data.duration);

						if (!(io->spellcast_data.spell_flags & SPELLCAST_FLAG_NOANIM)
								&&  (io->ioflags & IO_NPC))
						{
							ANIM_USE * ause1=&io->animlayer[1];
							AcquireLastAnim(io);
							FinishAnim(io,ause1->cur_anim);
							ANIM_Set(ause1,io->anims[ANIM_CAST]);
						}

						io->spellcast_data.castingspell=-1;
					}
				}
			}

			float rr=rnd();

			if (io->flarecount)
			{
				if (io->dynlight==-1) io->dynlight=(short)GetFreeDynLight();

				if (io->dynlight!=-1)
				{
					DynLight[io->dynlight].pos.x=io->pos.x-EEsin(DEG2RAD(MAKEANGLE(io->angle.b-45.f)))*60.f;
					DynLight[io->dynlight].pos.y=io->pos.y-120.f;
					DynLight[io->dynlight].pos.z=io->pos.z+EEcos(DEG2RAD(MAKEANGLE(io->angle.b-45.f)))*60.f;
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

			if (io->symboldraw)
			{
				SYMBOL_DRAW * sd=(SYMBOL_DRAW *)inter.iobj[i]->symboldraw;
				long tim=curtime-sd->starttime;
 


				if (tim>sd->duration)
				{
					if (io->dynlight!=-1)
					{
						DynLight[io->dynlight].time_creation = ARXTimeUL();
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

				EERIE_S2D pos1, vect, old_pos;
				long newtime=tim;
				long oldtime=sd->lasttim;

				if (oldtime>sd->duration) oldtime=sd->duration;

				if (newtime>sd->duration) newtime=sd->duration;

				sd->lasttim=(short)tim;

				pos1.x = (short)subj.centerx -OFFSET_X*2 + sd->cPosStartX*OFFSET_X;
				pos1.y = (short)subj.centery -OFFSET_Y*2 + sd->cPosStartY*OFFSET_Y;

				float div_ti=1.f/ti;

				if (io != inter.iobj[0])
				{
					old_pos.x=pos1.x;
					old_pos.y=pos1.y;

					for (long j=0;j<nbcomponents;j++)
					{
						GetSymbVector(sd->sequence[j],&vect);
						vect.x += vect.x >> 1;
						vect.y += vect.y >> 1;

						if (oldtime<=ti)
						{							
							float ratio=(float)(oldtime)*div_ti;
							old_pos.x+=(short)(float)(ratio*(float)vect.x);
							old_pos.y+=(short)(float)(ratio*(float)vect.y);
							break;
						}

						old_pos.x+=vect.x;
						old_pos.y+=vect.y;
						oldtime-=(long)ti;
					}						

					for (int j=0;j<nbcomponents;j++)
					{
						GetSymbVector(sd->sequence[j],&vect);
						vect.x += vect.x >> 1;
						vect.y += vect.y >> 1;

						if (newtime<=ti)
						{
							float ratio=(float)(newtime)*div_ti;
							pos1.x+=(short)(float)(ratio*(float)vect.x);
							pos1.y+=(short)(float)(ratio*(float)vect.y);
							AddFlare(&pos1,0.1f,1,inter.iobj[i]);
							FlareLine(&old_pos,&pos1,inter.iobj[i]);
							break;
						}

						pos1.x+=vect.x;
						pos1.y+=vect.y;
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
					ARX_CHECK_SHORT(pos1.x + lPosX);
					ARX_CHECK_SHORT(pos1.y + lPosY);

					pos1.x	+= ARX_CLEAN_WARN_CAST_SHORT(lPosX);
					pos1.y	+= ARX_CLEAN_WARN_CAST_SHORT(lPosY);



					lPosX =  ((lMaxSymbolDrawSizeX-iSizeX)>>1);
					lPosY =  ((lMaxSymbolDrawSizeY-iSizeY)>>1);
					ARX_CHECK_SHORT(pos1.x + lPosX);
					ARX_CHECK_SHORT(pos1.y + lPosY);

					pos1.x	+= ARX_CLEAN_WARN_CAST_SHORT(lPosX);
					pos1.y	+= ARX_CLEAN_WARN_CAST_SHORT(lPosY);



					int iX = pos1.x-iMinX;
					int iY = pos1.y-iMinY;
					ARX_CHECK_SHORT(iX);
					ARX_CHECK_SHORT(iY);

					pos1.x = ARX_CLEAN_WARN_CAST_SHORT(iX);
					pos1.y = ARX_CLEAN_WARN_CAST_SHORT(iY);


					for (long j=0;j<nbcomponents;j++)
					{

						GetSymbVector(sd->sequence[j],&vect);

						if (newtime<ti)
						{
							float ratio = (float)(newtime) * div_ti;
							

							float fX = pos1.x + (ratio*vect.x)*0.5f;
							float fY = pos1.y + (ratio*vect.y)*0.5f; 
							ARX_CHECK_SHORT(fX);
							ARX_CHECK_SHORT(fY);

							pos1.x = ARX_CLEAN_WARN_CAST_SHORT(fX);
							pos1.y = ARX_CLEAN_WARN_CAST_SHORT(fY);


							EERIE_S2D pos;
							pos.x=(short)(pos1.x*Xratio);	
							pos.y=(short)(pos1.y*Yratio);

							if (io == inter.iobj[0])
								AddFlare2(&pos,0.1f,1,inter.iobj[i]);
							else
								AddFlare(&pos,0.1f,1,inter.iobj[i]);


							break;
						}

						pos1.x+=vect.x;
						pos1.y+=vect.y;

						newtime-=(long)ti;
					}
				}				
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_ClearAllSymbolDraw()
{
	for (long i(0); i < inter.nbmax; i++) 
		if (inter.iobj[i] && inter.iobj[i]->symboldraw)
			free(inter.iobj[i]->symboldraw), inter.iobj[i]->symboldraw = NULL;
}

//*************************************************************************************
// 
//*************************************************************************************
void ARX_SPELLS_AnalyseSYMBOL()
{
	long csymb = -1;
	long sm = atoi(SpellMoves);

	switch (sm)
	{
		
		// COSUM
		case 62148  :
		case 632148 :
		case 62498  :
		case 62748  :
		case 6248   :
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_COSUM;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_COSUM);
			break;

		// COMUNICATUM
		case 632426 :
		case 627426 :
		case 634236 :
		case 624326 :
		case 62426  :
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_COMUNICATUM;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

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
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_FOLGORA;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_FOLGORA);
			break;

		// SPACIUM
		case 42368  :
		case 42678  :
		case 42698  :
		case 4268   :
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_SPACIUM;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_SPACIUM);
			break;

		// TERA
		case 9826   :
		case 92126  :
		case 9264   :
		case 9296   :
		case 926    :
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_TERA;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

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
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_CETRIUS;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_CETRIUS);
			break;

		// RHAA
		case 28    :
		case 2     :
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_RHAA;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_RHAA);
			break;

		// FRIDD
		case 98362	:
		case 8362	:
		case 8632	:
		case 8962	:
		case 862	:
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_FRIDD;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

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

				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_KAOM;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_KAOM);
			break;

		// STREGUM
		case 82328 :
		case 8328  :
		case 2328  :
		case 8938  :
		case 8238  :
		case 838   :
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_STREGUM;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_STREGUM);
			break;

		// MORTE
		case 628   :
		case 621   :
		case 62    :
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_MORTE;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_MORTE);
			break;

		// TEMPUS
		case 962686  :
		case 862686  :
		case 8626862 : 
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_TEMPUS;

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
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_MOVIS;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_MOVIS);
			break;

		// NHI
		case 46:
		case 4:
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_NHI;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_NHI);
			break;

		// AAM
		case 64:
		case 6:
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_AAM;

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
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_YOK;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_YOK);
			break;

		// TAAR
		case 6236:
		case 6264:
		case 626:
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_TAAR;

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

				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_MEGA;

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
				csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_VISTA;

				if (CurrSpellSymbol>=MAX_SPELL_SYMBOLS) CurrSpellSymbol=MAX_SPELL_SYMBOLS-1;

				ARX_SOUND_PlaySFX(SND_SYMB_VISTA);
			break;

		// VITAE
		case 698:
		case 68:
			csymb=SpellSymbol[CurrSpellSymbol++]=SYMBOL_VITAE;

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
		case 2389823898:
		case 2393239:
		case 38239:
		case 2982323989:
		case 3298232329:
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
		case 2389238989:
		case 289298989:
		case 23892398:
		case 238239:
		case 29298:
		case 2329298:
		case 232389829:
		case 2389829:
		case 239239:
		case 282398:
		case 2398982398:
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
			_TCHAR tex[512];

			if (strlen(SpellMoves)>=127)
				SpellMoves[127]=0;

			strcpy(LAST_FAILED_SEQUENCE,SpellMoves);

			if (!NO_TEXT_AT_ALL) 
			{
				_stprintf(tex, _T("Unknown Symbol - %S"), SpellMoves);
				ARX_SPEECH_Add(NULL, tex);
			}
		}
	}

	bPrecastSpell = false;

	// wanna precast?
	if (ARX_IMPULSE_Pressed(CONTROLS_CUST_STEALTHMODE))
	{
		bPrecastSpell = true;
	}
}

//*************************************************************************************
// 
//*************************************************************************************
BOOL ARX_SPELLS_AnalyseSPELL()
{
	long caster = 0; // Local Player
	long PRE_CAST=0;

	if ( ARX_IMPULSE_Pressed(CONTROLS_CUST_STEALTHMODE) || bPrecastSpell)
		PRE_CAST = SPELLCAST_FLAG_PRECAST;

	bPrecastSpell = false;

	if (	(SpellSymbol[0]==SYMBOL_MEGA)
		&&	(SpellSymbol[1]==SYMBOL_MEGA)
		&&	(SpellSymbol[2]==SYMBOL_MEGA)
		&&	(SpellSymbol[3]==SYMBOL_AAM)
		&&	(SpellSymbol[4]==SYMBOL_VITAE)
		&&	(SpellSymbol[5]==SYMBOL_TERA)		)
	{
		cur_mega=10;
		return ARX_SPELLS_Launch(SPELL_SUMMON_CREATURE,caster,PRE_CAST);
	}

	switch (SpellSymbol[0])
	{
		case SYMBOL_RHAA :

			switch (SpellSymbol[1])
			{
				case SYMBOL_STREGUM :

					switch (SpellSymbol[2])
					{
						case SYMBOL_VITAE :// CURSE Level 4

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_CURSE,caster,PRE_CAST);							

						break;
					}

					break;
				case SYMBOL_TEMPUS: // FREEZE TIME Level 10

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_FREEZE_TIME,caster,PRE_CAST);

					break;

				case SYMBOL_KAOM : // LOWER ARMOR Level 2

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_LOWER_ARMOR,caster,PRE_CAST);

					break;

				case SYMBOL_MOVIS : // SLOW DOWN Level 6

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_SLOW_DOWN,caster,PRE_CAST);

					break;

				case SYMBOL_VITAE : // HARM Level 2

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_HARM,caster,PRE_CAST);

					break;
				case SYMBOL_VISTA: // CONFUSE Level 7

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_CONFUSE,caster,PRE_CAST);

					break;
			}

			break;
		case SYMBOL_MEGA :			

			switch (SpellSymbol[1])
			{
				case SYMBOL_NHI:

					switch (SpellSymbol[2])
					{
						case SYMBOL_MOVIS: // MASS PARALYSIS Level 9

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_MASS_PARALYSE,caster,PRE_CAST);								

						break;
					}

					break;

				case SYMBOL_KAOM: //ARMOR Level 2

					if (SpellSymbol[2]==255)					
						return ARX_SPELLS_Launch(SPELL_ARMOR,caster,PRE_CAST);						

				break;

				case SYMBOL_VISTA: // MAGIC SIGHT Level 1

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_MAGIC_SIGHT,caster,PRE_CAST);

				break;

				case SYMBOL_VITAE: // HEAL Level 2

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_HEAL,caster,PRE_CAST);

				break;

				case SYMBOL_MOVIS: // SPEED Level 3

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_SPEED,caster,PRE_CAST);

				break;

				case SYMBOL_STREGUM:

					switch (SpellSymbol[2])
					{
						case SYMBOL_VITAE:// SANCTIFY OBJECT Level 4

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_BLESS,caster,PRE_CAST);

							break;

						case SYMBOL_COSUM: // ENCHANT WEAPON Level 8

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_ENCHANT_WEAPON,caster,PRE_CAST);

							break;
					}

					break;

					case SYMBOL_AAM:

						switch (SpellSymbol[2])
						{
							case SYMBOL_MEGA:

								switch (SpellSymbol[3])
								{
									case SYMBOL_YOK: // MASS INCINERATE Level 10

										if (SpellSymbol[4]==255)
											return ARX_SPELLS_Launch(SPELL_MASS_INCINERATE,caster,PRE_CAST);

									break;
								}

							break;
						}

					break;

					case SYMBOL_SPACIUM:

						switch (SpellSymbol[2])
						{
							case SYMBOL_NONE  :

								if (SpellSymbol[3]==255)
									return ARX_SPELLS_Launch(SPELL_ACTIVATE_PORTAL,caster,PRE_CAST);

							break;

							case SYMBOL_MOVIS : // LEVITATE Level 5

								if (SpellSymbol[3]==255)
									return ARX_SPELLS_Launch(SPELL_LEVITATE,caster,PRE_CAST);

							break;
						}

						break;
			}

			break;

		case SYMBOL_NHI:

			switch (SpellSymbol[1])
			{
				case SYMBOL_MOVIS: // PARALYSE Level 6

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_PARALYSE,caster,PRE_CAST);

				break;

				case SYMBOL_CETRIUS: // Cure POISON Level 5

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_CURE_POISON,caster,PRE_CAST);

				break;

				case SYMBOL_YOK: // DOUSE Level 1 Spell

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_DOUSE,caster,PRE_CAST);

				break;
				
				case SYMBOL_STREGUM: 

					switch (SpellSymbol[2])
					{
						case SYMBOL_VISTA: // DISPELL ILLUSION Level 3

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_DISPELL_ILLUSION,caster,PRE_CAST);

							break;

						case SYMBOL_SPACIUM: // NEGATE MAGIC Level 9

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_NEGATE_MAGIC,caster,PRE_CAST);

							break;
					}

					break;

				case SYMBOL_SPACIUM: // DISPEL FIELD Level 4

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_DISPELL_FIELD,caster,PRE_CAST);							

				break;
				
				case SYMBOL_MORTE: 

					switch (SpellSymbol[2])
					{
						case SYMBOL_COSUM: // DISARM TRAP Level 6

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_DISARM_TRAP,caster,PRE_CAST);

						break;
					}

					break;

				case SYMBOL_VISTA: 

					switch (SpellSymbol[2])
					{
						case SYMBOL_NONE :

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_INVISIBILITY,caster,PRE_CAST);

						break;
					}

					break;
			}

			break;
			
		case SYMBOL_VISTA:

			switch (SpellSymbol[1])
			{
				case SYMBOL_MOVIS: // FLYING EYE Level 7

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_FLYING_EYE,caster,PRE_CAST);

				break;
			}

			break;

		case SYMBOL_MORTE:

			switch (SpellSymbol[1])
			{
				case SYMBOL_KAOM: // REPEL UNDEAD Level 5

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_REPEL_UNDEAD,caster,PRE_CAST);

				break;
					
				case SYMBOL_COSUM:

					switch (SpellSymbol[2])
					{
						case SYMBOL_VISTA:	//DETECT_TRAP Level 2

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_DETECT_TRAP,caster,PRE_CAST);

						break;
					}

					break;
			}

			break;

		case SYMBOL_MOVIS:

			switch (SpellSymbol[1])
			{
				case SYMBOL_COMUNICATUM: // CONTROL TARGET Level 10

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_CONTROL_TARGET,caster,PRE_CAST);

				break;					
			}

			break;

		case SYMBOL_STREGUM:

			switch (SpellSymbol[1])
			{
				case SYMBOL_MOVIS: // MANA_DRAIN Level 8

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_MANA_DRAIN,caster,PRE_CAST);

				break;					
			}

			break;

		case SYMBOL_AAM:

			switch (SpellSymbol[1])
			{
				case SYMBOL_MEGA :

					switch (SpellSymbol[2])
					{
						case SYMBOL_YOK: // INCINERATE Level 9

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_INCINERATE,caster,PRE_CAST);

						break;
						case SYMBOL_MORTE: // EXPLOSION Level 8

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_EXPLOSION,caster,PRE_CAST);

						break;
					}

					break;

				case SYMBOL_KAOM:

					switch (SpellSymbol[2])
					{
						case SYMBOL_SPACIUM: // CREATE FIELD Level 6

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_CREATE_FIELD,caster,PRE_CAST);

						break;
					}

					break;

				case SYMBOL_MORTE:

					switch (SpellSymbol[2])
					{
						case SYMBOL_VITAE: // RISE DEAD Level 6

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_RISE_DEAD,caster,PRE_CAST);

						break;

						case SYMBOL_COSUM: // RUNE OF GUARDING Level 5

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_RUNE_OF_GUARDING,caster,PRE_CAST);

						break;
					}

					break;

				case SYMBOL_VITAE:

					switch (SpellSymbol[2])
					{
						case SYMBOL_TERA: // SUMMON CREATURE Level 9

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_SUMMON_CREATURE,caster,PRE_CAST);

						break;

						case SYMBOL_COSUM: // CREATE_FOOD Level 3

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_CREATE_FOOD,caster,PRE_CAST);

						break;
					}

					break;

				case SYMBOL_FOLGORA:

					switch (SpellSymbol[2])
					{
						case SYMBOL_TAAR: // LIGHTNING STRIKE Level 7

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_LIGHTNING_STRIKE,caster,PRE_CAST);

						break;

						case SYMBOL_SPACIUM: // MASS LIGHTNING Level 10

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_MASS_LIGHTNING_STRIKE,caster,PRE_CAST);

						break;
					}

					break;

				case SYMBOL_YOK:

					switch (SpellSymbol[2])
					{
						case SYMBOL_NONE : // IGNIT Level 1

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_IGNIT,caster,PRE_CAST);

						break;

						case SYMBOL_SPACIUM: // FIRE FIELD Level 7

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_FIRE_FIELD,caster,PRE_CAST);

						break;							

						case SYMBOL_TAAR: // FIREBALL Level 3

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_FIREBALL,caster,PRE_CAST);

						break;
					}

					break;

				case SYMBOL_FRIDD:

					switch (SpellSymbol[2])
					{
						case SYMBOL_SPACIUM: // ICE FIELD Level 7

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_ICE_FIELD,caster,PRE_CAST);

						break;							

						case SYMBOL_TAAR: // ICE PROJECTILE Level 3

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_ICE_PROJECTILE,caster,PRE_CAST);

						break;
					}

					break;

				case SYMBOL_CETRIUS:

					switch (SpellSymbol[2])
					{
						case SYMBOL_TAAR: // POISON PROJECTILE Level 5

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_POISON_PROJECTILE,caster,PRE_CAST);

						break;
					}

					break;

				case SYMBOL_TAAR: 

					switch (SpellSymbol[2])
					{
						case SYMBOL_NONE :// MAGIC MISSILE Level 1

							if (SpellSymbol[3]==255)
								return ARX_SPELLS_Launch(SPELL_MAGIC_MISSILE,caster,PRE_CAST);

						break;
					}

					break;
			}

			break;

		case SYMBOL_YOK:

			switch (SpellSymbol[1])
			{
				case SYMBOL_KAOM: // FIRE PROTECTION Level 4

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_FIRE_PROTECTION,caster,PRE_CAST);

				break;
			}

			break;

		case SYMBOL_FRIDD:

			switch (SpellSymbol[1])
			{
				case SYMBOL_KAOM: // ICE PROTECTION Level 4

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_COLD_PROTECTION,caster,PRE_CAST);

				break;
			}

			break;

		case SYMBOL_VITAE:

			switch (SpellSymbol[1])
			{
				case SYMBOL_MOVIS: // LIFE_DRAIN Level 8 Spell

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_LIFE_DRAIN,caster,PRE_CAST);

				break;
			}

			break;

		case SYMBOL_SPACIUM:

			switch (SpellSymbol[1])
			{
				case SYMBOL_COMUNICATUM: // TELEKINESIS Level 4

					if (SpellSymbol[2]==255)
						return ARX_SPELLS_Launch(SPELL_TELEKINESIS,caster,PRE_CAST);

				break;
			}

			break;
	}

	ARX_SPELLS_Fizzle(-1);

	if (player.SpellToMemorize.bSpell)
	{
		CurrSpellSymbol=0;
		player.SpellToMemorize.bSpell = false;
	}

	return -1;
}
long No_MagicAllowed()
{
	ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
	return -1;
}
extern long PLAYER_PARALYSED;
//*************************************************************************************
// 
//*************************************************************************************
void ARX_SPELLS_ManageMagic()
{
	if (ARXmenu.currentmode!=AMCM_OFF)
		return;

	INTERACTIVE_OBJ * io=inter.iobj[0];

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
		(ARX_IMPULSE_Pressed(CONTROLS_CUST_MAGICMODE))) && (!PLAYER_PARALYSED))
	{
		
		if (player.Interface & INTER_COMBATMODE)
		{
			WILLRETURNTOCOMBATMODE=1;

			ARX_INTERFACE_Combat_Mode(0);
			bGToggleCombatModeWithKey=false;

			
			ResetAnim(&inter.iobj[0]->animlayer[1]);
			inter.iobj[0]->animlayer[1].flags&=~EA_LOOP;
		}

		if ((TRUE_PLAYER_MOUSELOOK_ON))
		{
			WILLRETURNTOFREELOOK = 1;
			TRUE_PLAYER_MOUSELOOK_ON &= ~1;
		}

		if (player.doingmagic!=2)
		{
			player.doingmagic=2;

			if (inter.iobj[0]->anims[ANIM_CAST_START])
			{
				AcquireLastAnim(inter.iobj[0]);
				ANIM_Set(&inter.iobj[0]->animlayer[1],inter.iobj[0]->anims[ANIM_CAST_START]);
				MAGICMODE = 1;
			}
		}
		
		if (snip >= 2)
		{	
			if ((!EERIEMouseButton & 1) && (ARX_FLARES_broken==0)) 
			{
				ARX_FLARES_broken=2;
				PIPOrgb++;

				if (PIPOrgb>2) PIPOrgb=0;			
			}
			
			if (EERIEMouseButton & 1)
			{
				EERIE_S2D pos,pos2;
				pos.x = DANAEMouse.x; 
				pos.y = DANAEMouse.y;
				extern long TRUE_PLAYER_MOUSELOOK_ON;
				
				if (TRUE_PLAYER_MOUSELOOK_ON)
				{
					pos.x = MemoMouse.x;
					pos.y = MemoMouse.y;
				}

				pos2.x=Lm.x;
				pos2.y=Lm.y;

				if (!ARX_FLARES_broken) FlareLine(&pos2,&pos);

				if (rnd()>0.6) AddFlare(&pos,1.f,-1);
				else AddFlare(&pos,1.f,3);

				OPIPOrgb=PIPOrgb;
				Lm.x = DANAEMouse.x; 
				Lm.y = DANAEMouse.y;
				
				if (TRUE_PLAYER_MOUSELOOK_ON)
				{
					Lm.x = MemoMouse.x;
					Lm.y = MemoMouse.y;
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

			if (inter.iobj[0]->anims[ANIM_CAST_END])
			{
				AcquireLastAnim(inter.iobj[0]);
				ANIM_Set(&inter.iobj[0]->animlayer[1],inter.iobj[0]->anims[ANIM_CAST_END]);
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
			if (ARX_SPELLS_AnalyseSPELL()!=-1)
			{
				if (inter.iobj[0]->anims[ANIM_CAST])
				{
					AcquireLastAnim(inter.iobj[0]);
					ANIM_Set(&inter.iobj[0]->animlayer[1],inter.iobj[0]->anims[ANIM_CAST]);
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

			if(INTERNATIONAL_MODE)
			{
				TRUE_PLAYER_MOUSELOOK_ON|=1;
				bRenderInCursorMode=false;
			}
		}

		if (WILLRETURNTOFREELOOK)
		{
			TRUE_PLAYER_MOUSELOOK_ON |= 1;
			WILLRETURNTOFREELOOK = 0;
		}

		ARX_SPELLS_ResetRecognition();
	}
	else if (ARX_FLARES_broken==2)
	{
		ARX_SPELLS_Analyse();

		if (SpellMoves[0] != 0) 
		 ARX_SPELLS_AnalyseSYMBOL();
	
		ARX_FLARES_broken = 1;
	}
}

//-----------------------------------------------------------------------------
long CanPayMana(long num,float cost, bool _bSound = true)
{
	if (num<0) return 0;

	if (spells[num].flags & SPELLCAST_FLAG_NOMANA) return 1;

	if (spells[num].caster==0) 
	{
		if (player.mana<cost)
		{
			ARX_SPELLS_FizzleNoMana(num);

			if (_bSound)
			{
				ARX_SPEECH_AddLocalised(NULL, "[player_cantcast]");
				ARX_SPEECH_AddSpeech(	inter.iobj[0],
					"[player_cantcast]",
					PARAM_LOCALISED,
					ANIM_TALK_NEUTRAL);
			}

			return 0;
		}

		player.mana -= cost;
		return 1;
	}
	else if (spells[num].caster<inter.nbmax)
	{
		if (inter.iobj[spells[num].caster]->ioflags & IO_NPC)
		{
			if (inter.iobj[spells[num].caster]->_npcdata->mana<cost)
			{
				ARX_SPELLS_FizzleNoMana(num);
				return 0;
			}

			inter.iobj[spells[num].caster]->_npcdata->mana-=cost;
			return 1;
		}		
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Resets Spell Recognition
void ARX_SPELLS_ResetRecognition()
{
	for (int i = 0; i<MAX_SPELL_SYMBOLS; i++)
	{
		SpellSymbol[i] = 255;
	}

	for (int i=0;i<6;i++)
	{
		player.SpellToMemorize.iSpellSymbols[i] = 255;
	}

	CurrSpellSymbol=0;
}

//-----------------------------------------------------------------------------
// Adds a 2D point to currently drawn spell symbol
void ARX_SPELLS_AddPoint(const EERIE_S2D *pos)
{
	plist[CurrPoint].x = pos->x;
	plist[CurrPoint].y = pos->y;
	CurrPoint++;

	if (CurrPoint >= MAX_POINTS) CurrPoint = MAX_POINTS - 1;
}

//-----------------------------------------------------------------------------
long TemporaryGetSpellTarget(const EERIE_3D *from)
{
	float mindist(99999999.0F);
	long found(0);

	for (long i(1); i < inter.nbmax; i++)
		if (inter.iobj[i] && inter.iobj[i]->ioflags & IO_NPC)
		{
			float dist(EEDistance3D(from,&inter.iobj[i]->pos));

			if (dist < mindist)
			{
				found = i;
				mindist = dist;
			}
		}

	return found;
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_Analyse()
{

	long			x		= 0,
					xx		= 0,
					y		= 0,
					yy		= 0,
					dx		= 0,
					dy		= 0;
	long			i		= 0;
	float			pente	= 0,
					a		= 0,
					b		= 0;

	unsigned char	dirs[MAX_POINTS];
	unsigned char	lastdir			= 255;
	long			cdir			= 0;
	long			cx				= 0,
					cy				= 0;


	char chaine[4096];

	for ( i = 0 ; i < CurrPoint ; i++ )
	{
		x = plist[i].x;
		y = plist[i].y;

		if ( i > 0 ) 
		{
			dx = ( xx + cx ) - x;			
			dy = ( yy + cy ) - y;

			if ( Distance2D( (float)xx, (float)yy, (float)x, (float)y ) > 10 ) 
			{
				xx = xx + cx;
				yy = yy + cy;
				a = (float)( abs( dx ) );
				b = (float)( abs( dy ) );

				if ( b != 0.f )
				{
					pente = a / b;

					if ( ( pente > 0.4f ) && ( pente < 2.5f ) ) //une diagonale
					{
						if ( ( dx < 0 ) && ( dy < 0 ) ) //on a bougé vers droite/bas
						{
							if ( lastdir != ADOWNRIGHT ) 
							{
								lastdir = dirs[cdir] = ADOWNRIGHT;
								cdir++;
							}
						}
						else if ( ( dx > 0 ) && ( dy < 0 ) ) //on a bougé vers gauche/bas
						{
							if ( lastdir != ADOWNLEFT ) 
							{
								lastdir = dirs[cdir] = ADOWNLEFT;
								cdir++;
							}
						}
						else if ( ( dx < 0 ) && ( dy > 0 ) ) //on a bougé vers droite/haut
						{
							if ( lastdir != AUPRIGHT ) 
							{
								lastdir = dirs[cdir] = AUPRIGHT;
								cdir++;
							}
						}
						else if ( ( dx > 0 ) && ( dy > 0 ) ) //on a bougé vers gauche/haut
						{
							if ( lastdir != AUPLEFT ) 
							{
								lastdir = dirs[cdir] = AUPLEFT;
								cdir++;
							}
						}

						goto lasuite;
					}
				}

				if ( abs( dx ) > abs( dy ) ) //mouvement latéral plus important
				{
					if ( dx < 0 ) //on a bougé vers la droite
					{
						if ( lastdir != ARIGHT ) 
						{
							lastdir = dirs[cdir] = ARIGHT;
							cdir++;
						}
					}
					else //on a bougé vers la gauche
					{
						if ( lastdir != ALEFT ) 
						{
							lastdir = dirs[cdir] = ALEFT;
							cdir++;
						}
					}
				}
				else //mouvement vertical plus significatif
				{
					if ( dy < 0 ) //on a bougé vers le bas
					{
						if ( lastdir != ADOWN ) 
						{
							lastdir = dirs[cdir] = ADOWN;
							cdir++;
						}
					}
					else //on a bougé vers le haut
					{
						if ( lastdir != AUP ) 
						{
							lastdir = dirs[cdir] = AUP;
							cdir++;
						}
					}
				}			
			}
		}

	lasuite:
		;
		xx = x;
		yy = y;
	}

	strcpy( SpellMoves, "" );
	
	if ( cdir > 0 )
	{
		*chaine = 0;

		for ( i = 0 ; i < cdir ; i++ )
		{
			switch ( dirs[i] )
			{
				case AUP:
					strcat( chaine, "UP \n " );
					spell[CurrSlot].SlotDir = 0;
					strcat( SpellMoves, "8" ); //uses PAD values
					break;

				case ADOWN:
					strcat( chaine, "DOWN \n " );
					spell[CurrSlot].SlotDir = 4;
					strcat( SpellMoves, "2" );
					break;

				case ALEFT:
					strcat( chaine, "LEFT \n " );
					spell[CurrSlot].SlotDir = 6;
					strcat( SpellMoves, "4" );
					break;

				case ARIGHT:
					strcat( chaine, "RIGHT \n " );
					spell[CurrSlot].SlotDir = 2;
					strcat( SpellMoves, "6" );
					break;

				case AUPRIGHT:
					strcat( chaine, "UP-RIGHT \n " );
					spell[CurrSlot].SlotDir = 1;
					strcat( SpellMoves, "9" );
					break;

				case ADOWNRIGHT:
					strcat( chaine, "DOWN-RIGHT \n " );
					spell[CurrSlot].SlotDir = 3;
					strcat( SpellMoves, "3" );
					break;

				case AUPLEFT:
					strcat( chaine, "UP-LEFT \n " );
					spell[CurrSlot].SlotDir = 7;
					strcat( SpellMoves, "7" );
					break;

				case ADOWNLEFT:
					strcat( chaine, "DOWN-LEFT \n " );
					spell[CurrSlot].SlotDir = 5;
					strcat( SpellMoves, "1" );
					break;
			}
		}
	}
}

//KNOWNSPELLS knownspells;

//-----------------------------------------------------------------------------
static void ARX_SPEELS_GetMaxRect(char *_pcName)
{
	char tcTxt[32];
	int iMinX,iMinY,iMaxX,iMaxY;
	int iSizeX,iSizeY;

	ARX_SPELLS_RequestSymbolDraw3(_pcName,tcTxt);
	ReCenterSequence(tcTxt,iMinX,iMinY,iMaxX,iMaxY);
	iSizeX=iMaxX-iMinX;
	iSizeY=iMaxY-iMinY;
	lMaxSymbolDrawSizeX=__max(iSizeX,lMaxSymbolDrawSizeX);
	lMaxSymbolDrawSizeY=__max(iSizeY,lMaxSymbolDrawSizeY);
}
//-----------------------------------------------------------------------------
// Initializes Spell engine (Called once at DANAE startup)
void ARX_SPELLS_Init(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	lMaxSymbolDrawSizeX=INT_MIN;
	lMaxSymbolDrawSizeY=INT_MIN;

	ARX_SPEELS_GetMaxRect("AAM");
	ARX_SPEELS_GetMaxRect("CETRIUS");
	ARX_SPEELS_GetMaxRect("COMUNICATUM");
	ARX_SPEELS_GetMaxRect("COSUM");
	ARX_SPEELS_GetMaxRect("FOLGORA");
	ARX_SPEELS_GetMaxRect("FRIDD");
	ARX_SPEELS_GetMaxRect("KAOM");
	ARX_SPEELS_GetMaxRect("MEGA");
	ARX_SPEELS_GetMaxRect("MORTE");
	ARX_SPEELS_GetMaxRect("MOVIS");
	ARX_SPEELS_GetMaxRect("NHI");
	ARX_SPEELS_GetMaxRect("RHAA");
	ARX_SPEELS_GetMaxRect("SPACIUM");
	ARX_SPEELS_GetMaxRect("STREGUM");
	ARX_SPEELS_GetMaxRect("TAAR");
	ARX_SPEELS_GetMaxRect("TEMPUS");
	ARX_SPEELS_GetMaxRect("TERA");
	ARX_SPEELS_GetMaxRect("VISTA");
	ARX_SPEELS_GetMaxRect("VITAE");
	ARX_SPEELS_GetMaxRect("YOK");
	ARX_SPEELS_GetMaxRect("AKBAA");
}

//-----------------------------------------------------------------------------
void ARX_SPELLS_Init() 
{
	long i;

	for (i = 0; i < MAX_SPELLS; i++)
	{
		spells[i].tolive = 0;
		spells[i].exist = FALSE;
		spells[i].pSpellFx = NULL;
	}
}
void ARX_SPELLS_CancelAll() 
{
	long i;

	for (i = 0; i < MAX_SPELLS; i++)
	{
		if (spells[i].exist)
		{
			spells[i].tolive = 0;			
		}
	}

	ARX_SPELLS_Update(NULL);
	ARX_SPELLS_RemoveAllSpellsOn(inter.iobj[0]);
	inter.iobj[0]->speed_modif=0;
}

// Clears All Spells.
void ARX_SPELLS_ClearAll() 
{
	long i;

	for (i = 0; i < MAX_SPELLS; i++)
	{
		if (spells[i].exist)
		{
			spells[i].tolive = 0;
			spells[i].exist = FALSE;

			if (spells[i].pSpellFx)
			{
				delete spells[i].pSpellFx;
				spells[i].pSpellFx = NULL;
			}
		}
	}

	for (i = 0; i < inter.nbmax; i++) 
		if (inter.iobj[i]) ARX_SPELLS_RemoveAllSpellsOn(inter.iobj[i]);
}

// Obtains a Free Spell slot
long ARX_SPELLS_GetFree() 
{
	for (long i = 0; i < MAX_SPELLS; i++) 
	{
		if (spells[i].exist == FALSE) 
		{
			spells[i].longinfo = spells[i].longinfo2 = -1;
			spells[i].misc=NULL;
			return i;
		}
	}

	return -1;
}

// Checks for an existing instance of this spelltype
BOOL ARX_SPELLS_ExistAnyInstance(const long &typ) 
{
	for (long i = 0; i < MAX_SPELLS; i++) 
		if (spells[i].exist && (spells[i].type==typ)) return TRUE;

	return FALSE;
}

long ARX_SPELLS_GetInstance(const long &typ)
{
	for (long i = 0; i < MAX_SPELLS; i++)
		if (spells[i].exist && (spells[i].type==typ)) return i;

	return -1;
}

long ARX_SPELLS_GetInstanceForThisCaster(const long &typ, const long &caster)
{
	for (long i(0); i < MAX_SPELLS; i++)
		if (spells[i].exist && (spells[i].type == typ) && (spells[i].caster == caster))
			return i;

	return -1;
}

BOOL ARX_SPELLS_ExistAnyInstanceForThisCaster(const long &typ, const long &caster) 
{
	for (long i(0); i < MAX_SPELLS; i++)
		if (spells[i].exist && (spells[i].type == typ) && (spells[i].caster == caster))
			return TRUE;

	return FALSE;
}

// Plays the sound of aborted spell
void ARX_SPELLS_AbortSpellSound()
{
	ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
}

// Plays the sound of Fizzling spell
void ARX_SPELLS_Fizzle(const long &num)
{
	if (num < 0) ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE); // player fizzle
	else
	{
		spells[num].tolive = 0;

		if (spells[num].caster >= 0)
			ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &spells[num].caster_pos);
	}
}
void ARX_SPELLS_FizzleAllSpellsFromCaster(long num_caster)
{
	for (long i(0); i < MAX_SPELLS; i++)
	{
		if (	(spells[i].exist)
			&&	(spells[i].caster == num_caster)	)
		{
			spells[i].tolive=0;
		}
	}
}
// Plays the sound of Fizzling spell plus "NO MANA" speech
void ARX_SPELLS_FizzleNoMana(const long &num)
{
	if (num < 0) return;

	if (spells[num].caster >= 0)
	{
		spells[num].tolive = 0;
		ARX_SPELLS_Fizzle(num);
	}
}

PRECAST_STRUCT Precast[MAX_PRECAST];
void ARX_SPELLS_Precast_Reset()
{
	for (long i = 0; i < MAX_PRECAST; i++) Precast[i].typ = -1;
}

void ARX_SPELLS_Precast_Add(const long &typ, const long _level,long flags,long duration)
{
	long found=-1;

	for (long i = 0; i < MAX_PRECAST; i++)
		if (Precast[i].typ == -1)
		{
			found = i;
			break;
		}

	if (found == -1)
	{
		for (long i = 1; i < MAX_PRECAST; i++)
			memcpy(&Precast[i - 1], &Precast[i],sizeof(PRECAST_STRUCT));

		found = MAX_PRECAST - 1;
	}

	Precast[found].typ			= typ;
	Precast[found].level		= _level;
	Precast[found].launch_time	= 0;
	Precast[found].flags		= flags;
	Precast[found].duration     = duration;
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

	if (_bSound)
	{
		ARX_SPEECH_AddLocalised(NULL, "[player_cantcast]");
		ARX_SPEECH_AddSpeech(	inter.iobj[0],
			"[player_cantcast]",
			PARAM_LOCALISED,
			ANIM_TALK_NEUTRAL);
	}

	return 0;
}

void ARX_SPELLS_Precast_Launch(const long &num)
{
	if (ARXTime>=LAST_PRECAST_TIME+1000)
	{
		int iNumSpells=Precast[num].typ;
		float cost=ARX_SPELLS_GetManaCost(iNumSpells,-1);

		if(		(iNumSpells>=0)
			&&	(!PrecastCheckCanPayMana(num,cost)	)  )
			return;

		LAST_PRECAST_TIME = ARXTimeUL();

		if ((Precast[num].typ>=0) && (Precast[num].launch_time==0))
		{
			Precast[num].launch_time = ARXTimeUL();
			ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FIELD);
		}
	}
}
void ARX_SPELLS_Precast_Check()
{
	for (long i = 0; i < MAX_PRECAST; i++)
	{
		if ((Precast[i].typ>=0) && (Precast[i].launch_time>0) &&(ARXTime>=Precast[i].launch_time))
		{
			ANIM_USE *ause1 = &inter.iobj[0]->animlayer[1];
			
			if (player.Interface & INTER_COMBATMODE)
			{
				WILLRETURNTOCOMBATMODE=1;
				ARX_INTERFACE_Combat_Mode(0);
				bGToggleCombatModeWithKey=false;
				ResetAnim(&inter.iobj[0]->animlayer[1]);
				inter.iobj[0]->animlayer[1].flags&=~EA_LOOP;
			}

			if ((ause1->cur_anim) && (ause1->cur_anim==inter.iobj[0]->anims[ANIM_CAST]))
			{
				if (ause1->ctime>ause1->cur_anim->anims[ause1->altidx_cur]->anim_time-550)
				{
					ARX_SPELLS_Launch(	Precast[i].typ,
										0,
										Precast[i].flags | SPELLCAST_FLAG_LAUNCHPRECAST, 
										Precast[i].level, 
										-1, 
										Precast[i].duration);
					Precast[i].typ = -1;

					for (long li=i; li < MAX_PRECAST - 1; li++)
					{
						if (Precast[li + 1].typ != -1)
						{
							memcpy(&Precast[li], &Precast[li + 1], sizeof(PRECAST_STRUCT));
							Precast[li + 1].typ = -1;
						}
					}
				}
			}
			else ARX_SPELLS_Precast_Launch2(i);
		}
	}
}
void ARX_SPELLS_Precast_Launch2(const long &num)
{
	ANIM_USE *ause1 = &inter.iobj[0]->animlayer[1];

	AcquireLastAnim(inter.iobj[0]);
	FinishAnim(inter.iobj[0], ause1->cur_anim);
	ANIM_Set(ause1, inter.iobj[0]->anims[ANIM_CAST]);	
}
typedef struct
{
	long typ;
	long source;
	long flags;
	long level;
	long target;
	long duration;
} TARGETING_SPELL;
TARGETING_SPELL t_spell;

long LOOKING_FOR_SPELL_TARGET=0;
unsigned long LOOKING_FOR_SPELL_TARGET_TIME=0;
void ARX_SPELLS_CancelSpellTarget()
{
	t_spell.typ=-1;
	LOOKING_FOR_SPELL_TARGET=0;
}
void ARX_SPELLS_LaunchSpellTarget(INTERACTIVE_OBJ * io)
{
	long num=GetInterNum(io);

	if (num>=0)
	{
		long type=t_spell.typ;
		long source=t_spell.source;
		long flags=t_spell.flags;
		long level=t_spell.level;
		long duration=t_spell.duration;
		ARX_SPELLS_Launch(type, source, flags, level, num, duration);
	}	
}
extern long FINAL_RELEASE;
//-----------------------------------------------------------------------------
float ARX_SPELLS_GetManaCost(long _lNumSpell,long lNumSpellTab)
{
	float Player_Magic_Level;
	Player_Magic_Level = (float) player.Full_Skill_Casting + player.Full_Attribute_Mind;
	Player_Magic_Level= __max(1,Player_Magic_Level*DIV10);
	Player_Magic_Level= __min(10,Player_Magic_Level);

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
	case SPELL_COUNT:				
		return 0.f;
		break;
	//-----------------------------
	case SPELL_TELEPORT:	
		return 10.f;
		break;
	default:
		return 0.f;
	}
}

//-----------------------------------------------------------------------------
// Function used to launch a spell, returns Created Spell Ident
long ARX_SPELLS_Launch( const long& typ, const long& source, const long& flagss, const long& levell, const long& target, const long& duration) //const long &netspell)
{
	long flags = flagss;
	long level = levell;
	long i;

	if ( cur_rf == 3 )
	{ 
		flags |= SPELLCAST_FLAG_NOCHECKCANCAST | SPELLCAST_FLAG_NOMANA;	
		level += 2;
	}

	if ( sp_max ) 
	{ 
		level = __max( level, 15 );
	}
	
	if ( ( source == 0 ) && ( FINAL_RELEASE ) )
	if ( !( flags & SPELLCAST_FLAG_NOCHECKCANCAST ) )
	{
		for ( i = 0 ; i < MAX_SPELL_SYMBOLS ; i++ )
		{
			if ( SpellSymbol[i] != 255 )
			{
				if ( !( player.rune_flags & ( 1 << SpellSymbol[i] ) ) )
				{
					ARX_SOUND_PlaySpeech( "player_cantcast" );
					CurrSpellSymbol = 0;
					ARX_SPELLS_ResetRecognition();
					
					return -1;
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
			Player_Magic_Level = __max( 1, Player_Magic_Level * DIV10 );
			Player_Magic_Level = __min( 10, Player_Magic_Level );
		}
		else 
		{
			Player_Magic_Level = ARX_CLEAN_WARN_CAST_FLOAT(level);
		}
	}

	ARX_CHECK( !( source && (flags & SPELLCAST_FLAG_PRECAST) ) );


	if ( flags & SPELLCAST_FLAG_PRECAST )
	{
		int l = level;

		if (l <= 0)
		{


			ARX_CHECK_INT(Player_Magic_Level);
			l	= ARX_CLEAN_WARN_CAST_INT(Player_Magic_Level);


		}

		long flgs=flags;
		flgs&=~SPELLCAST_FLAG_PRECAST;
		ARX_SPELLS_Precast_Add( typ, l, flgs, duration);
		return -2;
	}

	if ( flags & SPELLCAST_FLAG_NOMANA )
	{
		Player_Magic_Level = ARX_CLEAN_WARN_CAST_FLOAT(level);
	}

	static TextureContainer * tc4 = MakeTCFromFile("Graph\\Particles\\smoke.bmp");


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
				LOOKING_FOR_SPELL_TARGET_TIME	= ARXTimeUL();	
			LOOKING_FOR_SPELL_TARGET		= 1;
			t_spell.typ						= typ;
			t_spell.source					= source;
			t_spell.flags					= flags;
			t_spell.level					= level;
			t_spell.target					= target;
			t_spell.duration				= duration;
			return -1;
		}			
		case SPELL_ENCHANT_WEAPON:		
		{
				LOOKING_FOR_SPELL_TARGET_TIME	= ARXTimeUL();
			LOOKING_FOR_SPELL_TARGET		= 2;
			t_spell.typ						= typ;
			t_spell.source					= source;
			t_spell.flags					= flags;
			t_spell.level					= level;
			t_spell.target					= target;
			t_spell.duration				= duration;
			return -1;
		}	
		break;
		case SPELL_CONTROL_TARGET:
		{
			long		tcount = 0;
			EERIE_3D	cpos;

			if ( !ValidIONum( source ) )
				return -1;

			Vector_Copy( &cpos, &inter.iobj[source]->pos );

			for ( long ii = 1 ; ii < inter.nbmax ; ii++ )
			{
				INTERACTIVE_OBJ * ioo = inter.iobj[ii];

				if (	( ioo )
					&&	( ioo->ioflags & IO_NPC )
					&&	( ioo->_npcdata->life > 0.f )
					&&	( ioo->show == SHOW_FLAG_IN_SCENE )
					&&	( IsIOGroup( ioo, "DEMON") )	
					&&	( EEDistance3D(&ioo->pos,&cpos) < 900.f )
					)
				{
					tcount++;					
				}
			}

			if ( tcount == 0 ) 
			{
				ARX_SOUND_PlaySFX( SND_MAGIC_FIZZLE, &cpos );
				return -1;
			}

			ARX_SOUND_PlaySpeech( "Player_follower_attack" );
				LOOKING_FOR_SPELL_TARGET_TIME	= ARXTimeUL();
			LOOKING_FOR_SPELL_TARGET		= 1;
			t_spell.typ						= typ;
			t_spell.source					= source;
			t_spell.flags					= flags;
			t_spell.level					= level;
			t_spell.target					= target;
			t_spell.duration				= duration;
			return -1;
		}
		break;
	}

	if ( source == 0 )
	{
		ARX_SPELLS_CancelSpellTarget();
	}


	// Try to create a new spell instance
	i = ARX_SPELLS_GetFree();

	if ( i < 0 )
	{
		return -1;
	}

	if ( source >= 0 && source < inter.nbmax )
	{
		if ( spellicons[typ].bAudibleAtStart )
		{
			ARX_NPC_SpawnAudibleSound(&inter.iobj[source]->pos,inter.iobj[source]);
		}
	}

	spells[i].caster = source;	// Caster...
	spells[i].target = target;	// No target if <0

	if ( target < 0 )
		spells[i].target = TemporaryGetSpellTarget( &inter.iobj[spells[i].caster]->pos );

	// Create hand position if a hand is defined
	if ( spells[i].caster == 0 ) 
	{
		spells[i].hand_group = inter.iobj[spells[i].caster]->obj->fastaccess.primary_attach;
	}
	else 
	{
		spells[i].hand_group = inter.iobj[spells[i].caster]->obj->fastaccess.left_attach;
	}

	if ( spells[i].hand_group != -1 )
	{
		spells[i].hand_pos.x = inter.iobj[spells[i].caster]->obj->vertexlist3[spells[i].hand_group].v.x;
		spells[i].hand_pos.y = inter.iobj[spells[i].caster]->obj->vertexlist3[spells[i].hand_group].v.y;
		spells[i].hand_pos.z = inter.iobj[spells[i].caster]->obj->vertexlist3[spells[i].hand_group].v.z;
	}

	if ( !source )
	{
		// Player source
		spells[i].caster_level = Player_Magic_Level;	// Level of caster

		spells[i].caster_pos.x = player.pos.x;
		spells[i].caster_pos.y = player.pos.y;
		spells[i].caster_pos.z = player.pos.z;

		spells[i].caster_angle.x = player.angle.x;
		spells[i].caster_angle.y = player.angle.y;
		spells[i].caster_angle.z = player.angle.z;		
		
	}
	else
	{
		// IO source
		spells[i].caster_level = level < 1 ? 1 : level > 10 ? 10 : ARX_CLEAN_WARN_CAST_FLOAT(level);

		spells[i].caster_pos.x = inter.iobj[source]->pos.x;
		spells[i].caster_pos.y = inter.iobj[source]->pos.y;
		spells[i].caster_pos.z = inter.iobj[source]->pos.z;

		spells[i].caster_angle.x = inter.iobj[source]->angle.x;
		spells[i].caster_angle.y = inter.iobj[source]->angle.y;
		spells[i].caster_angle.z = inter.iobj[source]->angle.z;		
	}

	if (flags & SPELLCAST_FLAG_LAUNCHPRECAST)
	{
		spells[i].caster_level = ARX_CLEAN_WARN_CAST_FLOAT(level);
	}

	// Checks target
	if (target<0) // no target... targeted by sight
	{
		if (source==0) // no target... player spell targeted by sight
		{
			spells[i].target_pos.x=player.pos.x-EEsin(DEG2RAD(player.angle.b))*60.f;
			spells[i].target_pos.y=player.pos.y+EEsin(DEG2RAD(player.angle.a))*60.f;
			spells[i].target_pos.z=player.pos.z+EEcos(DEG2RAD(player.angle.b))*60.f;

			spells[i].target_angle.a=0.f;
			spells[i].target_angle.b=0.f;
			spells[i].target_angle.g=0.f;
		}
		else
		{
			spells[i].target_pos.x=inter.iobj[target]->pos.x-EEsin(DEG2RAD(inter.iobj[target]->angle.b))*60.f;
			spells[i].target_pos.y=inter.iobj[target]->pos.y-120.f;
			spells[i].target_pos.z=inter.iobj[target]->pos.z+EEcos(DEG2RAD(inter.iobj[target]->angle.b))*60.f;

			spells[i].target_angle.a=0.f;
			spells[i].target_angle.b=0.f;
			spells[i].target_angle.g=0.f;
		}
	}
	// player target
	else if (target==0) 
	{
		spells[i].target_pos.x=player.pos.x;
		spells[i].target_pos.y=player.pos.x;
		spells[i].target_pos.z=player.pos.z;

		spells[i].target_angle.a=player.angle.a;
		spells[i].target_angle.b=player.angle.b;
		spells[i].target_angle.g=player.angle.g;
	}
	// IO target
	else 
	{
		spells[i].target_pos.x=inter.iobj[target]->pos.x;
		spells[i].target_pos.y=inter.iobj[target]->pos.y;
		spells[i].target_pos.z=inter.iobj[target]->pos.z;

		spells[i].target_angle.a=inter.iobj[target]->angle.a;
		spells[i].target_angle.b=inter.iobj[target]->angle.b;
		spells[i].target_angle.g=inter.iobj[target]->angle.g;				
	}			

	// spell direction
	spells[i].vector_dir.x=spells[i].target_pos.x-spells[i].caster_pos.x;
	spells[i].vector_dir.y=spells[i].target_pos.y-spells[i].caster_pos.y;
	spells[i].vector_dir.z=spells[i].target_pos.z-spells[i].caster_pos.z;
	float t=Vector_Magnitude(&spells[i].vector_dir);

	if (t<=0) t=0.0001f;

	t=1.f/t;
	spells[i].vector_dir.x*=t;
	spells[i].vector_dir.y*=t;
	spells[i].vector_dir.z*=t;

	spells[i].flags=flags;
	spells[i].cumul=0;
	spells[i].pSpellFx=NULL;
	spells[i].type = typ;
	spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();

	switch (typ)
	{
		//*********************************************************************************************
		// LEVEL 1 SPELLS -----------------------------------------------------------------------------
		case SPELL_MAGIC_SIGHT: // Launching MAGIC_SIGHT

			if (ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster)) return -1;

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			spells[i].exist=TRUE;
			spells[i].fManaCostPerSecond=0.36f;
			spells[i].bDuration = true;

			if (duration>-1) spells[i].tolive=duration;
			else spells[i].tolive=6000000;

			SPELLCAST_Notify(i);
			ARX_SOUND_PlaySFX(SND_SPELL_VISION_START, &spells[i].caster_pos);

			if (spells[i].caster==0)
			{
				Project.improve = 1;
				spells[i].snd_loop = SND_SPELL_VISION_LOOP;
				ARX_SOUND_PlaySFX(spells[i].snd_loop, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);
			}

		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_MAGIC_MISSILE: // Launching MAGIC_MISSILE
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			spells[i].exist = TRUE;
			spells[i].tolive = 20000;
				
			CSpellFx *pCSpellFx = NULL;
			long number;

			if ( ( sp_max ) || ( cur_rf == 3 ) )

			{
				ARX_CHECK_LONG(spells[i].caster_level);   
				number = ARX_CLEAN_WARN_CAST_LONG( spells[i].caster_level );
			}

			else 
			{
						if ( spells[i].caster_level < 3 ) number = 1;
				else	if ( spells[i].caster_level < 5 ) number = 2;
				else	if ( spells[i].caster_level < 7 ) number = 3;
				else	if ( spells[i].caster_level < 9 ) number = 4;
				else	number=5;
			}

			pCSpellFx = new CMultiMagicMissile(GDevice,number);			

			if (pCSpellFx != NULL)
			{
				EERIE_3D source_position,vector;
				pCSpellFx->spellinstance=i;
				Vector_Init(&source_position);
				Vector_Init(&vector);
				pCSpellFx->SetDuration((unsigned long) (6000));
				pCSpellFx->Create(source_position, vector );				
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}				

			SPELLCAST_Notify(i);
		}
		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_IGNIT:// Launching IGNIT
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			bool bLightInRadius = false;
					
			spells[i].exist = TRUE;
			spells[i].tolive = 20000;
					
			CSpellFx *pCSpellFx = NULL;
					
			pCSpellFx = new CIgnit();
			
			CIgnit *pIgnit=(CIgnit *)pCSpellFx;

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;

				if (spells[i].hand_group!=-1)
				{
					target.x=spells[i].hand_pos.x;
					target.y=spells[i].hand_pos.y;
					target.z=spells[i].hand_pos.z;
				}
				else
				{
					target.x=spells[i].caster_pos.x;
					target.y=spells[i].caster_pos.y-50.f;
					target.z=spells[i].caster_pos.z;
				}

				long id=GetFreeDynLight();

				if (id!=-1)
				{
					DynLight[id].exist		=	1;
					DynLight[id].intensity	=	1.8f;
					DynLight[id].fallend	=	450.f;
					DynLight[id].fallstart	=	380.f;
					DynLight[id].rgb.r		=	1.f;
					DynLight[id].rgb.g		=	0.75f;
					DynLight[id].rgb.b		=	0.5f;
					DynLight[id].pos.x		=	target.x;
					DynLight[id].pos.y		=	target.y;
					DynLight[id].pos.z		=	target.z;
					DynLight[id].duration	=	300;
				}

				
				float fPerimeter = 400.0f + spells[i].caster_level*30.0f;
				
				pIgnit->Create(&target, fPerimeter, 500);
				CheckForIgnition(&target,fPerimeter,1,1);

				for (long ii=0;ii<MAX_LIGHTS;ii++)
				{
					if (GLight[ii]==NULL) continue;

					if ( (GLight[ii]->extras & EXTRAS_EXTINGUISHABLE)
						&& ((spells[i].caster!=0) || 
							((spells[i].caster==0) && !(GLight[ii]->extras & EXTRAS_NO_IGNIT)))
						&&
						((GLight[ii]->extras & EXTRAS_SEMIDYNAMIC) 
							|| (GLight[ii]->extras & EXTRAS_SPAWNFIRE)
							|| (GLight[ii]->extras & EXTRAS_SPAWNSMOKE) )
						&& (!GLight[ii]->status))
					{
						float dist=EEDistance3D(&target,&GLight[ii]->pos);

						if (dist<=(pIgnit->GetPerimetre()))								
						{
								pIgnit->AddLight(ii);
								bLightInRadius = true;
							}
						}
					}

				for (long n=0;n<MAX_SPELLS;n++)
				{
					if (!spells[n].exist) continue;

					switch (spells[n].type)
					{
						case SPELL_FIREBALL:
						{							
							CSpellFx *pCSpellFX = spells[n].pSpellFx;

							if (pCSpellFX)
							{
								CFireBall *pCF = (CFireBall*) pCSpellFX;
						
							EERIE_SPHERE sphere;
							sphere.origin.x=pCF->eCurPos.x;
							sphere.origin.y=pCF->eCurPos.y;
							sphere.origin.z=pCF->eCurPos.z;
							sphere.radius=__max(spells[i].caster_level*2.f,12.f);

							if (EEDistance3D(&target,&sphere.origin)<pIgnit->GetPerimetre()+sphere.radius)
							{
									spells[n].caster_level += 1; 
							}
						}

						break;
					}
				}
				}
			}

			if (pCSpellFx)
			{
				spells[i].pSpellFx = pCSpellFx;
				CIgnit *pIgnit=(CIgnit *)pCSpellFx;
				spells[i].tolive = pIgnit->GetDuration();
			}

			SPELLCAST_Notify(i);
		}
		break;
		//---------------------------------------------------------------------------------------------
		case SPELL_DOUSE:// Launching DOUSE
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			bool bLightInRadius = false;
			spells[i].exist = TRUE;
			spells[i].tolive = 20000;
					
			CSpellFx *pCSpellFx = NULL;					
			pCSpellFx = new CDoze();
					
			CDoze *pDoze=(CDoze *)pCSpellFx;

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;

				if (spells[i].hand_group>=0)
				{
					target.x=spells[i].hand_pos.x;
					target.y=spells[i].hand_pos.y;
					target.z=spells[i].hand_pos.z;
				}
				else
				{
					target.x=spells[i].caster_pos.x;
					target.y=spells[i].caster_pos.y-50.f;
					target.z=spells[i].caster_pos.z;
				}
						
				float fPerimeter = 400.0f + spells[i].caster_level*30.0f;
				pDoze->CreateDoze(&target, fPerimeter, 500);						
				CheckForIgnition(&target,fPerimeter,0,1);		

				for (long ii=0;ii<MAX_LIGHTS;ii++)
				{
					if (GLight[ii]==NULL) continue;

					if ( (GLight[ii]->extras & EXTRAS_EXTINGUISHABLE)
						&&
						((GLight[ii]->extras & EXTRAS_SEMIDYNAMIC) 
							|| (GLight[ii]->extras & EXTRAS_SPAWNFIRE)
							|| (GLight[ii]->extras & EXTRAS_SPAWNSMOKE) )
							&& (GLight[ii]->status))
					{
						float dist=EEDistance3D(&target,&GLight[ii]->pos);

						if (dist <= (pDoze->GetPerimetre())) 
							{
								pDoze->AddLightDoze(ii);	
								bLightInRadius = true;
							}
						}
					}

				if ((CURRENT_TORCH) && (EEDistance3D(&target,&player.pos)<pDoze->GetPerimetre()))
				{
					ARX_PLAYER_ClickedOnTorch(CURRENT_TORCH);
				}

				for (long n=0;n<MAX_SPELLS;n++)
				{
					if (!spells[n].exist) continue;

					switch (spells[n].type)
					{
						case SPELL_FIREBALL:
						{							
							CSpellFx *pCSpellFX;
							pCSpellFX= spells[n].pSpellFx;

							if (pCSpellFX)
							{
								CFireBall *pCF = (CFireBall*) pCSpellFX;
						
							EERIE_SPHERE sphere;
							sphere.origin.x=pCF->eCurPos.x;
							sphere.origin.y=pCF->eCurPos.y;
							sphere.origin.z=pCF->eCurPos.z;
							sphere.radius=__max(spells[i].caster_level*2.f,12.f);

							if (EEDistance3D(&target,&sphere.origin)<pDoze->GetPerimetre()+sphere.radius)
							{
								spells[n].caster_level-=spells[i].caster_level;

								if (spells[n].caster_level<1)
									spells[n].tolive=0;
							}
						}

						break;
						case SPELL_FIRE_FIELD:
						{					
							EERIE_3D pos;

							if (GetSpellPosition(&pos,n))
							{
								if (EEDistance3D(&target,&pos)<pDoze->GetPerimetre()+200)
								{
									spells[n].caster_level-=spells[i].caster_level;

									if (spells[n].caster_level<1)
										spells[n].tolive=0;
								}
							}
						}
						break;
					}
				}

				}
			}

			if (pCSpellFx)
			{
				spells[i].pSpellFx = pCSpellFx;
				CDoze *pDoze=(CDoze *)pCSpellFx;
				spells[i].tolive = pDoze->GetDuration();
			}

			SPELLCAST_Notify(i);
		}
		break;
		//---------------------------------------------------------------------------------------------
		case SPELL_ACTIVATE_PORTAL:// Launching ACTIVATE_PORTAL
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			ARX_SOUND_PlayInterface(SND_SPELL_ACTIVATE_PORTAL);
			spells[i].exist = TRUE;
			spells[i].tolive = 20;
			SPELLCAST_Notify(i);
		}
		break;			
		//*************************************************************************************************
		// LEVEL 2 SPELLS -----------------------------------------------------------------------------
		case SPELL_HEAL:// Launching HEAL
		{		
		//	return No_MagicAllowed();
			if (ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster)) return -1;

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			if (!(spells[i].flags & SPELLCAST_FLAG_NOSOUND))
				ARX_SOUND_PlaySFX(SND_SPELL_HEALING, &spells[i].caster_pos);					

			spells[i].exist = TRUE;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.4f*spells[i].caster_level;

			if (duration>-1) spells[i].tolive=duration;
			else spells[i].tolive=3500;

			CSpellFx *pCSpellFx = NULL;
			pCSpellFx = new CHeal(GDevice);

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				pCSpellFx->Create();
				pCSpellFx->SetDuration(spells[i].tolive);
				
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			SPELLCAST_Notify(i);
		}
		break;
		//---------------------------------------------------------------------------------------------
		case SPELL_DETECT_TRAP:// Launching DETECT_TRAP
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_DETECT_TRAP,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			if (spells[i].caster==0)
			if (!(spells[i].flags & SPELLCAST_FLAG_NOSOUND))
				ARX_SOUND_PlayInterface(SND_SPELL_DETECT_TRAP);

			spells[i].snd_loop = SND_SPELL_DETECT_TRAP_LOOP;

			if (spells[i].caster==0)
			if (!(spells[i].flags & SPELLCAST_FLAG_NOSOUND))
				ARX_SOUND_PlaySFX(spells[i].snd_loop, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);

			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive = 60000;
			spells[i].fManaCostPerSecond=0.4f;
			spells[i].bDuration = true;
			SPELLCAST_Notify(i);
		}
		break;
		//---------------------------------------------------------------------------------------------
		case SPELL_ARMOR:// Launching ARMOR
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long idx=ARX_SPELLS_GetSpellOn(inter.iobj[spells[i].target],SPELL_ARMOR);

			if (idx>=0)
			{
				spells[idx].tolive = 0;
			}
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;						
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;						
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;						
			}

			if (spells[i].caster==0)
				spells[i].target=spells[i].caster;
			
			if (!(spells[i].flags & SPELLCAST_FLAG_NOSOUND))
				ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_START, &spells[i].target_pos);

			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_LOOP, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);
				
			spells[i].exist = TRUE;

			if (spells[i].caster==0)
				spells[i].tolive = 20000000;
			else
				spells[i].tolive = 20000;

			if (duration>-1) spells[i].tolive=duration;
			
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.2f*spells[i].caster_level;
				
			CSpellFx *pCSpellFx = NULL;
			pCSpellFx = new CArmor();				

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				CArmor *pArmor=(CArmor *)pCSpellFx;
				pArmor->Create(spells[i].tolive, 0);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pArmor->GetDuration();
			}

			ARX_SPELLS_AddSpellOn(spells[i].target,i);
			SPELLCAST_Notify(i);
		}
		break;
		//------------------------------------------------------------------------------------------------
		case SPELL_LOWER_ARMOR:// Launching LOWER_ARMOR
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long idx=ARX_SPELLS_GetSpellOn(inter.iobj[spells[i].target],SPELL_LOWER_ARMOR);

			if (idx>=0)
			{
				spells[idx].tolive = 0;
			}

			if (spells[i].target<0) return -1;

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}
				
			if (!(spells[i].flags & SPELLCAST_FLAG_NOSOUND))
				ARX_SOUND_PlaySFX(SND_SPELL_ARMOR_START, &spells[i].caster_pos);
				
			spells[i].exist = TRUE;

			if (spells[i].caster==0)
				spells[i].tolive = 20000000;
			else
				spells[i].tolive = 20000;

			if (duration>-1) spells[i].tolive=duration;

			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.2f*spells[i].caster_level;
				
			CSpellFx *pCSpellFx = NULL;				
			pCSpellFx = new CLowerArmor();

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				CLowerArmor *pLowerArmor=(CLowerArmor *)pCSpellFx;
				pLowerArmor->Create(spells[i].tolive, spells[i].target);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			ARX_SPELLS_AddSpellOn(spells[i].target,i);
			SPELLCAST_Notify(i);
		}
		break;
		//------------------------------------------------------------------------------------------------
		case SPELL_HARM:// Launching HARM
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			if (!(spells[i].flags & SPELLCAST_FLAG_NOSOUND))
				ARX_SOUND_PlaySFX(SND_SPELL_HARM, &spells[i].caster_pos);

			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LIFE_DRAIN,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_MANA_DRAIN,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			spells[i].exist=TRUE;
			spells[i].tolive=6000000;
			
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.4f;

			spells[i].longinfo=ARX_DAMAGES_GetFree();

			if (spells[i].longinfo!=-1)
			{
				damages[spells[i].longinfo].radius=150.f;
				damages[spells[i].longinfo].damages=4.f;//2.f;
				damages[spells[i].longinfo].area=DAMAGE_FULL;
				damages[spells[i].longinfo].duration=100000000;
				damages[spells[i].longinfo].source=spells[i].caster;
				damages[spells[i].longinfo].flags=DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
				damages[spells[i].longinfo].type=DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL;
				damages[spells[i].longinfo].exist=TRUE;
			}

			spells[i].longinfo2=GetFreeDynLight();

			if (spells[i].longinfo2 != -1)
			{
				long id = spells[i].longinfo2;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 2.3f;
				DynLight[id].fallend = 700.f;
				DynLight[id].fallstart = 500.f;
				DynLight[id].rgb.r = 1.0f;
				DynLight[id].rgb.g = 0.0f;
				DynLight[id].rgb.b = 0.0f;
				DynLight[id].pos.x = spells[i].caster_pos.x;
				DynLight[id].pos.y = spells[i].caster_pos.y;
				DynLight[id].pos.z = spells[i].caster_pos.z;
			}

			if (duration>-1) spells[i].tolive=duration;

			SPELLCAST_Notify(i);				
		}
		break;			
		//**********************************************************************************************
		// LEVEL 3 SPELLS -----------------------------------------------------------------------------
		case SPELL_SPEED:// Launching SPEED
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 2.f;
			ARX_SOUND_PlaySFX(SND_SPELL_SPEED_START, &spells[i].caster_pos);

			if (spells[i].caster==0)
			{			
				spells[i].target = spells[i].caster;
				spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_SPEED_LOOP, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);
			}
				
			spells[i].exist = TRUE;

			if (spells[i].caster==0)
				spells[i].tolive = 200000000;
			else
				spells[i].tolive = 20000;

			if (duration>-1) spells[i].tolive=duration;
			
			
			CSpellFx *pCSpellFx = NULL;
			
			pCSpellFx = new CSpeed();			
			CSpeed *pSpeed = (CSpeed *)pCSpellFx;

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				pSpeed->Create(spells[i].target, spells[i].tolive);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pSpeed->GetDuration();
			}

			SPELLCAST_Notify(i);
			ARX_SPELLS_AddSpellOn(spells[i].target,i);

			if ((spells[i].caster>=0) && (spells[i].target<inter.nbmax))
			{
				if (inter.iobj[spells[i].target])
					inter.iobj[spells[i].target]->speed_modif+=spells[i].caster_level*DIV10;
			}
		}
		break;
		
		//--------------------------------------------------------------------------------------------------
		case SPELL_DISPELL_ILLUSION:// Launching DISPELL_ILLUSION (REVEAL)
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			ARX_SOUND_PlaySFX(SND_SPELL_DISPELL_ILLUSION);
			spells[i].exist = TRUE;
			spells[i].tolive = 1000;
			SPELLCAST_Notify(i);

			for (long n=0;n<MAX_SPELLS;n++)
			{
				if (!spells[n].exist) continue;

				if (spells[n].target==spells[i].caster) continue;

				if (spells[n].caster_level>spells[i].caster_level) continue;

				switch (spells[n].type)
				{
					case SPELL_INVISIBILITY:
					{		
						if (ValidIONum(spells[n].target) && ValidIONum(spells[i].caster))
						{
							if (EEDistance3D(&inter.iobj[spells[n].target]->pos, &inter.iobj[spells[i].caster]->pos)
								<1000)
							{
								spells[n].tolive=0;
							}
						}

						break;
					}
				}
			}
		}
		break;
	
		//----------------------------------------------------------------------------------------------
		
		case SPELL_FIREBALL:// Launching FIREBALL
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive = 20000;
			
			CSpellFx *pCSpellFx = NULL;
			pCSpellFx = new CFireBall();

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;

				if (spells[i].caster!=0)
					spells[i].hand_group=-1;

				if (spells[i].hand_group>=0)
				{
					target.x=spells[i].hand_pos.x;
					target.y=spells[i].hand_pos.y;
					target.z=spells[i].hand_pos.z;
				}
				else
				{
					target.x=spells[i].caster_pos.x;
					target.y=spells[i].caster_pos.y;
					target.z=spells[i].caster_pos.z;

					if ((ValidIONum(spells[i].caster))
						&& (inter.iobj[spells[i].caster]->ioflags & IO_NPC))
					{
						target.x-=EEsin(DEG2RAD(inter.iobj[spells[i].caster]->angle.b))*30.f;
						target.y-=80.f;
						target.z+=EEcos(DEG2RAD(inter.iobj[spells[i].caster]->angle.b))*30.f;
					}
				}
			
				pCSpellFx->SetDuration((unsigned long) (6000));
				CFireBall * cf=(CFireBall *)pCSpellFx;

				if (spells[i].caster==0)
					cf->Create(target,MAKEANGLE(player.angle.b),player.angle.a,spells[i].caster_level);
				else
				{
					float angle;

					if ((spells[i].target>=0) && (spells[i].target<inter.nbmax) && inter.iobj[spells[i].target])
					{
						angle = 0;
					}
					else angle=0;

					EERIE_3D eCurPos;

					eCurPos.x = inter.iobj[spells[i].caster]->pos.x ;
					eCurPos.y = inter.iobj[spells[i].caster]->pos.y;
					eCurPos.z = inter.iobj[spells[i].caster]->pos.z ;

					if ((ValidIONum(spells[i].caster))
						&& (inter.iobj[spells[i].caster]->ioflags & IO_NPC))
					{
						eCurPos.y-=80.f;						
					}

					INTERACTIVE_OBJ * _io=inter.iobj[spells[i].caster];

					if (ValidIONum(_io->targetinfo))
					{
						EERIE_3D * p1=&eCurPos;
						EERIE_3D * p2=&inter.iobj[_io->targetinfo]->pos;
						angle=(RAD2DEG(GetAngle(p1->y,p1->z,p2->y,p2->z+TRUEDistance2D(p2->x,p2->z,p1->x,p1->z))));//alpha entre orgn et dest;				
					}

					cf->Create(target,MAKEANGLE(inter.iobj[spells[i].caster]->angle.b),angle,spells[i].caster_level);					
				}
			}

			if (pCSpellFx)
			{
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_LAUNCH, &spells[i].caster_pos);
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);
			SPELLCAST_Notify(i);
		}
		break;
		//-------------------------------------------------------------------------------------------------
		case SPELL_CREATE_FOOD:// Launching CREATE_FOOD
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FOOD, &spells[i].caster_pos);
			spells[i].exist = TRUE;
			spells[i].tolive = 3500;

			if (duration>-1) spells[i].tolive=duration;
					
			CSpellFx *pCSpellFx = NULL;

			if ((spells[i].caster==0) || (spells[i].target==0))
				player.hunger=100;
		
			pCSpellFx = new CCreateFood(GDevice);

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				pCSpellFx->Create();
				pCSpellFx->SetDuration(spells[i].tolive);						
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			SPELLCAST_Notify(i);
		}
		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_ICE_PROJECTILE:// Launching ICE_PROJECTILE
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			ARX_SOUND_PlaySFX(SND_SPELL_ICE_PROJECTILE_LAUNCH, &spells[i].caster_pos);
			spells[i].exist = TRUE;
			spells[i].tolive = 4200;
				
			CSpellFx *pCSpellFx = NULL;
				
			pCSpellFx = new CIceProjectile(GDevice);
				
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;

				if (spells[i].caster==0)
				{
					target.x = player.pos.x - EEsin(DEG2RAD(MAKEANGLE(player.angle.b)))*150.0f;
					target.y = player.pos.y+160;
					target.z = player.pos.z + EEcos(DEG2RAD(MAKEANGLE(player.angle.b)))*150.0f;
					CIceProjectile *pIP = (CIceProjectile*)pCSpellFx;
					pIP->Create(target, MAKEANGLE(player.angle.b), spells[i].caster_level);
				}
				else
				{
					target.x = inter.iobj[spells[i].caster]->pos.x - EEsin(DEG2RAD(MAKEANGLE(inter.iobj[spells[i].caster]->angle.b)))*150.0f;
					target.y = inter.iobj[spells[i].caster]->pos.y;
					target.z = inter.iobj[spells[i].caster]->pos.z + EEcos(DEG2RAD(MAKEANGLE(inter.iobj[spells[i].caster]->angle.b)))*150.0f;
					CIceProjectile *pIP = (CIceProjectile*)pCSpellFx;
					pIP->Create(target, MAKEANGLE(inter.iobj[spells[i].caster]->angle.b), spells[i].caster_level);
				}

				pCSpellFx->SetDuration(spells[i].tolive);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			SPELLCAST_Notify(i);
		}
		break;
		//***********************************************************************************************	
		// LEVEL 4 SPELLS -----------------------------------------------------------------------------
		case SPELL_BLESS:// Launching BLESS
		{
			if (ARX_SPELLS_ExistAnyInstance(typ)) 
			{
				return -1;
			}

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			if (spells[i].caster==0)
				spells[i].target=0;

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_BLESS,spells[i].target);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			ARX_SOUND_PlaySFX(SND_SPELL_BLESS);
			spells[i].exist = TRUE;
			spells[i].tolive = 2000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.5f*spells[i].caster_level*0.6666f;

			if (duration>-1) spells[i].tolive=duration;

			CSpellFx *pCSpellFx = NULL;
			
			pCSpellFx = new CBless(GDevice);

			
				
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;
				target.x = inter.iobj[spells[i].caster]->pos.x;
				target.y = inter.iobj[spells[i].caster]->pos.y;
				target.z = inter.iobj[spells[i].caster]->pos.z;
				pCSpellFx->Create(target, MAKEANGLE(player.angle.b));
				pCSpellFx->SetDuration(20000);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			ARX_SPELLS_AddSpellOn(spells[i].target,i);
			SPELLCAST_Notify(i);
		}
		break;
		//-----------------------------------------------------------------------------------------------
		case SPELL_DISPELL_FIELD:// Launching DISPELL_FIELD
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;					

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			SPELLCAST_Notify(i);
			spells[i].tolive = 10;
			EERIE_3D target;
			target.x = inter.iobj[spells[i].caster]->pos.x;
			target.y = inter.iobj[spells[i].caster]->pos.y;
			target.z = inter.iobj[spells[i].caster]->pos.z;
			long valid=0;
			long dispelled=0;

			for (long n=0;n<MAX_SPELLS;n++)
			{
				if (!spells[n].exist) continue;

				switch (spells[n].type)
				{					
					case SPELL_CREATE_FIELD:
					{							
						CSpellFx *pCSpellFX = spells[n].pSpellFx;

						if (pCSpellFX)
						{
							CCreateField *pCreateField = (CCreateField *) pCSpellFX;
					
							EERIE_SPHERE sphere;
							sphere.origin.x=pCreateField->eSrc.x;
							sphere.origin.y=pCreateField->eSrc.y;
							sphere.origin.z=pCreateField->eSrc.z;
							sphere.radius=400.f;

							if ((spells[i].caster!=0) || (spells[n].caster==0))
							{
								if (EEDistance3D(&target,&sphere.origin)<sphere.radius)
								{
									valid++;

									if (spells[n].caster_level<=spells[i].caster_level)
									{
										spells[n].tolive=0;
										dispelled++;
									}
								}
							}
						}

						break;
					}
					case SPELL_FIRE_FIELD:
					{							
						CSpellFx *pCSpellFX = spells[n].pSpellFx;

						if (pCSpellFX)
						{
							CFireField *pFireField = (CFireField *) pCSpellFX;
					
							EERIE_SPHERE sphere;
							sphere.origin.x=pFireField->pos.x;
							sphere.origin.y=pFireField->pos.y;
							sphere.origin.z=pFireField->pos.z;
							sphere.radius=400.f;

							if (EEDistance3D(&target,&sphere.origin)<sphere.radius)
							{
								valid++;

								if (spells[n].caster_level<=spells[i].caster_level)
								{
									spells[n].tolive=0;
									dispelled++;
								}
							}
						}

						break;
					}
					case SPELL_ICE_FIELD:
					{							
						CSpellFx *pCSpellFX = spells[n].pSpellFx;

						if (pCSpellFX)
						{
							CIceField *pIceField = (CIceField *) pCSpellFX;
					
							EERIE_SPHERE sphere;
							sphere.origin.x=pIceField->eSrc.x;
							sphere.origin.y=pIceField->eSrc.y;
							sphere.origin.z=pIceField->eSrc.z;
							sphere.radius=400.f;

							if (EEDistance3D(&target,&sphere.origin)<sphere.radius)
							{
								valid++;

								if (spells[n].caster_level<=spells[i].caster_level)
								{
									spells[n].tolive=0;
									dispelled++;
								}
							}
						}

						break;
					}
				}
			}

			if (valid>dispelled)
			{
				ARX_SPEECH_AddSpeech(inter.iobj[0],"[player_not_skilled_enough]",PARAM_LOCALISED,ANIM_TALK_NEUTRAL,ARX_SPEECH_FLAG_NOTEXT);

				if (dispelled>0)
				{
					ARX_SOUND_PlaySFX(SND_SPELL_DISPELL_FIELD);
				}
			}
			else if (valid>0)
			{
				ARX_SOUND_PlaySFX(SND_SPELL_DISPELL_FIELD);
			}
			else
			{
				ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &spells[i].caster_pos);
			}
		}
		break;
		//-----------------------------------------------------------------------------------------------
		case SPELL_FIRE_PROTECTION:// Launching FIRE_PROTECTION
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long idx=ARX_SPELLS_GetSpellOn(inter.iobj[spells[i].target],SPELL_FIRE_PROTECTION);

			if (idx>=0)
			{
				spells[idx].tolive = 0;
			}
	
			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_PROTECTION);
			
			if (duration>-1) spells[i].tolive=duration;

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_COLD_PROTECTION,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}
			
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive = 2000000;

			if (spells[i].caster==0)
				spells[i].target=0;
				
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.f;
			
			CSpellFx *pCSpellFx = NULL;
				
			pCSpellFx = new CFireProtection();
				
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;		
				CFireProtection *pFP=(CFireProtection *)pCSpellFx;
				pFP->Create(spells[i].tolive, spells[i].target);

				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			ARX_SPELLS_AddSpellOn(spells[i].target,i);
			SPELLCAST_Notify(i);
		}
		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_COLD_PROTECTION:// Launching COLD_PROTECTION
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long idx=ARX_SPELLS_GetSpellOn(inter.iobj[spells[i].target],SPELL_COLD_PROTECTION);

			if (idx>=0)
			{
				spells[idx].tolive = 0;
			}

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_ARMOR,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LOWER_ARMOR,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_FIRE_PROTECTION,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_START);

			if (spells[i].caster==0)
				spells[i].target=0;

			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive = 2000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.f;

			if (duration>-1) spells[i].tolive=duration;

			CSpellFx *pCSpellFx =  new CColdProtection();

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;		
				CColdProtection *pCP= (CColdProtection *)pCSpellFx;
				pCP->Create(spells[i].tolive, spells[i].target);
			
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_LOOP, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);
			ARX_SPELLS_AddSpellOn(spells[i].target,i);
			SPELLCAST_Notify(i);
		}
		break;
		//----------------------------------------------------------------------------------------------
		case SPELL_TELEKINESIS:// Launching TELEKINESIS
		{
			if (ARX_SPELLS_ExistAnyInstanceForThisCaster(SPELL_TELEKINESIS,spells[i].caster)) return -1;			

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			spells[i].exist=TRUE;						
			spells[i].tolive=6000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.9f;

			if (duration>-1) spells[i].tolive=duration;

			if (spells[i].caster==0)
				Project.telekinesis=1;

			ARX_SOUND_PlaySFX(SND_SPELL_TELEKINESIS_START, &spells[i].caster_pos);
			SPELLCAST_Notify(i);
		}
		break;
		//-----------------------------------------------------------------------------------------------
		case SPELL_CURSE:// Launching CURSE
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_CURSE,spells[i].target);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}
					
			ARX_SOUND_PlaySFX(SND_SPELL_CURSE, &spells[i].caster_pos);
			spells[i].exist = TRUE;
			spells[i].tolive = 2000000;

			if (duration>-1) spells[i].tolive=duration;

			CSpellFx *pCSpellFx = NULL;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 0.5f*spells[i].caster_level;
					
			pCSpellFx = new CCurse(GDevice);
			
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;
				target.x=spells[i].target_pos.x;
				target.y=spells[i].target_pos.y;

				if ((spells[i].target>=0) && (inter.iobj[spells[i].target]))
				{
					if (spells[i].target==0) target.y-=200.f;
					else target.y+=inter.iobj[spells[i].target]->physics.cyl.height-50.f;
				}

				target.z=spells[i].target_pos.z;
				pCSpellFx->Create(target, MAKEANGLE(player.angle.b));
				pCSpellFx->SetDuration(spells[i].tolive);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			ARX_SPELLS_AddSpellOn(spells[i].target,i);
			SPELLCAST_Notify(i);
		}
		break;
		//*********************************************************************
		// LEVEL 5 SPELLS -----------------------------------------------------------------------------
		case SPELL_RUNE_OF_GUARDING:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_RUNE_OF_GUARDING,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_RUNE_OF_GUARDING);
			spells[i].exist = TRUE;
			spells[i].tolive = 99999999;

			if (duration>-1) spells[i].tolive=duration;

			CSpellFx *pCSpellFx  = new CRuneOfGuarding(GDevice);
					
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;				
				target.x = inter.iobj[spells[i].caster]->pos.x;
				target.y = inter.iobj[spells[i].caster]->pos.y;
				target.z = inter.iobj[spells[i].caster]->pos.z;
				pCSpellFx->Create(target, 0);
				pCSpellFx->SetDuration(spells[i].tolive);

				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			SPELLCAST_Notify(i);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_LEVITATE:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LEVITATE,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_START);
			spells[i].exist = TRUE;
			spells[i].tolive = 2000000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.f;

			if (duration>-1) spells[i].tolive=duration;

			CSpellFx *pCSpellFx =  new CLevitate();
							
			if (pCSpellFx != NULL)
			{
				CLevitate *pLevitate=(CLevitate *)pCSpellFx;
				pCSpellFx->spellinstance=i;
				EERIE_3D target;

				if (	(spells[i].caster==0)
					||	(spells[i].target==0)	)
				{
					target.x=player.pos.x;
					target.y=player.pos.y+150.f;
					target.z=player.pos.z;
					spells[i].target = 0; 
					spells[i].tolive = 200000000;
					player.levitate=1;
				}
				else
				{
					target.x=inter.iobj[spells[i].target]->pos.x;
					target.y = inter.iobj[spells[i].target]->pos.y; 
					target.z=inter.iobj[spells[i].target]->pos.z;
				}

				pLevitate->Create(16,50.f,100.f,80.f,&target,spells[i].tolive);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			ARX_SPELLS_AddSpellOn(spells[i].target,i);
			SPELLCAST_Notify(i);				
		}
		break;
		//----------------------------------------------------------------------------
		case SPELL_CURE_POISON:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			float cure=spells[i].caster_level*10;

			if (spells[i].caster==0) 
				spells[i].target=0; 

			if (spells[i].target==0) 
			{
				player.poison-=cure;

				if (player.poison<0.f) player.poison=0;

				ARX_SOUND_PlaySFX(SND_SPELL_CURE_POISON);
			}
			else
			{
				if (ValidIONum(spells[i].target))
				{
					INTERACTIVE_OBJ * io=inter.iobj[spells[i].target];

					if (io->ioflags & IO_NPC)
					{
						io->_npcdata->poisonned-=cure;

						if (io->_npcdata->poisonned<0) io->_npcdata->poisonned=0;
					}

					ARX_SOUND_PlaySFX(SND_SPELL_CURE_POISON,&io->pos);
				}
			}
			
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive = 3500;
					
			CSpellFx *pCSpellFx = new CCurePoison(GDevice);
							
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				pCSpellFx->Create();
				pCSpellFx->SetDuration(spells[i].tolive);

				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
				
			}

			SPELLCAST_Notify(i);				
		}
		break;
		//----------------------------------------------------------------------------
		case SPELL_REPEL_UNDEAD:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_REPEL_UNDEAD,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}
			
			ARX_SOUND_PlaySFX(SND_SPELL_REPEL_UNDEAD);

			if (spells[i].caster==0)
				spells[i].target=0;

			if (spells[i].target==0)
				spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_REPEL_UNDEAD_LOOP, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);

			spells[i].exist = TRUE;
			spells[i].tolive = 20000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.f;

			if (duration>-1) spells[i].tolive=duration;

			CSpellFx *pCSpellFx =  new CRepelUndead(GDevice);
			
			if (pCSpellFx != NULL)
			{
				EERIE_3D target;
				target.x=player.pos.x;
				target.y=player.pos.y;
				target.z=player.pos.z;
				pCSpellFx->spellinstance=i;
				pCSpellFx->Create(target, MAKEANGLE(player.angle.b));
				pCSpellFx->SetDuration(spells[i].tolive);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			SPELLCAST_Notify(i);	
		}
		break;
		//----------------------------------------------------------------------------
		case SPELL_POISON_PROJECTILE:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			ARX_SOUND_PlaySFX(SND_SPELL_POISON_PROJECTILE_LAUNCH, &spells[i].caster_pos);
			spells[i].exist = TRUE;
			spells[i].tolive = 900000000;
					
			CSpellFx *pCSpellFx = NULL;

			ARX_CHECK_LONG(spells[i].caster_level);   
			pCSpellFx = new CMultiPoisonProjectile( GDevice, __max( ARX_CLEAN_WARN_CAST_LONG( spells[i].caster_level ), 1 ) );


			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;
				Vector_Init(&target);					
				pCSpellFx->SetDuration((unsigned long) (8000));
				float ang;

				if (spells[i].caster==0) ang=player.angle.b;
				else ang=inter.iobj[spells[i].caster]->angle.b;

				pCSpellFx->Create(target, MAKEANGLE(ang));
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();				
			}

			SPELLCAST_Notify(i);			
		}
		break;
		//***************************************************************************
		// LEVEL 6 -----------------------------------------------------------------------------
		case SPELL_RISE_DEAD:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_RISE_DEAD,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}
			
			float beta;
			EERIE_3D target;

			if (spells[i].caster==0)
			{
				target.x=player.pos.x - EEsin(DEG2RAD(MAKEANGLE(player.angle.b)))*300.f;
				target.y = player.pos.y + 170.f; 
				target.z=player.pos.z + EEcos(DEG2RAD(MAKEANGLE(player.angle.b)))*300.f;
				beta=MAKEANGLE(player.angle.b);
				
			}
			else
			{
				if (inter.iobj[spells[i].caster]->ioflags & IO_NPC)
				{
					target.x=inter.iobj[spells[i].caster]->pos.x - EEsin(DEG2RAD(MAKEANGLE(inter.iobj[spells[i].caster]->angle.b)))*300.f;
					target.y = inter.iobj[spells[i].caster]->pos.y; 
					target.z=inter.iobj[spells[i].caster]->pos.z + EEcos(DEG2RAD(MAKEANGLE(inter.iobj[spells[i].caster]->angle.b)))*300.f;					
				}
				else
				{
					target.x=inter.iobj[spells[i].caster]->pos.x;
					target.y=inter.iobj[spells[i].caster]->pos.y;
					target.z=inter.iobj[spells[i].caster]->pos.z;					
				}

				beta=MAKEANGLE(inter.iobj[spells[i].caster]->angle.b);
			}

			if (!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target))
			{
				ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
				return -1;
			}

			Vector_Copy(&spells[i].target_pos,&target);
			ARX_SOUND_PlaySFX(SND_SPELL_RAISE_DEAD, &spells[i].caster_pos);
			spells[i].exist = TRUE;
			spells[i].tolive = 2000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.2f;
			spells[i].longinfo=-1;

			if (duration>-1) spells[i].tolive=duration;

			CSpellFx *pCSpellFx = NULL;
					
			pCSpellFx = new CRiseDead(GDevice);
					
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;					
				{
					pCSpellFx->Create(target, beta);
					CRiseDead *pRiseDead = (CRiseDead*) pCSpellFx;
					pRiseDead->SetDuration(2000, 500, 1800);
					pRiseDead->SetColorBorder(0.5, 0.5, 0.5);
					pRiseDead->SetColorRays1(0.83f, 0.73f, 0.63f);
					pRiseDead->SetColorRays2(0,0,0);
					pRiseDead->SetColorRays1(0.5, 0.5, 0.5);
					pRiseDead->SetColorRays2(1, 0, 0);

					if (pRiseDead->lLightId == -1)
					{
						pRiseDead->lLightId = GetFreeDynLight();
					}

					if (pRiseDead->lLightId != -1)
					{
						long id=pRiseDead->lLightId;
						DynLight[id].exist=1;
						DynLight[id].intensity = 1.3f;
						DynLight[id].fallend=450.f;
						DynLight[id].fallstart=380.f;
						DynLight[id].rgb.r=0.0f;
						DynLight[id].rgb.g=0.0f;
						DynLight[id].rgb.b=0.0f;
						DynLight[id].pos.x = target.x;
						DynLight[id].pos.y = target.y - 100;
						DynLight[id].pos.z = target.z;
						DynLight[id].duration=200;
						DynLight[id].time_creation = ARXTimeUL();
					}

					spells[i].pSpellFx = pCSpellFx;
					spells[i].tolive = pCSpellFx->GetDuration();
				}
			}

			SPELLCAST_Notify(i);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_PARALYSE:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;			

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			ARX_SOUND_PlaySFX(SND_SPELL_PARALYSE, &spells[i].caster_pos);
			spells[i].exist = TRUE;
			spells[i].tolive = 5000;
			
			if (duration>-1) spells[i].tolive=duration;

			if (ValidIONum(spells[i].target))
			{
				if (	(spells[i].target==0)
					&& (spells[i].caster_level<=player.level)	)
				{
					float mul=player.resist_magic;

					if (rnd()*100.f<mul)
					{
						mul*=DIV200;
						mul=1.f-mul;

						if (mul<0.5f) mul=0.5f;

						spells[i].tolive=(long)(float)(spells[i].tolive*mul);
					}
				}
				else
				{
				INTERACTIVE_OBJ * ioo=inter.iobj[spells[i].target];

				if (ioo->ioflags & IO_NPC)
				{
					float mul=ioo->_npcdata->resist_magic;

					if (rnd()*100.f<mul)
					{
						mul*=DIV200;
						mul=1.f-mul;

						if (mul<0.5f) mul=0.5f;

						spells[i].tolive=(long)(float)(spells[i].tolive*mul);
					}
				}
				}
			}
					
			inter.iobj[spells[i].target]->ioflags |= IO_FREEZESCRIPT;
			ARX_SPELLS_AddSpellOn(spells[i].target,i);
			ARX_NPC_Kill_Spell_Launch(inter.iobj[spells[i].target]);
			SPELLCAST_Notify(i);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_CREATE_FIELD:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			spells[i].exist = TRUE;

			if (flags & SPELLCAST_FLAG_RESTORE)
			{				
				if ((float)ARXTime-4000>0)
					spells[i].lastupdate = spells[i].timcreation = ARXTimeUL() - 4000;
				else
					spells[i].lastupdate = spells[i].timcreation=0;
			}
			else
				spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();

			spells[i].tolive = 800000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.2f;

			if (duration>-1) spells[i].tolive=duration;


			EERIE_3D target;

			if (spells[i].caster==0)
			{
				target.x = inter.iobj[0]->pos.x - EEsin(DEG2RAD(MAKEANGLE(player.angle.b)))*250.f;
				target.y = inter.iobj[0]->pos.y;
				target.z = inter.iobj[0]->pos.z + EEcos(DEG2RAD(MAKEANGLE(player.angle.b)))*250.f;
			}
			else			
			{
				if (ValidIONum(spells[i].caster))
				{
					INTERACTIVE_OBJ * io=inter.iobj[spells[i].caster];

					if (io->ioflags & IO_NPC)
					{
						target.x = io->pos.x - EEsin(DEG2RAD(MAKEANGLE(io->angle.b)))*250.f;
						target.y = io->pos.y;
						target.z = io->pos.z + EEcos(DEG2RAD(MAKEANGLE(io->angle.b)))*250.f;
					}
					else
					{
						target.x=io->pos.x;					
						target.y=io->pos.y;
						target.z=io->pos.z;
					}
				}
			}

			ARX_SOUND_PlaySFX(SND_SPELL_CREATE_FIELD, &target);

			CSpellFx * pCSpellFx  = new CCreateField(GDevice);

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				char tmptext[256];
				sprintf(tmptext,"%sGraph\\Obj3D\\Interactive\\Fix_inter\\blue_cube\\blue_cube.asl",Project.workingdir);
				INTERACTIVE_OBJ * io;
				io=AddFix(GDevice,tmptext,IO_IMMEDIATELOAD);

				if (io)
				{
					ARX_INTERACTIVE_HideGore(io);								
					RestoreInitialIOStatusOfIO(io);
					spells[i].longinfo=GetInterNum(io);
					io->scriptload=1;
					io->ioflags|=IO_NOSAVE;
					io->ioflags|=IO_FIELD;

					io->initpos.x=io->pos.x=target.x;
					io->initpos.y=io->pos.y=target.y;
					io->initpos.z=io->pos.z=target.z;
					MakeTemporaryIOIdent(io);						
					SendInitScriptEvent(io);

					CCreateField *pCreateField = (CCreateField *) pCSpellFx;
					
					pCreateField->Create(target, 0);

					pCreateField->SetDuration(spells[i].tolive);
					pCreateField->lLightId = GetFreeDynLight();

					if (pCreateField->lLightId != -1)
					{
						long id=pCreateField->lLightId;
						DynLight[id].exist=1;
						DynLight[id].intensity = 0.7f + 2.3f;
						DynLight[id].fallend = 500.f;
						DynLight[id].fallstart = 400.f;
						DynLight[id].rgb.r = 0.8f;
						DynLight[id].rgb.g = 0.0f;
						DynLight[id].rgb.b = 1.0f;
						DynLight[id].pos.x = pCreateField->eSrc.x;
						DynLight[id].pos.y = pCreateField->eSrc.y-150;
						DynLight[id].pos.z = pCreateField->eSrc.z;
					}

					spells[i].pSpellFx = pCSpellFx;
					spells[i].tolive = pCSpellFx->GetDuration();

					if (flags & SPELLCAST_FLAG_RESTORE)
					{
						pCreateField->Update(4000);
					}
				}
				else spells[i].tolive=0;
			}

			SPELLCAST_Notify(i);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_DISARM_TRAP:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			ARX_SOUND_PlaySFX(SND_SPELL_DISARM_TRAP);
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive = 1;

			CSpellFx *pCSpellFx = new CDisarmTrap(GDevice);
					
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;
				target.x=player.pos.x;
				target.y=player.pos.y;
				target.z=player.pos.z;
				pCSpellFx->Create(target, MAKEANGLE(player.angle.b));
				pCSpellFx->SetDuration(spells[i].tolive);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
				
				SPELLCAST_Notify(i);

				for (long n=0;n<MAX_SPELLS;n++)
				{
					if (!spells[n].exist) continue;

					switch (spells[n].type)
					{					
						case SPELL_RUNE_OF_GUARDING:
						{							
							CSpellFx *pCSpellFX = spells[n].pSpellFx;

							if (pCSpellFX)
							{
								CRuneOfGuarding * crg=(CRuneOfGuarding *)pCSpellFX;
								EERIE_SPHERE sphere;
								sphere.origin.x=crg->eSrc.x;
								sphere.origin.y=crg->eSrc.y;
								sphere.origin.z=crg->eSrc.z;
								sphere.radius=400.f;

								if (EEDistance3D(&target,&sphere.origin)<sphere.radius)
								{
									spells[n].caster_level-=spells[i].caster_level;

									if (spells[n].caster_level<=0)
									{
										spells[n].tolive=0;
									}
								}
							}

							break;
						}
					}
				}
			}
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_SLOW_DOWN:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			bool bOk = true;

			INTERACTIVE_OBJ *io = inter.iobj[spells[i].target];

			for (int il=0; il<io->nb_spells_on; il++)
			{
				if (spells[io->spells_on[il]].type == SPELL_SLOW_DOWN)
				{
					bOk = false;
					spells[i].exist = false;
					return -1;
				}
			}

			if (bOk)
			{
				ARX_SOUND_PlaySFX(SND_SPELL_SLOW_DOWN, &spells[i].caster_pos);
				spells[i].exist = TRUE;

				if (spells[i].caster==0)
					spells[i].tolive = 10000000;
				else
					spells[i].tolive = 10000;

				if (duration>-1) spells[i].tolive=duration;

				spells[i].pSpellFx = NULL;
				spells[i].bDuration = true;
				spells[i].fManaCostPerSecond = 1.2f;
				
				CSpellFx *pCSpellFx = new CSlowDown(GDevice);
				
				if (pCSpellFx != NULL)
				{
					pCSpellFx->spellinstance=i;
					EERIE_3D target;
					target.x=spells[i].target_pos.x;
					target.y=spells[i].target_pos.y;
					target.z=spells[i].target_pos.z;
					pCSpellFx->Create(target, MAKEANGLE(player.angle.b));
					pCSpellFx->SetDuration(spells[i].tolive);
					spells[i].pSpellFx = pCSpellFx;
					spells[i].tolive = pCSpellFx->GetDuration();
				}

				SPELLCAST_Notify(i);
				
				
				ARX_SPELLS_AddSpellOn(spells[i].target,i);

				if ((spells[i].target>=0) && (spells[i].target<inter.nbmax))
				{
					if (inter.iobj[spells[i].target])
					{
							inter.iobj[spells[i].target]->speed_modif -= spells[i].caster_level*DIV20;
					}
				}
			}
		}	
		break;
		//****************************************************************************************
		// LEVEL 7 SPELLS -----------------------------------------------------------------------------
		case SPELL_FLYING_EYE:
		{	
			if (eyeball.exist!=0) return -1;

			if (ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster)) return -1;

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			if (spells[i].caster==0)
				spells[i].target=0;

			if (spells[i].target!=0)
				return -1;

			ARX_SOUND_PlaySFX(SND_SPELL_EYEBALL_IN);
			spells[i].exist=TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive=1000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 3.2f;
			eyeball.exist=1;
			eyeball.pos.x=player.pos.x-(float)EEsin(DEG2RAD(MAKEANGLE(player.angle.b)))*200.f;
			eyeball.pos.y=player.pos.y+50.f;
			eyeball.pos.z=player.pos.z+(float)EEcos(DEG2RAD(MAKEANGLE(player.angle.b)))*200.f;
			eyeball.angle.a=player.angle.a;
			spells[i].v.y=eyeball.angle.b=player.angle.b;
			eyeball.angle.g=player.angle.g;
			long j;

			for (long n=0;n<12;n++)
			{
				j=ARX_PARTICLES_GetFree();

				if ((j!=-1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					PARTICLE_DEF * pd=&particle[j];
					pd->exist=TRUE;
					pd->zdec=0;							
 
					pd->ov.x=eyeball.pos.x+5.f-rnd()*10.f;
					pd->ov.y=eyeball.pos.y+5.f-rnd()*10.f;
					pd->ov.z=eyeball.pos.z+5.f-rnd()*10.f;
					pd->move.x=2.f-4.f*rnd();
					pd->move.y=2.f-4.f*rnd();
					pd->move.z=2.f-4.f*rnd();
					pd->siz=28.f;
					pd->tolive=2000+(unsigned long)(float)(rnd()*4000.f);
					pd->scale.x=12.f;
					pd->scale.y=12.f;
					pd->scale.z=12.f;
					pd->timcreation=spells[i].lastupdate;
					pd->tc=tc4;
					pd->special=FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					pd->fparam=0.0000001f;
					pd->r=0.7f;
					pd->g=0.7f;
					pd->b=1.f;
				}
			}

			TRUE_PLAYER_MOUSELOOK_ON |= 1;	
			SLID_START=(float)ARXTime;
			bOldLookToggle=pMenuConfig->bMouseLookToggle;
			pMenuConfig->bMouseLookToggle=true;

			SPELLCAST_Notify(i);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_FIRE_FIELD:
		{
			if ( !CanPayMana( i, ARX_SPELLS_GetManaCost( typ, i ) ) )
			{
				return -1;
			}

			if ( !GLOBAL_MAGIC_MODE )
			{
				return No_MagicAllowed();
			}
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster( SPELL_FIRE_FIELD, spells[i].caster );
			
			if ( iCancel > -1 )
			{
				spells[iCancel].tolive = 0;
			}
					
			ARX_SOUND_PlaySFX( SND_SPELL_FIRE_FIELD_START );
			spells[i].exist					= TRUE;
			spells[i].tolive				= 100000;
			spells[i].bDuration				= true;
			spells[i].fManaCostPerSecond	= 2.8f;
			spells[i].longinfo2				= -1;
			
			
			if ( duration > -1 ) spells[i].tolive = duration;
			
			CSpellFx *pCSpellFx = new CFireField();

			if ( pCSpellFx != NULL )
			{
				pCSpellFx->spellinstance	= i;
				CFireField *pFireField		= (CFireField *)pCSpellFx;

				EERIE_3D target;

				if ( spells[i].caster == 0 )
				{					
					target.x = player.pos.x - EEsin(DEG2RAD(MAKEANGLE(player.angle.b)))*250.f;
					target.y = player.pos.y + 170;
					target.z = player.pos.z + EEcos(DEG2RAD(MAKEANGLE(player.angle.b)))*250.f;
				}
				else			
				{
					if ( ValidIONum( spells[i].caster ) )
					{
						INTERACTIVE_OBJ * io = inter.iobj[spells[i].caster];

						if ( io->ioflags & IO_NPC )
						{
							target.x = io->pos.x - EEsin( DEG2RAD( MAKEANGLE( io->angle.b ) ) ) * 250.f;
							target.y = io->pos.y;
							target.z = io->pos.z + EEcos( DEG2RAD( MAKEANGLE( io->angle.b ) ) ) * 250.f;
						}
						else
						{
							target.x = io->pos.x;					
							target.y = io->pos.y;
							target.z = io->pos.z;
						}
					}

					else
					{
						ARX_CHECK_NO_ENTRY();
						target.x = 0;					
						target.y = 0;
						target.z = 0;
					}


				}

				spells[i].longinfo = ARX_DAMAGES_GetFree();

				if ( spells[i].longinfo != -1 )
				{
					damages[spells[i].longinfo].radius  = 150.f;
					damages[spells[i].longinfo].damages = 10.f;
					damages[spells[i].longinfo].area	= DAMAGE_FULL;
					damages[spells[i].longinfo].duration= 100000000;
					damages[spells[i].longinfo].source	= spells[i].caster;
					damages[spells[i].longinfo].flags	= 0;
					damages[spells[i].longinfo].type	= DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_FIRE | DAMAGE_TYPE_FIELD;
					damages[spells[i].longinfo].exist	= TRUE;
					damages[spells[i].longinfo].pos.x	= target.x;
					damages[spells[i].longinfo].pos.y	= target.y;
					damages[spells[i].longinfo].pos.z	= target.z;
				}

				pFireField->Create( 200.f, &target, spells[i].tolive );

				spells[i].pSpellFx	= pCSpellFx;
				spells[i].tolive	= pFireField->GetDuration();
				spells[i].snd_loop	= ARX_SOUND_PlaySFX( SND_SPELL_FIRE_FIELD_LOOP, &target, 1.0F, ARX_SOUND_PLAY_LOOPED );
			}

			SPELLCAST_Notify( i );			
		}
		break;
		//----------------------------------------------------------------------------
		case SPELL_ICE_FIELD:
		{
			if ( !CanPayMana( i, ARX_SPELLS_GetManaCost( typ, i ) ) )
			{
				return -1;
			}

			if ( !GLOBAL_MAGIC_MODE )
			{
				return No_MagicAllowed();
			}
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster( SPELL_ICE_FIELD, spells[i].caster );

			if ( iCancel > -1 )
			{
				spells[iCancel].tolive = 0;
			}
				
			ARX_SOUND_PlaySFX(SND_SPELL_ICE_FIELD);
			spells[i].exist			= TRUE;
			spells[i].lastupdate	= spells[i].timcreation = ARXTimeUL();
			spells[i].tolive		= 100000;
			spells[i].bDuration		= true;
			spells[i].fManaCostPerSecond = 2.8f;
			spells[i].longinfo2		=-1;

			if ( duration > -1 ) 
			{
				spells[i].tolive = duration;
			}

			CSpellFx *pCSpellFx = new CIceField( GDevice );
					
			if ( pCSpellFx != NULL )
			{
				pCSpellFx->spellinstance = i;
				EERIE_3D target;

				if ( spells[i].caster == 0 )						
				{
					target.x = player.pos.x - EEsin( DEG2RAD( MAKEANGLE( player.angle.b ) ) ) * 250.f;
					target.y = player.pos.y + 170;
					target.z = player.pos.z + EEcos( DEG2RAD( MAKEANGLE( player.angle.b ) ) ) * 250.f;
				}
				else			
				{
					if ( ValidIONum( spells[i].caster ) )
					{
						INTERACTIVE_OBJ * io = inter.iobj[spells[i].caster];

						if ( io->ioflags & IO_NPC )
						{
							target.x = io->pos.x - EEsin( DEG2RAD( MAKEANGLE( io->angle.b ) ) ) * 250.f;
							target.y = io->pos.y;
							target.z = io->pos.z + EEcos( DEG2RAD( MAKEANGLE( io->angle.b ) ) ) * 250.f;
						}
						else
						{
							target.x = io->pos.x;					
							target.y = io->pos.y;
							target.z = io->pos.z;
						}
					}

					else
					{
						ARX_CHECK_NO_ENTRY();
						target.x = 0;					
						target.y = 0;
						target.z = 0;
					}


				}

				spells[i].longinfo = ARX_DAMAGES_GetFree();

				if ( spells[i].longinfo != -1 )
				{
					damages[spells[i].longinfo].radius  = 150.f;
					damages[spells[i].longinfo].damages = 10.f;
					damages[spells[i].longinfo].area	= DAMAGE_FULL;
					damages[spells[i].longinfo].duration= 100000000;
					damages[spells[i].longinfo].source	= spells[i].caster;
					damages[spells[i].longinfo].flags	= 0;
					damages[spells[i].longinfo].type	= DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_COLD | DAMAGE_TYPE_FIELD;
					damages[spells[i].longinfo].exist	= TRUE;
					damages[spells[i].longinfo].pos.x	= target.x;
					damages[spells[i].longinfo].pos.y	= target.y;
					damages[spells[i].longinfo].pos.z	= target.z;
				}

				pCSpellFx->Create( target, MAKEANGLE( player.angle.b ) );
				pCSpellFx->SetDuration( spells[i].tolive );
				spells[i].pSpellFx	= pCSpellFx;
				spells[i].tolive	= pCSpellFx->GetDuration();
			}

			SPELLCAST_Notify( i );
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_LIGHTNING_STRIKE:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			spells[i].exist = TRUE;
				
			CSpellFx *pCSpellFx = new CLightning();
				
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				CLightning *pL = (CLightning*) pCSpellFx;
				EERIE_3D source, target;
				source.x = 0;
				source.y = 0;
				source.z = 0;
				target.x = 0;
				target.y = 0;
				target.z = -500;
				pL->Create(source, target, MAKEANGLE(player.angle.b));
				pL->SetDuration((long)(500*spells[i].caster_level));
				pL->lSrc = 0;
					
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_START, &spells[i].caster_pos);
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_LOOP, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);
			SPELLCAST_Notify(i);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_CONFUSE:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			ARX_SOUND_PlaySFX(SND_SPELL_CONFUSE);
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.5f;

			if (duration>-1) spells[i].tolive=duration;	

			CSpellFx *pCSpellFx = new CConfuse(GDevice);			

			if (pCSpellFx)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;
				target.x=player.pos.x;
				target.y=player.pos.y;
				target.z=player.pos.z;
				pCSpellFx->Create(target, MAKEANGLE(player.angle.b));
				pCSpellFx->SetDuration(spells[i].tolive);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

		
			ARX_SPELLS_AddSpellOn(spells[i].target,i);
			long source = spells[i].caster;
			char spell[128];

			if (MakeSpellName(spell,spells[i].type))
			{
				if (ValidIONum(spells[i].target))				
				{
					if (source >= 0) 
						EVENT_SENDER = inter.iobj[source];
					else 
						EVENT_SENDER = NULL;

					char param[256];
					sprintf(param,"%s %d",spell,(long)spells[i].caster_level);
					SendIOScriptEvent(inter.iobj[spells[i].target], SM_SPELLCAST, param);
				}
			}	
		
		}	
		break;
		//*********************************************************************************
		// LEVEL 8 SPELLS -----------------------------------------------------------------------------
		case SPELL_INVISIBILITY:
		{
			if (ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster)) return -1;

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			spells[i].exist=TRUE;
			spells[i].tolive=6000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 3.f;

			if (duration>-1) spells[i].tolive=duration;

			if (spells[i].caster==0)
				spells[i].target=0;

			inter.iobj[spells[i].target]->GameFlags|=GFLAG_INVISIBILITY;
			inter.iobj[spells[i].target]->invisibility=0.f;
			ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_START, &spells[i].caster_pos);
			SPELLCAST_Notify(i);				
			ARX_SPELLS_AddSpellOn(spells[i].target,i);
		}
		break;
		//----------------------------------------------------------------------------
		case SPELL_MANA_DRAIN:
		{
			if (ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster)) return -1;
			
			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LIFE_DRAIN,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_HARM,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			spells[i].exist=TRUE;
			spells[i].tolive=6000000;					
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 2.f;
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD, &spells[i].caster_pos, 1.2F, ARX_SOUND_PLAY_LOOPED);

			if (duration>-1) spells[i].tolive=duration;

			spells[i].longinfo=ARX_DAMAGES_GetFree();

			if (spells[i].longinfo!=-1)
			{
				damages[spells[i].longinfo].radius=150.f;
				damages[spells[i].longinfo].damages = 8.f;
				damages[spells[i].longinfo].area=DAMAGE_FULL;
				damages[spells[i].longinfo].duration=100000000;
				damages[spells[i].longinfo].source=spells[i].caster;
				damages[spells[i].longinfo].flags=DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
				damages[spells[i].longinfo].type=DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_DRAIN_MANA;
				damages[spells[i].longinfo].exist=TRUE;
			}

			spells[i].longinfo2=GetFreeDynLight();

			if (spells[i].longinfo2 != -1)
			{
				long id = spells[i].longinfo2;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 2.3f;
				DynLight[id].fallend = 700.f;
				DynLight[id].fallstart = 500.f;
				DynLight[id].rgb.r = 0.0f;
				DynLight[id].rgb.g = 0.0f;
				DynLight[id].rgb.b = 1.0f;
				DynLight[id].pos.x = spells[i].caster_pos.x;
				DynLight[id].pos.y = spells[i].caster_pos.y;
				DynLight[id].pos.z = spells[i].caster_pos.z;
				DynLight[id].duration=900;
			}

			SPELLCAST_Notify(i);				
		}
		break;
		//----------------------------------------------------------------------------
		case SPELL_EXPLOSION:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			ARX_SOUND_PlaySFX(SND_SPELL_EXPLOSION);
					
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive = 2000;
			
			spells[i].longinfo=ARX_DAMAGES_GetFree();
			EERIE_3D target;
			target.x=inter.iobj[spells[i].caster]->pos.x;
			target.y=inter.iobj[spells[i].caster]->pos.y-60.f;

			if (spells[i].caster==0)
					target.y+=120.f;

			target.z=inter.iobj[spells[i].caster]->pos.z;

			if (spells[i].longinfo!=-1)
			{
				damages[spells[i].longinfo].radius=350.f;
				damages[spells[i].longinfo].damages=10.f;
				damages[spells[i].longinfo].area = 0; 
				damages[spells[i].longinfo].duration=spells[i].tolive;
				damages[spells[i].longinfo].source=spells[i].caster;
				damages[spells[i].longinfo].flags=DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
				damages[spells[i].longinfo].type=DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL;
				damages[spells[i].longinfo].exist=TRUE;
				damages[spells[i].longinfo].pos.x=target.x;
				damages[spells[i].longinfo].pos.y=target.y;
				damages[spells[i].longinfo].pos.z=target.z;
			}

			spells[i].longinfo2=GetFreeDynLight();

			if (spells[i].longinfo2 != -1)
			{
				long id = spells[i].longinfo2;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 2.3f;
				DynLight[id].fallend = 700.f;
				DynLight[id].fallstart = 500.f;
				DynLight[id].rgb.r = 0.1f+rnd()*DIV3;
				DynLight[id].rgb.g = 0.1f+rnd()*DIV3;
				DynLight[id].rgb.b = 0.8f+rnd()*DIV5;
				DynLight[id].pos.x = target.x;
				DynLight[id].pos.y = target.y;
				DynLight[id].pos.z = target.z;
				DynLight[id].duration=200;
			}

			AddQuakeFX(300,2000,400,1);

			for ( long i_angle = 0 ; i_angle < 360 ; i_angle += 12 )
			{
				for ( long j = -100 ; j < 100 ; j += 50 )
				{	
					long		lvl;
					float		rr,		r2;
					EERIE_3D	pos,	dir;

					F2L( ( rnd() * 9.f + 4.f ), &lvl );
					rr		=	DEG2RAD( i_angle );
					r2		=	DEG2RAD( (float) ( j + 100 ) * DIV200 * 360.f ); 
					pos.x	=	target.x - EEsin(rr) * 360.f;  
					pos.y	=	target.y;
					pos.z	=	target.z + EEcos(rr) * 360.f;  
					dir.x	=	pos.x - target.x;
					dir.y	=	0;
					dir.z	=	pos.z - target.z;
					TRUEVector_Normalize( &dir );
					dir.x	*=	60.f;
					dir.y	*=	60.f;
					dir.z	*=	60.f;

					EERIE_RGB	rgb; 
					rgb.r	=	0.1f + rnd() * DIV3;
					rgb.g	=	0.1f + rnd() * DIV3;
					rgb.b	=	0.8f + rnd() * DIV5;

					EERIE_3D posi;
					posi.x	=	target.x;
					posi.y	=	target.y + j * 2;
					posi.z	=	target.z;
					
					LaunchFireballBoom( &posi, 16, &dir, &rgb );
				}
			}



			ARX_SOUND_PlaySFX(SND_SPELL_FIRE_WIND);
			SPELLCAST_Notify(i);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_ENCHANT_WEAPON:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			spells[i].exist = TRUE;
			spells[i].tolive = 20;
				
			SPELLCAST_NotifyOnlyTarget(i);
		}
		break;			
		//----------------------------------------------------------------------------
		case SPELL_LIFE_DRAIN:
		{
			if (ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster)) return -1;

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_HARM,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}

			iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_MANA_DRAIN,spells[i].caster);

			if (iCancel > -1)
			{
				spells[iCancel].tolive = 0;
			}				

			spells[i].exist=TRUE;
			spells[i].tolive=6000000;
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 12.f;

			if (duration>-1) spells[i].tolive=duration;

			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_MAGICAL_SHIELD, &spells[i].caster_pos, 0.8F, ARX_SOUND_PLAY_LOOPED);
			spells[i].longinfo=ARX_DAMAGES_GetFree();

			if (spells[i].longinfo!=-1)
			{
				long id=spells[i].longinfo;
				damages[id].radius=150.f;
				damages[id].damages=spells[i].caster_level*DIV10*.8f;
				damages[id].area = 0; 
				damages[id].duration=100000000;
				damages[id].source=spells[i].caster;
				damages[id].flags=DAMAGE_FLAG_DONT_HURT_SOURCE | DAMAGE_FLAG_FOLLOW_SOURCE | DAMAGE_FLAG_ADD_VISUAL_FX;
				damages[id].type=DAMAGE_TYPE_FAKEFIRE | DAMAGE_TYPE_MAGICAL | DAMAGE_TYPE_DRAIN_LIFE;
				damages[id].exist=TRUE;
			}

			spells[i].longinfo2=GetFreeDynLight();

			if (spells[i].longinfo2 != -1)
			{
				long id = spells[i].longinfo2;
				DynLight[id].exist = 1;
				DynLight[id].intensity = 2.3f;
				DynLight[id].fallend = 700.f;
				DynLight[id].fallstart = 500.f;
				DynLight[id].rgb.r = 1.0f;
				DynLight[id].rgb.g = 0.0f;
				DynLight[id].rgb.b = 0.0f;
				DynLight[id].pos.x = spells[i].caster_pos.x;
				DynLight[id].pos.y = spells[i].caster_pos.y;
				DynLight[id].pos.z = spells[i].caster_pos.z;
				DynLight[id].duration=900;
			}

			SPELLCAST_Notify(i);				
		}
		break;
		//*****************************************************************************************
		// LEVEL 9 SPELLS -----------------------------------------------------------------------------
		case SPELL_SUMMON_CREATURE:
		{
			if (spells[i].caster_level>=9)
			{
				if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;
			}
			else if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE);
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.9f;
			spells[i].longinfo=0;
			spells[i].longinfo2=0;

			if (duration>-1) 
				spells[i].tolive=duration;	
			else
				spells[i].tolive = 2000000;
			

			float beta;
			EERIE_3D target;

			if (spells[i].caster==0)
			{
				target.x=player.pos.x - EEsin(DEG2RAD(MAKEANGLE(player.angle.b)))*300.f;
				target.y = player.pos.y + 170.f; 
				target.z=player.pos.z + EEcos(DEG2RAD(MAKEANGLE(player.angle.b)))*300.f;
				beta=MAKEANGLE(player.angle.b);
				
			}
			else
			{
				if (inter.iobj[spells[i].caster]->ioflags & IO_NPC)
				{
					target.x=inter.iobj[spells[i].caster]->pos.x - EEsin(DEG2RAD(MAKEANGLE(inter.iobj[spells[i].caster]->angle.b)))*300.f;
					target.y = inter.iobj[spells[i].caster]->pos.y;
					target.z=inter.iobj[spells[i].caster]->pos.z + EEcos(DEG2RAD(MAKEANGLE(inter.iobj[spells[i].caster]->angle.b)))*300.f;					
				}
				else
				{
					target.x=inter.iobj[spells[i].caster]->pos.x;
					target.y=inter.iobj[spells[i].caster]->pos.y;
					target.z=inter.iobj[spells[i].caster]->pos.z;					
				}

				beta=MAKEANGLE(inter.iobj[spells[i].caster]->angle.b);
			}

			if (!ARX_INTERACTIVE_ConvertToValidPosForIO(NULL, &target))
			{
				spells[i].exist = false;
				ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
				return -1;
			}

			if ((spells[i].caster==0) && (cur_mega==10))
				spells[i].fdata=1.f;
			else
				spells[i].fdata=0.f;

			Vector_Copy(&spells[i].target_pos,&target);
			CSpellFx *pCSpellFx = new CSummonCreature(GDevice);

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				CSummonCreature *pSummon = (CSummonCreature*) pCSpellFx;
					
				pSummon->Create(target, MAKEANGLE(player.angle.b));
					pSummon->SetDuration(2000, 500, 1500);
					pSummon->SetColorBorder(1, 0, 0);
					pSummon->SetColorRays1(0.93f, 0.93f, 0.63f);
					pSummon->SetColorRays2(0,0,0);
					pSummon->SetColorRays1(1, 0, 0);
					pSummon->SetColorRays2(0.5f, 0.5f, 0);
					
					pCSpellFx->lLightId = GetFreeDynLight();

					if (pCSpellFx->lLightId > -1)
					{
						long id = pCSpellFx->lLightId;
						DynLight[id].exist = 1;
						DynLight[id].intensity = 0.3f;
						DynLight[id].fallend = 500.f;
						DynLight[id].fallstart = 400.f;
						DynLight[id].rgb.r = 1.0f;
						DynLight[id].rgb.g = 0.0f;
						DynLight[id].rgb.b = 0.0f;
						DynLight[id].pos.x = pSummon->eSrc.x;
						DynLight[id].pos.y = pSummon->eSrc.y;
						DynLight[id].pos.z = pSummon->eSrc.z;
					}

				spells[i].pSpellFx = pCSpellFx;
			}

			SPELLCAST_Notify(i);
		}	
		break;
		case SPELL_FAKE_SUMMON:
		{
			if (spells[i].caster_level>=9)
			{
				if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;
			}
			else if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (	(spells[i].caster<=0)
				||	(!ValidIONum(spells[i].target))		)
				return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			ARX_SOUND_PlaySFX(SND_SPELL_SUMMON_CREATURE);
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 1.9f;

			if (duration>-1) 
				spells[i].tolive = 4000; 
			else
				spells[i].tolive = 4000;
			
			EERIE_3D target;
			
				target.x=inter.iobj[spells[i].target]->pos.x;
				target.y=inter.iobj[spells[i].target]->pos.y;
				target.z=inter.iobj[spells[i].target]->pos.z;					

				if (spells[i].target!=0)
				target.y-=170.f;
			
			
			Vector_Copy(&spells[i].target_pos,&target);
			CSpellFx *pCSpellFx = new CSummonCreature(GDevice);

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				CSummonCreature *pSummon = (CSummonCreature*) pCSpellFx;
					
				pSummon->Create(target, MAKEANGLE(player.angle.b));
				pSummon->SetDuration(2000, 500, 1500);
				pSummon->SetColorBorder(1, 0, 0);
				pSummon->SetColorRays1(0.93f, 0.93f, 0.63f);
				pSummon->SetColorRays2(0,0,0);
				pSummon->SetColorRays1(1, 0, 0);
				pSummon->SetColorRays2(0.5f, 0.5f, 0);
					
				pCSpellFx->lLightId = GetFreeDynLight();

				if (pCSpellFx->lLightId > -1)
				{
					long id = pCSpellFx->lLightId;
					DynLight[id].exist = 1;
					DynLight[id].intensity = 0.3f;
					DynLight[id].fallend = 500.f;
					DynLight[id].fallstart = 400.f;
					DynLight[id].rgb.r = 1.0f;
					DynLight[id].rgb.g = 0.0f;
					DynLight[id].rgb.b = 0.0f;
					DynLight[id].pos.x = pSummon->eSrc.x;
					DynLight[id].pos.y = pSummon->eSrc.y;
					DynLight[id].pos.z = pSummon->eSrc.z;
					
				}

				spells[i].pSpellFx = pCSpellFx;
			}

			SPELLCAST_Notify(i);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_NEGATE_MAGIC:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			ARX_SOUND_PlaySFX(SND_SPELL_NEGATE_MAGIC);
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 2.f;

			if (duration>-1) 
				spells[i].tolive=duration;	
			else
				spells[i].tolive = 1000000;

			CSpellFx *pCSpellFx = new CNegateMagic(GDevice);
			
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;
				target.x=player.pos.x;
				target.y=player.pos.y;
				target.z=player.pos.z;
				pCSpellFx->Create(target, MAKEANGLE(player.angle.b));
				pCSpellFx->SetDuration(spells[i].tolive);
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
			}

			if (spells[i].caster==0)
				spells[i].target=0;

			if (ValidIONum(spells[i].target))
				LaunchAntiMagicField(&inter.iobj[spells[i].target]->pos,i);

			SPELLCAST_Notify(i);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_INCINERATE:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			INTERACTIVE_OBJ * tio=inter.iobj[spells[i].target];

			if	((tio->ioflags & IO_NPC) && (tio->_npcdata->life<=0.f))
				return -1;
			
			ARX_SOUND_PlaySFX(SND_SPELL_INCINERATE);
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive = 20000;
			
			tio->sfx_flag|=SFX_TYPE_YLSIDE_DEATH;
			tio->sfx_flag|=SFX_TYPE_INCINERATE;
			tio->sfx_time = ARXTimeUL();
			ARX_SPELLS_AddSpellOn(spells[i].target,i);
			SPELLCAST_Notify(i);
			spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_FIREPLACE, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_MASS_PARALYSE:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			ARX_SOUND_PlaySFX(SND_SPELL_MASS_PARALYSE);
			spells[i].exist = TRUE;

			if (duration>-1) 
				spells[i].tolive=duration;
			else
				spells[i].tolive = 10000;

			spells[i].longinfo2=0;

			for (long ii=0;ii<inter.nbmax;ii++)
			{
				INTERACTIVE_OBJ * tio=inter.iobj[ii];

				if (	(ii==spells[i].caster)
					||	(!tio)
					||	(!(tio->ioflags & IO_NPC))
					||	(tio->show!=SHOW_FLAG_IN_SCENE)
					||	(tio->ioflags & IO_FREEZESCRIPT)
					||	(EEDistance3D(&tio->pos,&inter.iobj[spells[i].caster]->pos)>500.f)	
					)
					continue;

				tio->ioflags |= IO_FREEZESCRIPT;
				ARX_NPC_Kill_Spell_Launch(tio);
				ARX_SPELLS_AddSpellOn(ii,i);
				spells[i].longinfo2++;
				spells[i].misc=realloc(spells[i].misc,sizeof(long)*spells[i].longinfo2);
				long * ptr=(long *)spells[i].misc;
				ptr[spells[i].longinfo2-1]=ii;
			}			

					SPELLCAST_Notify(i);
		}	
		break;
		//********************************************************************************************
		// LEVEL 10 SPELLS -----------------------------------------------------------------------------
		case SPELL_MASS_LIGHTNING_STRIKE:
		{	
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			for (long ii=0;ii<MAX_SPELLS;ii++) 
			{
				if ((spells[ii].exist) && (spells[ii].type==typ))
				{
					if (spells[ii].longinfo!=-1) 
						DynLight[spells[ii].longinfo].exist=0;

					spells[ii].longinfo=-1;
					spells[ii].tolive=0;
				}
			}
					
			spells[i].exist=TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive=5000;			
			spells[i].siz=0;
			spells[i].longinfo=GetFreeDynLight();

			if (spells[i].longinfo!=-1)
			{
				long id=spells[i].longinfo;
				DynLight[id].exist=1;
				DynLight[id].intensity=1.8f;
				DynLight[id].fallend=450.f;
				DynLight[id].fallstart=380.f;
				DynLight[id].rgb.r=1.f;
				DynLight[id].rgb.g=0.75f;
				DynLight[id].rgb.b=0.75f;
				DynLight[id].pos.x=spells[i].vsource.x;
				DynLight[id].pos.y=spells[i].vsource.y;
				DynLight[id].pos.z=spells[i].vsource.z;
			}

			CSpellFx *pCSpellFx = NULL;
					

			ARX_CHECK_LONG(spells[i].caster_level);   
			pCSpellFx = new CMassLightning( GDevice, __max( ARX_CLEAN_WARN_CAST_LONG( spells[i].caster_level ), 1 ) );

		
			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;
				float beta;

				if (spells[i].caster==0)
				{
					target.x=player.pos.x  - EEsin(DEG2RAD(MAKEANGLE(player.angle.b)))*500.f;
					target.y=player.pos.y  + 150.f;
					target.z=player.pos.z  + EEcos(DEG2RAD(MAKEANGLE(player.angle.b)))*500.f;
					beta=player.angle.b;
				}
				else
				{
					INTERACTIVE_OBJ * io=inter.iobj[spells[i].caster];
					target.x=io->pos.x  - EEsin(DEG2RAD(MAKEANGLE(io->angle.b)))*500.f;
					target.y=io->pos.y  - 20.f;
					target.z=io->pos.z  + EEcos(DEG2RAD(MAKEANGLE(io->angle.b)))*500.f;
					beta=io->angle.b;
				}

				pCSpellFx->SetDuration((long)(500*spells[i].caster_level));
				pCSpellFx->Create(target, MAKEANGLE(player.angle.b));
			
				spells[i].pSpellFx = pCSpellFx;
				spells[i].tolive = pCSpellFx->GetDuration();
				spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_LOOP, &target, 1.0F, ARX_SOUND_PLAY_LOOPED);
			}
					
			ARX_SOUND_PlaySFX(SND_SPELL_LIGHTNING_START);
			SPELLCAST_Notify(i);
			// Draws White Flash on Screen
			GDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ONE);
			GDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);												
			SETALPHABLEND(GDevice,TRUE);
			float val = 1.f; 

			EERIEDrawBitmap(GDevice,0.f,0.f,(float)DANAESIZX,(float)DANAESIZY,0.00009f,NULL,D3DRGB(0.5f+val*DIV2,val,val));
			SETALPHABLEND(GDevice,FALSE);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_CONTROL_TARGET:
		{
			if (ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster)) return -1;

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!ValidIONum(spells[i].target))
				return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			long tcount=0;
			INTERACTIVE_OBJ * tmp_io=inter.iobj[spells[i].target];

			for (long ii=1;ii<inter.nbmax;ii++)
			{
				INTERACTIVE_OBJ * ioo=inter.iobj[ii];

				if (	(ioo)
					&&	(ioo->ioflags & IO_NPC)
					&&	(ioo->_npcdata->life>0.f)
					&&	(ioo->show==SHOW_FLAG_IN_SCENE)
					&&	(IsIOGroup(ioo,"DEMON"))	
					&&	(EEDistance3D(&ioo->pos,&spells[i].caster_pos)<900.f)
					)
				{
					tcount++;
					char param[256];
					long n;
					F2L(spells[i].caster_level,&n);
					sprintf(param,"%s_%04d %d"
						,GetName(tmp_io->filename)
						,tmp_io->ident
						,n
						);
					
					SendIOScriptEvent(ioo,-1,param,"NPC_CONTROL");
				}
			}

			if (tcount==0) return -1;
			
			ARX_SOUND_PlaySFX(SND_SPELL_CONTROL_TARGET);
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive = 1000;
			
			CSpellFx *pCSpellFx = NULL;
			pCSpellFx = new CControlTarget(GDevice);

			if (pCSpellFx != NULL)
			{
				pCSpellFx->spellinstance=i;
				EERIE_3D target;
				target.x=player.pos.x;
				target.y=player.pos.y;
				target.z=player.pos.z;
				pCSpellFx->Create(target, MAKEANGLE(player.angle.b));
				pCSpellFx->SetDuration(spells[i].tolive);
				spells[i].pSpellFx = pCSpellFx;
			}

			SPELLCAST_Notify(i);
		}	
		break;

		//----------------------------------------------------------------------------	
		case SPELL_FREEZE_TIME:
		{	
			if (ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster)) return -1;

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			ARX_SOUND_PlaySFX(SND_SPELL_FREEZETIME);
			//ARX_TIME_Pause();
			spells[i].siz=spells[i].caster_level*0.08f;
			GLOBAL_SLOWDOWN -= spells[i].siz;
			
			spells[i].exist=TRUE;						
			spells[i].tolive=200000;		

			if (duration>-1) spells[i].tolive=duration;

			spells[i].bDuration = true;
			spells[i].fManaCostPerSecond = 30.f*spells[i].siz;
			spells[i].longinfo=(long)ARX_TIME_Get();
			SPELLCAST_Notify(i);
		}	
		break;
		//----------------------------------------------------------------------------
		case SPELL_MASS_INCINERATE:
		{
			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();
			
			ARX_SOUND_PlaySFX(SND_SPELL_MASS_INCINERATE);
			spells[i].exist = TRUE;
			spells[i].lastupdate = spells[i].timcreation = ARXTimeUL();
			spells[i].tolive = 20000;
			long nb_targets=0;

			for (long ii=0;ii<inter.nbmax;ii++)
			{
				INTERACTIVE_OBJ * tio=inter.iobj[ii];

				if (	(ii==spells[i].caster)
					||	(!tio)
					||	(!(tio->ioflags & IO_NPC))
					||	((tio->ioflags & IO_NPC) && (tio->_npcdata->life<=0.f))
					||	(tio->show!=SHOW_FLAG_IN_SCENE)
					||	(EEDistance3D(&tio->pos,&inter.iobj[spells[i].caster]->pos)>500.f)	)
					continue;

				tio->sfx_flag|=SFX_TYPE_YLSIDE_DEATH;
				tio->sfx_flag|=SFX_TYPE_INCINERATE;
				tio->sfx_time = ARXTimeUL();
				nb_targets++;
				ARX_SPELLS_AddSpellOn(ii,i);
			}			

			SPELLCAST_Notify(i);

			if (nb_targets)
				spells[i].snd_loop = ARX_SOUND_PlaySFX(SND_FIREPLACE, &spells[i].caster_pos, 1.0F, ARX_SOUND_PLAY_LOOPED);
			else
				spells[i].snd_loop = -1;
		}	
		break;			
		//----------------------------------------------------------------------------
		case SPELL_TELEPORT:
		{
			if (ARX_SPELLS_ExistAnyInstanceForThisCaster(typ,spells[i].caster)) return -1;

			if (!CanPayMana(i,ARX_SPELLS_GetManaCost(typ,i))) return -1;

			if (!GLOBAL_MAGIC_MODE)
				return No_MagicAllowed();

			spells[i].exist=TRUE;
			spells[i].tolive=7000;
			ARX_SOUND_PlaySFX(SND_SPELL_TELEPORT, &spells[i].caster_pos);

			if (spells[i].caster==0) LASTTELEPORT = 0.0F;

			SPELLCAST_Notify(i);
		}
		break;
	}

	return i;
}

//*************************************************************************************
// Used for specific Spell-End FX
//*************************************************************************************
void ARX_SPELLS_Kill(const long &i)
{
	static TextureContainer * tc4=MakeTCFromFile("Graph\\Particles\\smoke.bmp");

	if (!spells[i].exist) return;

	spells[i].exist=FALSE;

	// All Levels - Kill Light
	if (spells[i].pSpellFx && spells[i].pSpellFx->lLightId != -1)
	{
		DynLight[spells[i].pSpellFx->lLightId].duration = 500; 
		DynLight[spells[i].pSpellFx->lLightId].time_creation = ARXTimeUL();
	}

	switch(spells[i].type)
	{
		//----------------------------------------------------------------------------
		case SPELL_FIREBALL :

			if (spells[i].longinfo!=-1) 
			{
				DynLight[spells[i].longinfo].duration = 500; 
				DynLight[spells[i].longinfo].time_creation = ARXTimeUL();
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
				DynLight[spells[i].longinfo].time_creation = ARXTimeUL();
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
				DynLight[spells[i].longinfo].time_creation = ARXTimeUL();
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
				DynLight[spells[i].longinfo].time_creation = ARXTimeUL();
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

			if (spells[i].longinfo!=-1) damages[spells[i].longinfo].exist=FALSE;	

			if (spells[i].longinfo2!=-1) 
			{
				DynLight[spells[i].longinfo2].time_creation = ARXTimeUL();
				DynLight[spells[i].longinfo2].duration = 600; 
			}

			ARX_SOUND_Stop(spells[i].snd_loop);
			break;
		//----------------------------------------------------------------------------
		case SPELL_FLYING_EYE :
		{
			ARX_SOUND_PlaySFX(SND_SPELL_EYEBALL_OUT);
			eyeball.exist = -100;

			for (long n=0;n<12;n++)
			{
				long j = ARX_PARTICLES_GetFree();

				if ((j!=-1) && (!ARXPausedTimer))
				{
					ParticleCount++;
					PARTICLE_DEF * pd=&particle[j];
					pd->exist=TRUE;
					pd->zdec=0;					
 
					pd->ov.x=eyeball.pos.x+5.f-rnd()*10.f;
					pd->ov.y=eyeball.pos.y+5.f-rnd()*10.f;
					pd->ov.z=eyeball.pos.z+5.f-rnd()*10.f;
					pd->move.x=2.f-4.f*rnd();
					pd->move.y=2.f-4.f*rnd();
					pd->move.z=2.f-4.f*rnd();
					pd->siz=28.f;
					pd->tolive=2000+(unsigned long)(float)(rnd()*4000.f);
					Vector_Init(&pd->scale,12.f,12.f,12.f);
					pd->timcreation=spells[i].lastupdate;
					pd->tc=tc4;
					pd->special=FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
					pd->fparam=0.0000001f;
					pd->r=0.7f;
					pd->g=0.7f;
					pd->b=1.f;
				}
			}

			pMenuConfig->bMouseLookToggle = bOldLookToggle; 
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
				if (	(inter.iobj[spells[i].longinfo2]->scriptload)
					&&	(inter.iobj[spells[i].longinfo2]->ioflags & IO_NOSAVE)	)
				{
					AddRandomSmoke(inter.iobj[spells[i].longinfo2],100);
					EERIE_3D posi;
					Vector_Copy(&posi,&inter.iobj[spells[i].longinfo2]->pos);
					posi.y-=100.f;
					MakeCoolFx(&posi);
					long nn=GetFreeDynLight();

					if (nn>=0)
					{
						DynLight[nn].exist=1;
						DynLight[nn].intensity = 0.7f + 2.f*rnd();
						DynLight[nn].fallend = 600.f;
						DynLight[nn].fallstart = 400.f;
						DynLight[nn].rgb.r = 1.0f;
						DynLight[nn].rgb.g = 0.8f;
						DynLight[nn].rgb.b = .0f;
						DynLight[nn].pos.x = posi.x;
						DynLight[nn].pos.y = posi.y;
						DynLight[nn].pos.z = posi.z;
						DynLight[nn].duration=600;
					}

					ARX_INTERACTIVE_DestroyIO(inter.iobj[spells[i].longinfo2]);
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

EERIE_3D cabalangle;
EERIE_3D cabalpos;
EERIE_3D cabalscale;
EERIE_RGB cabalcolor;


float ARX_SPELLS_ApplyFireProtection(INTERACTIVE_OBJ * io,float damages)
{
	if (io)
	{
		long idx=ARX_SPELLS_GetSpellOn(io,SPELL_FIRE_PROTECTION);

		if (idx>=0)
		{
			float modif=1.f-((float)spells[idx].caster_level*DIV10);

			if (modif>1.f) modif=1.f;
			else if (modif<0.f) modif=0.f;

			damages*=modif;
		}

		if (io->ioflags & IO_NPC)
		{
			damages-=io->_npcdata->resist_fire*DIV100*damages;

			if (damages<0.f) damages=0.f;
		}
	}

	return damages;
}
float ARX_SPELLS_ApplyColdProtection(INTERACTIVE_OBJ * io,float damages)
{
	long idx=ARX_SPELLS_GetSpellOn(io,SPELL_COLD_PROTECTION);

	if (idx>=0)
	{
		float modif=1.f-((float)spells[idx].caster_level*DIV10);

		if (modif>1.f) modif=1.f;
		else if (modif<0.f) modif=0.f;

		damages*=modif;
	}

	return damages;
}

//*************************************************************************************
// Updates all currently working spells.
//*************************************************************************************
extern bool bSoftRender;
void ARX_SPELLS_Update(LPDIRECT3DDEVICE7 m_pd3dDevice)
{
	bool bNoVB	=	false;
	if( bSoftRender )
	{
		bNoVB	=	GET_FORCE_NO_VB();
		SET_FORCE_NO_VB( true );
	}
	register long i;
	register unsigned long tim;
	register long framediff,framediff2,framediff3;

	ucFlick++;

	tim = ARXTimeUL();

	for (i=0;i<MAX_SPELLS;i++)
	{
		if (!m_pd3dDevice) spells[i].exist=0;

		if (!GLOBAL_MAGIC_MODE) spells[i].tolive=0;

		if (spells[i].exist) 
		{
			if (spells[i].bDuration && !CanPayMana(i,spells[i].fManaCostPerSecond * (float)FrameDiff * DIV1000, false))
				ARX_SPELLS_Fizzle(i);

			framediff=spells[i].timcreation+spells[i].tolive-tim;
			framediff2=spells[i].timcreation+spells[i].tolive-spells[i].lastupdate;
			framediff3=tim-spells[i].lastupdate;

			if (framediff<0) 
			{
				SPELLEND_Notify(i);

				switch (spells[i].type)
				{
				//----------------------------------------------------------------------------
				case SPELL_TELEPORT:
					TELEPORT=0.f;					
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
					INTERACTIVE_OBJ * io=inter.iobj[spells[i].target];

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
					INTERACTIVE_OBJ * io=inter.iobj[spells[i].target];

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

					if ((spells[i].target>=0) && (spells[i].target<inter.nbmax))
					{
						if (inter.iobj[spells[i].target])
							inter.iobj[spells[i].target]->speed_modif-=spells[i].caster_level*DIV10;
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
						ARX_HALO_SetToNative(inter.iobj[spells[i].target]);

				break;
				case SPELL_COLD_PROTECTION:
					ARX_SOUND_Stop(spells[i].snd_loop);
					ARX_SOUND_PlaySFX(SND_SPELL_COLD_PROTECTION_END);
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);;

					if (ValidIONum(spells[i].target))
						ARX_HALO_SetToNative(inter.iobj[spells[i].target]);

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
					inter.iobj[spells[i].target]->ioflags&=~IO_FREEZESCRIPT;											
				break;
				//----------------------------------------------------------------------------------
				case SPELL_RISE_DEAD:
					ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].caster_pos);

					if (ValidIONum(spells[i].longinfo) && spells[i].longinfo!=0)
					{				
						if (	(inter.iobj[spells[i].longinfo]->scriptload)
							&&	(inter.iobj[spells[i].longinfo]->ioflags & IO_NOSAVE)	)
						{
							AddRandomSmoke(inter.iobj[spells[i].longinfo],100);
							EERIE_3D posi;
							Vector_Copy(&posi,&inter.iobj[spells[i].longinfo]->pos);
							posi.y-=100.f;
							MakeCoolFx(&posi);
							long nn=GetFreeDynLight();

							if (nn>=0)
							{
								DynLight[nn].exist=1;
								DynLight[nn].intensity = 0.7f + 2.f*rnd();
								DynLight[nn].fallend = 600.f;
								DynLight[nn].fallstart = 400.f;
								DynLight[nn].rgb.r = 1.0f;
								DynLight[nn].rgb.g = 0.8f;
								DynLight[nn].rgb.b = .0f;
								DynLight[nn].pos.x = posi.x;
								DynLight[nn].pos.y = posi.y;
								DynLight[nn].pos.z = posi.z;
								DynLight[nn].duration=600;
							}

							ARX_INTERACTIVE_DestroyIO(inter.iobj[spells[i].longinfo]);
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

					if (ValidIONum(spells[i].longinfo))
					{				
						ReleaseInter(inter.iobj[spells[i].longinfo]);
					}					

				break;
				//----------------------------------------------------------------------------
				case SPELL_SLOW_DOWN:
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);

					if ((spells[i].target>=0) && (spells[i].target<inter.nbmax))
					{
						if (inter.iobj[spells[i].target])
							inter.iobj[spells[i].target]->speed_modif+=spells[i].caster_level*DIV20;
					}

				break;				
				//----------------------------------------------------------------------------------
				//**********************************************************************************
				// LEVEL 7 -------------------------------------------------------------------------
				//----------------------------------------------------------------------------------				
				case SPELL_ICE_FIELD:

					if (spells[i].longinfo!=-1)
						damages[spells[i].longinfo].exist=FALSE;					

				break;
				case SPELL_FIRE_FIELD:

					if (spells[i].longinfo!=-1)
						damages[spells[i].longinfo].exist=FALSE;					

				break;
				//----------------------------------------------------------------------------
				case SPELL_LIGHTNING_STRIKE:					
					ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].caster_pos);					
				break;
				//----------------------------------------------------------------------------
				case SPELL_FLYING_EYE:					
					ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE, &inter.iobj[spells[i].caster]->pos);
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
					inter.iobj[spells[i].target]->GameFlags&=~GFLAG_INVISIBILITY;											
					ARX_SOUND_PlaySFX(SND_SPELL_INVISIBILITY_END, &inter.iobj[spells[i].target]->pos);					
					ARX_SPELLS_RemoveSpellOn(spells[i].target,i);
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
								inter.iobj[ptr[in]]->ioflags&=~IO_FREEZESCRIPT;											
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
			//**************************************************************************************
			// LEVEL 1 -----------------------------------------------------------------------------
			case SPELL_MAGIC_MISSILE:
				{
					CSpellFx *pCSpellFX = spells[i].pSpellFx;

					if (pCSpellFX)
					{
						CMultiMagicMissile *pMMM = (CMultiMagicMissile *) pCSpellFX;
							pMMM->CheckCollision(1.f);
						
						// Update
						pCSpellFX->Update(FrameDiff);

						if (pCSpellFX->Render(m_pd3dDevice)==-1)
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
					pCSpellFX->Render(m_pd3dDevice);
				}

				CHeal * ch=(CHeal *)pCSpellFX;

				if (ch)
				for (long ii=0;ii<inter.nbmax;ii++)
				{
					if ((inter.iobj[ii]) 						
						&& (inter.iobj[ii]->show==SHOW_FLAG_IN_SCENE) 
						&& (inter.iobj[ii]->GameFlags & GFLAG_ISINTREATZONE)
								        && (inter.iobj[ii]->ioflags & IO_NPC)
						&& (inter.iobj[ii]->_npcdata->life>0.f)
						)
					{
						float dist;

						if (ii==spells[i].caster) dist=0;
						else dist=EEDistance3D(&ch->eSrc,&inter.iobj[ii]->pos);

						if (dist<300.f)
						{
							float gain=((rnd()*1.6f+0.8f)*spells[i].caster_level)*(300.f-dist)*DIV300*_framedelay*DIV1000;

							if (ii==0) 
							{
								if (!BLOCK_PLAYER_CONTROLS)
									player.life=min(player.life+gain,player.Full_maxlife);									
							}
							else inter.iobj[ii]->_npcdata->life=min(inter.iobj[ii]->_npcdata->life+gain,inter.iobj[ii]->_npcdata->maxlife);
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
					pCSpellFX->Render(m_pd3dDevice);
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
					pCSpellFX->Render(m_pd3dDevice);
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
					else scaley=EEfabs(inter.iobj[spells[i].caster]->physics.cyl.height*DIV2)+30.f;

 
					float mov=EEsin((float)FrameTime*DIV800)*scaley;

					if (spells[i].caster==0)
					{
								cabalpos.x = player.pos.x; 
						cabalpos.y=player.pos.y+60.f-mov;
								cabalpos.z = player.pos.z; 
						refpos=player.pos.y+60.f;							
					}
					else
					{							
								cabalpos.x = inter.iobj[spells[i].caster]->pos.x; 
						cabalpos.y=inter.iobj[spells[i].caster]->pos.y-scaley-mov;
								cabalpos.z = inter.iobj[spells[i].caster]->pos.z; 
						refpos=inter.iobj[spells[i].caster]->pos.y-scaley;							
					}

					float Es=EEsin((float)FrameTime*DIV800 + DEG2RAD(scaley));

					if (spells[i].longinfo2!=-1)
					{
						DynLight[spells[i].longinfo2].pos.x = cabalpos.x;
						DynLight[spells[i].longinfo2].pos.y = refpos;
						DynLight[spells[i].longinfo2].pos.z = cabalpos.z; 
						DynLight[spells[i].longinfo2].rgb.r=rnd()*0.2f+0.8f;
						DynLight[spells[i].longinfo2].rgb.g=rnd()*0.2f+0.6f;
						DynLight[spells[i].longinfo2].fallstart=Es*1.5f;
					}

					GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,   D3DBLEND_ONE );
					GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE );
					SETALPHABLEND(GDevice,TRUE);
					SETZWRITE(GDevice, FALSE );
					cabalangle.b=spells[i].fdata+(float)FrameDiff*0.1f;
					spells[i].fdata=cabalangle.b;

							cabalangle.g = 0.f; 
							cabalcolor.r = 0.8f;
							cabalcolor.g = 0.4f;
							cabalcolor.b = 0.f;
					cabalscale.z=cabalscale.y=cabalscale.x=Es;				
					DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
					mov=EEsin((float)(FrameTime-30.f)*DIV800)*scaley;
					cabalpos.y=refpos-mov;						
							cabalcolor.b = 0.f;
							cabalcolor.g = 3.f;
							cabalcolor.r = 0.5f;
					DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
					mov=EEsin((float)(FrameTime-60.f)*DIV800)*scaley;
					cabalpos.y=refpos-mov;
							cabalcolor.b = 0.f;
							cabalcolor.g = 0.1f;
							cabalcolor.r = 0.25f;
					DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
					mov=EEsin((float)(FrameTime-120.f)*DIV800)*scaley;
					cabalpos.y=refpos-mov;
							cabalcolor.b = 0.f;
							cabalcolor.g = 0.1f;
							cabalcolor.r = 0.15f;
					DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
					SETALPHABLEND(GDevice,FALSE);		
					SETZWRITE(GDevice, TRUE );	
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
						Vector_Copy(&el->pos,&pCF->eCurPos);
						el->intensity = 2.2f;
						el->fallend = 500.f;
						el->fallstart = 400.f;
						el->rgb.r = 1.0f-rnd()*0.3f;
						el->rgb.g = 0.6f-rnd()*0.1f;;
						el->rgb.b = 0.3f-rnd()*0.1f;;
					}

					EERIE_SPHERE sphere;
					sphere.origin.x=pCF->eCurPos.x;
					sphere.origin.y=pCF->eCurPos.y;
					sphere.origin.z=pCF->eCurPos.z;
					sphere.radius=__max(spells[i].caster_level*2.f,12.f);
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
								EERIE_3D move;
								Vector_Init(&move);
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
						pCF->eMove.x *= 0.5f;
						pCF->eMove.y *= 0.5f;
						pCF->eMove.z *= 0.5f;
						pCF->pPSSmoke.iParticleNbMax = 0;
						pCF->SetTTL(1500);
						pCF->bExplo = true;
						
						DoSphericDamage(&pCF->eCurPos,3.f*spells[i].caster_level,30.f*spells[i].caster_level,DAMAGE_AREA,DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL,spells[i].caster);
						spells[i].tolive=0;
						ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &sphere.origin);
						ARX_NPC_SpawnAudibleSound(&sphere.origin, inter.iobj[spells[i].caster]);
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
				spells[i].pSpellFx->Render(m_pd3dDevice);

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
				spells[i].pSpellFx->Render(m_pd3dDevice);
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
							Vector_Copy(&pBless->eSrc,&inter.iobj[spells[i].target]->pos);
							EERIE_3D angle;
							Vector_Init(&angle);

							if (spells[i].target==0)
								angle.b=player.angle.b;	
							else 
								angle.b=inter.iobj[spells[i].target]->angle.b;

							pBless->Set_Angle(angle);
						}
					}

					spells[i].pSpellFx->Update(FrameDiff);
					spells[i].pSpellFx->Render(m_pd3dDevice);
				}
			}
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_CURSE:

			if (spells[i].pSpellFx)
			{
				CCurse * curse=(CCurse *)spells[i].pSpellFx;
				EERIE_3D target;
				Vector_Init(&target);
					
				if ((spells[i].target>=0) && (inter.iobj[spells[i].target]))
				{
					target.x=inter.iobj[spells[i].target]->pos.x;
					target.y=inter.iobj[spells[i].target]->pos.y;
					target.z=inter.iobj[spells[i].target]->pos.z;

					if (spells[i].target==0) target.y-=200.f;
					else target.y+=inter.iobj[spells[i].target]->physics.cyl.height-30.f;
				}
					
				

				ARX_CHECK_ULONG(FrameDiff);
				curse->Update(ARX_CLEAN_WARN_CAST_ULONG(FrameDiff));

				
				curse->Render(m_pd3dDevice, &target);
				SETCULL(GDevice,D3DCULL_NONE);
			}

			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_FIRE_PROTECTION:
				spells[i].pSpellFx->Update(FrameDiff);
				spells[i].pSpellFx->Render(m_pd3dDevice);
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_COLD_PROTECTION:
				spells[i].pSpellFx->Update(FrameDiff);
				spells[i].pSpellFx->Render(m_pd3dDevice);
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
					spells[i].pSpellFx->Render(m_pd3dDevice);					
				}
			}
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_RUNE_OF_GUARDING:
			{
				if (spells[i].pSpellFx)
				{
					spells[i].pSpellFx->Update(FrameDiff);
					spells[i].pSpellFx->Render(m_pd3dDevice);
					CRuneOfGuarding * pCRG=(CRuneOfGuarding *)spells[i].pSpellFx;

					if (pCRG)
					{
						EERIE_SPHERE sphere;
						sphere.origin.x=pCRG->eSrc.x;
						sphere.origin.y=pCRG->eSrc.y;
						sphere.origin.z=pCRG->eSrc.z;
						sphere.radius=__max(spells[i].caster_level*15.f,50.f);

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
					spells[i].pSpellFx->Render(m_pd3dDevice);					

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
				spells[i].pSpellFx->Render(m_pd3dDevice);
						}

			break;
			//-----------------------------------------------------------------------------------------	
			case SPELL_LEVITATE:
			{
				CLevitate *pLevitate=(CLevitate *)spells[i].pSpellFx;
				EERIE_3D target;

				if (spells[i].target==0)
				{
					target.x=player.pos.x;
					target.y=player.pos.y+150.f;
					target.z=player.pos.z;
					player.levitate=1;
				}
				else
				{
					target.x=inter.iobj[spells[i].caster]->pos.x;
							target.y = inter.iobj[spells[i].caster]->pos.y; 
					target.z=inter.iobj[spells[i].caster]->pos.z;
				}

				pLevitate->ChangePos(&target);
					
				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render(m_pd3dDevice);
					SETCULL(GDevice,D3DCULL_NONE);
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
							pCSpellFX->Render(m_pd3dDevice);

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
								DynLight[id].time_creation = ARXTimeUL();
					}

					unsigned long tim=pCSpellFX->GetCurrentTime();

					if ((tim>3000) && (spells[i].longinfo==-1))
					{
						ARX_SOUND_PlaySFX(SND_SPELL_ELECTRIC, &spells[i].caster_pos);
						char tmptext[256];						
						CRiseDead *prise;
						prise= (CRiseDead *)spells[i].pSpellFx;

						if (prise)
						{	
							EERIE_CYLINDER phys;
							phys.height=-200;
							phys.radius=50;
							phys.origin.x=spells[i].target_pos.x;
							phys.origin.y=spells[i].target_pos.y;
							phys.origin.z=spells[i].target_pos.z;
	
									float anything = CheckAnythingInCylinder(&phys, NULL, CFLAG_JUST_TEST);

							if (EEfabs(anything)<30)
							{
								sprintf(tmptext,"%sGraph\\Obj3D\\Interactive\\NPC\\Undead_base\\Undead_base.asl",Project.workingdir);
								INTERACTIVE_OBJ * io;
								io=AddNPC(m_pd3dDevice,tmptext,IO_IMMEDIATELOAD);

								if (io)
								{
									ARX_INTERACTIVE_HideGore(io);								
									RestoreInitialIOStatusOfIO(io);
											

									long lSpellsCaster = spells[i].caster ; 
									ARX_CHECK_SHORT(lSpellsCaster);
									io->summoner=ARX_CLEAN_WARN_CAST_SHORT(lSpellsCaster);

										
									io->ioflags|=IO_NOSAVE;
									spells[i].longinfo=GetInterNum(io);
									io->scriptload=1;
											
									ARX_INTERACTIVE_Teleport(io,&phys.origin,0);
									MakeTemporaryIOIdent(io);						
									SendInitScriptEvent(io);

									if ((spells[i].caster>=0) && (spells[i].caster<inter.nbmax))
										EVENT_SENDER=inter.iobj[spells[i].caster];
									else EVENT_SENDER=NULL;

									SendIOScriptEvent(io,SM_SUMMONED,"",NULL);
										
									EERIE_3D pos;
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
							
						}
						else if ((!ARXPausedTimer) && (tim<4000))
						{
						  if (rnd()>0.95f) 
							{
								CRiseDead *pRD = (CRiseDead*)pCSpellFX;
								EERIE_3D pos;
								pos.x = pRD->eSrc.x;
								pos.y = pRD->eSrc.y;
								pos.z = pRD->eSrc.z;
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
					pCSpellFX->Render(m_pd3dDevice);
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
						INTERACTIVE_OBJ * io=inter.iobj[spells[i].longinfo];
						CCreateField * ccf=(CCreateField *)pCSpellFX;
						io->pos.x = ccf->eSrc.x;
						io->pos.y = ccf->eSrc.y;
						io->pos.z = ccf->eSrc.z;

						if (IsAnyNPCInPlatform(io))
						{
							spells[i].tolive=0;
						}
					
						pCSpellFX->Update(FrameDiff);

						if (VisibleSphere(ccf->eSrc.x,ccf->eSrc.y-120.f,ccf->eSrc.z,400.f))					
							pCSpellFX->Render(m_pd3dDevice);
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
					pCSpellFX->Render(m_pd3dDevice);
				}
			}
			break;
			case SPELL_FIRE_FIELD:
			{
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
						el->rgb.r = 1.f-rnd()*DIV10;
						el->rgb.g = 0.8f;
						el->rgb.b = 0.6f;
						el->duration = 600;
						el->extras=0;						
					}

 

					if (VisibleSphere(pf->pos.x,pf->pos.y-120.f,pf->pos.z,350.f))					
					{
						pCSpellFX->Render(m_pd3dDevice);


						float fDiff = FrameDiff/8.f ;
						ARX_CHECK_INT(fDiff);

						int nTime	= ARX_CLEAN_WARN_CAST_INT(fDiff);

						
						for (long nn=0;nn<=nTime+1;nn++)
						{
							long j=ARX_PARTICLES_GetFree();

							if ((j!=-1) && (!ARXPausedTimer) )
							{
								ParticleCount++;
								PARTICLE_DEF * pd=&particle[j];
								pd->exist=TRUE;
								pd->zdec=0;
								float sy=rnd()*3.14159f*2.f-3.14159f;
								float sx=EEsin(sy);
								float sz=EEcos(sy);
								sy=EEsin(sy); 
								pd->ov.x=pf->pos.x+120.f*sx*rnd();
								pd->ov.y=pf->pos.y+15.f*sy*rnd();
								pd->ov.z=pf->pos.z+120.f*sz*rnd();
								
								pd->move.x=(2.f-4.f*rnd());
								pd->move.y=(1.f-8.f*rnd());
								pd->move.z=(2.f-4.f*rnd());
								
								pd->siz			=	7.f;
								pd->tolive		=	500+(unsigned long)(rnd()*1000.f);
								pd->special		=	0;
								pd->tc			=	fire2;						
										pd->special		|=	ROTATING | MODULATE_ROTATION | FIRE_TO_SMOKE;
								pd->fparam		=	0.1f-rnd()*0.2f;
								pd->scale.x		=	-8.f;
								pd->scale.y		=	-8.f;
								pd->scale.z		=	-8.f;
										pd->timcreation	=	lARXTime;
								pd->r			=	pd->g	=	pd->b	=	1.f;
								long j2			=	ARX_PARTICLES_GetFree();

								if (j2!=-1)
								{
									ParticleCount++;
									PARTICLE_DEF * pd2=&particle[j2];
									memcpy(pd2,pd,sizeof(PARTICLE_DEF));
									pd2->delay=(long)(float)(rnd()*150.f+60.f);
								}
							}
						}
					}
				}
			}
			break;
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
						el->rgb.b = 1.0f-rnd()*DIV10;
						el->duration = 600;
						el->extras=0;						
					}

 

					if (VisibleSphere(pf->eSrc.x,pf->eSrc.y-120.f,pf->eSrc.z,350.f))
					{
						pCSpellFX->Render(m_pd3dDevice);
					}
				}

				SETCULL(GDevice,D3DCULL_NONE);
			}
			break;
			//-----------------------------------------------------------------------------------------
			case SPELL_LIGHTNING_STRIKE:
				{
					CSpellFx *pCSpellFX = spells[i].pSpellFx;

					if (pCSpellFX)
					{
						pCSpellFX->Update(FrameDiff);
						pCSpellFX->Render(m_pd3dDevice);
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
					pCSpellFX->Render(m_pd3dDevice);
				}
			}
			case SPELL_EXPLOSION:
			{
				if (spells[i].longinfo2 == -1)
					spells[i].longinfo2=GetFreeDynLight();

				if (spells[i].longinfo2 != -1)
				{
					long id = spells[i].longinfo2;
					DynLight[id].rgb.r = 0.1f+rnd()*DIV3;;
					DynLight[id].rgb.g = 0.1f+rnd()*DIV3;;
					DynLight[id].rgb.b = 0.8f+rnd()*DIV5;;
					DynLight[id].duration=200;
				
					long lvl;
					float rr,r2;
					EERIE_3D pos;

					if (rnd()>0.8f)
					{					
						F2L((rnd()*9.f+4.f),&lvl);
						rr=DEG2RAD(rnd()*360.f);
						r2=DEG2RAD(rnd()*360.f);
						pos.x=DynLight[id].pos.x-EEsin(rr)*260;
						pos.y=DynLight[id].pos.y-EEsin(r2)*260;
						pos.z=DynLight[id].pos.z+EEcos(rr)*260;						
						EERIE_RGB rgb; 
						rgb.r=0.1f+rnd()*DIV3;
						rgb.g=0.1f+rnd()*DIV3;
						rgb.b=0.8f+rnd()*DIV5;
								LaunchFireballBoom(&pos, ARX_CLEAN_WARN_CAST_FLOAT(lvl), NULL, &rgb);
					}
					else if (rnd()>0.76f)
					{					
						F2L((rnd()*9.f+4.f),&lvl);
						rr=DEG2RAD(rnd()*360.f);
						r2=DEG2RAD(rnd()*360.f);
						pos.x=DynLight[id].pos.x-EEsin(rr)*260;
						pos.y=DynLight[id].pos.y-EEsin(r2)*260;
						pos.z=DynLight[id].pos.z+EEcos(rr)*260;						
						MakeCoolFx(&pos);		
					}
					else if (rnd()>0.66f)
					{					
						F2L((rnd()*9.f+4.f),&lvl);
						rr=DEG2RAD(rnd()*360.f);
						r2=DEG2RAD(rnd()*360.f);
						pos.x=DynLight[id].pos.x-EEsin(rr)*160;
						pos.y=DynLight[id].pos.y-EEsin(r2)*160;
						pos.z=DynLight[id].pos.z+EEcos(rr)*160;						
						ARX_PARTICLES_Add_Smoke(&pos,2,20); // flag 1 = randomize pos
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
 

				if (!ARXPausedTimer)
				{

					if ((float)ARXTime-(float)spells[i].timcreation<=4000)
					{
						if (rnd()>0.7f) 
						{
							EERIE_3D pos;
							CSummonCreature *pSummon;
							pSummon= (CSummonCreature *)spells[i].pSpellFx;

							if (pSummon)
							{
								pos.x=pSummon->eSrc.x;
								pos.y=pSummon->eSrc.y;
								pos.z=pSummon->eSrc.z;
								MakeCoolFx(&pos);
							}
						}

						CSpellFx *pCSpellFX = spells[i].pSpellFx;

						if (pCSpellFX)
						{
							pCSpellFX->Update(FrameDiff);
							pCSpellFX->Render(m_pd3dDevice);
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
						char tmptext[256];						
						CSummonCreature *pSummon;
						pSummon= (CSummonCreature *)spells[i].pSpellFx;

						if (pSummon)
						{			
							EERIE_CYLINDER phys;
							phys.height=-200;
							phys.radius=50;
							phys.origin.x=spells[i].target_pos.x;
							phys.origin.y=spells[i].target_pos.y;
							phys.origin.z=spells[i].target_pos.z;
									float anything = CheckAnythingInCylinder(&phys, NULL, CFLAG_JUST_TEST);

							if (EEfabs(anything)<30)
							{
 
							long tokeep=0;
							

							if (spells[i].caster_level>=9)
							{
								tokeep=1;
								sprintf(tmptext,"%sGraph\\Obj3D\\Interactive\\NPC\\Demon\\Demon.asl",Project.workingdir);
							}
							else 
							{
								tokeep=0;

								if (rnd()>0.98f)
								{
									sprintf(tmptext,"%sGraph\\Obj3D\\Interactive\\NPC\\WRat_base\\WRat_base.asl",Project.workingdir);
									tokeep=-1;
								}
								else
									sprintf(tmptext,"%sGraph\\Obj3D\\Interactive\\NPC\\Chicken_Base\\Chicken_Base.asl",Project.workingdir);
							}

							if ((rnd()>0.997f) || ((cur_rf>=3) && (rnd()>0.8f)) || ((cur_mr>=3) && (rnd()>0.3f)))
							{
								sprintf(tmptext,"%sGraph\\Obj3D\\Interactive\\NPC\\WRat_base\\WRat_base.asl",Project.workingdir);
								tokeep=-1;
							}

							if ((rnd()>0.997f) || (sp_max && (rnd()>0.8f)) || ((cur_mr>=3) && (rnd()>0.3f)))
							{
								sprintf(tmptext,"%sGraph\\Obj3D\\Interactive\\NPC\\Y_mx\\Y_mx.asl",Project.workingdir);
								tokeep=0;
							}

							if (spells[i].fdata==1.f)
							{
								if (rnd()>0.5) 
								{
									sprintf(tmptext,"%sGraph\\Obj3D\\Interactive\\NPC\\WRat_base\\WRat_base.asl",Project.workingdir);
									tokeep=-1;
								}
								else
								{
									sprintf(tmptext,"%sGraph\\Obj3D\\Interactive\\NPC\\Y_mx\\Y_mx.asl",Project.workingdir);
									tokeep=0;
								}
							}

							INTERACTIVE_OBJ * io;
							io=AddNPC(m_pd3dDevice,tmptext,IO_IMMEDIATELOAD);

							if (!io)
							{
								sprintf(tmptext,"%sGraph\\Obj3D\\Interactive\\NPC\\Chicken_Base\\Chicken_Base.asl",Project.workingdir);
								tokeep=0;
								io=AddNPC(m_pd3dDevice,tmptext,IO_IMMEDIATELOAD);
							}

							if (io)
							{
								RestoreInitialIOStatusOfIO(io);
								

								long lSpellsCaster = spells[i].caster ; 
								ARX_CHECK_SHORT(lSpellsCaster);
								io->summoner=ARX_CLEAN_WARN_CAST_SHORT(lSpellsCaster);


								io->scriptload=1;								

								if (tokeep==1)
									io->ioflags|=IO_NOSAVE;

											io->pos.x = phys.origin.x; 
											io->pos.y = phys.origin.y;
											io->pos.z = phys.origin.z;
								MakeTemporaryIOIdent(io);						
								SendInitScriptEvent(io);

								if (tokeep<0)
								{
									io->scale=1.65f;
									io->physics.cyl.radius=25;
									io->physics.cyl.height=-43;
									io->speed_modif=1.f;
								}

								if ((spells[i].caster>=0) && (spells[i].caster<inter.nbmax))
									EVENT_SENDER=inter.iobj[spells[i].caster];
								else EVENT_SENDER=NULL;

								SendIOScriptEvent(io,SM_SUMMONED,"",NULL);
								
											EERIE_3D pos;
								
								for (long j=0;j<3;j++)
								{
									pos.x=pSummon->eSrc.x+rnd()*100.f-50.f;
									pos.y=pSummon->eSrc.y+100+rnd()*100.f-50.f;
									pos.z=pSummon->eSrc.z+rnd()*100.f-50.f;
									MakeCoolFx(&pos);
								}

								if (tokeep==1)	spells[i].longinfo2=GetInterNum(io);
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
 

					if (!ARXPausedTimer)
						if (rnd()>0.7f) 
						{
							EERIE_3D pos;
							CSummonCreature *pSummon;
							pSummon= (CSummonCreature *)spells[i].pSpellFx;

							if (pSummon)
							{
								pos.x=pSummon->eSrc.x;
								pos.y=pSummon->eSrc.y;
								pos.z=pSummon->eSrc.z;
								MakeCoolFx(&pos);
							}
						}

				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render(m_pd3dDevice);
				}					
			}
			break;
			//-----------------------------------------------------------------------------------------	
			
			case SPELL_INCINERATE:
			{
				if (ValidIONum(spells[i].caster))
				{
					ARX_SOUND_RefreshPosition(spells[i].snd_loop, &inter.iobj[spells[i].caster]->pos);
				}
			}
			break;
			case SPELL_NEGATE_MAGIC:
			{
				if (ValidIONum(spells[i].target))
					LaunchAntiMagicField(&inter.iobj[spells[i].target]->pos,i);

				CSpellFx *pCSpellFX = spells[i].pSpellFx;

				if (pCSpellFX)
				{
					pCSpellFX->Update(FrameDiff);
					pCSpellFX->Render(m_pd3dDevice);
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
					pCSpellFX->Render(m_pd3dDevice);
				}
			}
			break;
			case SPELL_MASS_INCINERATE:
			{
				if (ValidIONum(spells[i].caster))
				{
					ARX_SOUND_RefreshPosition(spells[i].snd_loop, &inter.iobj[spells[i].caster]->pos);
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
					pCSpellFX->Render(m_pd3dDevice);
				}
				
						EERIE_3D _source;
				_source.x=spells[i].vsource.x;
				_source.y=spells[i].vsource.y;
				_source.z=spells[i].vsource.z;
						float _fx;
						_fx = 0.5f;
						unsigned long _gct;
						_gct = 0;

				EERIE_3D position;

				spells[i].lastupdate=tim;

				position.x=_source.x+rnd()*500.f-250.f;
				position.y=_source.y+rnd()*500.f-250.f;
				position.z=_source.z+rnd()*500.f-250.f;
				ARX_SOUND_RefreshPosition(spells[i].snd_loop, &position);
				ARX_SOUND_RefreshVolume(spells[i].snd_loop, _fx + 0.5F);
				ARX_SOUND_RefreshPitch(spells[i].snd_loop, 0.8F + 0.4F * rnd());

				if (rnd()>0.62f) 
				{
					position.x=_source.x+rnd()*500.f-250.f;
					position.y=_source.y+rnd()*500.f-250.f;
					position.z=_source.z+rnd()*500.f-250.f;
					ARX_SOUND_PlaySFX(SND_SPELL_SPARK, &position, 0.8F + 0.4F * rnd());
				} 

				if (rnd()>0.82f) 
				{
					position.x=_source.x+rnd()*500.f-250.f;
					position.y=_source.y+rnd()*500.f-250.f;
					position.z=_source.z+rnd()*500.f-250.f;
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
					DynLight[spells[i].longinfo].rgb.r=1.f*fxx;					
					DynLight[spells[i].longinfo].rgb.g=0.f*fxx;
					DynLight[spells[i].longinfo].rgb.b=0.f*fxx;
				}
			}
		break;
		//-----------------------------------------------------------------------------------------				
		case SPELL_TELEPORT:
				{
					TELEPORT=(float)(((float)tim-(float)spells[i].timcreation)/(float)spells[i].tolive);

					if ((LASTTELEPORT<0.5f) && (TELEPORT>=0.5f))
					{
						EERIE_3D pos;
						
						pos.x=lastteleport.x;
						pos.y=lastteleport.y;
						pos.z=lastteleport.z;							
						lastteleport.x=player.pos.x;
						lastteleport.y=player.pos.y;
						lastteleport.z=player.pos.z;
						player.pos.x=pos.x;
						player.pos.y=pos.y;
						player.pos.z=pos.z;
						LASTTELEPORT=32.f;
						ARX_SOUND_PlaySFX(SND_SPELL_TELEPORTED, &player.pos);
					}
					else LASTTELEPORT=TELEPORT;

					if (TELEPORT>=0.5f) 
					{
						TELEPORT=1.f-(TELEPORT-0.5f)*2.f;
					}
					else TELEPORT*=2.f;
					
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
						if (!(inter.iobj[spells[i].target]->GameFlags & GFLAG_INVISIBILITY))
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
						else scaley=EEfabs(inter.iobj[spells[i].caster]->physics.cyl.height*DIV2)+30.f;

						float mov1=EEsin((float)LastFrameTime*DIV800)*scaley;
						float mov=EEsin((float)FrameTime*DIV800)*scaley;

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
								cabalpos.x = inter.iobj[spells[i].caster]->pos.x; 
							cabalpos.y=inter.iobj[spells[i].caster]->pos.y-scaley-mov;
								cabalpos.z = inter.iobj[spells[i].caster]->pos.z; 
							refpos=inter.iobj[spells[i].caster]->pos.y-scaley;							
						}

						float Es=EEsin((float)FrameTime*DIV800 + DEG2RAD(scaley));

						if (spells[i].longinfo2!=-1)
						{
							DynLight[spells[i].longinfo2].pos.x = cabalpos.x;
							DynLight[spells[i].longinfo2].pos.y = refpos;
							DynLight[spells[i].longinfo2].pos.z = cabalpos.z;
							DynLight[spells[i].longinfo2].rgb.b=rnd()*0.2f+0.8f;
							DynLight[spells[i].longinfo2].fallstart=Es*1.5f;
						}

						GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,   D3DBLEND_ONE );
						GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE );
						SETALPHABLEND(GDevice,TRUE);
						SETZWRITE(GDevice, FALSE );
						cabalangle.b=spells[i].fdata+(float)FrameDiff*0.1f;
						spells[i].fdata=cabalangle.b;

							cabalangle.g = 0.f; 
						
							cabalcolor.r = cabalcolor.g = 0.4f;
							cabalcolor.b = 0.8f;
						
						cabalscale.z=cabalscale.y=cabalscale.x=Es;				
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime-30.f)*DIV800)*scaley;
						cabalpos.y=refpos-mov;						
							cabalcolor.r = cabalcolor.g = 0.2f;
							cabalcolor.b = 0.5f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime-60.f)*DIV800)*scaley;
						cabalpos.y=refpos-mov;
							cabalcolor.r = cabalcolor.g = 0.1f;
							cabalcolor.b = 0.25f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime-120.f)*DIV800)*scaley;
						cabalpos.y=refpos-mov;
							cabalcolor.r = cabalcolor.g = 0.f;
							cabalcolor.b = 0.15f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						cabalangle.b=-cabalangle.b;
						cabalpos.y=refpos-mov;
						cabalscale.x=cabalscale.y=cabalscale.z=Es;
							cabalcolor.r = cabalcolor.g = 0.f;
							cabalcolor.b = 0.15f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime+30.f)*DIV800)*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.r = cabalcolor.g = 0.1f;
							cabalcolor.b = 0.25f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime+60.f)*DIV800)*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.r = cabalcolor.g = 0.2f;
							cabalcolor.b = 0.5f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime+120.f)*DIV800)*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.r = cabalcolor.g = 0.4f;
							cabalcolor.b = 0.8f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						cabalangle.b=-cabalangle.b;
						SETALPHABLEND(GDevice,FALSE);		
						SETZWRITE(GDevice, TRUE );	

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
						else scaley=EEfabs(inter.iobj[spells[i].caster]->physics.cyl.height*DIV2)+30.f;

 
						float mov=EEsin((float)FrameTime*DIV800)*scaley;

						if (spells[i].caster==0)
						{
								cabalpos.x = player.pos.x; 
							cabalpos.y=player.pos.y+60.f-mov;
								cabalpos.z = player.pos.z; 
							refpos=player.pos.y+60.f;							
						}
						else
						{							
								cabalpos.x = inter.iobj[spells[i].caster]->pos.x; 
							cabalpos.y=inter.iobj[spells[i].caster]->pos.y-scaley-mov;
								cabalpos.z = inter.iobj[spells[i].caster]->pos.z; 
							refpos=inter.iobj[spells[i].caster]->pos.y-scaley;							
						}

						float Es=EEsin((float)FrameTime*DIV800 + DEG2RAD(scaley));

						if (spells[i].longinfo2!=-1)
						{
							DynLight[spells[i].longinfo2].pos.x = cabalpos.x;
							DynLight[spells[i].longinfo2].pos.y = refpos;
							DynLight[spells[i].longinfo2].pos.z = cabalpos.z; 
							DynLight[spells[i].longinfo2].rgb.r=rnd()*0.2f+0.8f;
							DynLight[spells[i].longinfo2].fallstart=Es*1.5f;
						}

						SETCULL(GDevice,D3DCULL_NONE);
						GDevice->SetRenderState( D3DRENDERSTATE_SRCBLEND,   D3DBLEND_ONE );
						GDevice->SetRenderState( D3DRENDERSTATE_DESTBLEND,  D3DBLEND_ONE );
						SETALPHABLEND(GDevice,TRUE);
						SETZWRITE(GDevice, FALSE );
						cabalangle.b=spells[i].fdata+(float)FrameDiff*0.1f;
						spells[i].fdata=cabalangle.b;
							cabalangle.g = 0.f;
							cabalcolor.r = 0.8f;
							cabalcolor.g = 0.f;
							cabalcolor.b = 0.f;
						cabalscale.z=cabalscale.y=cabalscale.x=Es;				
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime-30.f)*DIV800)*scaley;
						cabalpos.y=refpos-mov;						
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.5f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime-60.f)*DIV800)*scaley;
						cabalpos.y=refpos-mov;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.25f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime-120.f)*DIV800)*scaley;
						cabalpos.y=refpos-mov;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.15f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						cabalangle.b=-cabalangle.b;
						cabalpos.y=refpos-mov;
						cabalscale.x=cabalscale.y=cabalscale.z=Es;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.15f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime+30.f)*DIV800)*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.25f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime+60.f)*DIV800)*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.5f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						mov=EEsin((float)(FrameTime+120.f)*DIV800)*scaley;
						cabalpos.y=refpos+mov;
							cabalcolor.b = cabalcolor.g = 0.f;
							cabalcolor.r = 0.8f;
						DrawEERIEObjEx(GDevice,cabal,&cabalangle,&cabalpos,&cabalscale,&cabalcolor);	
						cabalangle.b=-cabalangle.b;
						SETALPHABLEND(GDevice,FALSE);		
						SETZWRITE(GDevice, TRUE );	

						ARX_SOUND_RefreshPosition(spells[i].snd_loop, &cabalpos);
						}
					}
				break;				
				//-----------------------------------------------------------------------------------------
				case SPELL_FLYING_EYE:
					{
						eyeball.floating=EEsin((spells[i].lastupdate-spells[i].timcreation)*DIV1000)*10.f;					
						
							if (spells[i].lastupdate-spells[i].timcreation<=3000)
							{
								F2L((float)(spells[i].lastupdate-spells[i].timcreation)*DIV30,&eyeball.exist);
								float d=(float)eyeball.exist*DIV100;
							
							eyeball.size.x = 1.f - d; 
							eyeball.size.y = 1.f - d; 
							eyeball.size.z = 1.f - d; 
								
								eyeball.angle.b+=framediff3*DIV10*6.f;
							}
							else 
							{
								eyeball.exist=2;
							}
						
						spells[i].lastupdate=tim;
					}
				break;
			}		
		}
	}
	if( bSoftRender ) SET_FORCE_NO_VB( bNoVB );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
void TryToCastSpell(INTERACTIVE_OBJ * io, const long &spellid, const long &level, const long &target, const long &flags, const long &duration)
{
	if (!io || io->spellcast_data.castingspell >= 0) return;

	if (!(flags & SPELLCAST_FLAG_NOMANA)
			&& (io->ioflags & IO_NPC) && (io->_npcdata->mana<=0.f))
		return;

	unsigned long i(0);

	for (; i < SPELL_COUNT; i++)
		if (spellicons[i].spellid == spellid) break;

	if ( i >= SPELL_COUNT) return; // not an existing spell...

	for (unsigned long j(0); j < 4; j++) io->spellcast_data.symb[j] = 255;

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
	

 	ARX_CHECK_SHORT(flags);
	ARX_CHECK_SHORT(level);
	io->spellcast_data.spell_flags = ARX_CLEAN_WARN_CAST_SHORT(flags);
	io->spellcast_data.spell_level = ARX_CLEAN_WARN_CAST_SHORT(level);


	io->spellcast_data.duration = duration;
	io->spellcast_data.target = target;
	
	io->GameFlags &=~GFLAG_INVISIBILITY;
	
	if (	((io->spellcast_data.spell_flags & SPELLCAST_FLAG_NOANIM)
		&&	(io->spellcast_data.spell_flags & SPELLCAST_FLAG_NODRAW) )
		||	(io->spellcast_data.spell_flags & SPELLCAST_FLAG_PRECAST))	
	{
		
		ARX_SPELLS_Launch(io->spellcast_data.castingspell,GetInterNum(io),io->spellcast_data.spell_flags,io->spellcast_data.spell_level,io->spellcast_data.target,io->spellcast_data.duration);
		io->spellcast_data.castingspell=-1;
	}

	io->spellcast_data.spell_flags &=~SPELLCAST_FLAG_NODRAW; // temporary, removes colored flares
}

void ApplySPWep()
{
	if (!sp_wep)
	{		
		ARX_SPSound();
		char tex[256];
		char tex2[256];
		sprintf(tex2,"%sGraph\\Obj3D\\Interactive\\Items\\Weapons\\sword_mx\\sword_mx.teo",Project.workingdir);
		File_Standardize(tex2,tex);
		INTERACTIVE_OBJ * ioo=(INTERACTIVE_OBJ *)AddItem(GDevice,tex,IO_IMMEDIATELOAD);

		if (ioo!=NULL)
		{			
			sp_wep=1;
			MakeCoolFx(&player.pos);
			MakeCoolFx(&player.pos);
			ioo->scriptload=1;
			MakeTemporaryIOIdent(ioo);
			SendInitScriptEvent(ioo);
			ioo->show=SHOW_FLAG_IN_INVENTORY;									

			if (!CanBePutInInventory(ioo))
			{
				PutInFrontOfPlayer(ioo,1);
			}

			MakeSpCol();
			strcpy(sp_max_ch,"!!!_Grosbillite_!!!");
			sp_max_nb=strlen(sp_max_ch);
			sp_max_start=ARX_TIME_Get();
		}
	}
}
void MakeSpCol()
{
	ARX_SPSound();

	for (long i=0;i<64;i++)
	{
		sp_max_y[i]=0;
	}

	sp_max_col[0]=0x00FF0000;
	sp_max_col[1]=0x0000FF00;
	sp_max_col[2]=0x000000FF;
	sp_max_col[3]=0x00FFFF00;
	sp_max_col[4]=0x00FF00FF;
	sp_max_col[5]=0x0000FFFF;
	sp_max_col[6]=0x00FF0000;
	sp_max_col[7]=0x0000FF00;
	sp_max_col[8]=0x000000FF;
	sp_max_col[9]=0x00FFFF00;
	sp_max_col[10]=0x00FF00FF;
	sp_max_col[11]=0x0000FFFF;
	sp_max_col[12]=0x00FF0000;
	sp_max_col[13]=0x0000FF00;
	sp_max_col[14]=0x000000FF;
	sp_max_col[15]=0x00FFFF00;
	sp_max_col[16]=0x00FF00FF;
	sp_max_col[17]=0x0000FFFF;
	sp_max_col[18]=0x00FF0000;
	sp_max_col[19]=0x0000FF00;
	sp_max_col[20]=0x000000FF;
	sp_max_col[21]=0x00FFFF00;
	sp_max_col[22]=0x00FF00FF;
	sp_max_col[23]=0x0000FFFF;
	sp_max_col[24]=0x00FFFF00;
	sp_max_col[25]=0x00FF00FF;
	sp_max_col[26]=0x0000FFFF;
	sp_max_col[27]=0x00FF0000;
	sp_max_col[28]=0x0000FF00;
	sp_max_col[29]=0x000000FF;
	sp_max_col[30]=0x00FFFF00;
	sp_max_col[31]=0x00FF00FF;
	sp_max_col[32]=0x0000FFFF;
}
void ApplyCurSOS()
{
	MakeSpCol();
	ARX_MINIMAP_Reveal();
	strcpy(sp_max_ch,"!!!_Temple of Elemental Lavis_!!!");
	sp_max_nb=strlen(sp_max_ch);	
	sp_max_start=ARX_TIME_Get();
}
void ApplySPBow()
{
	{		
		ARX_SPSound();
		char tex[256];
		char tex2[256];
		sprintf(tex2,"%sGraph\\Obj3D\\Interactive\\Items\\Weapons\\bow_mx\\bow_mx.teo",Project.workingdir);
		File_Standardize(tex2,tex);
		INTERACTIVE_OBJ * ioo=(INTERACTIVE_OBJ *)AddItem(GDevice,tex,IO_IMMEDIATELOAD);

		if (ioo!=NULL)
		{			
			MakeCoolFx(&player.pos);
			MakeCoolFx(&player.pos);
			ioo->scriptload=1;
			MakeTemporaryIOIdent(ioo);
			SendInitScriptEvent(ioo);
			ioo->show=SHOW_FLAG_IN_INVENTORY;									

			if (!CanBePutInInventory(ioo))
			{
				PutInFrontOfPlayer(ioo,1);
			}

			MakeSpCol();
			strcpy(sp_max_ch,"!!!_Bow to Samy & Anne_!!!");
			sp_max_nb=strlen(sp_max_ch);
			sp_max_start=ARX_TIME_Get();
		}
	}
}
void ApplySPArm()
{
	ARX_SPSound();
	char tex[256];
	char tex2[256];

	switch (sp_arm)
	{
		case 0:
			sprintf(tex2,"%sGraph\\Obj3D\\Interactive\\Items\\Armor\\Helmet_plate_cm\\Helmet_plate_cm.teo",Project.workingdir);
		break;
		case 1:
			sprintf(tex2,"%sGraph\\Obj3D\\Interactive\\Items\\Armor\\Legging_plate_cm\\Legging_plate_cm.teo",Project.workingdir);
		break;
		case 2:
			sprintf(tex2,"%sGraph\\Obj3D\\Interactive\\Items\\Armor\\Chest_plate_cm\\Chest_plate_cm.teo",Project.workingdir);
		break;
		default:
			return;
		break;
	}

	File_Standardize(tex2,tex);
	INTERACTIVE_OBJ * ioo=(INTERACTIVE_OBJ *)AddItem(GDevice,tex,IO_IMMEDIATELOAD);

	if (ioo!=NULL)
	{			
		sp_wep=1;
		MakeCoolFx(&player.pos);
		MakeCoolFx(&player.pos);
		ioo->scriptload=1;
		MakeTemporaryIOIdent(ioo);
		SendInitScriptEvent(ioo);
		ioo->show=SHOW_FLAG_IN_INVENTORY;									

		if (!CanBePutInInventory(ioo))
		{
			PutInFrontOfPlayer(ioo,1);
		}

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
		sp_max_start=ARX_TIME_Get();
	}

	sp_arm++;
}

extern long SPECIAL_PNUX;
void ApplyCurPNux()
{
	
	{
		MakeSpCol();
		strcpy(sp_max_ch,"! PhilNux & Gluonne !");
		sp_max_nb=strlen(sp_max_ch);
		
		if (SPECIAL_PNUX<3)
		{
			SPECIAL_PNUX++;
		}
		else
		{
			SPECIAL_PNUX=0;			
		}

		D3DTextr_InvalidateAllTextures();
		D3DTextr_RestoreAllTextures(GDevice);
		ARX_PLAYER_Restore_Skin();
		cur_pnux=0;
		sp_max_start=ARX_TIME_Get();
	}
}
void ApplyPasswall()
{
	MakeSpCol();
	strcpy(sp_max_ch,"!!! PassWall !!!");
	sp_max_nb=strlen(sp_max_ch);
	sp_max_start=ARX_TIME_Get();

	if (USE_PLAYERCOLLISIONS)
		USE_PLAYERCOLLISIONS=0;
	else
		USE_PLAYERCOLLISIONS=1;
}

void ApplySPRf()
{
	if (cur_rf==3)
	{
		MakeSpCol();
		strcpy(sp_max_ch,"!!! RaFMode !!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=ARX_TIME_Get();
	}
}
void ApplyCurMr()
{
	if (cur_mr==3)
	{
		MakeSpCol();
		strcpy(sp_max_ch,"!!! Marianna !!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=ARX_TIME_Get();
	}
}

void ApplySPuw()
{
	uw_mode_pos=0;
	uw_mode=~uw_mode;
	ARX_SOUND_PlayCinematic("menestrel_uw2.wav");
	MakeCoolFx(&player.pos);

	if (uw_mode)
	{
		MakeSpCol();
		strcpy(sp_max_ch,"~-__-~~-__.U.W.__-~~-__-~");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=ARX_TIME_Get();
	}
}
void ApplySPMax()
{
	MakeCoolFx(&player.pos);
	sp_max=~sp_max;

	if (sp_max)
	{
		MakeSpCol();
		strcpy(sp_max_ch,"!!!_FaNt0mAc1e_!!!");
		sp_max_nb=strlen(sp_max_ch);
		sp_max_start=ARX_TIME_Get();

			player.skin=4;

			ARX_EQUIPMENT_RecreatePlayerMesh();
		
		ARX_PLAYER_Rune_Add_All();
		_TCHAR UText[512];
		MultiByteToWideChar(CP_ACP, 0, "!!!!!!! FanTomAciE !!!!!!!", -1, UText, 256);
		ARX_SPEECH_Add(NULL, UText);		
		player.Attribute_Redistribute+=10;
		player.Skill_Redistribute+=50;
		player.level=__max(player.level,10);
		player.xp=GetXPforLevel(10);
	}
	else
	{
		TextureContainer * tcm=MakeTCFromFile("Graph\\Obj3D\\Textures\\npc_human_cm_hero_head.bmp");

		if (tcm)
		{
			D3DTextr_KillTexture(tcm);
			player.heads[0]=MakeTCFromFile("Graph\\Obj3D\\Textures\\npc_human_base_hero_head.bmp");	
			player.heads[1]=MakeTCFromFile("Graph\\Obj3D\\Textures\\npc_human_base_hero2_head.bmp");	
			player.heads[2]=MakeTCFromFile("Graph\\Obj3D\\Textures\\npc_human_base_hero3_head.bmp");	
			ARX_EQUIPMENT_RecreatePlayerMesh();
		}
	}	
}
