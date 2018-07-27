/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/TimeTypes.h"
#include "audio/AudioTypes.h"
#include "game/GameTypes.h"
#include "game/magic/Rune.h"
#include "math/Types.h"

class Entity;
namespace res { class path; }

enum SoundLoopMode {
	ARX_SOUND_PLAY_LOOPED = 0,
	ARX_SOUND_PLAY_ONCE = 1
};

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

struct StaticSamples {
// Menu samples
audio::SampleHandle MENU_CLICK;
audio::SourcedSample MENU_RELEASE;

// Interface samples
audio::SourcedSample BACKPACK;
audio::SourcedSample BOOK_OPEN;
audio::SourcedSample BOOK_CLOSE;
audio::SourcedSample BOOK_PAGE_TURN;
audio::SourcedSample GOLD;
audio::SourcedSample INVSTD;
audio::SourcedSample SCROLL_OPEN;
audio::SourcedSample SCROLL_CLOSE;
audio::SourcedSample TORCH_START;
audio::SourcedSample TORCH_LOOP;
audio::SourcedSample TORCH_END;

// Other SFX samples
audio::SourcedSample FIREPLACE;
audio::SourcedSample PLOUF;
audio::SourcedSample QUAKE;
audio::SourcedSample WHOOSH;

// Player samples
audio::SourcedSample PLAYER_DEATH_BY_FIRE;
audio::SourcedSample PLAYER_HEART_BEAT;
audio::SourcedSample PLAYER_LEVEL_UP;
audio::SourcedSample PLAYER_POISONED;

// Magic drawing samples
audio::SourcedSample MAGIC_AMBIENT;
audio::SourcedSample MAGIC_DRAW;
audio::SourcedSample MAGIC_FIZZLE;

// Magic symbols samples
audio::SourcedSample SYMB[RUNE_COUNT];

// Spells samples
audio::SourcedSample SPELL_ACTIVATE_PORTAL;
audio::SourcedSample SPELL_ARMOR_START;
audio::SourcedSample SPELL_ARMOR_END;
audio::SourcedSample SPELL_ARMOR_LOOP;
audio::SourcedSample SPELL_BLESS;
audio::SourcedSample SPELL_COLD_PROTECTION_START;
audio::SourcedSample SPELL_COLD_PROTECTION_LOOP;
audio::SourcedSample SPELL_COLD_PROTECTION_END;
audio::SourcedSample SPELL_CONFUSE;
audio::SourcedSample SPELL_CONTROL_TARGET;
audio::SourcedSample SPELL_CREATE_FIELD;
audio::SourcedSample SPELL_CREATE_FOOD;
audio::SourcedSample SPELL_CURE_POISON;
audio::SourcedSample SPELL_CURSE;
audio::SourcedSample SPELL_DETECT_TRAP;
audio::SourcedSample SPELL_DETECT_TRAP_LOOP;
audio::SourcedSample SPELL_DISARM_TRAP;
audio::SourcedSample SPELL_DISPELL_FIELD;
audio::SourcedSample SPELL_DISPELL_ILLUSION;
audio::SourcedSample SPELL_DOUSE;
audio::SourcedSample SPELL_ELECTRIC;
audio::SourcedSample SPELL_EXPLOSION;
audio::SourcedSample SPELL_EYEBALL_IN;
audio::SourcedSample SPELL_EYEBALL_OUT;
audio::SourcedSample SPELL_FIRE_HIT;
audio::SourcedSample SPELL_FIRE_LAUNCH;
audio::SourcedSample SPELL_FIRE_PROTECTION;
audio::SourcedSample SPELL_FIRE_PROTECTION_LOOP;
audio::SourcedSample SPELL_FIRE_PROTECTION_END;
audio::SourcedSample SPELL_FIRE_WIND;
audio::SourcedSample SPELL_FREEZETIME;
audio::SourcedSample SPELL_HARM;
audio::SourcedSample SPELL_HEALING;
audio::SourcedSample SPELL_ICE_FIELD;
audio::SourcedSample SPELL_ICE_FIELD_LOOP;
audio::SourcedSample SPELL_ICE_FIELD_END;
audio::SourcedSample SPELL_ICE_PROJECTILE_LAUNCH;
audio::SourcedSample SPELL_INCINERATE;
audio::SourcedSample SPELL_INCINERATE_LOOP;
audio::SourcedSample SPELL_INCINERATE_END;
audio::SourcedSample SPELL_IGNITE;
audio::SourcedSample SPELL_INVISIBILITY_START;
audio::SourcedSample SPELL_INVISIBILITY_END;
audio::SourcedSample SPELL_LEVITATE_START;
audio::SourcedSample SPELL_LEVITATE_LOOP;
audio::SourcedSample SPELL_LEVITATE_END;
audio::SourcedSample SPELL_LIGHTNING_START;
audio::SourcedSample SPELL_LIGHTNING_LOOP;
audio::SourcedSample SPELL_LIGHTNING_END;
audio::SourcedSample SPELL_LOWER_ARMOR;
audio::SourcedSample SPELL_LOWER_ARMOR_END;
audio::SourcedSample SPELL_FIRE_FIELD_START;
audio::SourcedSample SPELL_FIRE_FIELD_LOOP;
audio::SourcedSample SPELL_FIRE_FIELD_END;

audio::SourcedSample SPELL_MAGICAL_HIT;
audio::SourcedSample SPELL_MAGICAL_SHIELD;
audio::SourcedSample SPELL_MASS_INCINERATE;
audio::SourcedSample SPELL_MASS_PARALYSE;
audio::SourcedSample SPELL_MM_CREATE;
audio::SourcedSample SPELL_MM_HIT;
audio::SourcedSample SPELL_MM_LAUNCH;
audio::SourcedSample SPELL_MM_LOOP;
audio::SourcedSample SPELL_NEGATE_MAGIC;
audio::SourcedSample SPELL_PARALYSE;
audio::SourcedSample SPELL_PARALYSE_END;
audio::SourcedSample SPELL_POISON_PROJECTILE_LAUNCH;
audio::SourcedSample SPELL_RAISE_DEAD;
audio::SourcedSample SPELL_REPEL_UNDEAD;
audio::SourcedSample SPELL_REPEL_UNDEAD_LOOP;
audio::SourcedSample SPELL_RUNE_OF_GUARDING;
audio::SourcedSample SPELL_RUNE_OF_GUARDING_END;
audio::SourcedSample SPELL_SLOW_DOWN;
audio::SourcedSample SPELL_SLOW_DOWN_END;
audio::SourcedSample SPELL_SPARK;
audio::SourcedSample SPELL_SPEED_START;
audio::SourcedSample SPELL_SPEED_LOOP;
audio::SourcedSample SPELL_SPEED_END;
audio::SourcedSample SPELL_SUMMON_CREATURE;
audio::SourcedSample SPELL_TELEKINESIS_START;
audio::SourcedSample SPELL_TELEKINESIS_END;
audio::SourcedSample SPELL_VISION_START;
audio::SourcedSample SPELL_VISION_LOOP;
};
extern StaticSamples g_snd;

// inter-material sounds
const char * ARX_MATERIAL_GetNameById(Material id);

bool ARX_SOUND_Init();
void ARX_SOUND_Release();

void ARX_SOUND_SetReverb(bool enabled);

long ARX_SOUND_IsEnabled();

void ARX_SOUND_SetListener(const Vec3f & position, const Vec3f & front, const Vec3f & up);

audio::SourcedSample ARX_SOUND_Load(const res::path & name);
void ARX_SOUND_Free(const audio::SourcedSample & sample);

audio::SourcedSample ARX_SOUND_PlaySFX(audio::SourcedSample & sample_id, const Vec3f * position = NULL,
                                  float pitch = 1.f, SoundLoopMode loop = ARX_SOUND_PLAY_ONCE);
void ARX_SOUND_PlayInterface(audio::SourcedSample & sample_id, float pitch = 1.f);
void ARX_SOUND_PlayMenu(audio::SampleHandle sample_id);

audio::SourcedSample ARX_SOUND_PlaySpeech(const res::path & name, bool * tooFar = NULL, const Entity * io = NULL);
long ARX_SOUND_PlayCollision(Material mat1, Material mat2, float volume, float power, const Vec3f & position, Entity * source);
long ARX_SOUND_PlayCollision(const std::string & name1, const std::string & name2, float volume, float power, const Vec3f & position, Entity * source);

audio::SourcedSample ARX_SOUND_PlayScript(const res::path & name, bool &tooFar, const Entity * io = NULL, float pitch = 1.0F, SoundLoopMode loop = ARX_SOUND_PLAY_ONCE);
void ARX_SOUND_PlayAnim(audio::SourcedSample & sample_id, const Vec3f * position = NULL);
audio::SourcedSample ARX_SOUND_PlayCinematic(const res::path & name, bool isSpeech);
bool ARX_SOUND_IsPlaying(audio::SourcedSample & sample_id);
GameDuration ARX_SOUND_GetDuration(audio::SampleHandle sample_id);

void ARX_SOUND_RefreshVolume(audio::SourcedSample & sample_id, float volume);
void ARX_SOUND_RefreshPosition(audio::SourcedSample & sample_id, const Vec3f & position);
void ARX_SOUND_RefreshPitch(audio::SourcedSample & sample_id, float pitch);
void ARX_SOUND_RefreshSpeechPosition(audio::SourcedSample & sample_id, const Entity * io = NULL);

void ARX_SOUND_Stop(audio::SourcedSample & sample_id);

bool ARX_SOUND_PlayScriptAmbiance(const res::path & ambiance_name, SoundLoopMode loop = ARX_SOUND_PLAY_LOOPED, float volume = 1.0F);
bool ARX_SOUND_PlayZoneAmbiance(const res::path & ambiance_name, SoundLoopMode loop = ARX_SOUND_PLAY_LOOPED, float volume = 1.0F);
audio::AmbianceId ARX_SOUND_PlayMenuAmbiance(const res::path & ambiance_name);
void ARX_SOUND_KillAmbiances();
std::string ARX_SOUND_AmbianceSavePlayList();
void ARX_SOUND_AmbianceRestorePlayList(const char * playlist, size_t size);

void ARX_SOUND_MixerSetVolume(audio::MixerId mixer_id, float volume);
void ARX_SOUND_MixerStop(audio::MixerId mixer_id);
void ARX_SOUND_MixerPause(audio::MixerId mixer_id);
void ARX_SOUND_MixerResume(audio::MixerId mixer_id);
void ARX_SOUND_MixerSwitch(audio::MixerId from, audio::MixerId to);

#endif // ARX_SCENE_GAMESOUND_H
