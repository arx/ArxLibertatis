/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
audio::SampleHandle MENU_RELEASE;

// Interface samples
audio::SampleHandle BACKPACK;
audio::SampleHandle BOOK_OPEN;
audio::SampleHandle BOOK_CLOSE;
audio::SampleHandle BOOK_PAGE_TURN;
audio::SampleHandle GOLD;
audio::SampleHandle INVSTD;
audio::SampleHandle SCROLL_OPEN;
audio::SampleHandle SCROLL_CLOSE;

// Player samples
audio::SampleHandle PLAYER_DEATH_BY_FIRE;
audio::SampleHandle PLAYER_HEART_BEAT;
audio::SampleHandle PLAYER_LEVEL_UP;
audio::SampleHandle PLAYER_POISONED;

// Other SFX samples
audio::SampleHandle TORCH_START;
audio::SampleHandle TORCH_LOOP;
audio::SampleHandle TORCH_END;
audio::SampleHandle FIREPLACE_LOOP;
audio::SampleHandle PLOUF;
audio::SampleHandle WHOOSH;
audio::SampleHandle DISMEMBER;

// Magic drawing samples
audio::SampleHandle MAGIC_AMBIENT_LOOP;
audio::SampleHandle MAGIC_DRAW_LOOP;
audio::SampleHandle MAGIC_FIZZLE;

// Magic symbols samples
audio::SampleHandle SYMB[RUNE_COUNT];

// Spells samples
audio::SampleHandle SPELL_ACTIVATE_PORTAL;
audio::SampleHandle SPELL_ARMOR_START;
audio::SampleHandle SPELL_ARMOR_END;
audio::SampleHandle SPELL_ARMOR_LOOP;
audio::SampleHandle SPELL_BLESS;
audio::SampleHandle SPELL_COLD_PROTECTION_START;
audio::SampleHandle SPELL_COLD_PROTECTION_LOOP;
audio::SampleHandle SPELL_COLD_PROTECTION_END;
audio::SampleHandle SPELL_CONFUSE;
audio::SampleHandle SPELL_CONTROL_TARGET;
audio::SampleHandle SPELL_CREATE_FIELD;
audio::SampleHandle SPELL_CREATE_FOOD;
audio::SampleHandle SPELL_CURE_POISON;
audio::SampleHandle SPELL_CURSE;
audio::SampleHandle SPELL_DETECT_TRAP;
audio::SampleHandle SPELL_DETECT_TRAP_LOOP;
audio::SampleHandle SPELL_DISARM_TRAP;
audio::SampleHandle SPELL_DISPELL_FIELD;
audio::SampleHandle SPELL_DISPELL_ILLUSION;
audio::SampleHandle SPELL_DOUSE;
audio::SampleHandle SPELL_ELECTRIC;
audio::SampleHandle SPELL_EXPLOSION;
audio::SampleHandle SPELL_EYEBALL_IN;
audio::SampleHandle SPELL_EYEBALL_OUT;
audio::SampleHandle SPELL_FIRE_HIT;
audio::SampleHandle SPELL_FIRE_LAUNCH;
audio::SampleHandle SPELL_FIRE_PROTECTION;
audio::SampleHandle SPELL_FIRE_PROTECTION_LOOP;
audio::SampleHandle SPELL_FIRE_PROTECTION_END;
audio::SampleHandle SPELL_FIRE_WIND_LOOP;
audio::SampleHandle SPELL_FREEZETIME;
audio::SampleHandle SPELL_HARM;
audio::SampleHandle SPELL_HEALING;
audio::SampleHandle SPELL_ICE_FIELD;
audio::SampleHandle SPELL_ICE_FIELD_LOOP;
audio::SampleHandle SPELL_ICE_FIELD_END;
audio::SampleHandle SPELL_ICE_PROJECTILE_LAUNCH;
audio::SampleHandle SPELL_INCINERATE;
audio::SampleHandle SPELL_INCINERATE_LOOP;
audio::SampleHandle SPELL_INCINERATE_END;
audio::SampleHandle SPELL_IGNITE;
audio::SampleHandle SPELL_INVISIBILITY_START;
audio::SampleHandle SPELL_INVISIBILITY_END;
audio::SampleHandle SPELL_LEVITATE_START;
audio::SampleHandle SPELL_LEVITATE_LOOP;
audio::SampleHandle SPELL_LEVITATE_END;
audio::SampleHandle SPELL_LIGHTNING_START;
audio::SampleHandle SPELL_LIGHTNING_LOOP;
audio::SampleHandle SPELL_LIGHTNING_END;
audio::SampleHandle SPELL_LOWER_ARMOR;
audio::SampleHandle SPELL_LOWER_ARMOR_END;
audio::SampleHandle SPELL_FIRE_FIELD_START;
audio::SampleHandle SPELL_FIRE_FIELD_LOOP;
audio::SampleHandle SPELL_FIRE_FIELD_END;

audio::SampleHandle SPELL_MAGICAL_HIT;
audio::SampleHandle SPELL_MAGICAL_SHIELD_LOOP;
audio::SampleHandle SPELL_MASS_INCINERATE;
audio::SampleHandle SPELL_MASS_PARALYSE;
audio::SampleHandle SPELL_MM_CREATE;
audio::SampleHandle SPELL_MM_HIT;
audio::SampleHandle SPELL_MM_LAUNCH;
audio::SampleHandle SPELL_MM_LOOP;
audio::SampleHandle SPELL_NEGATE_MAGIC;
audio::SampleHandle SPELL_PARALYSE;
audio::SampleHandle SPELL_PARALYSE_END;
audio::SampleHandle SPELL_POISON_PROJECTILE_LAUNCH;
audio::SampleHandle SPELL_RAISE_DEAD;
audio::SampleHandle SPELL_REPEL_UNDEAD;
audio::SampleHandle SPELL_REPEL_UNDEAD_LOOP;
audio::SampleHandle SPELL_RUNE_OF_GUARDING;
audio::SampleHandle SPELL_RUNE_OF_GUARDING_END;
audio::SampleHandle SPELL_SLOW_DOWN;
audio::SampleHandle SPELL_SLOW_DOWN_END;
audio::SampleHandle SPELL_SPARK;
audio::SampleHandle SPELL_SPEED_START;
audio::SampleHandle SPELL_SPEED_LOOP;
audio::SampleHandle SPELL_SPEED_END;
audio::SampleHandle SPELL_SUMMON_CREATURE;
audio::SampleHandle SPELL_TELEKINESIS_START;
audio::SampleHandle SPELL_TELEKINESIS_END;
audio::SampleHandle SPELL_VISION_START;
audio::SampleHandle SPELL_VISION_LOOP;
};
extern StaticSamples g_snd;

// inter-material sounds
const char * ARX_MATERIAL_GetNameById(Material id);

bool ARX_SOUND_Init();
void ARX_SOUND_Release();

void ARX_SOUND_SetReverb(bool enabled);

long ARX_SOUND_IsEnabled();

void ARX_SOUND_SetListener(const Vec3f & position, const Vec3f & front, const Vec3f & up);

audio::SampleHandle ARX_SOUND_Load(const res::path & name);
void ARX_SOUND_Free(const audio::SampleHandle & sample);

void ARX_SOUND_PlaySFX(audio::SampleHandle sample_id, const Vec3f * position = NULL, float pitch = 1.f);

audio::SourcedSample ARX_SOUND_PlaySFX_loop(audio::SampleHandle sample_id, const Vec3f * position = NULL,
                                  float pitch = 1.f);

void ARX_SOUND_PlayInterface(audio::SampleHandle sample_id, float pitch = 1.f);
void ARX_SOUND_PlayMenu(audio::SampleHandle sample_id);

audio::SourcedSample ARX_SOUND_PlaySpeech(const res::path & name, bool * tooFar = NULL, const Entity * io = NULL);
void ARX_SOUND_PlayCollision(Material mat1, Material mat2, float volume, float power, const Vec3f & position, Entity * source);
void ARX_SOUND_PlayCollision(const std::string & name1, const std::string & name2, float volume, float power, const Vec3f & position, Entity * source);

audio::SourcedSample ARX_SOUND_PlayScript(const res::path & name, bool & tooFar, const Entity * io = NULL,
                                          float pitch = 1.f, SoundLoopMode loop = ARX_SOUND_PLAY_ONCE);
void ARX_SOUND_PlayAnim(audio::SampleHandle sample_id, const Vec3f * position = NULL);
audio::SourcedSample ARX_SOUND_PlayCinematic(const res::path & name, bool isSpeech);
bool ARX_SOUND_IsPlaying(audio::SourcedSample & sample_id);
GameDuration ARX_SOUND_GetDuration(audio::SampleHandle sample_id);

void ARX_SOUND_RefreshVolume(audio::SourcedSample & sample_id, float volume);
void ARX_SOUND_RefreshPosition(audio::SourcedSample & sample_id, const Vec3f & position);
void ARX_SOUND_RefreshPitch(audio::SourcedSample & sample_id, float pitch);
void ARX_SOUND_RefreshSpeechPosition(audio::SourcedSample & sample_id, const Entity * io = NULL);

void ARX_SOUND_Stop(const audio::SourcedSample & sample_id);

bool ARX_SOUND_PlayScriptAmbiance(const res::path & ambiance_name, SoundLoopMode loop = ARX_SOUND_PLAY_LOOPED,
                                  float volume = 1.f);
bool ARX_SOUND_PlayZoneAmbiance(const res::path & ambiance_name, SoundLoopMode loop = ARX_SOUND_PLAY_LOOPED,
                                float volume = 1.f);
audio::AmbianceId ARX_SOUND_PlayMenuAmbiance(const res::path & ambiance_name);
void ARX_SOUND_KillAmbiances();
std::string ARX_SOUND_AmbianceSavePlayList();
void ARX_SOUND_AmbianceRestorePlayList(const char * playlist, size_t size);

void ARX_SOUND_MixerSetVolume(audio::MixerId mixer_id, float volume);
void ARX_SOUND_MixerStop(audio::MixerId mixer_id);
void ARX_SOUND_MixerPause(audio::MixerId mixer_id);
void ARX_SOUND_MixerResume(audio::MixerId mixer_id);

#endif // ARX_SCENE_GAMESOUND_H
