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

#ifndef ARX_SCENE_GAMESOUND_H
#define ARX_SCENE_GAMESOUND_H

#include <string>

#include "platform/Platform.h"
#include "platform/math/Vector3.h"

struct INTERACTIVE_OBJ;

enum SoundLoopMode {
	ARX_SOUND_PLAY_LOOPED = 0,
	ARX_SOUND_PLAY_ONCE = 1
};

typedef s32 ArxSound;
typedef s32 ArxMixer;

const ArxSound ARX_SOUND_INVALID_RESOURCE = -1;

extern ArxMixer ARX_SOUND_MixerGame;
extern ArxMixer ARX_SOUND_MixerGameSample;
extern ArxMixer ARX_SOUND_MixerGameSpeech;
extern ArxMixer ARX_SOUND_MixerGameAmbiance;
extern ArxMixer ARX_SOUND_MixerMenu;
extern ArxMixer ARX_SOUND_MixerMenuSample;
extern ArxMixer ARX_SOUND_MixerMenuSpeech;
extern ArxMixer ARX_SOUND_MixerMenuAmbiance;

// Menu ambiances
const std::string AMB_MENU = "ambient_menu.amb";
const std::string AMB_CREDITS = "ambient_credits.amb";

// Menu samples
extern ArxSound SND_MENU_CLICK;
extern ArxSound SND_MENU_RELEASE;

// Interface samples
extern ArxSound SND_BACKPACK;
extern ArxSound SND_BOOK_OPEN;
extern ArxSound SND_BOOK_CLOSE;
extern ArxSound SND_BOOK_PAGE_TURN;
extern ArxSound SND_GOLD;
extern ArxSound SND_INVSTD;
extern ArxSound SND_SCROLL_OPEN;
extern ArxSound SND_SCROLL_CLOSE;
extern ArxSound SND_TORCH_START;
extern ArxSound SND_TORCH_LOOP;
extern ArxSound SND_TORCH_END;

// Other SFX samples
extern ArxSound SND_FIREPLACE;
extern ArxSound SND_PLOUF;
extern ArxSound SND_QUAKE;
extern ArxSound SND_WHOOSH;

// Player samples
extern ArxSound SND_PLAYER_DEATH_BY_FIRE;
extern ArxSound SND_PLAYER_FILLLIFEMANA;
extern ArxSound SND_PLAYER_HEART_BEAT;
extern ArxSound SND_PLAYER_LEVEL_UP;
extern ArxSound SND_PLAYER_POISONED;

// Magic drawing samples
extern ArxSound SND_MAGIC_AMBIENT;
extern ArxSound SND_MAGIC_DRAW;
extern ArxSound SND_MAGIC_FIZZLE;

// Magic symbols samples
extern ArxSound SND_SYMB_AAM;
extern ArxSound SND_SYMB_CETRIUS;
extern ArxSound SND_SYMB_COSUM;
extern ArxSound SND_SYMB_COMUNICATUM;
extern ArxSound SND_SYMB_FOLGORA;
extern ArxSound SND_SYMB_FRIDD;
extern ArxSound SND_SYMB_KAOM;
extern ArxSound SND_SYMB_MEGA;
extern ArxSound SND_SYMB_MORTE;
extern ArxSound SND_SYMB_MOVIS;
extern ArxSound SND_SYMB_NHI;
extern ArxSound SND_SYMB_RHAA;
extern ArxSound SND_SYMB_SPACIUM;
extern ArxSound SND_SYMB_STREGUM;
extern ArxSound SND_SYMB_TAAR;
extern ArxSound SND_SYMB_TEMPUS;
extern ArxSound SND_SYMB_TERA;
extern ArxSound SND_SYMB_VISTA;
extern ArxSound SND_SYMB_VITAE;
extern ArxSound SND_SYMB_YOK;

// Spells samples
extern ArxSound SND_SPELL_ACTIVATE_PORTAL;
extern ArxSound SND_SPELL_ARMOR_START;
extern ArxSound SND_SPELL_ARMOR_END;
extern ArxSound SND_SPELL_ARMOR_LOOP;
extern ArxSound SND_SPELL_BLESS;
extern ArxSound SND_SPELL_COLD_PROTECTION_START;
extern ArxSound SND_SPELL_COLD_PROTECTION_LOOP;
extern ArxSound SND_SPELL_COLD_PROTECTION_END;
extern ArxSound SND_SPELL_CONFUSE;
extern ArxSound SND_SPELL_CONTROL_TARGET;
extern ArxSound SND_SPELL_CREATE_FIELD;
extern ArxSound SND_SPELL_CREATE_FOOD;
extern ArxSound SND_SPELL_CURE_POISON;
extern ArxSound SND_SPELL_CURSE;
extern ArxSound SND_SPELL_DETECT_TRAP;
extern ArxSound SND_SPELL_DETECT_TRAP_LOOP;
extern ArxSound SND_SPELL_DISARM_TRAP;
extern ArxSound SND_SPELL_DISPELL_FIELD;
extern ArxSound SND_SPELL_DISPELL_ILLUSION;
extern ArxSound SND_SPELL_DOUSE;
extern ArxSound SND_SPELL_ELECTRIC;
extern ArxSound SND_SPELL_ENCHANT_WEAPON;
extern ArxSound SND_SPELL_EXPLOSION;
extern ArxSound SND_SPELL_EYEBALL_IN;
extern ArxSound SND_SPELL_EYEBALL_OUT;
extern ArxSound SND_SPELL_FIRE_FIELD;
extern ArxSound SND_SPELL_FIRE_HIT;
extern ArxSound SND_SPELL_FIRE_LAUNCH;
extern ArxSound SND_SPELL_FIRE_PROTECTION;
extern ArxSound SND_SPELL_FIRE_WIND;
extern ArxSound SND_SPELL_FREEZETIME;
extern ArxSound SND_SPELL_HARM;
extern ArxSound SND_SPELL_HEALING;
extern ArxSound SND_SPELL_ICE_FIELD;
extern ArxSound SND_SPELL_ICE_PROJECTILE_LAUNCH;
extern ArxSound SND_SPELL_INCINERATE;
extern ArxSound SND_SPELL_IGNITE;
extern ArxSound SND_SPELL_INVISIBILITY_START;
extern ArxSound SND_SPELL_INVISIBILITY_END;
extern ArxSound SND_SPELL_LEVITATE_START;
extern ArxSound SND_SPELL_LIGHTNING;
extern ArxSound SND_SPELL_LIGHTNING_START;
extern ArxSound SND_SPELL_LIGHTNING_LOOP;
extern ArxSound SND_SPELL_LIGHTNING_END;

extern ArxSound SND_SPELL_FIRE_FIELD_START;
extern ArxSound SND_SPELL_FIRE_FIELD_LOOP;
extern ArxSound SND_SPELL_FIRE_FIELD_END;

extern ArxSound SND_SPELL_MAGICAL_HIT;
extern ArxSound SND_SPELL_MAGICAL_SHIELD;
extern ArxSound SND_SPELL_MASS_INCINERATE;
extern ArxSound SND_SPELL_MASS_PARALYSE;
extern ArxSound SND_SPELL_MM_CREATE;
extern ArxSound SND_SPELL_MM_HIT;
extern ArxSound SND_SPELL_MM_LAUNCH;
extern ArxSound SND_SPELL_MM_LOOP;
extern ArxSound SND_SPELL_NEGATE_MAGIC;
extern ArxSound SND_SPELL_NO_EFFECT;
extern ArxSound SND_SPELL_PARALYSE;
extern ArxSound SND_SPELL_PARALYSE_END;
extern ArxSound SND_SPELL_POISON_PROJECTILE_LAUNCH;
extern ArxSound SND_SPELL_RAISE_DEAD;
extern ArxSound SND_SPELL_REPEL_UNDEAD;
extern ArxSound SND_SPELL_REPEL_UNDEAD_LOOP;
extern ArxSound SND_SPELL_RUNE_OF_GUARDING;
extern ArxSound SND_SPELL_SLOW_DOWN;
extern ArxSound SND_SPELL_SPARK;
extern ArxSound SND_SPELL_SPEED_START;
extern ArxSound SND_SPELL_SPEED_LOOP;
extern ArxSound SND_SPELL_SPEED_END;
extern ArxSound SND_SPELL_SUMMON_CREATURE;
extern ArxSound SND_SPELL_TELEKINESIS_START;
extern ArxSound SND_SPELL_TELEKINESIS_END;
extern ArxSound SND_SPELL_TELEPORT;
extern ArxSound SND_SPELL_TELEPORTED;
extern ArxSound SND_SPELL_VISION_START;
extern ArxSound SND_SPELL_VISION_LOOP;

// inter-material sounds
long ARX_MATERIAL_GetIdByName(const std::string & name);
bool ARX_MATERIAL_GetNameById(long id, char * name);

bool ARX_SOUND_Init();
void ARX_SOUND_Release();

long ARX_SOUND_IsEnabled();

void ARX_SOUND_SetListener(const Vec3f * position, const Vec3f * front, const Vec3f * up);

ArxSound ARX_SOUND_Load(const std::string & name);
void ARX_SOUND_Free(const ArxSound & sample);

long ARX_SOUND_PlaySFX(ArxSound & sample_id, const Vec3f * position = NULL, float pitch = 1.0F, const SoundLoopMode = ARX_SOUND_PLAY_ONCE);
long ARX_SOUND_PlayInterface(ArxSound & sample_id, float pitch = 1.0F, SoundLoopMode loop = ARX_SOUND_PLAY_ONCE);

long ARX_SOUND_PlaySpeech(const std::string & name, const INTERACTIVE_OBJ * io = NULL);
long ARX_SOUND_PlayCollision(long mat1, long mat2, float volume, float power, Vec3f * position, INTERACTIVE_OBJ * source);
long ARX_SOUND_PlayCollision(const std::string& name1, const std::string& name2, float volume, float power, Vec3f* position, INTERACTIVE_OBJ* source);

long ARX_SOUND_PlayScript(const std::string & name, const INTERACTIVE_OBJ * io = NULL, float pitch = 1.0F, SoundLoopMode loop = ARX_SOUND_PLAY_ONCE);
long ARX_SOUND_PlayAnim(ArxSound & sample_id, const Vec3f * position = NULL);
long ARX_SOUND_PlayCinematic(const std::string & name);
long ARX_SOUND_PlayMenu(ArxSound & sample_id, float pitch = 1.0F, SoundLoopMode loop = ARX_SOUND_PLAY_ONCE);
long ARX_SOUND_IsPlaying(ArxSound & sample_id);
float ARX_SOUND_GetDuration(ArxSound & sample_id);

void ARX_SOUND_RefreshVolume(ArxSound & sample_id, float volume);
void ARX_SOUND_RefreshPosition(ArxSound & sample_id, const Vec3f * position = NULL);
void ARX_SOUND_RefreshPitch(ArxSound & sample_id, float pitch);
void ARX_SOUND_RefreshSpeechPosition(ArxSound & sample_id, const INTERACTIVE_OBJ * io = NULL);

void ARX_SOUND_Stop(ArxSound & sample_id);

long ARX_SOUND_PlayScriptAmbiance(const std::string & ambiance_name, SoundLoopMode loop = ARX_SOUND_PLAY_LOOPED, float volume = 1.0F);
long ARX_SOUND_PlayZoneAmbiance(const std::string & ambiance_name, SoundLoopMode loop = ARX_SOUND_PLAY_LOOPED, float volume = 1.0F);
long ARX_SOUND_PlayMenuAmbiance(const std::string & ambiance_name);
long ARX_SOUND_SetAmbianceTrackStatus(const std::string & ambiance_name, const std::string & track_name, unsigned long status); //0 = off; 1 = on
void ARX_SOUND_KillAmbiances();
char * ARX_SOUND_AmbianceSavePlayList(size_t & size);
void ARX_SOUND_AmbianceRestorePlayList(const char * play_list, size_t size);

void ARX_SOUND_MixerSetVolume(ArxMixer mixer_id, float volume);
void ARX_SOUND_MixerStop(ArxMixer mixer_id);
void ARX_SOUND_MixerPause(ArxMixer mixer_id);
void ARX_SOUND_MixerResume(ArxMixer mixer_id);
void ARX_SOUND_MixerSwitch(ArxMixer from, ArxMixer to);

#endif // ARX_SCENE_GAMESOUND_H
