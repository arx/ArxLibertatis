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

#include "scene/GameSound.h"

#include <map>
#include <vector>
#include <sstream>
#include <cstdio>

#include <boost/algorithm/string/case_conv.hpp>

#include "animation/Animation.h"

#include "audio/Audio.h"

#include "core/Config.h"

#include "game/EntityManager.h"
#include "game/Inventory.h"
#include "game/NPC.h"
#include "game/Player.h"

#include "graphics/Math.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/IniReader.h"
#include "io/log/Logger.h"

#include "platform/Platform.h"
#include "platform/Thread.h"

#include "scene/Interactive.h"

#include "util/String.h"

using std::map;
using std::string;
using std::istringstream;
using std::ostringstream;
using std::vector;
using std::sprintf;

using audio::AmbianceId;
using audio::SampleId;
using audio::SourceId;
using audio::EnvId;
using audio::INVALID_ID;
using audio::FLAG_VOLUME;
using audio::FLAG_POSITION;
using audio::FLAG_REVERBERATION;
using audio::FLAG_FALLOFF;
using audio::FLAG_PITCH;
using audio::FLAG_RELATIVE;
using audio::FLAG_AUTOFREE;

extern bool EXTERNALVIEW;
extern Entity * CAMERACONTROLLER;


enum PlayingAmbianceType {
	PLAYING_AMBIANCE_MENU,
	PLAYING_AMBIANCE_SCRIPT,
	PLAYING_AMBIANCE_ZONE
};

// TODO used for saving
struct PlayingAmbiance {
	char name[256];
	f32 volume;
	s32 loop;
	s32 type;
};

static const unsigned long ARX_SOUND_UPDATE_INTERVAL(100);  
static const unsigned long ARX_SOUND_STREAMING_LIMIT(176400); 
static const unsigned long MAX_MATERIALS(17);
static const unsigned long MAX_VARIANTS(5);
static const unsigned long AMBIANCE_FADE_TIME(2000);
static const float ARX_SOUND_UNIT_FACTOR(0.01F);
static const float ARX_SOUND_ROLLOFF_FACTOR(1.3F);
static const float ARX_SOUND_DEFAULT_FALLSTART(200.0F);

static const float ARX_SOUND_DEFAULT_FALLEND(2200.0F);
static const float ARX_SOUND_REFUSE_DISTANCE(2500.0F);

static const res::path ARX_SOUND_PATH_INI = "localisation";
static const char ARX_SOUND_PATH_SAMPLE[] = "sfx";
static const char ARX_SOUND_PATH_AMBIANCE[] = "sfx/ambiance";
static const char ARX_SOUND_PATH_ENVIRONMENT[] = "sfx/environment";
static const res::path ARX_SOUND_PRESENCE_NAME = "presence";
static const string ARX_SOUND_FILE_EXTENSION_WAV = ".wav";
static const string ARX_SOUND_FILE_EXTENSION_INI = ".ini";

static const unsigned long ARX_SOUND_COLLISION_MAP_COUNT = 3;
static const res::path ARX_SOUND_COLLISION_MAP_NAMES[] = {
	"snd_armor",
	"snd_step",
	"snd_weapon"
};

static bool bIsActive(false);


static AmbianceId ambiance_zone(INVALID_ID);
static AmbianceId ambiance_menu = INVALID_ID;

static long Inter_Materials[MAX_MATERIALS][MAX_MATERIALS][MAX_VARIANTS];

namespace {

struct SoundMaterial {
	
	vector<SampleId> variants;
	
	SoundMaterial() : current(0) { }
	
	~SoundMaterial() {
		for(vector<SampleId>::const_iterator i = variants.begin(); i !=  variants.end(); ++i) {
			audio::deleteSample(*i);
		}
	}
	
	SampleId next() {
		arx_assert(current < variants.size());
		SampleId sample = variants[current];
		current = (current + 1) % variants.size();
		return sample;
	}
	
private:
	
	size_t current;
	
};

typedef map<string, SoundMaterial> CollisionMap;
typedef map<string, CollisionMap> CollisionMaps;
static CollisionMaps collisionMaps;

namespace Section {
static const string presence = "presence";
}

typedef map<res::path, float> PresenceFactors;
static PresenceFactors presence;

}

 

SampleId ARX_SOUND_MixerGame(INVALID_ID);
SampleId ARX_SOUND_MixerGameSample(INVALID_ID);
SampleId ARX_SOUND_MixerGameSpeech(INVALID_ID);
SampleId ARX_SOUND_MixerGameAmbiance(INVALID_ID);
SampleId ARX_SOUND_MixerMenu(INVALID_ID);
SampleId ARX_SOUND_MixerMenuSample(INVALID_ID);
SampleId ARX_SOUND_MixerMenuSpeech(INVALID_ID);
SampleId ARX_SOUND_MixerMenuAmbiance(INVALID_ID);

// Menu samples
SampleId SND_MENU_CLICK(INVALID_ID);
SampleId SND_MENU_RELEASE(INVALID_ID);

// Interface samples
SampleId SND_BACKPACK(INVALID_ID);
SampleId SND_BOOK_OPEN(INVALID_ID);
SampleId SND_BOOK_CLOSE(INVALID_ID);
SampleId SND_BOOK_PAGE_TURN(INVALID_ID);
SampleId SND_GOLD(INVALID_ID);
SampleId SND_INVSTD(INVALID_ID);
SampleId SND_SCROLL_OPEN(INVALID_ID);
SampleId SND_SCROLL_CLOSE(INVALID_ID);
SampleId SND_TORCH_START(INVALID_ID);
SampleId SND_TORCH_LOOP(INVALID_ID);
SampleId SND_TORCH_END(INVALID_ID);

// Other SFX samples
SampleId SND_FIREPLACE(INVALID_ID);
SampleId SND_PLOUF(INVALID_ID);
SampleId SND_QUAKE(INVALID_ID);
SampleId SND_WHOOSH(INVALID_ID);

// Player samples
SampleId SND_PLAYER_DEATH_BY_FIRE(INVALID_ID);

SampleId SND_PLAYER_FILLLIFEMANA(INVALID_ID);
SampleId SND_PLAYER_HEART_BEAT(INVALID_ID);
SampleId SND_PLAYER_LEVEL_UP(INVALID_ID);
SampleId SND_PLAYER_POISONED(INVALID_ID);

// Magic drawing samples
SampleId SND_MAGIC_AMBIENT(INVALID_ID);
SampleId SND_MAGIC_DRAW(INVALID_ID);
SampleId SND_MAGIC_FIZZLE(INVALID_ID);

// Magic symbols samples
SampleId SND_SYMB_AAM(INVALID_ID);
SampleId SND_SYMB_CETRIUS(INVALID_ID);
SampleId SND_SYMB_COSUM(INVALID_ID);
SampleId SND_SYMB_COMUNICATUM(INVALID_ID);
SampleId SND_SYMB_FOLGORA(INVALID_ID);
SampleId SND_SYMB_FRIDD(INVALID_ID);
SampleId SND_SYMB_KAOM(INVALID_ID);
SampleId SND_SYMB_MEGA(INVALID_ID);
SampleId SND_SYMB_MORTE(INVALID_ID);
SampleId SND_SYMB_MOVIS(INVALID_ID);
SampleId SND_SYMB_NHI(INVALID_ID);
SampleId SND_SYMB_RHAA(INVALID_ID);
SampleId SND_SYMB_SPACIUM(INVALID_ID);
SampleId SND_SYMB_STREGUM(INVALID_ID);
SampleId SND_SYMB_TAAR(INVALID_ID);
SampleId SND_SYMB_TEMPUS(INVALID_ID);
SampleId SND_SYMB_TERA(INVALID_ID);
SampleId SND_SYMB_VISTA(INVALID_ID);
SampleId SND_SYMB_VITAE(INVALID_ID);
SampleId SND_SYMB_YOK(INVALID_ID);

// Spells samples
SampleId SND_SPELL_ACTIVATE_PORTAL(INVALID_ID);
SampleId SND_SPELL_ARMOR_START(INVALID_ID);
SampleId SND_SPELL_ARMOR_END(INVALID_ID);
SampleId SND_SPELL_ARMOR_LOOP(INVALID_ID);
SampleId SND_SPELL_LOWER_ARMOR(INVALID_ID);
SampleId SND_SPELL_LOWER_ARMOR_END(INVALID_ID);
SampleId SND_SPELL_BLESS(INVALID_ID);
SampleId SND_SPELL_COLD_PROTECTION_START(INVALID_ID);
SampleId SND_SPELL_COLD_PROTECTION_LOOP(INVALID_ID);
SampleId SND_SPELL_COLD_PROTECTION_END(INVALID_ID);
SampleId SND_SPELL_CONFUSE(INVALID_ID);
SampleId SND_SPELL_CONTROL_TARGET(INVALID_ID);
SampleId SND_SPELL_CREATE_FIELD(INVALID_ID);
SampleId SND_SPELL_CREATE_FOOD(INVALID_ID);
SampleId SND_SPELL_CURE_POISON(INVALID_ID);
SampleId SND_SPELL_CURSE(INVALID_ID);
SampleId SND_SPELL_DETECT_TRAP(INVALID_ID);
SampleId SND_SPELL_DETECT_TRAP_LOOP(INVALID_ID);
SampleId SND_SPELL_DISARM_TRAP(INVALID_ID);
SampleId SND_SPELL_DISPELL_FIELD(INVALID_ID);
SampleId SND_SPELL_DISPELL_ILLUSION(INVALID_ID);
SampleId SND_SPELL_DOUSE(INVALID_ID);
SampleId SND_SPELL_ELECTRIC(INVALID_ID);
SampleId SND_SPELL_ENCHANT_WEAPON(INVALID_ID);
SampleId SND_SPELL_EXPLOSION(INVALID_ID);
SampleId SND_SPELL_EYEBALL_IN(INVALID_ID);
SampleId SND_SPELL_EYEBALL_OUT(INVALID_ID);
SampleId SND_SPELL_FIRE_FIELD(INVALID_ID);
SampleId SND_SPELL_FIRE_HIT(INVALID_ID);
SampleId SND_SPELL_FIRE_LAUNCH(INVALID_ID);
SampleId SND_SPELL_FIRE_PROTECTION(INVALID_ID);
SampleId SND_SPELL_FIRE_PROTECTION_LOOP(INVALID_ID);
SampleId SND_SPELL_FIRE_PROTECTION_END(INVALID_ID);
SampleId SND_SPELL_FIRE_WIND(INVALID_ID);
SampleId SND_SPELL_FREEZETIME(INVALID_ID);
SampleId SND_SPELL_HARM(INVALID_ID);
SampleId SND_SPELL_HEALING(INVALID_ID);
SampleId SND_SPELL_ICE_FIELD(INVALID_ID);
SampleId SND_SPELL_ICE_FIELD_LOOP(INVALID_ID);
SampleId SND_SPELL_ICE_FIELD_END(INVALID_ID);
SampleId SND_SPELL_ICE_PROJECTILE_LAUNCH(INVALID_ID);
SampleId SND_SPELL_INCINERATE(INVALID_ID);
SampleId SND_SPELL_INCINERATE_LOOP(INVALID_ID);
SampleId SND_SPELL_INCINERATE_END(INVALID_ID);
SampleId SND_SPELL_IGNITE(INVALID_ID);
SampleId SND_SPELL_INVISIBILITY_START(INVALID_ID);
SampleId SND_SPELL_INVISIBILITY_END(INVALID_ID);
SampleId SND_SPELL_LEVITATE_START(INVALID_ID);
SampleId SND_SPELL_LEVITATE_LOOP(INVALID_ID);
SampleId SND_SPELL_LEVITATE_END(INVALID_ID);
SampleId SND_SPELL_LIGHTNING_START(INVALID_ID);
SampleId SND_SPELL_LIGHTNING_LOOP(INVALID_ID);
SampleId SND_SPELL_LIGHTNING_END(INVALID_ID);
SampleId SND_SPELL_MAGICAL_HIT(INVALID_ID);

//SampleId SND_SPELL_MASS_LIGHTNING_END(INVALID_ID);
SampleId SND_SPELL_FIRE_FIELD_START(INVALID_ID);
SampleId SND_SPELL_FIRE_FIELD_LOOP(INVALID_ID);
SampleId SND_SPELL_FIRE_FIELD_END(INVALID_ID);


SampleId SND_SPELL_MAGICAL_SHIELD(INVALID_ID);
SampleId SND_SPELL_MASS_INCINERATE(INVALID_ID);
SampleId SND_SPELL_MASS_PARALYSE(INVALID_ID);
SampleId SND_SPELL_MM_CREATE(INVALID_ID);
SampleId SND_SPELL_MM_HIT(INVALID_ID);
SampleId SND_SPELL_MM_LAUNCH(INVALID_ID);
SampleId SND_SPELL_MM_LOOP(INVALID_ID);
SampleId SND_SPELL_NEGATE_MAGIC(INVALID_ID);
SampleId SND_SPELL_NO_EFFECT(INVALID_ID);
SampleId SND_SPELL_PARALYSE(INVALID_ID);
SampleId SND_SPELL_PARALYSE_END(INVALID_ID);
SampleId SND_SPELL_POISON_PROJECTILE_LAUNCH(INVALID_ID);
SampleId SND_SPELL_RAISE_DEAD(INVALID_ID);
SampleId SND_SPELL_REPEL_UNDEAD(INVALID_ID);
SampleId SND_SPELL_REPEL_UNDEAD_LOOP(INVALID_ID);
SampleId SND_SPELL_RUNE_OF_GUARDING(INVALID_ID);
SampleId SND_SPELL_RUNE_OF_GUARDING_END(INVALID_ID);
SampleId SND_SPELL_SLOW_DOWN(INVALID_ID);
SampleId SND_SPELL_SLOW_DOWN_END(INVALID_ID);
SampleId SND_SPELL_SPARK(INVALID_ID);
SampleId SND_SPELL_SPEED_START(INVALID_ID);
SampleId SND_SPELL_SPEED_LOOP(INVALID_ID);
SampleId SND_SPELL_SPEED_END(INVALID_ID);
SampleId SND_SPELL_SUMMON_CREATURE(INVALID_ID);
SampleId SND_SPELL_TELEKINESIS_START(INVALID_ID);
SampleId SND_SPELL_TELEKINESIS_END(INVALID_ID);
SampleId SND_SPELL_TELEPORT(INVALID_ID);
SampleId SND_SPELL_TELEPORTED(INVALID_ID);
SampleId SND_SPELL_VISION_START(INVALID_ID);
SampleId SND_SPELL_VISION_LOOP(INVALID_ID);

static void ARX_SOUND_EnvironmentSet(const res::path & name);
static void ARX_SOUND_CreateEnvironments();
static void ARX_SOUND_CreateStaticSamples();
static void ARX_SOUND_ReleaseStaticSamples();
static void ARX_SOUND_LoadCollision(const long & mat1, const long & mat2, const char * name);
static void ARX_SOUND_CreateCollisionMaps();
static void ARX_SOUND_CreateMaterials();
static void ARX_SOUND_CreatePresenceMap();
static float GetSamplePresenceFactor(const res::path & name);
static void ARX_SOUND_LaunchUpdateThread();
static void ARX_SOUND_KillUpdateThread();

bool ARX_SOUND_Init() {
	
	if (bIsActive) ARX_SOUND_Release();
	
	if(audio::init(config.audio.backend,  config.audio.eax)) {
		audio::clean();
		return false;
	}
	
	if(audio::setSamplePath(ARX_SOUND_PATH_SAMPLE)
	   || audio::setAmbiancePath(ARX_SOUND_PATH_AMBIANCE)
	   || audio::setEnvironmentPath(ARX_SOUND_PATH_ENVIRONMENT)) {
		audio::clean();
		return false;
	}
	
	// Create game mixers
	ARX_SOUND_MixerGame = audio::createMixer();
	ARX_SOUND_MixerGameSample = audio::createMixer();
	audio::setMixerParent(ARX_SOUND_MixerGameSample, ARX_SOUND_MixerGame);
	ARX_SOUND_MixerGameSpeech = audio::createMixer();
	audio::setMixerParent(ARX_SOUND_MixerGameSpeech, ARX_SOUND_MixerGame);
	ARX_SOUND_MixerGameAmbiance = audio::createMixer();
	audio::setMixerParent(ARX_SOUND_MixerGameAmbiance, ARX_SOUND_MixerGame);
	
	// Create menu mixers
	ARX_SOUND_MixerMenu = audio::createMixer();
	ARX_SOUND_MixerMenuSample = audio::createMixer();
	audio::setMixerParent(ARX_SOUND_MixerMenuSample, ARX_SOUND_MixerMenu);
	ARX_SOUND_MixerMenuSpeech = audio::createMixer();
	audio::setMixerParent(ARX_SOUND_MixerMenuSpeech, ARX_SOUND_MixerMenu);
	ARX_SOUND_MixerMenuAmbiance = audio::createMixer();
	audio::setMixerParent(ARX_SOUND_MixerMenuAmbiance, ARX_SOUND_MixerMenu);
	
	if(ARX_SOUND_MixerGame == INVALID_ID
	   || ARX_SOUND_MixerGameSample == INVALID_ID
	   || ARX_SOUND_MixerGameSpeech == INVALID_ID
	   || ARX_SOUND_MixerGameAmbiance == INVALID_ID
	   || ARX_SOUND_MixerMenu == INVALID_ID
	   || ARX_SOUND_MixerMenuSample == INVALID_ID
	   || ARX_SOUND_MixerMenuSpeech == INVALID_ID
	   || ARX_SOUND_MixerMenuAmbiance == INVALID_ID) {
		audio::clean();
		return false;
	}
	
	audio::setStreamLimit(ARX_SOUND_STREAMING_LIMIT);
	
	audio::setUnitFactor(ARX_SOUND_UNIT_FACTOR);
	audio::setRolloffFactor(ARX_SOUND_ROLLOFF_FACTOR);
	
	ARX_SOUND_LaunchUpdateThread();
	
	bIsActive = true;
	
	return true;
}

void ARX_SOUND_LoadData() {
	
	if(!bIsActive) {
		return;
	}
	
	// Load samples
	ARX_SOUND_CreateStaticSamples();
	ARX_SOUND_CreateMaterials();
	ARX_SOUND_CreateCollisionMaps();
	ARX_SOUND_CreatePresenceMap();
	
	// Load environments, enable environment system and set default one if required
	ARX_SOUND_CreateEnvironments();
	
	if(config.audio.eax) {
		audio::setReverbEnabled(true);
		ARX_SOUND_EnvironmentSet("alley.aef");
	}
}
 
void ARX_SOUND_Release() {
	
	if(!bIsActive) {
		return;
	}
	
	ARX_SOUND_ReleaseStaticSamples();
	collisionMaps.clear();
	presence.clear();
	ARX_SOUND_KillUpdateThread();
	audio::clean();
	bIsActive = false;
}

long ARX_SOUND_IsEnabled()
{
	return bIsActive ? 1 : 0;
}

void ARX_SOUND_MixerSetVolume(audio::MixerId mixer_id, float volume) {
	if(bIsActive) {
		audio::setMixerVolume(mixer_id, volume);
	}
}

void ARX_SOUND_MixerStop(audio::MixerId mixer_id) {
	if(bIsActive) {
		audio::mixerStop(mixer_id);
	}
}

void ARX_SOUND_MixerPause(audio::MixerId mixer_id) {
	if(bIsActive) {
		audio::mixerPause(mixer_id);
	}
}

void ARX_SOUND_MixerResume(audio::MixerId mixer_id) {
	if(bIsActive) {
		audio::mixerResume(mixer_id);
	}
}

void ARX_SOUND_MixerSwitch(audio::MixerId from, audio::MixerId to) {
	ARX_SOUND_MixerPause(from);
	ARX_SOUND_MixerResume(to);
}

// Sets the position of the listener
void ARX_SOUND_SetListener(const Vec3f * position, const Vec3f * front, const Vec3f * up) {
	if(bIsActive) {
		audio::setListenerPosition(*position);
		audio::setListenerDirection(*front, *up);
	}
}

void ARX_SOUND_EnvironmentSet(const res::path & name) {
	
	if(bIsActive) {
		EnvId e_id = audio::getEnvironment(name);
		if(e_id != INVALID_ID) {
			audio::setListenerEnvironment(e_id);
			audio::setRoomRolloffFactor(ARX_SOUND_ROLLOFF_FACTOR);
		}
	}
}

long ARX_SOUND_PlaySFX(SourceId & sample_id, const Vec3f * position, float pitch, SoundLoopMode loop) {
	
	if(!bIsActive || sample_id == INVALID_ID) {
		return INVALID_ID;
	}
	
	audio::Channel channel;
	float presence;
	
	channel.mixer = ARX_SOUND_MixerGameSample;
	channel.flags = FLAG_VOLUME | FLAG_POSITION | FLAG_REVERBERATION | FLAG_FALLOFF;
	channel.volume = 1.0F;
	
	if(position) {
		if(ACTIVECAM && fartherThan(ACTIVECAM->orgTrans.pos, *position, ARX_SOUND_REFUSE_DISTANCE)) {
			return -1;
		}
	}
	
	res::path sample_name;
	audio::getSampleName(sample_id, sample_name);
	presence = GetSamplePresenceFactor(sample_name);
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;
	
	if(pitch != 1.0F) {
		channel.flags |= FLAG_PITCH;
		channel.pitch = pitch;
	}
	
	if(position) {
		channel.position = *position;
	} else {
		channel.flags |= FLAG_RELATIVE;
		channel.position = Vec3f_Z_AXIS;
	}
	
	audio::samplePlay(sample_id, channel, loop);
	
	return sample_id;
}


long ARX_SOUND_PlayInterface(SourceId & sample_id, float pitch, SoundLoopMode loop) {
	
	if (!bIsActive || sample_id == INVALID_ID) return INVALID_ID;

	audio::Channel channel;

	channel.mixer = ARX_SOUND_MixerGameSample;
	channel.flags = FLAG_VOLUME;
	channel.volume = 1.0F;

	if (pitch != 1.0F) channel.flags |= FLAG_PITCH, channel.pitch = pitch;

	audio::samplePlay(sample_id, channel, loop);

	return sample_id;
}

long ARX_SOUND_PlayMenu(SourceId & sample_id, float pitch, SoundLoopMode loop) {
	
	if (!bIsActive || sample_id == INVALID_ID) return INVALID_ID;

	audio::Channel channel;

	channel.mixer = ARX_SOUND_MixerMenuSample;
	channel.flags = FLAG_VOLUME;
	channel.volume = 1.0F;

	if (pitch != 1.0F) channel.flags |= FLAG_PITCH, channel.pitch = pitch;

	audio::samplePlay(sample_id, channel, loop);

	return sample_id;
}


void ARX_SOUND_IOFrontPos(const Entity * io, Vec3f & pos) {
	if(io) {
		pos.x = io->pos.x - EEsin(radians(MAKEANGLE(io->angle.getPitch()))) * 100.0F;
		pos.y = io->pos.y - 100.0F;
		pos.z = io->pos.z + EEcos(radians(MAKEANGLE(io->angle.getPitch()))) * 100.0F;
	} else if(ACTIVECAM) {
		pos.x = ACTIVECAM->orgTrans.pos.x - EEsin(radians(MAKEANGLE(ACTIVECAM->angle.getPitch()))) * 100.0F;
		pos.y = ACTIVECAM->orgTrans.pos.y - 100.0F;
		pos.z = ACTIVECAM->orgTrans.pos.z + EEcos(radians(MAKEANGLE(ACTIVECAM->angle.getPitch()))) * 100.0F;
	} else {
		pos = Vec3f_ZERO;
	}
}

static res::path speechFileName(const res::path & name) {
	return res::path("speech") / config.language / name;
}

long ARX_SOUND_PlaySpeech(const res::path & name, const Entity * io)
{
	if (!bIsActive) return INVALID_ID;

	audio::Channel channel;
	SampleId sample_id;
	
	res::path file = speechFileName(name);
	file.set_ext(ARX_SOUND_FILE_EXTENSION_WAV);
	
	sample_id = audio::createSample(file);

	channel.mixer = ARX_SOUND_MixerGameSpeech;
	channel.flags = FLAG_VOLUME | FLAG_POSITION | FLAG_REVERBERATION | FLAG_AUTOFREE | FLAG_FALLOFF;
	channel.volume = 1.f;
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND;

	if(io) {
		if((io == entities.player() && !EXTERNALVIEW) || ((io->ioflags & IO_CAMERA) && io == CAMERACONTROLLER))
			ARX_SOUND_IOFrontPos(io, channel.position);
		else
			channel.position = io->pos;

		if(ACTIVECAM && fartherThan(ACTIVECAM->orgTrans.pos, io->pos, ARX_SOUND_REFUSE_DISTANCE)) {
			return ARX_SOUND_TOO_FAR; // TODO sample is never freed!
		}

		if(io->ioflags & IO_NPC && io->_npcdata->speakpitch != 1.f) {
			channel.flags |= FLAG_PITCH;
			channel.pitch = io->_npcdata->speakpitch;
		}
	} else {
		channel.flags |= FLAG_RELATIVE;
		channel.position = Vec3f_Z_AXIS * 100.f;
	}

	audio::samplePlay(sample_id, channel);

	return sample_id;
}

long ARX_SOUND_PlayCollision(long mat1, long mat2, float volume, float power, Vec3f * position, Entity * source)
{
	if (!bIsActive) return 0;

	if (mat1 == MATERIAL_NONE || mat2 == MATERIAL_NONE) return 0;

	if (mat1 == MATERIAL_WATER || mat2 == MATERIAL_WATER)
		ARX_PARTICLES_SpawnWaterSplash(position);

	SampleId sample_id;

	sample_id = Inter_Materials[mat1][mat2][0];

	if (sample_id == INVALID_ID) return 0;

	audio::Channel channel;
	float presence;

	channel.mixer = ARX_SOUND_MixerGameSample;

	channel.flags = FLAG_VOLUME | FLAG_PITCH | FLAG_POSITION | FLAG_REVERBERATION | FLAG_FALLOFF;

	res::path sample_name;
	audio::getSampleName(sample_id, sample_name);
	presence = GetSamplePresenceFactor(sample_name);
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;

	if (position)
	{
		if (ACTIVECAM && fartherThan(ACTIVECAM->orgTrans.pos, *position, ARX_SOUND_REFUSE_DISTANCE))
			return -1;
	}
	
	//Launch 'ON HEAR' script event
	ARX_NPC_SpawnAudibleSound(position, source, power, presence);
	
	if(position) {
		channel.position = *position;
	} else {
		ARX_PLAYER_FrontPos(&channel.position);
	}
	
	channel.pitch = 0.9F + 0.2F * rnd();
	channel.volume = volume;
	audio::samplePlay(sample_id, channel);
	
	size_t length;
	audio::getSampleLength(sample_id, length);
	
	return (long)(channel.pitch * length);
}

long ARX_SOUND_PlayCollision(const string & name1, const string & name2, float volume, float power, Vec3f * position, Entity * source) {
	
	if(!bIsActive) {
		return 0;
	}
	
	if(name1.empty() || name2.empty()) {
		return 0;
	}
	
	if(name2 == "water") {
		ARX_PARTICLES_SpawnWaterSplash(position);
	}
	
	CollisionMaps::iterator mi = collisionMaps.find(name1);
	if(mi == collisionMaps.end()) {
		return 0;
	}
	CollisionMap & map = mi->second;
	
	CollisionMap::iterator ci = map.find(name2);
	if(ci == map.end()) {
		return 0;
	}
	SoundMaterial & mat = ci->second;
	
	SampleId sample_id = mat.next();
	arx_assert(sample_id != INVALID_ID);
	
	audio::Channel channel;
	channel.mixer = ARX_SOUND_MixerGameSample;
	channel.flags = FLAG_VOLUME | FLAG_PITCH | FLAG_POSITION | FLAG_REVERBERATION | FLAG_FALLOFF;
	
	res::path sample_name;
	audio::getSampleName(sample_id, sample_name);
	float presence = GetSamplePresenceFactor(sample_name);
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;
	
	// Launch 'ON HEAR' script event
	ARX_NPC_SpawnAudibleSound(position, source, power, presence);
	
	if(position) {
		channel.position = *position;
		if(ACTIVECAM && fartherThan(ACTIVECAM->orgTrans.pos, *position, ARX_SOUND_REFUSE_DISTANCE)) {
			return -1;
		}
	} else {
		ARX_PLAYER_FrontPos(&channel.position);
	}
	
	channel.pitch = 0.975f + 0.5f * rnd();
	channel.volume = volume;
	audio::samplePlay(sample_id, channel);
	
	size_t length;
	audio::getSampleLength(sample_id, length);
	
	return (long)(channel.pitch * length);
}

long ARX_SOUND_PlayScript(const res::path & name, const Entity * io, float pitch, SoundLoopMode loop)
{
	if (!bIsActive) {
		return INVALID_ID;
	}

	audio::Channel channel;
	SampleId sample_id;

	sample_id = audio::createSample(name);

	if (sample_id == INVALID_ID) {
		return INVALID_ID;
	}
	
	channel.mixer = ARX_SOUND_MixerGameSample;
	channel.flags = FLAG_VOLUME | FLAG_AUTOFREE | FLAG_POSITION | FLAG_REVERBERATION | FLAG_FALLOFF;
	channel.volume = 1.0F;
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * GetSamplePresenceFactor(name);
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND;
	
	if(io) {
		GetItemWorldPositionSound(io, &channel.position);
		if(loop != ARX_SOUND_PLAY_LOOPED) {
			if (ACTIVECAM && fartherThan(ACTIVECAM->orgTrans.pos, channel.position, ARX_SOUND_REFUSE_DISTANCE)) {
				// TODO the sample will never be freed!
				return ARX_SOUND_TOO_FAR;
			}
		}
	} else {
		channel.flags |= FLAG_RELATIVE;
		channel.position = Vec3f_Z_AXIS * 100.f;
	}
	
	if(pitch != 1.0F) {
		channel.flags |= FLAG_PITCH;
		channel.pitch = pitch;
	}
	
	audio::samplePlay(sample_id, channel, loop);
	
	return sample_id;
}

long ARX_SOUND_PlayAnim(SourceId & sample_id, const Vec3f * position)
{
	if(!bIsActive || sample_id == INVALID_ID)
		return INVALID_ID;

	audio::Channel channel;

	channel.mixer = ARX_SOUND_MixerGameSample;
	channel.flags = FLAG_VOLUME;
	channel.volume = 1.0F;

	if(position) {
		if(ACTIVECAM && fartherThan(ACTIVECAM->orgTrans.pos, *position, ARX_SOUND_REFUSE_DISTANCE)) {
			return -1;
		}
		channel.flags |= FLAG_POSITION | FLAG_REVERBERATION | FLAG_FALLOFF;
		res::path sample_name;
		audio::getSampleName(sample_id, sample_name);
		float presence = GetSamplePresenceFactor(sample_name);
		channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
		channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;
		channel.position = *position;
	}

	audio::samplePlay(sample_id, channel);

	return sample_id;
}

long ARX_SOUND_PlayCinematic(const res::path & name, bool isSpeech) {
	
	res::path file = (isSpeech) ? speechFileName(name) : name;
	file.set_ext(ARX_SOUND_FILE_EXTENSION_WAV);
	
	s32 sample_id = audio::createSample(file);
	if(sample_id == INVALID_ID) {
		LogError << "Cannot load sound for cinematic: " << file;
		return INVALID_ID;
	}
	
	audio::Channel channel;
	channel.mixer = ARX_SOUND_MixerGameSpeech;
	channel.flags = FLAG_VOLUME | FLAG_AUTOFREE | FLAG_POSITION | FLAG_FALLOFF
	                | FLAG_REVERBERATION | FLAG_POSITION;
	channel.volume = 1.0f;
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND;
	
	if (ACTIVECAM) {
		float t = radians(MAKEANGLE(ACTIVECAM->angle.getPitch()));
		Vec3f front(-EEsin(t), 0.f, EEcos(t));
		front = glm::normalize(front);

		//TODO Hardcoded up vector
		Vec3f up(0.f, 1.f, 0.f);
		ARX_SOUND_SetListener(&ACTIVECAM->orgTrans.pos, &front, &up);
	}
	
	ARX_SOUND_IOFrontPos(NULL, channel.position); 
	
	audio::samplePlay(sample_id, channel);
	
	return sample_id;
}

long ARX_SOUND_IsPlaying(SourceId & sample_id) {
	return bIsActive ? audio::isSamplePlaying(sample_id) : 0;
}


float ARX_SOUND_GetDuration(SampleId & sample_id) {
	
	if(bIsActive && sample_id != INVALID_ID) {
		size_t length;
		audio::getSampleLength(sample_id, length);
		return static_cast<float>(length);
	}

	return 0.f;
}

void ARX_SOUND_RefreshVolume(SourceId & sample_id, float volume) {
	if(bIsActive && sample_id != INVALID_ID) {
		audio::setSampleVolume(sample_id, volume);
	}
}

void ARX_SOUND_RefreshPitch(SourceId & sample_id, float pitch) {
	if(bIsActive && sample_id != INVALID_ID) {
		audio::setSamplePitch(sample_id, pitch);
	}
}

void ARX_SOUND_RefreshPosition(SourceId & sample_id, const Vec3f * position) {
	
	if(bIsActive && sample_id != INVALID_ID) {
		if(position) {
			audio::setSamplePosition(sample_id, *position);
		} else {
			Vec3f pos;
			ARX_PLAYER_FrontPos(&pos);
			audio::setSamplePosition(sample_id, pos);
		}
	}
}

void ARX_SOUND_RefreshSpeechPosition(SourceId & sample_id, const Entity * io) {
	
	if(!bIsActive || !io || sample_id == INVALID_ID) {
		return;
	}
	
	Vec3f position;
	if((io == entities.player() && !EXTERNALVIEW) || ((io->ioflags & IO_CAMERA) && io == CAMERACONTROLLER)) {
		ARX_SOUND_IOFrontPos(io, position);
	} else {
		position = io->pos;
	}
	
	audio::setSamplePosition(sample_id, position);
}

SampleId ARX_SOUND_Load(const res::path & name) {
	
	if(!bIsActive) {
		return INVALID_ID;
	}
	
	res::path sample_name = name;
	
	return audio::createSample(sample_name.set_ext(ARX_SOUND_FILE_EXTENSION_WAV));
}

void ARX_SOUND_Free(const SampleId & sample) {
	if(bIsActive && sample != INVALID_ID) {
		audio::deleteSample(sample);
	}
}

void ARX_SOUND_Stop(SourceId & sample_id) {
	if(bIsActive && sample_id != INVALID_ID) {
		audio::sampleStop(sample_id);
	}
}

bool ARX_SOUND_PlayScriptAmbiance(const res::path & name, SoundLoopMode loop, float volume) {
	
	if (!bIsActive) return INVALID_ID;

	res::path temp = res::path(name).set_ext("amb");

	AmbianceId ambiance_id = audio::getAmbiance(temp);

	if (ambiance_id == INVALID_ID)
	{
		if (volume == 0.f) return true;

		ambiance_id = audio::createAmbiance(temp);
		if(ambiance_id == INVALID_ID) {
			return false;
		}
		
		audio::setAmbianceUserData(ambiance_id, reinterpret_cast<void *>(PLAYING_AMBIANCE_SCRIPT));

		audio::Channel channel;

		channel.mixer = ARX_SOUND_MixerGameAmbiance;
		channel.flags = FLAG_VOLUME | FLAG_AUTOFREE;
		channel.volume = volume;

		audio::ambiancePlay(ambiance_id, channel, loop == ARX_SOUND_PLAY_LOOPED);
	}
	else
	{
		if (volume <= 0.0F)
		{
			audio::deleteAmbiance(ambiance_id);
			return true;
		}

		audio::setAmbianceVolume(ambiance_id, volume);
	}

	return true;
}

bool ARX_SOUND_PlayZoneAmbiance(const res::path & name, SoundLoopMode loop, float volume) {
	
	if (!bIsActive) return true;

	if(name == "none") {
		audio::ambianceStop(ambiance_zone, AMBIANCE_FADE_TIME);
		ambiance_zone = INVALID_ID;
		return true;
	}

	res::path temp = res::path(name).set_ext("amb");

	AmbianceId ambiance_id = audio::getAmbiance(temp);

	if (ambiance_id == INVALID_ID)
	{
		ambiance_id = audio::createAmbiance(temp);
		if(ambiance_id == INVALID_ID) {
			return false;
		}
		audio::setAmbianceUserData(ambiance_id, reinterpret_cast<void *>(PLAYING_AMBIANCE_ZONE));
	}
	else if (ambiance_id == ambiance_zone)
		return true;

	audio::Channel channel;

	channel.mixer = ARX_SOUND_MixerGameAmbiance;
	channel.flags = FLAG_VOLUME | FLAG_AUTOFREE;
	channel.volume = volume;

	audio::ambianceStop(ambiance_zone, AMBIANCE_FADE_TIME);
	ambiance_zone = ambiance_id;
	audio::ambiancePlay(ambiance_zone, channel, loop == ARX_SOUND_PLAY_LOOPED, AMBIANCE_FADE_TIME);

	return true;
}

AmbianceId ARX_SOUND_SetAmbianceTrackStatus(const string & ambiance_name, const string & track_name, unsigned long status) {
	
	if(!bIsActive) {
		return INVALID_ID;
	}
	
	AmbianceId ambiance_id = audio::getAmbiance(res::path(ambiance_name).set_ext("amb"));
	if(ambiance_id == INVALID_ID) {
		return INVALID_ID;
	}
	
	audio::muteAmbianceTrack(ambiance_id, track_name, status != 0);
	
	return ambiance_id;
}

void ARX_SOUND_KillAmbiances() {
	
	if(!bIsActive) {
		return;
	}
	
	AmbianceId ambiance_id = audio::getNextAmbiance();
	
	while(ambiance_id != INVALID_ID) {
		audio::deleteAmbiance(ambiance_id);
		ambiance_id = audio::getNextAmbiance(ambiance_id);
	}
	
	ambiance_zone = INVALID_ID;
}

AmbianceId ARX_SOUND_PlayMenuAmbiance(const res::path & ambiance_name) {
	
	if(!bIsActive) {
		return INVALID_ID;
	}
	
	audio::deleteAmbiance(ambiance_menu);
	ambiance_menu = audio::createAmbiance(ambiance_name);
	
	audio::setAmbianceUserData(ambiance_menu, reinterpret_cast<void *>(PLAYING_AMBIANCE_MENU));
	
	audio::Channel channel;
	
	channel.mixer = ARX_SOUND_MixerMenuAmbiance;
	channel.flags = FLAG_VOLUME;
	channel.volume = 1.0F;
	
	audio::ambiancePlay(ambiance_menu, channel, true);
	
	return ambiance_menu;
}

char * ARX_SOUND_AmbianceSavePlayList(size_t & size) {
	
	unsigned long count(0);
	PlayingAmbiance * play_list = NULL;
	long ambiance_id(INVALID_ID);

	ambiance_id = audio::getNextAmbiance();

	while (ambiance_id != INVALID_ID)
	{
		void * storedType;
		audio::getAmbianceUserData(ambiance_id, &storedType);
		long type = reinterpret_cast<long>(storedType);

		if (type == PLAYING_AMBIANCE_SCRIPT || type == PLAYING_AMBIANCE_ZONE)
		{
			void * ptr;
			PlayingAmbiance * playing;

			ptr = realloc(play_list, (count + 1) * sizeof(PlayingAmbiance));

			if (!ptr) break;

			play_list = (PlayingAmbiance *)ptr;
			playing = &play_list[count];
			
			memset(playing->name, 0, sizeof(playing->name));
			
			res::path name;
			audio::getAmbianceName(ambiance_id, name);
			arx_assert(name.string().length() + 1 < ARRAY_SIZE(playing->name));
			strcpy(playing->name, name.string().c_str());
			audio::getAmbianceVolume(ambiance_id, playing->volume);
			playing->loop = audio::isAmbianceLooped(ambiance_id) ? ARX_SOUND_PLAY_LOOPED : ARX_SOUND_PLAY_ONCE;
			playing->type = type;

			count++;
		}

		ambiance_id = audio::getNextAmbiance(ambiance_id);
	}

	size = count * sizeof(PlayingAmbiance);
	return reinterpret_cast<char *>(play_list);
}

void ARX_SOUND_AmbianceRestorePlayList(const char * _play_list, size_t size) {
	
	size_t count = size / sizeof(PlayingAmbiance);
	const PlayingAmbiance * play_list = reinterpret_cast<const PlayingAmbiance *>(_play_list);
	
	for(size_t i = 0; i < count; i++) {
		
		const PlayingAmbiance * playing = &play_list[i];
		
		res::path name = res::path::load(util::loadString(playing->name));
		
		// TODO save/load enum
		switch (playing->type) {
			
			case PLAYING_AMBIANCE_SCRIPT :
				ARX_SOUND_PlayScriptAmbiance(name, (SoundLoopMode)playing->loop, playing->volume);
				break;
			
			case PLAYING_AMBIANCE_ZONE :
				ARX_SOUND_PlayZoneAmbiance(name, (SoundLoopMode)playing->loop, playing->volume);
				break;
		}
	}
}

static void ARX_SOUND_CreateEnvironments() {
	
	PakDirectory * dir = resources->getDirectory(ARX_SOUND_PATH_ENVIRONMENT);
	if(!dir) {
		return;
	}
	
	for(PakDirectory::files_iterator i = dir->files_begin(); i != dir->files_end(); i++) {
		audio::createEnvironment(i->first);
	}
}

static void ARX_SOUND_CreateStaticSamples() {
	
	// Interface
	SND_BACKPACK                       = audio::createSample("interface_backpack.wav");
	SND_BOOK_OPEN                      = audio::createSample("book_open.wav");
	SND_BOOK_CLOSE                     = audio::createSample("book_close.wav");
	SND_BOOK_PAGE_TURN                 = audio::createSample("book_page_turn.wav");
	SND_SCROLL_OPEN                    = audio::createSample("scroll_open.wav");
	SND_SCROLL_CLOSE                   = audio::createSample("scroll_close.wav");
	SND_TORCH_START                    = audio::createSample("torch_start.wav");
	SND_TORCH_LOOP                     = audio::createSample("sfx_torch_11khz.wav");
	SND_TORCH_END                      = audio::createSample("torch_end.wav");
	SND_INVSTD                         = audio::createSample("interface_invstd.wav");
	SND_GOLD                           = audio::createSample("drop_coin.wav");
	
	//Menu
	SND_MENU_CLICK                     = audio::createSample("menu_click.wav");
	SND_MENU_RELEASE                   = audio::createSample("menu_release.wav");
	
	//Other SFX samples
	SND_FIREPLACE                      = audio::createSample("fire_place.wav");
	SND_PLOUF                          = audio::createSample("fishing_plouf.wav");
	SND_QUAKE                          = audio::createSample("sfx_quake.wav");
	SND_WHOOSH                         = audio::createSample("whoosh07.wav");
	
	// Player
	SND_PLAYER_FILLLIFEMANA            = audio::createSample("player_filllifemana.wav");
	SND_PLAYER_HEART_BEAT              = audio::createSample("player_heartb.wav");
	SND_PLAYER_LEVEL_UP                = audio::createSample("player_level_up.wav");
	SND_PLAYER_POISONED                = audio::createSample("player_poisoned.wav");
	SND_PLAYER_DEATH_BY_FIRE           = audio::createSample("lava_death.wav");
	
	// Magic draw
	SND_MAGIC_AMBIENT                  = audio::createSample("magic_ambient.wav");
	SND_MAGIC_DRAW                     = audio::createSample("magic_draw.wav");
	SND_MAGIC_FIZZLE                   = audio::createSample("magic_fizzle.wav");
	
	// Magic symbols
	SND_SYMB_AAM                       = audio::createSample("magic_aam.wav");
	SND_SYMB_CETRIUS                   = audio::createSample("magic_citrius.wav");
	SND_SYMB_COSUM                     = audio::createSample("magic_cosum.wav");
	SND_SYMB_COMUNICATUM               = audio::createSample("magic_comunicatum.wav");
	SND_SYMB_FOLGORA                   = audio::createSample("magic_folgora.wav");
	SND_SYMB_FRIDD                     = audio::createSample("magic_fridd.wav");
	SND_SYMB_KAOM                      = audio::createSample("magic_kaom.wav");
	SND_SYMB_MEGA                      = audio::createSample("magic_mega.wav");
	SND_SYMB_MORTE                     = audio::createSample("magic_morte.wav");
	SND_SYMB_MOVIS                     = audio::createSample("magic_movis.wav");
	SND_SYMB_NHI                       = audio::createSample("magic_nhi.wav");
	SND_SYMB_RHAA                      = audio::createSample("magic_rhaa.wav");
	SND_SYMB_SPACIUM                   = audio::createSample("magic_spacium.wav");
	SND_SYMB_STREGUM                   = audio::createSample("magic_stregum.wav");
	SND_SYMB_TAAR                      = audio::createSample("magic_taar.wav");
	SND_SYMB_TEMPUS                    = audio::createSample("magic_tempus.wav");
	SND_SYMB_TERA                      = audio::createSample("magic_tera.wav");
	SND_SYMB_VISTA                     = audio::createSample("magic_vista.wav");
	SND_SYMB_VITAE                     = audio::createSample("magic_vitae.wav");
	SND_SYMB_YOK                       = audio::createSample("magic_yok.wav");
	
	// Spells
	SND_SPELL_ACTIVATE_PORTAL          = audio::createSample("magic_spell_activate_portal.wav");
	SND_SPELL_ARMOR_START              = audio::createSample("magic_spell_armor_start.wav");
	SND_SPELL_ARMOR_END                = audio::createSample("magic_spell_armor_end.wav");
	SND_SPELL_ARMOR_LOOP               = audio::createSample("magic_spell_armor_loop.wav");
	SND_SPELL_LOWER_ARMOR              = audio::createSample("magic_spell_decrease_armor.wav");
	SND_SPELL_LOWER_ARMOR_END          = audio::createSample("magic_spell_lower_armor.wav");
	SND_SPELL_BLESS                    = audio::createSample("magic_spell_bless.wav");
	SND_SPELL_COLD_PROTECTION_START    = audio::createSample("magic_spell_cold_protection.wav");
	SND_SPELL_COLD_PROTECTION_LOOP     = audio::createSample("magic_spell_cold_protection_loop.wav");
	SND_SPELL_COLD_PROTECTION_END      = audio::createSample("magic_spell_cold_protection_end.wav");
	SND_SPELL_CONFUSE                  = audio::createSample("magic_spell_confuse.wav");
	SND_SPELL_CONTROL_TARGET           = audio::createSample("magic_spell_control_target.wav");
	SND_SPELL_CREATE_FIELD             = audio::createSample("magic_spell_create_field.wav");
	SND_SPELL_CREATE_FOOD              = audio::createSample("magic_spell_create_food.wav");
	SND_SPELL_CURE_POISON              = audio::createSample("magic_spell_cure_poison.wav");
	SND_SPELL_CURSE                    = audio::createSample("magic_spell_curse.wav");
	SND_SPELL_DETECT_TRAP              = audio::createSample("magic_spell_detect_trap.wav");
	SND_SPELL_DETECT_TRAP_LOOP         = audio::createSample("magic_spell_detect_trap_loop.wav");
	SND_SPELL_DISARM_TRAP              = audio::createSample("magic_spell_disarm_trap.wav");
	SND_SPELL_DISPELL_FIELD            = audio::createSample("magic_spell_dispell_field.wav");
	SND_SPELL_DISPELL_ILLUSION         = audio::createSample("magic_spell_dispell_illusion.wav");
	SND_SPELL_DOUSE                    = audio::createSample("magic_spell_douse.wav");
	SND_SPELL_ELECTRIC                 = audio::createSample("sfx_electric.wav");
	SND_SPELL_ENCHANT_WEAPON           = audio::createSample("magic_spell_enchant_weapon.wav");
	SND_SPELL_EXPLOSION                = audio::createSample("magic_spell_explosion.wav");
	SND_SPELL_EYEBALL_IN               = audio::createSample("magic_spell_eyeball_in.wav");
	SND_SPELL_EYEBALL_OUT              = audio::createSample("magic_spell_eyeball_out.wav");
	SND_SPELL_FIRE_HIT                 = audio::createSample("magic_spell_firehit.wav");
	SND_SPELL_FIRE_LAUNCH              = audio::createSample("magic_spell_firelaunch.wav");
	SND_SPELL_FIRE_PROTECTION          = audio::createSample("magic_spell_fire_protection.wav");
	SND_SPELL_FIRE_PROTECTION_LOOP     = audio::createSample("magic_spell_fire_protection_loop.wav");
	SND_SPELL_FIRE_PROTECTION_END      = audio::createSample("magic_spell_fire_protection_end.wav");
	SND_SPELL_FIRE_WIND                = audio::createSample("magic_spell_firewind.wav");
	SND_SPELL_FREEZETIME               = audio::createSample("magic_spell_freezetime.wav");
	SND_SPELL_HARM                     = audio::createSample("magic_spell_harm.wav");
	SND_SPELL_HEALING                  = audio::createSample("magic_spell_healing.wav");
	SND_SPELL_ICE_FIELD                = audio::createSample("magic_spell_ice_field.wav");
	SND_SPELL_ICE_FIELD_LOOP           = audio::createSample("magic_spell_ice_fieldloop.wav");
	SND_SPELL_ICE_FIELD_END            = audio::createSample("magic_spell_ice_field_end.wav");
	SND_SPELL_ICE_PROJECTILE_LAUNCH    = audio::createSample("magic_spell_ice_projectile_launch.wav");
	SND_SPELL_INCINERATE               = audio::createSample("magic_spell_incinerate.wav");
	SND_SPELL_INCINERATE_LOOP          = audio::createSample("magic_spell_incinerate_loop.wav");
	SND_SPELL_INCINERATE_END           = audio::createSample("magic_spell_incinerate_end.wav");
	SND_SPELL_IGNITE                   = audio::createSample("magic_spell_ignite.wav");
	SND_SPELL_INVISIBILITY_START       = audio::createSample("magic_spell_invisibilityon.wav");
	SND_SPELL_INVISIBILITY_END         = audio::createSample("magic_spell_invisibilityoff.wav");
	SND_SPELL_LEVITATE_START           = audio::createSample("magic_spell_levitate_start.wav");
	SND_SPELL_LEVITATE_LOOP            = audio::createSample("magic_spell_levitate_loop.wav");
	SND_SPELL_LEVITATE_END             = audio::createSample("magic_spell_levitate_end.wav");
	SND_SPELL_LIGHTNING_START          = audio::createSample("magic_spell_lightning_start.wav");
	SND_SPELL_LIGHTNING_LOOP           = audio::createSample("magic_spell_lightning_loop.wav");
	SND_SPELL_LIGHTNING_END            = audio::createSample("magic_spell_lightning_end.wav");
	SND_SPELL_MAGICAL_HIT              = audio::createSample("magic_spell_magicalhit.wav");
	
	SND_SPELL_FIRE_FIELD_START         = audio::createSample("magic_spell_fire_field.wav");
	SND_SPELL_FIRE_FIELD_LOOP          = audio::createSample("magic_spell_fire_field_loop.wav");
	SND_SPELL_FIRE_FIELD_END           = audio::createSample("magic_spell_fire_field_end.wav");
	
	SND_SPELL_MAGICAL_SHIELD           = audio::createSample("magic_spell_magicalshield.wav");
	SND_SPELL_MASS_INCINERATE          = audio::createSample("magic_spell_mass_incinerate.wav");
	SND_SPELL_MASS_PARALYSE            = audio::createSample("magic_spell_mass_paralyse.wav");
	SND_SPELL_MM_CREATE                = audio::createSample("magic_spell_missilecreate.wav");
	SND_SPELL_MM_HIT                   = audio::createSample("magic_spell_missilehit.wav");
	SND_SPELL_MM_LAUNCH                = audio::createSample("magic_spell_missilelaunch.wav");
	SND_SPELL_MM_LOOP                  = audio::createSample("magic_spell_missileloop.wav");
	SND_SPELL_NEGATE_MAGIC             = audio::createSample("magic_spell_negate_magic.wav");
	SND_SPELL_NO_EFFECT                = audio::createSample("magic_spell_noeffect.wav");
	SND_SPELL_PARALYSE                 = audio::createSample("magic_spell_paralyse.wav");
	SND_SPELL_PARALYSE_END             = audio::createSample("magic_spell_paralyse_end.wav");
	SND_SPELL_POISON_PROJECTILE_LAUNCH = audio::createSample("magic_spell_poison_projectile_launch.wav");
	SND_SPELL_RAISE_DEAD               = audio::createSample("magic_spell_raise_dead.wav");
	SND_SPELL_REPEL_UNDEAD             = audio::createSample("magic_spell_repel_undead.wav");
	SND_SPELL_REPEL_UNDEAD_LOOP        = audio::createSample("magic_spell_repell_loop.wav");
	SND_SPELL_RUNE_OF_GUARDING         = audio::createSample("magic_spell_rune_of_guarding.wav");
	SND_SPELL_RUNE_OF_GUARDING_END     = audio::createSample("magic_spell_rune_of_guarding_explode.wav");
	SND_SPELL_SLOW_DOWN                = audio::createSample("magic_spell_slow_down.wav");
	SND_SPELL_SLOW_DOWN_END            = audio::createSample("magic_spell_slow_down_end.wav");
	SND_SPELL_SPARK                    = audio::createSample("sfx_spark.wav");
	SND_SPELL_SPEED_START              = audio::createSample("magic_spell_speedstart.wav");
	SND_SPELL_SPEED_LOOP               = audio::createSample("magic_spell_speed.wav");
	SND_SPELL_SPEED_END                = audio::createSample("magic_spell_speedend.wav");
	SND_SPELL_SUMMON_CREATURE          = audio::createSample("magic_spell_summon_creature.wav");
	SND_SPELL_TELEKINESIS_START        = audio::createSample("magic_spell_telekinesison.wav");
	SND_SPELL_TELEKINESIS_END          = audio::createSample("magic_spell_telekinesisoff.wav");
	SND_SPELL_TELEPORT                 = audio::createSample("magic_spell_teleport.wav");
	SND_SPELL_TELEPORTED               = audio::createSample("magic_spell_teleported.wav");
	SND_SPELL_VISION_START             = audio::createSample("magic_spell_vision2.wav");
	SND_SPELL_VISION_LOOP              = audio::createSample("magic_spell_vision.wav");
}

// Reset each static sample to INVALID_ID
// Those samples are freed from memory when by audio::clean() is deleted
static void ARX_SOUND_ReleaseStaticSamples() {
	
	// Interface samples
	SND_BACKPACK = INVALID_ID;
	SND_BOOK_OPEN = INVALID_ID;
	SND_BOOK_CLOSE = INVALID_ID;
	SND_BOOK_PAGE_TURN = INVALID_ID;
	SND_GOLD = INVALID_ID;
	SND_INVSTD = INVALID_ID;
	SND_SCROLL_OPEN = INVALID_ID;
	SND_SCROLL_CLOSE = INVALID_ID;
	SND_TORCH_START = INVALID_ID;
	SND_TORCH_LOOP = INVALID_ID;
	SND_TORCH_END = INVALID_ID;

	// Other SFX samples
	SND_FIREPLACE = INVALID_ID;
	SND_PLOUF = INVALID_ID;
	SND_QUAKE = INVALID_ID;

	// Menu samples
	SND_MENU_CLICK = INVALID_ID;
	SND_MENU_RELEASE = INVALID_ID;

	// Player samples
	SND_PLAYER_DEATH_BY_FIRE = INVALID_ID;
	SND_PLAYER_FILLLIFEMANA = INVALID_ID;
	SND_PLAYER_HEART_BEAT = INVALID_ID;
	SND_PLAYER_LEVEL_UP = INVALID_ID;
	SND_PLAYER_POISONED = INVALID_ID;

	// Magic drawing samples
	SND_MAGIC_AMBIENT = INVALID_ID;
	SND_MAGIC_DRAW = INVALID_ID;
	SND_MAGIC_FIZZLE = INVALID_ID;

	// Magic symbols samples
	SND_SYMB_AAM = INVALID_ID;
	SND_SYMB_CETRIUS = INVALID_ID;
	SND_SYMB_COSUM = INVALID_ID;
	SND_SYMB_COMUNICATUM = INVALID_ID;
	SND_SYMB_FOLGORA = INVALID_ID;
	SND_SYMB_FRIDD = INVALID_ID;
	SND_SYMB_KAOM = INVALID_ID;
	SND_SYMB_MEGA = INVALID_ID;
	SND_SYMB_MORTE = INVALID_ID;
	SND_SYMB_MOVIS = INVALID_ID;
	SND_SYMB_NHI = INVALID_ID;
	SND_SYMB_RHAA = INVALID_ID;
	SND_SYMB_SPACIUM = INVALID_ID;
	SND_SYMB_STREGUM = INVALID_ID;
	SND_SYMB_TAAR = INVALID_ID;
	SND_SYMB_TEMPUS = INVALID_ID;
	SND_SYMB_TERA = INVALID_ID;
	SND_SYMB_VISTA = INVALID_ID;
	SND_SYMB_VITAE = INVALID_ID;
	SND_SYMB_YOK = INVALID_ID;

	// Spells samples
	SND_SPELL_ACTIVATE_PORTAL = INVALID_ID;
	SND_SPELL_ARMOR_START	= INVALID_ID;
	SND_SPELL_ARMOR_END		= INVALID_ID;
	SND_SPELL_ARMOR_LOOP	= INVALID_ID;
	SND_SPELL_LOWER_ARMOR = INVALID_ID;
	SND_SPELL_LOWER_ARMOR_END = INVALID_ID;
	SND_SPELL_BLESS = INVALID_ID;
	SND_SPELL_COLD_PROTECTION_START = INVALID_ID;
	SND_SPELL_COLD_PROTECTION_LOOP = INVALID_ID;
	SND_SPELL_COLD_PROTECTION_END = INVALID_ID;
	SND_SPELL_CONFUSE = INVALID_ID;
	SND_SPELL_CONTROL_TARGET = INVALID_ID;
	SND_SPELL_CREATE_FIELD = INVALID_ID;
	SND_SPELL_CREATE_FOOD = INVALID_ID;
	SND_SPELL_CURE_POISON = INVALID_ID;
	SND_SPELL_CURSE = INVALID_ID;
	SND_SPELL_DETECT_TRAP = INVALID_ID;
	SND_SPELL_DETECT_TRAP_LOOP = INVALID_ID;
	SND_SPELL_DISARM_TRAP = INVALID_ID;
	SND_SPELL_DISPELL_FIELD = INVALID_ID;
	SND_SPELL_DISPELL_ILLUSION = INVALID_ID;
	SND_SPELL_DOUSE = INVALID_ID;
	SND_SPELL_ELECTRIC = INVALID_ID;
	SND_SPELL_ENCHANT_WEAPON = INVALID_ID;
	SND_SPELL_EXPLOSION = INVALID_ID;
	SND_SPELL_EYEBALL_IN = INVALID_ID;
	SND_SPELL_EYEBALL_OUT = INVALID_ID;
	SND_SPELL_FIRE_FIELD = INVALID_ID;
	SND_SPELL_FIRE_HIT = INVALID_ID;
	SND_SPELL_FIRE_LAUNCH = INVALID_ID;
	SND_SPELL_FIRE_PROTECTION = INVALID_ID;
	SND_SPELL_FIRE_PROTECTION_LOOP = INVALID_ID;
	SND_SPELL_FIRE_PROTECTION_END = INVALID_ID;
	SND_SPELL_FIRE_WIND = INVALID_ID;
	SND_SPELL_FREEZETIME = INVALID_ID;
	SND_SPELL_HARM = INVALID_ID;
	SND_SPELL_HEALING = INVALID_ID;
	SND_SPELL_ICE_FIELD = INVALID_ID;
	SND_SPELL_ICE_FIELD_LOOP = INVALID_ID;
	SND_SPELL_ICE_FIELD_END = INVALID_ID;
	SND_SPELL_ICE_PROJECTILE_LAUNCH = INVALID_ID;
	SND_SPELL_INCINERATE = INVALID_ID;
	SND_SPELL_INCINERATE_LOOP = INVALID_ID;
	SND_SPELL_INCINERATE_END = INVALID_ID;
	SND_SPELL_IGNITE = INVALID_ID;
	SND_SPELL_INVISIBILITY_START = INVALID_ID;
	SND_SPELL_INVISIBILITY_END = INVALID_ID;
	SND_SPELL_LEVITATE_START = INVALID_ID;
	SND_SPELL_LEVITATE_LOOP = INVALID_ID;
	SND_SPELL_LEVITATE_END = INVALID_ID;
	SND_SPELL_LIGHTNING_START = INVALID_ID;
	SND_SPELL_LIGHTNING_LOOP = INVALID_ID;
	SND_SPELL_LIGHTNING_END = INVALID_ID;
	SND_SPELL_MAGICAL_HIT = INVALID_ID;

	//SND_SPELL_MASS_LIGHTNING_END = INVALID_ID;
	SND_SPELL_FIRE_FIELD_START = INVALID_ID;
	SND_SPELL_FIRE_FIELD_LOOP = INVALID_ID;
	SND_SPELL_FIRE_FIELD_END = INVALID_ID;

	SND_SPELL_MAGICAL_SHIELD = INVALID_ID;
	SND_SPELL_MASS_INCINERATE = INVALID_ID;
	SND_SPELL_MASS_PARALYSE = INVALID_ID;
	SND_SPELL_MM_CREATE = INVALID_ID;
	SND_SPELL_MM_HIT = INVALID_ID;
	SND_SPELL_MM_LAUNCH = INVALID_ID;
	SND_SPELL_MM_LOOP = INVALID_ID;
	SND_SPELL_NEGATE_MAGIC = INVALID_ID;
	SND_SPELL_PARALYSE = INVALID_ID;
	SND_SPELL_PARALYSE_END = INVALID_ID;
	SND_SPELL_POISON_PROJECTILE_LAUNCH = INVALID_ID;
	SND_SPELL_RAISE_DEAD = INVALID_ID;
	SND_SPELL_REPEL_UNDEAD = INVALID_ID;
	SND_SPELL_REPEL_UNDEAD_LOOP = INVALID_ID;
	SND_SPELL_RUNE_OF_GUARDING = INVALID_ID;
	SND_SPELL_RUNE_OF_GUARDING_END = INVALID_ID;
	SND_SPELL_SLOW_DOWN = INVALID_ID;
	SND_SPELL_SLOW_DOWN_END = INVALID_ID;
	SND_SPELL_SPARK = INVALID_ID;
	SND_SPELL_SPEED_START = INVALID_ID;
	SND_SPELL_SPEED_LOOP = INVALID_ID;
	SND_SPELL_SPEED_END = INVALID_ID;
	SND_SPELL_SUMMON_CREATURE = INVALID_ID;
	SND_SPELL_TELEKINESIS_START = INVALID_ID;
	SND_SPELL_TELEKINESIS_END = INVALID_ID;
	SND_SPELL_TELEPORT = INVALID_ID;
	SND_SPELL_TELEPORTED = INVALID_ID;
	SND_SPELL_VISION_START = INVALID_ID;
	SND_SPELL_VISION_LOOP = INVALID_ID;
}

bool ARX_MATERIAL_GetNameById(long id, char * name)
{
	switch (id)
	{
		case MATERIAL_WEAPON:
			strcpy(name, "weapon");
			return true;
			break;
		case MATERIAL_FLESH:
			strcpy(name, "flesh");
			return true;
			break;
		case MATERIAL_METAL:
			strcpy(name, "metal");
			return true;
			break;
		case MATERIAL_GLASS:
			strcpy(name, "glass");
			return true;
			break;
		case MATERIAL_CLOTH:
			strcpy(name, "cloth");
			return true;
			break;
		case MATERIAL_WOOD:
			strcpy(name, "wood");
			return true;
			break;
		case MATERIAL_EARTH:
			strcpy(name, "earth");
			return true;
			break;
		case MATERIAL_WATER:
			strcpy(name, "water");
			return true;
			break;
		case MATERIAL_ICE:
			strcpy(name, "ice");
			return true;
			break;
		case MATERIAL_GRAVEL:
			strcpy(name, "gravel");
			return true;
			break;
		case MATERIAL_STONE:
			strcpy(name, "stone");
			return true;
			break;
		case MATERIAL_FOOT_LARGE:
			strcpy(name, "foot_large");
			return true;
			break;
		case MATERIAL_FOOT_BARE:
			strcpy(name, "foot_bare");
			return true;
			break;
		case MATERIAL_FOOT_SHOE:
			strcpy(name, "foot_shoe");
			return true;
			break;
		case MATERIAL_FOOT_METAL:
			strcpy(name, "foot_metal");
			return true;
			break;
		case MATERIAL_FOOT_STEALTH:
			strcpy(name, "foot_stealth");
			return true;
			break;
	}

	strcpy(name, "none");
	return false;
}
static void ARX_SOUND_LoadCollision(const long & mat1, const long & mat2, const char * name)
{
	char path[256];

	for (size_t i = 0; i < MAX_VARIANTS; i++)
	{
		sprintf(path, "%s_%lu.wav", name, (unsigned long)(i + 1));
		Inter_Materials[mat1][mat2][i] = audio::createSample(path);

		if (mat1 != mat2)
			Inter_Materials[mat2][mat1][i] = Inter_Materials[mat1][mat2][i];
	}
}

static void ARX_SOUND_CreateCollisionMaps() {
	
	collisionMaps.clear();
	
	for(size_t i = 0; i < ARX_SOUND_COLLISION_MAP_COUNT; i++) {
		
		res::path file = ARX_SOUND_PATH_INI / ARX_SOUND_COLLISION_MAP_NAMES[i];
		file.set_ext(ARX_SOUND_FILE_EXTENSION_INI);
		
		size_t fileSize;
		char * data = resources->readAlloc(file, fileSize);
		if(!data) {
			LogWarning << "Could not find collision map " << file;
			return;
		}
		
		istringstream iss(string(data, fileSize));
		free(data);
		
		IniReader reader;
		if(!reader.read(iss)) {
			LogWarning << "Errors while parsing collision map " << file;
		}
		
		for(IniReader::iterator si = reader.begin(); si != reader.end(); ++si) {
			const IniSection & section = si->second;
			CollisionMap & map = collisionMaps[si->first];
			
			for(IniSection::iterator ki = section.begin(); ki != section.end(); ++ki) {
				const IniKey & key = *ki;
				SoundMaterial & mat = map[key.getName()];
				
				for(size_t mi = 0; mi < MAX_VARIANTS; mi++) {
					
					ostringstream oss;
					oss << boost::to_lower_copy(key.getValue());
					if(mi) {
						oss << mi;
					}
					oss << ARX_SOUND_FILE_EXTENSION_WAV;
					SampleId sample = audio::createSample(oss.str());
					
					if(sample == INVALID_ID) {
						ostringstream oss2;
						oss2 << boost::to_lower_copy(key.getValue()) << '_' << mi << ARX_SOUND_FILE_EXTENSION_WAV;
						sample = audio::createSample(oss2.str());
					}
					
					if(sample != INVALID_ID) {
						mat.variants.push_back(sample);
					}
				}
				
				if(mat.variants.empty()) {
					map.erase(key.getName());
				}
				
			}
			
			if(map.empty()) {
				collisionMaps.erase(si->first);
			}
			
		}
		
	}
	
}

static void ARX_SOUND_CreateMaterials()
{
	memset(Inter_Materials, -1, sizeof(long) * MAX_MATERIALS * MAX_MATERIALS * MAX_VARIANTS);

	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_WEAPON,       "weapon_on_weapon");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_FLESH,        "weapon_on_flesh");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_METAL,        "weapon_on_metal");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_GLASS,        "weapon_on_glass");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_CLOTH,        "weapon_on_cloth");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_WOOD,         "weapon_on_wood");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_EARTH,        "weapon_on_earth");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_WATER,        "weapon_on_water");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_ICE,          "weapon_on_ice");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_GRAVEL,       "weapon_on_gravel");
	ARX_SOUND_LoadCollision(MATERIAL_WEAPON, MATERIAL_STONE,        "weapon_on_stone");

	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_FLESH,        "flesh_on_flesh");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_METAL,        "flesh_on_metal");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_GLASS,        "flesh_on_glass");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_CLOTH,        "flesh_on_cloth");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_WOOD,         "flesh_on_wood");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_EARTH,        "flesh_on_earth");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_WATER,        "flesh_on_water");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_ICE,          "flesh_on_ice");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_GRAVEL,       "flesh_on_gravel");
	ARX_SOUND_LoadCollision(MATERIAL_FLESH,  MATERIAL_STONE,        "flesh_on_stone");

	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_METAL,        "metal_on_metal");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_GLASS,        "metal_on_glass");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_CLOTH,        "metal_on_cloth");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_WOOD,         "metal_on_wood");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_EARTH,        "metal_on_earth");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_WATER,        "metal_on_water");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_ICE,          "metal_on_ice");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_GRAVEL,       "metal_on_gravel");
	ARX_SOUND_LoadCollision(MATERIAL_METAL,  MATERIAL_STONE,        "metal_on_stone");

	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_GLASS,        "glass_on_glass");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_CLOTH,        "glass_on_cloth");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_WOOD,         "glass_on_wood");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_EARTH,        "glass_on_earth");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_WATER,        "glass_on_water");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_ICE,          "glass_on_ice");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_GRAVEL,       "glass_on_gravel");
	ARX_SOUND_LoadCollision(MATERIAL_GLASS,  MATERIAL_STONE,        "glass_on_stone");

	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_CLOTH,        "cloth_on_cloth");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_WOOD,         "cloth_on_wood");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_EARTH,        "cloth_on_earth");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_WATER,        "cloth_on_water");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_ICE,          "cloth_on_ice");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_GRAVEL,       "cloth_on_gravel");
	ARX_SOUND_LoadCollision(MATERIAL_CLOTH,  MATERIAL_STONE,        "cloth_on_stone");

	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_WOOD,         "wood_on_wood");
	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_EARTH,        "wood_on_earth");
	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_WATER,        "wood_on_water");
	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_ICE,          "wood_on_ice");
	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_GRAVEL,       "wood_on_gravel");
	ARX_SOUND_LoadCollision(MATERIAL_WOOD,   MATERIAL_STONE,        "wood_on_stone");

	ARX_SOUND_LoadCollision(MATERIAL_EARTH,  MATERIAL_EARTH,        "earth_on_earth");
	ARX_SOUND_LoadCollision(MATERIAL_EARTH,  MATERIAL_WATER,        "earth_on_water");
	ARX_SOUND_LoadCollision(MATERIAL_EARTH,  MATERIAL_ICE,          "earth_on_ice");
	ARX_SOUND_LoadCollision(MATERIAL_EARTH,  MATERIAL_GRAVEL,       "earth_on_gravel");
	ARX_SOUND_LoadCollision(MATERIAL_EARTH,  MATERIAL_STONE,        "earth_on_stone");

	ARX_SOUND_LoadCollision(MATERIAL_WATER,  MATERIAL_WATER,        "water_on_water");
	ARX_SOUND_LoadCollision(MATERIAL_WATER,  MATERIAL_ICE,          "water_on_ice");
	ARX_SOUND_LoadCollision(MATERIAL_WATER,  MATERIAL_GRAVEL,       "water_on_gravel");
	ARX_SOUND_LoadCollision(MATERIAL_WATER,  MATERIAL_STONE,        "water_on_stone");

	ARX_SOUND_LoadCollision(MATERIAL_ICE,    MATERIAL_ICE,          "ice_on_ice");
	ARX_SOUND_LoadCollision(MATERIAL_ICE,    MATERIAL_GRAVEL,       "ice_on_gravel");
	ARX_SOUND_LoadCollision(MATERIAL_ICE,    MATERIAL_STONE,        "ice_on_stone");

	ARX_SOUND_LoadCollision(MATERIAL_GRAVEL, MATERIAL_GRAVEL,       "gravel_on_gravel");
	ARX_SOUND_LoadCollision(MATERIAL_GRAVEL, MATERIAL_STONE,        "gravel_on_stone");

	ARX_SOUND_LoadCollision(MATERIAL_STONE,  MATERIAL_STONE,        "stone_on_stone");
}


static void ARX_SOUND_CreatePresenceMap() {
	
	presence.clear();
	
	res::path file = (ARX_SOUND_PATH_INI / ARX_SOUND_PRESENCE_NAME).set_ext(ARX_SOUND_FILE_EXTENSION_INI);
	
	size_t fileSize;
	char * data = resources->readAlloc(file, fileSize);
	if(!data) {
		LogWarning << "Could not find presence map " << file;
		return;
	}
	
	istringstream iss(string(data, fileSize));
	free(data);
	
	IniReader reader;
	if(!reader.read(iss)) {
		LogWarning << "Errors while parsing presence map " << file;
	}
	
	const IniSection * section = reader.getSection(Section::presence);
	if(!section) {
		LogWarning << "No [" << Section::presence << "] section in presence map " << file;
		return;
	}
	
	for(IniSection::iterator i = section->begin(); i != section->end(); ++i) {
		float factor = i->getValue(100.f) / 100.f;
		presence[res::path::load(i->getName()).set_ext(ARX_SOUND_FILE_EXTENSION_WAV)] = factor;
	}
	
}

static float GetSamplePresenceFactor(const res::path & name) {
	
	arx_assert_msg(name.string().find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == string::npos,
	               "bad sample name: \"%s\"", name.string().c_str());
	
	PresenceFactors::const_iterator it = presence.find(name);
	if(it != presence.end()) {
		return it->second;
	}
	
	return 1.f;
}

class SoundUpdateThread : public StoppableThread {
	
	void run() {
		
		while(!isStopRequested()) {
			
			sleep(ARX_SOUND_UPDATE_INTERVAL);
			
			audio::update();
		}
		
	}
	
};

static SoundUpdateThread * updateThread = NULL;

static void ARX_SOUND_LaunchUpdateThread() {
	
	arx_assert(!updateThread);
	
	updateThread = new SoundUpdateThread();
	updateThread->setThreadName("Sound Update");
	updateThread->start();
}

static void ARX_SOUND_KillUpdateThread() {
	
	if(!updateThread) {
		return;
	}
	
	updateThread->stop();
	delete updateThread, updateThread = NULL;
}
