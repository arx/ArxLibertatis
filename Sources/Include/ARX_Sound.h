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
//////////////////////////////////////////////////////////////////////////////////////
//
// ARX_Sound.h
// ARX Sound Management
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_SOUND_H
#define ARX_SOUND_H

#include <EERIEPoly.h>

enum
{
	ARX_SOUND_ON     = 0x00000001,
	ARX_SOUND_REVERB = 0x00000002
};

enum
{
	ARX_SOUND_PLAY_LOOPED = 0,
	ARX_SOUND_PLAY_ONCE =   1
};

const long ARX_SOUND_INVALID_RESOURCE(-1);

extern long ARX_SOUND_MixerGame;
extern long ARX_SOUND_MixerGameSample;
extern long ARX_SOUND_MixerGameSpeech;
extern long ARX_SOUND_MixerGameAmbiance;
extern long ARX_SOUND_MixerMenu;
extern long ARX_SOUND_MixerMenuSample;
extern long ARX_SOUND_MixerMenuSpeech;
extern long ARX_SOUND_MixerMenuAmbiance;

// Menu ambiances
extern char AMB_MENU[];
extern char AMB_CREDITS[];

// Menu samples
extern long SND_MENU_CLICK;
extern long SND_MENU_CREDITS_LOOP;
extern long SND_MENU_LOOP;
extern long SND_MENU_OPTIONS_LOOP;
extern long SND_MENU_PUSH;
extern long SND_MENU_RELEASE;

// Interface samples
extern long SND_BACKPACK;
extern long SND_BOOK_OPEN;
extern long SND_BOOK_CLOSE;
extern long SND_BOOK_PAGE_TURN;
extern long SND_GOLD;
extern long SND_INVSTD;
extern long SND_MAP;
extern long SND_SCROLL_OPEN;
extern long SND_SCROLL_CLOSE;
extern long SND_TORCH_START;
extern long SND_TORCH_LOOP;
extern long SND_TORCH_END;

// Other SFX samples
extern long SND_FIREPLACE;
extern long SND_PLOUF;
extern long SND_QUAKE;
extern long SND_WHOOSH;

// Player samples
extern long SND_PLAYER_DEATH;
extern long SND_PLAYER_DEATH_BY_FIRE;
extern long SND_PLAYER_FILLLIFEMANA;
extern long SND_PLAYER_HEART_BEAT;
extern long SND_PLAYER_JUMP;
extern long SND_PLAYER_JUMP_END;
extern long SND_PLAYER_LEVEL_UP;
extern long SND_PLAYER_POISONED;

// Magic drawing samples
extern long SND_MAGIC_AMBIENT;
extern long SND_MAGIC_DRAW;
extern long SND_MAGIC_FIZZLE;

// Magic symbols samples
extern long SND_SYMB_AAM;
extern long SND_SYMB_CETRIUS;
extern long SND_SYMB_COSUM;
extern long SND_SYMB_COMUNICATUM;
extern long SND_SYMB_FOLGORA;
extern long SND_SYMB_FRIDD;
extern long SND_SYMB_KAOM;
extern long SND_SYMB_MEGA;
extern long SND_SYMB_MORTE;
extern long SND_SYMB_MOVIS;
extern long SND_SYMB_NHI;
extern long SND_SYMB_RHAA;
extern long SND_SYMB_SPACIUM;
extern long SND_SYMB_STREGUM;
extern long SND_SYMB_TAAR;
extern long SND_SYMB_TEMPUS;
extern long SND_SYMB_TERA;
extern long SND_SYMB_VISTA;
extern long SND_SYMB_VITAE;
extern long SND_SYMB_YOK;

// Spells samples
extern long SND_SPELL_ACTIVATE_PORTAL;
extern long SND_SPELL_ARMOR_START;
extern long SND_SPELL_ARMOR_END;
extern long SND_SPELL_ARMOR_LOOP;
extern long SND_SPELL_BLESS;
extern long SND_SPELL_COLD_PROTECTION_START;
extern long SND_SPELL_COLD_PROTECTION_LOOP;
extern long SND_SPELL_COLD_PROTECTION_END;
extern long SND_SPELL_CONFUSE;
extern long SND_SPELL_CONTROL_TARGET;
extern long SND_SPELL_CREATE_FIELD;
extern long SND_SPELL_CREATE_FOOD;
extern long SND_SPELL_CURE_POISON;
extern long SND_SPELL_CURSE;
extern long SND_SPELL_DETECT_TRAP;
extern long SND_SPELL_DETECT_TRAP_LOOP;
extern long SND_SPELL_DISARM_TRAP;
extern long SND_SPELL_DISPELL_FIELD;
extern long SND_SPELL_DISPELL_ILLUSION;
extern long SND_SPELL_DOUSE;
extern long SND_SPELL_ELECTRIC;
extern long SND_SPELL_ENCHANT_WEAPON;
extern long SND_SPELL_EXPLOSION;
extern long SND_SPELL_EYEBALL_IN;
extern long SND_SPELL_EYEBALL_OUT;
extern long SND_SPELL_FIRE_FIELD;
extern long SND_SPELL_FIRE_HIT;
extern long SND_SPELL_FIRE_LAUNCH;
extern long SND_SPELL_FIRE_PROTECTION;
extern long SND_SPELL_FIRE_WIND;
extern long SND_SPELL_FREEZETIME;
extern long SND_SPELL_HARM;
extern long SND_SPELL_HEALING;
extern long SND_SPELL_ICE_FIELD;
extern long SND_SPELL_ICE_PROJECTILE_LAUNCH;
extern long SND_SPELL_INCINERATE;
extern long SND_SPELL_IGNITE;
extern long SND_SPELL_INVISIBILITY_START;
extern long SND_SPELL_INVISIBILITY_END;
extern long SND_SPELL_LEVITATE_START;
extern long SND_SPELL_LIGHTNING;
extern long SND_SPELL_LIGHTNING_START;
extern long SND_SPELL_LIGHTNING_LOOP;
extern long SND_SPELL_LIGHTNING_END;

extern long SND_SPELL_MASS_LIGHTNING_END;
extern long SND_SPELL_FIRE_FIELD_START;
extern long SND_SPELL_FIRE_FIELD_LOOP;
extern long SND_SPELL_FIRE_FIELD_END;

extern long SND_SPELL_MAGICAL_HIT;
extern long SND_SPELL_MAGICAL_SHIELD;
extern long SND_SPELL_MASS_INCINERATE;
extern long SND_SPELL_MASS_PARALYSE;
extern long SND_SPELL_MM_CREATE;
extern long SND_SPELL_MM_HIT;
extern long SND_SPELL_MM_LAUNCH;
extern long SND_SPELL_MM_LOOP;
extern long SND_SPELL_NEGATE_MAGIC;
extern long SND_SPELL_NO_EFFECT;
extern long SND_SPELL_PARALYSE;
extern long SND_SPELL_PARALYSE_END;
extern long SND_SPELL_POISON_PROJECTILE_LAUNCH;
extern long SND_SPELL_RAISE_DEAD;
extern long SND_SPELL_REPEL_UNDEAD;
extern long SND_SPELL_REPEL_UNDEAD_LOOP;
extern long SND_SPELL_RUNE_OF_GUARDING;
extern long SND_SPELL_SLOW_DOWN;
extern long SND_SPELL_SPARK;
extern long SND_SPELL_SPEED_START;
extern long SND_SPELL_SPEED_LOOP;
extern long SND_SPELL_SPEED_END;
extern long SND_SPELL_SUMMON_CREATURE;
extern long SND_SPELL_TELEKINESIS_START;
extern long SND_SPELL_TELEKINESIS_END;
extern long SND_SPELL_TELEPORT;
extern long SND_SPELL_TELEPORTED;
extern long SND_SPELL_VISION_START;
extern long SND_SPELL_VISION_LOOP;

// inter-material sounds
long ARX_MATERIAL_GetIdByName(char * name);
bool ARX_MATERIAL_GetNameById(long id, char * name);

long ARX_SOUND_Init(HWND hwnd);
void ARX_SOUND_Release();

long ARX_SOUND_IsEnabled();
void ARX_SOUND_EnableReverb(const long & status);
long ARX_SOUND_IsReverbEnabled();
void ARX_SOUND_EnvironmentSet(const char * name);

void ARX_SOUND_SetListener(const EERIE_3D * position, const EERIE_3D * front, const EERIE_3D * up);

long ARX_SOUND_Load(const char * name);
void ARX_SOUND_Free(const long & sample);

long ARX_SOUND_PlaySFX(long & sample_id, const EERIE_3D * position = NULL, const float & pitch = 1.0F, const long & loop = ARX_SOUND_PLAY_ONCE);
long ARX_SOUND_PlayInterface(long & sample_id, const float & pitch = 1.0F, const long & loop = ARX_SOUND_PLAY_ONCE);
 
long ARX_SOUND_PlaySpeech(const char * name, const INTERACTIVE_OBJ * io = NULL);
long ARX_SOUND_PlayCollision(const long & mat1, const long & mat2, const float & volume, const float & power, EERIE_3D * position, INTERACTIVE_OBJ * source);
long ARX_SOUND_PlayCollision(const char * mat1, const char * mat2, const float & volume, const float & power, EERIE_3D * position, INTERACTIVE_OBJ * source);
long ARX_SOUND_PlayScript(const char * name, const INTERACTIVE_OBJ * io = NULL, const float & pitch = 1.0F, const long & loop = ARX_SOUND_PLAY_ONCE);
long ARX_SOUND_PlayAnim(long & sample_id, const EERIE_3D * position = NULL);
long ARX_SOUND_PlayCinematic(const char * name);
long ARX_SOUND_PlayMenu(long & sample_id, const float & pitch = 1.0F, const long & loop = ARX_SOUND_PLAY_ONCE);
long ARX_SOUND_IsPlaying(long & sample_id);
float ARX_SOUND_GetDuration(long & sample_id);
 
void ARX_SOUND_RefreshVolume(long & sample_id, const float & volume);
void ARX_SOUND_RefreshPosition(long & sample_id, const EERIE_3D * position = NULL);
void ARX_SOUND_RefreshPitch(long & sample_id, const float & pitch);
void ARX_SOUND_RefreshSpeechPosition(long & sample_id, const INTERACTIVE_OBJ * io = NULL);
 
 
void ARX_SOUND_Stop(long & sample_id);

long ARX_SOUND_PlayScriptAmbiance(const char * ambiance_name, const long & loop = ARX_SOUND_PLAY_LOOPED, const float & volume = 1.0F); //, const EERIE_3D *position = NULL);
long ARX_SOUND_PlayZoneAmbiance(const char * ambiance_name, const long & loop = ARX_SOUND_PLAY_LOOPED, const float & volume = 1.0F); //, const EERIE_3D *position = NULL);
long ARX_SOUND_PlayMenuAmbiance(const char * ambiance_name);
long ARX_SOUND_SetAmbianceTrackStatus(const char * ambiance_name, const char * track_name, const unsigned long & status); //0 = off; 1 = on
void ARX_SOUND_KillAmbiances();
void ARX_SOUND_AmbianceSavePlayList(void ** play_list, unsigned long * size);
void ARX_SOUND_AmbianceRestorePlayList(void * play_list, unsigned long size);

void ARX_SOUND_MixerSetVolume(const long & mixer_id, const float & volume);
float ARX_SOUND_MixerGetVolume(const long & mixer_id);
void ARX_SOUND_MixerStop(const long & mixer_id);
void ARX_SOUND_MixerPause(const long & mixer_id);
void ARX_SOUND_MixerResume(const long & mixer_id);
void ARX_SOUND_MixerSwitch(const long & from, const long & to);

#endif//ARX_SOUND_H
