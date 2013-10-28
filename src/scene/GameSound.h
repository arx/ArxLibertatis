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

#ifndef ARX_SCENE_GAMESOUND_H
#define ARX_SCENE_GAMESOUND_H

#include <string>

#include "audio/AudioTypes.h"
#include "math/Types.h"

class Entity;
namespace res { class path; }

enum SoundLoopMode {
	ARX_SOUND_PLAY_LOOPED = 0,
	ARX_SOUND_PLAY_ONCE = 1
};

const audio::SampleId ARX_SOUND_TOO_FAR = -2;

extern audio::MixerId ARX_SOUND_MixerGame;
extern audio::MixerId ARX_SOUND_MixerGameSample;
extern audio::MixerId ARX_SOUND_MixerGameSpeech;
extern audio::MixerId ARX_SOUND_MixerGameAmbiance;
extern audio::MixerId ARX_SOUND_MixerMenu;
extern audio::MixerId ARX_SOUND_MixerMenuSample;
extern audio::MixerId ARX_SOUND_MixerMenuSpeech;
extern audio::MixerId ARX_SOUND_MixerMenuAmbiance;

// Menu ambiances
const std::string AMB_MENU = "ambient_menu.amb";
const std::string AMB_CREDITS = "ambient_credits.amb";

// Menu samples
extern audio::SampleId SND_MENU_CLICK;
extern audio::SampleId SND_MENU_RELEASE;

// Interface samples
extern audio::SampleId SND_BACKPACK;
extern audio::SampleId SND_BOOK_OPEN;
extern audio::SampleId SND_BOOK_CLOSE;
extern audio::SampleId SND_BOOK_PAGE_TURN;
extern audio::SampleId SND_GOLD;
extern audio::SampleId SND_INVSTD;
extern audio::SampleId SND_SCROLL_OPEN;
extern audio::SampleId SND_SCROLL_CLOSE;
extern audio::SampleId SND_TORCH_START;
extern audio::SampleId SND_TORCH_LOOP;
extern audio::SampleId SND_TORCH_END;

// Other SFX samples
extern audio::SampleId SND_FIREPLACE;
extern audio::SampleId SND_PLOUF;
extern audio::SampleId SND_QUAKE;
extern audio::SampleId SND_WHOOSH;

// Player samples
extern audio::SampleId SND_PLAYER_DEATH_BY_FIRE;
extern audio::SampleId SND_PLAYER_FILLLIFEMANA;
extern audio::SampleId SND_PLAYER_HEART_BEAT;
extern audio::SampleId SND_PLAYER_LEVEL_UP;
extern audio::SampleId SND_PLAYER_POISONED;

// Magic drawing samples
extern audio::SampleId SND_MAGIC_AMBIENT;
extern audio::SampleId SND_MAGIC_DRAW;
extern audio::SampleId SND_MAGIC_FIZZLE;

// Magic symbols samples
extern audio::SampleId SND_SYMB_AAM;
extern audio::SampleId SND_SYMB_CETRIUS;
extern audio::SampleId SND_SYMB_COSUM;
extern audio::SampleId SND_SYMB_COMUNICATUM;
extern audio::SampleId SND_SYMB_FOLGORA;
extern audio::SampleId SND_SYMB_FRIDD;
extern audio::SampleId SND_SYMB_KAOM;
extern audio::SampleId SND_SYMB_MEGA;
extern audio::SampleId SND_SYMB_MORTE;
extern audio::SampleId SND_SYMB_MOVIS;
extern audio::SampleId SND_SYMB_NHI;
extern audio::SampleId SND_SYMB_RHAA;
extern audio::SampleId SND_SYMB_SPACIUM;
extern audio::SampleId SND_SYMB_STREGUM;
extern audio::SampleId SND_SYMB_TAAR;
extern audio::SampleId SND_SYMB_TEMPUS;
extern audio::SampleId SND_SYMB_TERA;
extern audio::SampleId SND_SYMB_VISTA;
extern audio::SampleId SND_SYMB_VITAE;
extern audio::SampleId SND_SYMB_YOK;

// Spells samples
extern audio::SampleId SND_SPELL_ACTIVATE_PORTAL;
extern audio::SampleId SND_SPELL_ARMOR_START;
extern audio::SampleId SND_SPELL_ARMOR_END;
extern audio::SampleId SND_SPELL_ARMOR_LOOP;
extern audio::SampleId SND_SPELL_BLESS;
extern audio::SampleId SND_SPELL_COLD_PROTECTION_START;
extern audio::SampleId SND_SPELL_COLD_PROTECTION_LOOP;
extern audio::SampleId SND_SPELL_COLD_PROTECTION_END;
extern audio::SampleId SND_SPELL_CONFUSE;
extern audio::SampleId SND_SPELL_CONTROL_TARGET;
extern audio::SampleId SND_SPELL_CREATE_FIELD;
extern audio::SampleId SND_SPELL_CREATE_FOOD;
extern audio::SampleId SND_SPELL_CURE_POISON;
extern audio::SampleId SND_SPELL_CURSE;
extern audio::SampleId SND_SPELL_DETECT_TRAP;
extern audio::SampleId SND_SPELL_DETECT_TRAP_LOOP;
extern audio::SampleId SND_SPELL_DISARM_TRAP;
extern audio::SampleId SND_SPELL_DISPELL_FIELD;
extern audio::SampleId SND_SPELL_DISPELL_ILLUSION;
extern audio::SampleId SND_SPELL_DOUSE;
extern audio::SampleId SND_SPELL_ELECTRIC;
extern audio::SampleId SND_SPELL_ENCHANT_WEAPON;
extern audio::SampleId SND_SPELL_EXPLOSION;
extern audio::SampleId SND_SPELL_EYEBALL_IN;
extern audio::SampleId SND_SPELL_EYEBALL_OUT;
extern audio::SampleId SND_SPELL_FIRE_FIELD;
extern audio::SampleId SND_SPELL_FIRE_HIT;
extern audio::SampleId SND_SPELL_FIRE_LAUNCH;
extern audio::SampleId SND_SPELL_FIRE_PROTECTION;
extern audio::SampleId SND_SPELL_FIRE_PROTECTION_LOOP;
extern audio::SampleId SND_SPELL_FIRE_PROTECTION_END;
extern audio::SampleId SND_SPELL_FIRE_WIND;
extern audio::SampleId SND_SPELL_FREEZETIME;
extern audio::SampleId SND_SPELL_HARM;
extern audio::SampleId SND_SPELL_HEALING;
extern audio::SampleId SND_SPELL_ICE_FIELD;
extern audio::SampleId SND_SPELL_ICE_FIELD_LOOP;
extern audio::SampleId SND_SPELL_ICE_FIELD_END;
extern audio::SampleId SND_SPELL_ICE_PROJECTILE_LAUNCH;
extern audio::SampleId SND_SPELL_INCINERATE;
extern audio::SampleId SND_SPELL_INCINERATE_LOOP;
extern audio::SampleId SND_SPELL_INCINERATE_END;
extern audio::SampleId SND_SPELL_IGNITE;
extern audio::SampleId SND_SPELL_INVISIBILITY_START;
extern audio::SampleId SND_SPELL_INVISIBILITY_END;
extern audio::SampleId SND_SPELL_LEVITATE_START;
extern audio::SampleId SND_SPELL_LEVITATE_LOOP;
extern audio::SampleId SND_SPELL_LEVITATE_END;
extern audio::SampleId SND_SPELL_LIGHTNING;
extern audio::SampleId SND_SPELL_LIGHTNING_START;
extern audio::SampleId SND_SPELL_LIGHTNING_LOOP;
extern audio::SampleId SND_SPELL_LIGHTNING_END;
extern audio::SampleId SND_SPELL_LOWER_ARMOR;
extern audio::SampleId SND_SPELL_LOWER_ARMOR_END;
extern audio::SampleId SND_SPELL_FIRE_FIELD_START;
extern audio::SampleId SND_SPELL_FIRE_FIELD_LOOP;
extern audio::SampleId SND_SPELL_FIRE_FIELD_END;

extern audio::SampleId SND_SPELL_MAGICAL_HIT;
extern audio::SampleId SND_SPELL_MAGICAL_SHIELD;
extern audio::SampleId SND_SPELL_MASS_INCINERATE;
extern audio::SampleId SND_SPELL_MASS_PARALYSE;
extern audio::SampleId SND_SPELL_MM_CREATE;
extern audio::SampleId SND_SPELL_MM_HIT;
extern audio::SampleId SND_SPELL_MM_LAUNCH;
extern audio::SampleId SND_SPELL_MM_LOOP;
extern audio::SampleId SND_SPELL_NEGATE_MAGIC;
extern audio::SampleId SND_SPELL_NO_EFFECT;
extern audio::SampleId SND_SPELL_PARALYSE;
extern audio::SampleId SND_SPELL_PARALYSE_END;
extern audio::SampleId SND_SPELL_POISON_PROJECTILE_LAUNCH;
extern audio::SampleId SND_SPELL_RAISE_DEAD;
extern audio::SampleId SND_SPELL_REPEL_UNDEAD;
extern audio::SampleId SND_SPELL_REPEL_UNDEAD_LOOP;
extern audio::SampleId SND_SPELL_RUNE_OF_GUARDING;
extern audio::SampleId SND_SPELL_RUNE_OF_GUARDING_END;
extern audio::SampleId SND_SPELL_SLOW_DOWN;
extern audio::SampleId SND_SPELL_SLOW_DOWN_END;
extern audio::SampleId SND_SPELL_SPARK;
extern audio::SampleId SND_SPELL_SPEED_START;
extern audio::SampleId SND_SPELL_SPEED_LOOP;
extern audio::SampleId SND_SPELL_SPEED_END;
extern audio::SampleId SND_SPELL_SUMMON_CREATURE;
extern audio::SampleId SND_SPELL_TELEKINESIS_START;
extern audio::SampleId SND_SPELL_TELEKINESIS_END;
extern audio::SampleId SND_SPELL_TELEPORT;
extern audio::SampleId SND_SPELL_TELEPORTED;
extern audio::SampleId SND_SPELL_VISION_START;
extern audio::SampleId SND_SPELL_VISION_LOOP;

// inter-material sounds
bool ARX_MATERIAL_GetNameById(long id, char * name);

bool ARX_SOUND_Init();
void ARX_SOUND_LoadData();
void ARX_SOUND_Release();

long ARX_SOUND_IsEnabled();

void ARX_SOUND_SetListener(const Vec3f * position, const Vec3f * front, const Vec3f * up);

audio::SampleId ARX_SOUND_Load(const res::path & name);
void ARX_SOUND_Free(const audio::SampleId & sample);

long ARX_SOUND_PlaySFX(audio::SourceId & sample_id, const Vec3f * position = NULL, float pitch = 1.0F, const SoundLoopMode = ARX_SOUND_PLAY_ONCE);
long ARX_SOUND_PlayInterface(audio::SourceId & sample_id, float pitch = 1.0F, SoundLoopMode loop = ARX_SOUND_PLAY_ONCE);

long ARX_SOUND_PlaySpeech(const res::path & name, const Entity * io = NULL);
long ARX_SOUND_PlayCollision(long mat1, long mat2, float volume, float power, Vec3f * position, Entity * source);
long ARX_SOUND_PlayCollision(const std::string& name1, const std::string& name2, float volume, float power, Vec3f* position, Entity* source);

long ARX_SOUND_PlayScript(const res::path & name, const Entity * io = NULL, float pitch = 1.0F, SoundLoopMode loop = ARX_SOUND_PLAY_ONCE);
long ARX_SOUND_PlayAnim(audio::SourceId & sample_id, const Vec3f * position = NULL);
long ARX_SOUND_PlayCinematic(const res::path & name, bool isSpeech);
long ARX_SOUND_PlayMenu(audio::SourceId & sample_id, float pitch = 1.0F, SoundLoopMode loop = ARX_SOUND_PLAY_ONCE);
long ARX_SOUND_IsPlaying(audio::SourceId & sample_id);
float ARX_SOUND_GetDuration(audio::SampleId & sample_id);

void ARX_SOUND_RefreshVolume(audio::SourceId & sample_id, float volume);
void ARX_SOUND_RefreshPosition(audio::SourceId & sample_id, const Vec3f * position = NULL);
void ARX_SOUND_RefreshPitch(audio::SourceId & sample_id, float pitch);
void ARX_SOUND_RefreshSpeechPosition(audio::SourceId & sample_id, const Entity * io = NULL);

void ARX_SOUND_Stop(audio::SourceId & sample_id);

bool ARX_SOUND_PlayScriptAmbiance(const res::path & ambiance_name, SoundLoopMode loop = ARX_SOUND_PLAY_LOOPED, float volume = 1.0F);
bool ARX_SOUND_PlayZoneAmbiance(const res::path & ambiance_name, SoundLoopMode loop = ARX_SOUND_PLAY_LOOPED, float volume = 1.0F);
audio::AmbianceId ARX_SOUND_PlayMenuAmbiance(const res::path & ambiance_name);
audio::AmbianceId ARX_SOUND_SetAmbianceTrackStatus(const res::path & ambiance_name, const std::string & track_name, unsigned long status); //0 = off; 1 = on TODO this is wrong?
void ARX_SOUND_KillAmbiances();
char * ARX_SOUND_AmbianceSavePlayList(size_t & size);
void ARX_SOUND_AmbianceRestorePlayList(const char * play_list, size_t size);

void ARX_SOUND_MixerSetVolume(audio::MixerId mixer_id, float volume);
void ARX_SOUND_MixerStop(audio::MixerId mixer_id);
void ARX_SOUND_MixerPause(audio::MixerId mixer_id);
void ARX_SOUND_MixerResume(audio::MixerId mixer_id);
void ARX_SOUND_MixerSwitch(audio::MixerId from, audio::MixerId to);

#endif // ARX_SCENE_GAMESOUND_H
