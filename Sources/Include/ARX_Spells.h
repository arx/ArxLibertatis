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
// ARX_Spells.h
// ARX Spells Management & Projectiles
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ARX_SPELLS_H
#define ARX_SPELLS_H

#include <EERIEPoly.h>
#include "ARX_Common.h"

//#include "ARX_CSpellFx.h"
class CSpellFx;

void ARX_SPELLS_CancelAll();

// Spells symbol list
enum ARX_SPELLS_SYMBOL
{
	SYMBOL_AAM,         // Create
	SYMBOL_NHI,         // Negate
	SYMBOL_MEGA,        // Improve
	SYMBOL_YOK,         // Fire
	SYMBOL_TAAR,        // Projectile
	SYMBOL_KAOM,        // Protection
	SYMBOL_VITAE,       // Life
	SYMBOL_VISTA,       // Vision
	SYMBOL_STREGUM,     // Magic
	SYMBOL_MORTE,       // Death
	SYMBOL_COSUM,       // Object
	SYMBOL_COMUNICATUM, // Communication
	SYMBOL_MOVIS,       // Movement
	SYMBOL_TEMPUS,      // Time
	SYMBOL_FOLGORA,     // Storm
	SYMBOL_SPACIUM,     // Space
	SYMBOL_TERA,        // Earth
	SYMBOL_CETRIUS,     // Poison
	SYMBOL_RHAA,        // Lower
	SYMBOL_FRIDD,       // Ice
	SYMBOL_AKBAA,       // Akbaa
	SYMBOL_NONE = 255
};

#define RUNE_AAM			SYMBOL_AAM
#define RUNE_CETRIUS		SYMBOL_CETRIUS
#define RUNE_COMUNICATUM	SYMBOL_COMUNICATUM
#define RUNE_COSUM			SYMBOL_COSUM
#define RUNE_FOLGORA		SYMBOL_FOLGORA
#define RUNE_FRIDD			SYMBOL_FRIDD
#define RUNE_KAOM			SYMBOL_KAOM
#define	RUNE_MEGA			SYMBOL_MEGA
#define RUNE_MORTE			SYMBOL_MORTE
#define RUNE_MOVIS			SYMBOL_MOVIS
#define RUNE_NHI			SYMBOL_NHI
#define RUNE_RHAA			SYMBOL_RHAA
#define RUNE_SPACIUM		SYMBOL_SPACIUM
#define RUNE_STREGUM		SYMBOL_STREGUM
#define RUNE_TAAR			SYMBOL_TAAR
#define RUNE_TEMPUS			SYMBOL_TEMPUS
#define RUNE_TERA			SYMBOL_TERA
#define RUNE_VISTA			SYMBOL_VISTA
#define RUNE_VITAE			SYMBOL_VITAE
#define RUNE_YOK			SYMBOL_YOK
#define NB_RUNES			(SYMBOL_AKBAA)+1

enum ARX_SPELLS_SPELLCAST_FLAG
{
	SPELLCAST_FLAG_NODRAW			= (1 << 0),
	SPELLCAST_FLAG_NOANIM			= (1 << 1),
	SPELLCAST_FLAG_NOMANA			= (1 << 2),
	SPELLCAST_FLAG_PRECAST			= (1 << 3),
	SPELLCAST_FLAG_LAUNCHPRECAST	= (1 << 4),
	SPELLCAST_FLAG_NOCHECKCANCAST	= (1 << 5),
	SPELLCAST_FLAG_NOSOUND			= (1 << 6),
	SPELLCAST_FLAG_RESTORE			= (1 << 7),
};

const unsigned long MAX_PRECAST = 3;

typedef struct
{
	long typ;
	long level;
	unsigned long launch_time;
	long flags;
	long duration;
} PRECAST_STRUCT;

extern PRECAST_STRUCT Precast[MAX_PRECAST];

void ARX_SPELLS_Precast_Reset();
void ARX_SPELLS_Precast_Add(const long & typ, long level = 1, long flags = 0);
void ARX_SPELLS_Precast_Launch(const long & num);

long GetSpellId(const char * spell);
BOOL MakeSpellName(char * spell, const long & num);
void TryToCastSpell(INTERACTIVE_OBJ * io, const long & spellid, const long & level, const long & target, const long & flags, const long & duration);
void ARX_SPELLS_Precast_Check();
void ARX_SPELLS_Precast_Launch2(const long & num);
typedef struct
{
	long	exist;
	EERIE_3D pos;
	EERIE_3D angle;
	EERIE_3D size;
	float floating;
} EYEBALL_DEF;

extern EYEBALL_DEF eyeball;

const unsigned long MAX_SPELL_SYMBOLS = 6;
extern long SpellSymbol[MAX_SPELL_SYMBOLS];
extern long CurrSpellSymbol;


// Spell list
enum ARX_SPELLS_SPELLS
{
	// LEVEL 1
	SPELL_MAGIC_SIGHT           ,//= 11,
	SPELL_MAGIC_MISSILE         ,//= 12,
	SPELL_IGNIT                 ,//= 13,
	SPELL_DOUSE                 ,//= 14,
	SPELL_ACTIVATE_PORTAL       ,//= 15,

	// LEVEL 2
	SPELL_HEAL                  ,//= 21,
	SPELL_DETECT_TRAP           ,//= 22,
	SPELL_ARMOR                 ,//= 23,
	SPELL_LOWER_ARMOR           ,//= 24,
	SPELL_HARM                  ,//= 25,

	// LEVEL 3
	SPELL_SPEED                 ,//= 31,
	SPELL_DISPELL_ILLUSION      ,//= 32,
	SPELL_FIREBALL              ,//= 33,
	SPELL_CREATE_FOOD           ,//= 34,
	SPELL_ICE_PROJECTILE        ,//= 35,

	// LEVEL 4
	SPELL_BLESS                 ,//= 41,
	SPELL_DISPELL_FIELD         ,//= 42,
	SPELL_FIRE_PROTECTION       ,//= 43,
	SPELL_TELEKINESIS           ,//= 44,
	SPELL_CURSE                 ,//= 45,
	SPELL_COLD_PROTECTION       ,//= 46,

	// LEVEL 5
	SPELL_RUNE_OF_GUARDING      ,//= 51,
	SPELL_LEVITATE              ,//= 52,
	SPELL_CURE_POISON           ,//= 53,
	SPELL_REPEL_UNDEAD          ,//= 54,
	SPELL_POISON_PROJECTILE     ,//= 55,

	// LEVEL 6
	SPELL_RISE_DEAD             ,//= 61,
	SPELL_PARALYSE              ,//= 62,
	SPELL_CREATE_FIELD          ,//= 63,
	SPELL_DISARM_TRAP           ,//= 64,
	SPELL_SLOW_DOWN             ,//= 65, //secret

	// LEVEL 7
	SPELL_FLYING_EYE            ,//= 71,
	SPELL_FIRE_FIELD            ,//= 72,
	SPELL_ICE_FIELD             ,//= 73,
	SPELL_LIGHTNING_STRIKE      ,//= 74,
	SPELL_CONFUSE               ,//= 75,

	// LEVEL 8
	SPELL_INVISIBILITY          ,//= 81,
	SPELL_MANA_DRAIN            ,//= 82,
	SPELL_EXPLOSION             ,//= 83,
	SPELL_ENCHANT_WEAPON        ,//= 84,
	SPELL_LIFE_DRAIN            ,//= 85, //secret

	// LEVEL 9
	SPELL_SUMMON_CREATURE       ,//= 91,
	SPELL_NEGATE_MAGIC          ,//= 92,
	SPELL_INCINERATE            ,//= 93,
	SPELL_MASS_PARALYSE         ,//= 94,

	// LEVEL 10
	SPELL_MASS_LIGHTNING_STRIKE	,//= 101,
	SPELL_CONTROL_TARGET        ,//= 102,
	SPELL_FREEZE_TIME           ,//= 103,
	SPELL_MASS_INCINERATE       ,//= 104

	SPELL_FAKE_SUMMON			, // special =105
	SPELL_COUNT					,

	// LEVEL ZOB
	SPELL_TELEPORT				,
};

extern float TELEPORT;
extern float LASTTELEPORT;

const unsigned long MAX_LINFO(20);
const unsigned long MAX_SPELLS(20);

typedef struct
{
	BOOL		exist;
	long		caster;         // Number of the source interactive obj (0==player)
	long		target;         // Number of the target interactive obj if any
	float		caster_level;   // Level of Magic 1-10

	long		hand_group;
	EERIE_3D	hand_pos;     // Only valid if hand_group>=0
	EERIE_3D	caster_pos;
	EERIE_3D	caster_angle;
	EERIE_3D	target_pos;
	EERIE_3D	target_angle;
	EERIE_3D	vector_dir;

	float		fdata;          // Specific use for each spell

	long		type;
	EERIE_3D	vsource;
	EERIE_3D	v;

	EERIE_3D	move;
	EERIE_3D	scale;
	float		siz;
	unsigned long		timcreation;
	unsigned long		lastupdate;
	unsigned long		tolive;

	TextureContainer * tc;
	long		longinfo;
	long		longinfo2;
	long		linfo[MAX_LINFO];
	unsigned long		cumul;
	bool		bDuration;
	float		fManaCostPerSecond;

	long		flags;
	long lSpellSource;
	long snd_loop;
	CSpellFx	* pSpellFx;
	void 	*	misc;
} SPELL;

extern SPELL spells[MAX_SPELLS];

const unsigned long MAX_SLOT = 10;    // nombre maximum de directions dans une sequence
const unsigned long NAME_LENGTH = 8;  // longueur du nom des sorts
extern long CurrSlot;
extern long LastSlot;
extern long CurrPoint;
extern long NETSPELL;

long ARX_SPELLS_Launch(const long & typ, const long & source, const long & flags = 0, const long & level = -1, const long & target = -1, const long & duration = -1); //const long &net = 0);
long ARX_SPELLS_GetFree();
void ARX_SPELLS_ResetRecognition();
void ARX_SPELLS_AddPoint(const EERIE_S2D * pos);
void ARX_SPELLS_Analyse();
void ARX_SPELLS_AbortSpellSound();
void ARX_SPELLS_InitKnownSpells();
void ARX_SPELLS_Init();
void ARX_SPELLS_ClearAll();
void ARX_SPELLS_Update(LPDIRECT3DDEVICE7 m_pd3dDevice);
void ARX_SPELLS_Fizzle(const long & num);
void ARX_SPELLS_FizzleNoMana(const long & num);
void ARX_SPELLS_Kill(const long & i);
void ARX_SPELLS_ManageMagic();

void ARX_SPELLS_Update_Lightning(LPDIRECT3DDEVICE7 m_pd3dDevice);

void ClearParticles();

void ARX_SPELLS_RequestSymbolDraw(INTERACTIVE_OBJ * io, const char * name, const float & duration);

void ARX_SPELLS_UpdateSymbolDraw(LPDIRECT3DDEVICE7 pd3dDevice);
void ARX_SPELLS_ClearAllSymbolDraw();

void ARX_SPELLS_Init(LPDIRECT3DDEVICE7 m_pd3dDevice);

BOOL ARX_SPELLS_ExistAnyInstance(const long & typ);
void ARX_SPELLS_RemoveAllSpellsOn(INTERACTIVE_OBJ * io);
long ARX_SPELLS_GetSpellOn(const INTERACTIVE_OBJ * io, const long & spellid);
long ARX_SPELLS_GetInstanceForThisCaster(const long & typ, const long & caster);

void ARX_SPELLS_CancelSpellTarget();
void ARX_SPELLS_LaunchSpellTarget(INTERACTIVE_OBJ * io);
float ARX_SPELLS_ApplyFireProtection(INTERACTIVE_OBJ * io, float damages);
float ARX_SPELLS_ApplyColdProtection(INTERACTIVE_OBJ * io, float damages);
BOOL ARX_SPELLS_ExistAnyInstanceForThisCaster(const long & typ, const long & caster);
void ApplySPWep();
void ARX_SPELLS_FizzleAllSpellsFromCaster(long num);
#endif//ARX_SPELLS_H