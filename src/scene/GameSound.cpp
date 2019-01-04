/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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
#include <cstring>

#include <boost/foreach.hpp>
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
#include "platform/profiler/Profiler.h"


using audio::AmbianceId;
using audio::EnvId;
using audio::FLAG_VOLUME;
using audio::FLAG_POSITION;
using audio::FLAG_REVERBERATION;
using audio::FLAG_FALLOFF;
using audio::FLAG_PITCH;
using audio::FLAG_RELATIVE;
using audio::FLAG_AUTOFREE;

extern bool EXTERNALVIEW;

// TODO used for saving
struct PlayingAmbiance {
	char name[256];
	f32 volume;
	s32 loop;
	s32 type;
};

static const unsigned long MAX_VARIANTS = 5;
static const PlatformDuration AMBIANCE_FADE_TIME = PlatformDurationMs(2000);
static const float ARX_SOUND_DEFAULT_FALLSTART = 200.f;

static const float ARX_SOUND_DEFAULT_FALLEND = 2200.f;
static const float ARX_SOUND_REFUSE_DISTANCE = 2500.f;

static const res::path sample_path = "sfx";

static const res::path ARX_SOUND_PATH_INI = "localisation";
static const char ARX_SOUND_PATH_ENVIRONMENT[] = "sfx/environment";
static const res::path ARX_SOUND_PRESENCE_NAME = "presence";
static const std::string ARX_SOUND_FILE_EXTENSION_WAV = ".wav";
static const std::string ARX_SOUND_FILE_EXTENSION_INI = ".ini";

static const res::path ARX_SOUND_COLLISION_MAP_NAMES[] = {
	"snd_armor",
	"snd_step",
	"snd_weapon"
};

static bool g_soundInitialized = false;

static AmbianceId g_zoneAmbiance = AmbianceId();
static AmbianceId g_menuAmbiance = AmbianceId();

static audio::SampleHandle g_soundMaterials[MAX_MATERIALS][MAX_MATERIALS];

namespace {

struct SoundMaterial {
	
	std::vector<audio::SampleHandle> variants;
	
	SoundMaterial() : current(0) { }
	
	~SoundMaterial() {
		for(std::vector<audio::SampleHandle>::const_iterator i = variants.begin(); i !=  variants.end(); ++i) {
			audio::deleteSample(*i);
		}
	}
	
	audio::SampleHandle next() {
		arx_assert(current < variants.size());
		audio::SampleHandle sample = variants[current];
		current = (current + 1) % variants.size();
		return sample;
	}
	
private:
	
	size_t current;
	
};

typedef std::map<std::string, SoundMaterial> CollisionMap;
typedef std::map<std::string, CollisionMap> CollisionMaps;
CollisionMaps collisionMaps;

namespace Section {
const std::string presence = "presence";
} // namespace Section

typedef std::map<res::path, float> PresenceFactors;
PresenceFactors g_presenceFactors;

} // anonymous namespace

audio::MixerId ARX_SOUND_MixerGame;
audio::MixerId ARX_SOUND_MixerGameSample;
audio::MixerId ARX_SOUND_MixerGameSpeech;
audio::MixerId ARX_SOUND_MixerGameAmbiance;
audio::MixerId ARX_SOUND_MixerMenu;
audio::MixerId ARX_SOUND_MixerMenuSample;
audio::MixerId ARX_SOUND_MixerMenuSpeech;
audio::MixerId ARX_SOUND_MixerMenuAmbiance;

StaticSamples g_snd;

static void ARX_SOUND_EnvironmentSet(const res::path & name);
static void ARX_SOUND_CreateEnvironments();
static void ARX_SOUND_CreateStaticSamples();
static void ARX_SOUND_ReleaseStaticSamples();
static void ARX_SOUND_CreateCollisionMaps();
static void ARX_SOUND_CreateMaterials();
static void ARX_SOUND_CreatePresenceMap();
static float GetSamplePresenceFactor(const res::path & name);

bool ARX_SOUND_Init() {
	
	if(g_soundInitialized) {
		ARX_SOUND_Release();
	}
	
	if(audio::init(config.audio.backend, config.audio.device, config.audio.hrtf)) {
		audio::clean();
		return false;
	}
	
	audio::setAmbiancePath("sfx/ambiance");
	audio::setEnvironmentPath(ARX_SOUND_PATH_ENVIRONMENT);
	audio::setUnitFactor(0.01f);
	audio::setRolloffFactor(1.3f);
	
	audio::threadStart();
	
	// Create game mixers
	ARX_SOUND_MixerGame = audio::createMixer();
	ARX_SOUND_MixerGameSample = audio::createMixer(ARX_SOUND_MixerGame);
	ARX_SOUND_MixerGameSpeech = audio::createMixer(ARX_SOUND_MixerGame);
	ARX_SOUND_MixerGameAmbiance = audio::createMixer(ARX_SOUND_MixerGame);
	
	// Create menu mixers
	ARX_SOUND_MixerMenu = audio::createMixer();
	ARX_SOUND_MixerMenuSample = audio::createMixer(ARX_SOUND_MixerMenu);
	ARX_SOUND_MixerMenuSpeech = audio::createMixer(ARX_SOUND_MixerMenu);
	ARX_SOUND_MixerMenuAmbiance = audio::createMixer(ARX_SOUND_MixerMenu);
	
	// Load samples
	ARX_SOUND_CreateStaticSamples();
	ARX_SOUND_CreateMaterials();
	ARX_SOUND_CreateCollisionMaps();
	ARX_SOUND_CreatePresenceMap();
	
	// Load environments, enable environment system and set default one if required
	ARX_SOUND_CreateEnvironments();
	
	g_soundInitialized = true;
	
	ARX_SOUND_SetReverb(config.audio.eax);
	
	return true;
}

void ARX_SOUND_SetReverb(bool enabled) {
	audio::setReverbEnabled(enabled);
	if(enabled) {
		ARX_SOUND_EnvironmentSet("alley.aef");
	}
}

void ARX_SOUND_Release() {
	
	if(!g_soundInitialized) {
		return;
	}
	
	ARX_SOUND_ReleaseStaticSamples();
	collisionMaps.clear();
	g_presenceFactors.clear();
	audio::threadStop();
	audio::clean();
	g_soundInitialized = false;
}

long ARX_SOUND_IsEnabled()
{
	return g_soundInitialized ? 1 : 0;
}

void ARX_SOUND_MixerSetVolume(audio::MixerId mixer_id, float volume) {
	if(g_soundInitialized) {
		audio::setMixerVolume(mixer_id, volume);
	}
}

void ARX_SOUND_MixerStop(audio::MixerId mixer_id) {
	if(g_soundInitialized) {
		audio::mixerStop(mixer_id);
	}
}

void ARX_SOUND_MixerPause(audio::MixerId mixer_id) {
	if(g_soundInitialized) {
		audio::mixerPause(mixer_id);
	}
}

void ARX_SOUND_MixerResume(audio::MixerId mixer_id) {
	if(g_soundInitialized) {
		audio::mixerResume(mixer_id);
	}
}

// Sets the position of the listener
void ARX_SOUND_SetListener(const Vec3f & position, const Vec3f & front, const Vec3f & up) {
	
	ARX_PROFILE_FUNC();
	
	if(g_soundInitialized) {
		audio::setListenerPosition(position, front, up);
	}
}

void ARX_SOUND_EnvironmentSet(const res::path & name) {
	
	if(g_soundInitialized) {
		EnvId e_id = audio::getEnvironment(name);
		if(e_id != audio::EnvId()) {
			audio::setListenerEnvironment(e_id);
		}
	}
}

static audio::SourcedSample ARX_SOUND_PlaySFX_int(audio::SampleHandle sample_id, const Vec3f * position,
                                  float pitch, SoundLoopMode loop) {
	
	if(!g_soundInitialized || sample_id == audio::SampleHandle()) {
		return audio::SourcedSample();
	}
	
	audio::Channel channel(ARX_SOUND_MixerGameSample);
	channel.flags = FLAG_VOLUME | FLAG_POSITION | FLAG_REVERBERATION | FLAG_FALLOFF;
	channel.volume = 1.f;
	
	if(position) {
		if(g_camera && fartherThan(g_camera->m_pos, *position, ARX_SOUND_REFUSE_DISTANCE)) {
			return audio::SourcedSample();
		}
		channel.position = *position;
	} else {
		channel.flags |= FLAG_RELATIVE;
		channel.position = Vec3f(0.f, 0.f, 1.f);
	}
	
	res::path sample_name;
	audio::getSampleName(sample_id, sample_name);
	float presence = GetSamplePresenceFactor(sample_name);
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;
	
	if(pitch != 1.f) {
		channel.flags |= FLAG_PITCH;
		channel.pitch = pitch;
	}
	
	return audio::samplePlay(sample_id, channel, loop);
}

void ARX_SOUND_PlaySFX(audio::SampleHandle sample_id, const Vec3f * position, float pitch) {
	ARX_SOUND_PlaySFX_int(sample_id, position, pitch, ARX_SOUND_PLAY_ONCE);
}

audio::SourcedSample ARX_SOUND_PlaySFX_loop(audio::SampleHandle sample_id, const Vec3f * position, float pitch) {
	
	return ARX_SOUND_PlaySFX_int(sample_id, position, pitch, ARX_SOUND_PLAY_LOOPED);
}

static void playSample(audio::SampleHandle sample_id, float pitch, SoundLoopMode loop, audio::MixerId mixer) {
	
	if(!g_soundInitialized || sample_id == audio::SampleHandle()) {
		return;
	}
	
	audio::Channel channel(mixer);
	channel.flags = FLAG_VOLUME;
	channel.volume = 1.f;
	if(pitch != 1.f) {
		channel.flags |= FLAG_PITCH;
		channel.pitch = pitch;
	}
	
	audio::samplePlay(sample_id, channel, loop);
}

void ARX_SOUND_PlayInterface(audio::SampleHandle sample_id, float pitch) {
	playSample(sample_id, pitch, ARX_SOUND_PLAY_ONCE, ARX_SOUND_MixerGameSample);
}

void ARX_SOUND_PlayMenu(audio::SampleHandle sample_id) {
	playSample(sample_id, 1.f, ARX_SOUND_PLAY_ONCE, ARX_SOUND_MixerMenuSample);
}

static Vec3f ARX_SOUND_IOFrontPos(const Entity * io) {
	if(io == entities.player()) {
		return ARX_PLAYER_FrontPos();
	}
	if(io) {
		Vec3f pos = io->pos;
		pos += angleToVectorXZ(io->angle.getYaw()) * 100.f;
		pos += Vec3f(0.f, -100.f, 0.f);
		return pos;
	}
	if(g_camera) {
		Vec3f pos = g_camera->m_pos;
		pos += angleToVectorXZ(g_camera->angle.getYaw()) * 100.f;
		pos += Vec3f(0.f, -100.f, 0.f);
		return pos;
	}
	return Vec3f(0.f);
}

static res::path speechFileName(const res::path & name) {
	return res::path("speech") / config.language / name;
}

audio::SourcedSample ARX_SOUND_PlaySpeech(const res::path & name, bool * tooFar, const Entity * io) {
	
	if(!g_soundInitialized) {
		return audio::SourcedSample();
	}
	
	res::path file = speechFileName(name);
	file.set_ext(ARX_SOUND_FILE_EXTENSION_WAV);
	
	audio::SampleHandle sample_id = audio::createSample(file);
	
	audio::Channel channel(ARX_SOUND_MixerGameSpeech);
	channel.flags = FLAG_VOLUME | FLAG_POSITION | FLAG_REVERBERATION | FLAG_AUTOFREE | FLAG_FALLOFF;
	channel.volume = 1.f;
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND;

	if(io) {
		if((io == entities.player() && !EXTERNALVIEW))
			channel.position = ARX_SOUND_IOFrontPos(io);
		else
			channel.position = io->pos;

		if(g_camera && fartherThan(g_camera->m_pos, io->pos, ARX_SOUND_REFUSE_DISTANCE)) {
			if(tooFar) {
				*tooFar = true;
			}
			// TODO sample is never freed!
			return audio::SourcedSample();
		}

		if(io->ioflags & IO_NPC && io->_npcdata->speakpitch != 1.f) {
			channel.flags |= FLAG_PITCH;
			channel.pitch = io->_npcdata->speakpitch;
		}
	} else {
		channel.flags |= FLAG_RELATIVE;
		channel.position = Vec3f(0.f, 0.f, 100.f);
	}
	
	return audio::samplePlay(sample_id, channel);
}

void ARX_SOUND_PlayCollision(Material mat1, Material mat2, float volume, float power, const Vec3f & position, Entity * source) {
	
	if(!g_soundInitialized)
		return;

	if(mat1 == MATERIAL_NONE || mat2 == MATERIAL_NONE)
		return;

	if(mat1 == MATERIAL_WATER || mat2 == MATERIAL_WATER)
		ARX_PARTICLES_SpawnWaterSplash(position);

	audio::SampleHandle sample_id = g_soundMaterials[mat1][mat2];

	if(sample_id == audio::SampleHandle())
		return;

	audio::Channel channel(ARX_SOUND_MixerGameSample);
	channel.flags = FLAG_VOLUME | FLAG_PITCH | FLAG_POSITION | FLAG_REVERBERATION | FLAG_FALLOFF;
	
	res::path sample_name;
	audio::getSampleName(sample_id, sample_name);
	float presence = GetSamplePresenceFactor(sample_name);
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;
	
	if(g_camera && fartherThan(g_camera->m_pos, position, ARX_SOUND_REFUSE_DISTANCE))
		return;
	
	// Launch 'ON HEAR' script event
	ARX_NPC_SpawnAudibleSound(position, source, power, presence);
	
	channel.position = position;
	channel.pitch = Random::getf(0.9f, 1.1f);
	channel.volume = volume;
	
	audio::samplePlay(sample_id, channel);
}

void ARX_SOUND_PlayCollision(const std::string & name1, const std::string & name2, float volume, float power, const Vec3f & position, Entity * source) {
	
	if(!g_soundInitialized)
		return;
	
	if(name1.empty() || name2.empty())
		return;
	
	if(name2 == "water")
		ARX_PARTICLES_SpawnWaterSplash(position);
	
	CollisionMaps::iterator mi = collisionMaps.find(name1);
	if(mi == collisionMaps.end()) {
		return;
	}
	CollisionMap & map = mi->second;
	
	CollisionMap::iterator ci = map.find(name2);
	if(ci == map.end()) {
		return;
	}
	SoundMaterial & mat = ci->second;
	
	audio::SampleHandle sample_id = mat.next();
	arx_assert(sample_id != audio::SampleHandle());
	
	audio::Channel channel(ARX_SOUND_MixerGameSample);
	channel.flags = FLAG_VOLUME | FLAG_PITCH | FLAG_POSITION | FLAG_REVERBERATION | FLAG_FALLOFF;
	
	res::path sample_name;
	audio::getSampleName(sample_id, sample_name);
	float presence = GetSamplePresenceFactor(sample_name);
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * presence;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND * presence;
	
	// Launch 'ON HEAR' script event
	ARX_NPC_SpawnAudibleSound(position, source, power, presence);
	
	if(g_camera && fartherThan(g_camera->m_pos, position, ARX_SOUND_REFUSE_DISTANCE))
		return;
	
	channel.position = position;
	channel.pitch = Random::getf(0.975f, 1.475f);
	channel.volume = volume;
	
	audio::samplePlay(sample_id, channel);
}

audio::SourcedSample ARX_SOUND_PlayScript(const res::path & name, bool & tooFar, const Entity * io,
                                     float pitch, SoundLoopMode loop) {
	
	if(!g_soundInitialized) {
		return audio::SourcedSample();
	}
	
	audio::SampleHandle sample_id = audio::createSample(sample_path / name);
	if (sample_id == audio::SampleHandle()) {
		return audio::SourcedSample();
	}
	
	audio::Channel channel(ARX_SOUND_MixerGameSample);
	channel.flags = FLAG_VOLUME | FLAG_AUTOFREE | FLAG_POSITION | FLAG_REVERBERATION | FLAG_FALLOFF;
	channel.volume = 1.f;
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART * GetSamplePresenceFactor(name);
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND;
	
	if(io) {
		channel.position = GetItemWorldPositionSound(io);
		if(loop != ARX_SOUND_PLAY_LOOPED) {
			if (g_camera && fartherThan(g_camera->m_pos, channel.position, ARX_SOUND_REFUSE_DISTANCE)) {
				// TODO the sample will never be freed!
				tooFar = true;
				return audio::SourcedSample();
			}
		}
	} else {
		channel.flags |= FLAG_RELATIVE;
		channel.position = Vec3f(0.f, 0.f, 100.f);
	}
	
	if(pitch != 1.f) {
		channel.flags |= FLAG_PITCH;
		channel.pitch = pitch;
	}
	
	return audio::samplePlay(sample_id, channel, loop);
}

void ARX_SOUND_PlayAnim(audio::SampleHandle sample_id, const Vec3f * position) {
	
	if(!g_soundInitialized || sample_id == audio::SampleHandle()) {
		return;
	}
	
	audio::Channel channel(ARX_SOUND_MixerGameSample);
	channel.flags = FLAG_VOLUME;
	channel.volume = 1.f;

	if(position) {
		if(g_camera && fartherThan(g_camera->m_pos, *position, ARX_SOUND_REFUSE_DISTANCE)) {
			return;
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
}

audio::SourcedSample ARX_SOUND_PlayCinematic(const res::path & name, bool isSpeech) {
	
	res::path file = (isSpeech) ? speechFileName(name) : (sample_path / name);
	file.set_ext(ARX_SOUND_FILE_EXTENSION_WAV);
	
	audio::SampleHandle sample_id = audio::createSample(file);
	if(sample_id == audio::SampleHandle()) {
		LogError << "Cannot load sound for cinematic: " << file;
		return audio::SourcedSample();
	}
	
	audio::Channel channel(ARX_SOUND_MixerGameSpeech);
	channel.flags = FLAG_VOLUME | FLAG_AUTOFREE | FLAG_POSITION | FLAG_FALLOFF
	                | FLAG_REVERBERATION;
	channel.volume = 1.0f;
	channel.falloff.start = ARX_SOUND_DEFAULT_FALLSTART;
	channel.falloff.end = ARX_SOUND_DEFAULT_FALLEND;
	
	if(g_camera) {
		std::pair<Vec3f, Vec3f> frontUp = angleToFrontUpVec(g_camera->angle);
		ARX_SOUND_SetListener(g_camera->m_pos, frontUp.first, frontUp.second);
	}
	
	channel.position = ARX_SOUND_IOFrontPos(NULL);
	
	return audio::samplePlay(sample_id, channel);
}

bool ARX_SOUND_IsPlaying(audio::SourcedSample & sample_id) {
	return g_soundInitialized ? audio::isSamplePlaying(sample_id) : false;
}


GameDuration ARX_SOUND_GetDuration(audio::SampleHandle sample_id) {
	
	if(g_soundInitialized && sample_id != audio::SampleHandle()) {
		size_t length;
		audio::getSampleLength(sample_id, length);
		return GameDurationMs(length);
	}

	return 0;
}

void ARX_SOUND_RefreshVolume(audio::SourcedSample & sample_id, float volume) {
	if(g_soundInitialized && sample_id != audio::SourcedSample()) {
		audio::setSampleVolume(sample_id, volume);
	}
}

void ARX_SOUND_RefreshPitch(audio::SourcedSample & sample_id, float pitch) {
	if(g_soundInitialized && sample_id != audio::SourcedSample()) {
		audio::setSamplePitch(sample_id, pitch);
	}
}

void ARX_SOUND_RefreshPosition(audio::SourcedSample & sample_id, const Vec3f & position) {
	
	if(g_soundInitialized && sample_id != audio::SourcedSample()) {
		audio::setSamplePosition(sample_id, position);
	}
}

void ARX_SOUND_RefreshSpeechPosition(audio::SourcedSample & sample_id, const Entity * io) {
	
	if(!g_soundInitialized || !io || sample_id == audio::SourcedSample()) {
		return;
	}
	
	Vec3f position = io->pos;
	if((io == entities.player() && !EXTERNALVIEW)) {
		position = ARX_SOUND_IOFrontPos(io);
	}
	
	audio::setSamplePosition(sample_id, position);
}

audio::SampleHandle ARX_SOUND_Load(const res::path & name) {
	
	if(!g_soundInitialized) {
		return audio::SampleHandle();
	}
	
	res::path sample_name = sample_path / name;
	
	return audio::createSample(sample_name.set_ext(ARX_SOUND_FILE_EXTENSION_WAV));
}

void ARX_SOUND_Free(const audio::SampleHandle & sample) {
	if(g_soundInitialized && sample != audio::SampleHandle()) {
		audio::deleteSample(sample);
	}
}

void ARX_SOUND_Stop(const audio::SourcedSample & sample_id) {
	if(g_soundInitialized && sample_id != audio::SourcedSample()) {
		audio::sampleStop(sample_id);
	}
}

bool ARX_SOUND_PlayScriptAmbiance(const res::path & name, SoundLoopMode loop, float volume) {
	
	if(!g_soundInitialized) {
		return true;
	}
	
	res::path temp = res::path(name).set_ext("amb");
	
	AmbianceId ambiance_id = audio::getAmbiance(temp);
	
	if(ambiance_id == AmbianceId()) {
		if(volume == 0.f) {
			return true;
		}
		
		ambiance_id = audio::createAmbiance(temp, audio::PLAYING_AMBIANCE_SCRIPT);
		if(ambiance_id == AmbianceId()) {
			return false;
		}
		
		audio::Channel channel(ARX_SOUND_MixerGameAmbiance);
		channel.flags = FLAG_VOLUME | FLAG_AUTOFREE;
		channel.volume = volume;
		
		audio::ambiancePlay(ambiance_id, channel, loop == ARX_SOUND_PLAY_LOOPED);
	} else {
		if(volume <= 0.f) {
			audio::deleteAmbiance(ambiance_id);
			return true;
		}
		audio::setAmbianceVolume(ambiance_id, volume);
	}
	
	return true;
}

bool ARX_SOUND_PlayZoneAmbiance(const res::path & name, SoundLoopMode loop, float volume) {
	
	if (!g_soundInitialized) return true;

	if(name == "none") {
		audio::ambianceStop(g_zoneAmbiance, AMBIANCE_FADE_TIME);
		g_zoneAmbiance = AmbianceId();
		return true;
	}

	res::path temp = res::path(name).set_ext("amb");

	AmbianceId ambiance_id = audio::getAmbiance(temp);

	if(ambiance_id == AmbianceId()) {
		ambiance_id = audio::createAmbiance(temp, audio::PLAYING_AMBIANCE_ZONE);
		if(ambiance_id == AmbianceId()) {
			return false;
		}
	} else if(ambiance_id == g_zoneAmbiance) {
		return true;
	}

	audio::Channel channel(ARX_SOUND_MixerGameAmbiance);
	channel.flags = FLAG_VOLUME | FLAG_AUTOFREE;
	channel.volume = volume;

	audio::ambianceStop(g_zoneAmbiance, AMBIANCE_FADE_TIME);
	g_zoneAmbiance = ambiance_id;
	audio::ambiancePlay(g_zoneAmbiance, channel, loop == ARX_SOUND_PLAY_LOOPED, AMBIANCE_FADE_TIME);

	return true;
}

void ARX_SOUND_KillAmbiances() {
	
	if(!g_soundInitialized) {
		return;
	}
	
	audio::deleteAmbianceAll();
	
	g_zoneAmbiance = AmbianceId();
}

AmbianceId ARX_SOUND_PlayMenuAmbiance(const res::path & ambiance_name) {
	
	if(!g_soundInitialized) {
		return AmbianceId();
	}
	
	audio::deleteAmbiance(g_menuAmbiance);
	g_menuAmbiance = audio::createAmbiance(ambiance_name, audio::PLAYING_AMBIANCE_MENU);
	
	audio::Channel channel(ARX_SOUND_MixerMenuAmbiance);
	channel.flags = FLAG_VOLUME;
	channel.volume = 1.f;
	
	audio::ambiancePlay(g_menuAmbiance, channel, true);
	
	return g_menuAmbiance;
}

std::string ARX_SOUND_AmbianceSavePlayList() {
	
	std::string result;
	
	std::vector<audio::AmbianceInfo> infos;
	audio::getAmbianceInfos(infos);
	
	BOOST_FOREACH(const audio::AmbianceInfo & info, infos) {
		
		if(info.type != audio::PLAYING_AMBIANCE_SCRIPT && info.type != audio::PLAYING_AMBIANCE_ZONE) {
			continue;
		}
		
		result.resize(result.size() + sizeof(PlayingAmbiance));
		
		char * data = &result[result.size() - sizeof(PlayingAmbiance)];
		PlayingAmbiance * playing = reinterpret_cast<PlayingAmbiance *>(data);
		
		std::memset(playing->name, 0, sizeof(playing->name));
		arx_assert(info.name.string().length() + 1 < ARRAY_SIZE(playing->name));
		util::storeString(playing->name, info.name.string());
		
		playing->volume = info.volume;
		
		playing->loop = info.isLooped ?  ARX_SOUND_PLAY_LOOPED : ARX_SOUND_PLAY_ONCE;
		
		playing->type = (info.type == audio::PLAYING_AMBIANCE_SCRIPT ? 1 : 2);
		
	}
	
	return result;
}

void ARX_SOUND_AmbianceRestorePlayList(const char * playlist, size_t size) {
	
	size_t count = size / sizeof(PlayingAmbiance);
	const PlayingAmbiance * play_list = reinterpret_cast<const PlayingAmbiance *>(playlist);
	
	for(size_t i = 0; i < count; i++) {
		
		const PlayingAmbiance * playing = &play_list[i];
		
		res::path name = res::path::load(util::loadString(playing->name));
		
		switch(playing->type) {
			
			case 1:
				ARX_SOUND_PlayScriptAmbiance(name, SoundLoopMode(playing->loop), playing->volume);
				break;
			
			case 2:
				ARX_SOUND_PlayZoneAmbiance(name, SoundLoopMode(playing->loop), playing->volume);
				break;
			
			default:
				LogWarning << "Unknown ambiance type " << playing->type << " for " << name;
			
		}
	}
}

static void ARX_SOUND_CreateEnvironments() {
	
	PakDirectory * dir = g_resources->getDirectory(ARX_SOUND_PATH_ENVIRONMENT);
	if(!dir) {
		return;
	}
	
	for(PakDirectory::files_iterator i = dir->files_begin(); i != dir->files_end(); i++) {
		audio::createEnvironment(i->first);
	}
}

static audio::SampleHandle createEffectSample(const res::path & path) {
	return audio::createSample(sample_path / path);
}

static void ARX_SOUND_CreateStaticSamples() {
	
	// Menu
	g_snd.MENU_CLICK                     = createEffectSample("menu_click.wav");
	g_snd.MENU_RELEASE                   = createEffectSample("menu_release.wav");
	
	// Interface
	g_snd.BACKPACK                       = createEffectSample("interface_backpack.wav");
	g_snd.BOOK_OPEN                      = createEffectSample("book_open.wav");
	g_snd.BOOK_CLOSE                     = createEffectSample("book_close.wav");
	g_snd.BOOK_PAGE_TURN                 = createEffectSample("book_page_turn.wav");
	g_snd.SCROLL_OPEN                    = createEffectSample("scroll_open.wav");
	g_snd.SCROLL_CLOSE                   = createEffectSample("scroll_close.wav");
	g_snd.INVSTD                         = createEffectSample("interface_invstd.wav");
	g_snd.GOLD                           = createEffectSample("drop_coin.wav");
	
	// Player
	g_snd.PLAYER_HEART_BEAT              = createEffectSample("player_heartb.wav");
	g_snd.PLAYER_LEVEL_UP                = createEffectSample("player_level_up.wav");
	g_snd.PLAYER_POISONED                = createEffectSample("player_poisoned.wav");
	g_snd.PLAYER_DEATH_BY_FIRE           = createEffectSample("lava_death.wav");
	
	// Other SFX samples
	g_snd.TORCH_START                    = createEffectSample("torch_start.wav");
	g_snd.TORCH_LOOP                     = createEffectSample("sfx_torch_11khz.wav");
	g_snd.TORCH_END                      = createEffectSample("torch_end.wav");
	g_snd.FIREPLACE_LOOP                 = createEffectSample("fire_place.wav");
	g_snd.PLOUF                          = createEffectSample("fishing_plouf.wav");
	g_snd.WHOOSH                         = createEffectSample("whoosh07.wav");
	g_snd.DISMEMBER                      = createEffectSample("flesh_critical.wav");
	
	// Magic draw
	g_snd.MAGIC_AMBIENT_LOOP             = createEffectSample("magic_ambient.wav");
	g_snd.MAGIC_DRAW_LOOP                = createEffectSample("magic_draw.wav");
	g_snd.MAGIC_FIZZLE                   = createEffectSample("magic_fizzle.wav");
	
	// Magic symbols
	g_snd.SYMB[RUNE_AAM]                 = createEffectSample("magic_aam.wav");
	g_snd.SYMB[RUNE_CETRIUS]             = createEffectSample("magic_citrius.wav");
	g_snd.SYMB[RUNE_COSUM]               = createEffectSample("magic_cosum.wav");
	g_snd.SYMB[RUNE_COMUNICATUM]         = createEffectSample("magic_comunicatum.wav");
	g_snd.SYMB[RUNE_FOLGORA]             = createEffectSample("magic_folgora.wav");
	g_snd.SYMB[RUNE_FRIDD]               = createEffectSample("magic_fridd.wav");
	g_snd.SYMB[RUNE_KAOM]                = createEffectSample("magic_kaom.wav");
	g_snd.SYMB[RUNE_MEGA]                = createEffectSample("magic_mega.wav");
	g_snd.SYMB[RUNE_MORTE]               = createEffectSample("magic_morte.wav");
	g_snd.SYMB[RUNE_MOVIS]               = createEffectSample("magic_movis.wav");
	g_snd.SYMB[RUNE_NHI]                 = createEffectSample("magic_nhi.wav");
	g_snd.SYMB[RUNE_RHAA]                = createEffectSample("magic_rhaa.wav");
	g_snd.SYMB[RUNE_SPACIUM]             = createEffectSample("magic_spacium.wav");
	g_snd.SYMB[RUNE_STREGUM]             = createEffectSample("magic_stregum.wav");
	g_snd.SYMB[RUNE_TAAR]                = createEffectSample("magic_taar.wav");
	g_snd.SYMB[RUNE_TEMPUS]              = createEffectSample("magic_tempus.wav");
	g_snd.SYMB[RUNE_TERA]                = createEffectSample("magic_tera.wav");
	g_snd.SYMB[RUNE_VISTA]               = createEffectSample("magic_vista.wav");
	g_snd.SYMB[RUNE_VITAE]               = createEffectSample("magic_vitae.wav");
	g_snd.SYMB[RUNE_YOK]                 = createEffectSample("magic_yok.wav");
	
	// Spells
	g_snd.SPELL_ACTIVATE_PORTAL          = createEffectSample("magic_spell_activate_portal.wav");
	g_snd.SPELL_ARMOR_START              = createEffectSample("magic_spell_armor_start.wav");
	g_snd.SPELL_ARMOR_END                = createEffectSample("magic_spell_armor_end.wav");
	g_snd.SPELL_ARMOR_LOOP               = createEffectSample("magic_spell_armor_loop.wav");
	g_snd.SPELL_LOWER_ARMOR              = createEffectSample("magic_spell_decrease_armor.wav");
	g_snd.SPELL_LOWER_ARMOR_END          = createEffectSample("magic_spell_lower_armor.wav");
	g_snd.SPELL_BLESS                    = createEffectSample("magic_spell_bless.wav");
	g_snd.SPELL_COLD_PROTECTION_START    = createEffectSample("magic_spell_cold_protection.wav");
	g_snd.SPELL_COLD_PROTECTION_LOOP     = createEffectSample("magic_spell_cold_protection_loop.wav");
	g_snd.SPELL_COLD_PROTECTION_END      = createEffectSample("magic_spell_cold_protection_end.wav");
	g_snd.SPELL_CONFUSE                  = createEffectSample("magic_spell_confuse.wav");
	g_snd.SPELL_CONTROL_TARGET           = createEffectSample("magic_spell_control_target.wav");
	g_snd.SPELL_CREATE_FIELD             = createEffectSample("magic_spell_create_field.wav");
	g_snd.SPELL_CREATE_FOOD              = createEffectSample("magic_spell_create_food.wav");
	g_snd.SPELL_CURE_POISON              = createEffectSample("magic_spell_cure_poison.wav");
	g_snd.SPELL_CURSE                    = createEffectSample("magic_spell_curse.wav");
	g_snd.SPELL_DETECT_TRAP              = createEffectSample("magic_spell_detect_trap.wav");
	g_snd.SPELL_DETECT_TRAP_LOOP         = createEffectSample("magic_spell_detect_trap_loop.wav");
	g_snd.SPELL_DISARM_TRAP              = createEffectSample("magic_spell_disarm_trap.wav");
	g_snd.SPELL_DISPELL_FIELD            = createEffectSample("magic_spell_dispell_field.wav");
	g_snd.SPELL_DISPELL_ILLUSION         = createEffectSample("magic_spell_dispell_illusion.wav");
	g_snd.SPELL_DOUSE                    = createEffectSample("magic_spell_douse.wav");
	g_snd.SPELL_ELECTRIC                 = createEffectSample("sfx_electric.wav");
	g_snd.SPELL_EXPLOSION                = createEffectSample("magic_spell_explosion.wav");
	g_snd.SPELL_EYEBALL_IN               = createEffectSample("magic_spell_eyeball_in.wav");
	g_snd.SPELL_EYEBALL_OUT              = createEffectSample("magic_spell_eyeball_out.wav");
	g_snd.SPELL_FIRE_HIT                 = createEffectSample("magic_spell_firehit.wav");
	g_snd.SPELL_FIRE_LAUNCH              = createEffectSample("magic_spell_firelaunch.wav");
	g_snd.SPELL_FIRE_PROTECTION          = createEffectSample("magic_spell_fire_protection.wav");
	g_snd.SPELL_FIRE_PROTECTION_LOOP     = createEffectSample("magic_spell_fire_protection_loop.wav");
	g_snd.SPELL_FIRE_PROTECTION_END      = createEffectSample("magic_spell_fire_protection_end.wav");
	g_snd.SPELL_FIRE_WIND_LOOP           = createEffectSample("magic_spell_firewind.wav");
	g_snd.SPELL_FREEZETIME               = createEffectSample("magic_spell_freezetime.wav");
	g_snd.SPELL_HARM                     = createEffectSample("magic_spell_harm.wav");
	g_snd.SPELL_HEALING                  = createEffectSample("magic_spell_healing.wav");
	g_snd.SPELL_ICE_FIELD                = createEffectSample("magic_spell_ice_field.wav");
	g_snd.SPELL_ICE_FIELD_LOOP           = createEffectSample("magic_spell_ice_fieldloop.wav");
	g_snd.SPELL_ICE_FIELD_END            = createEffectSample("magic_spell_ice_field_end.wav");
	g_snd.SPELL_ICE_PROJECTILE_LAUNCH    = createEffectSample("magic_spell_ice_projectile_launch.wav");
	g_snd.SPELL_INCINERATE               = createEffectSample("magic_spell_incinerate.wav");
	g_snd.SPELL_INCINERATE_LOOP          = createEffectSample("magic_spell_incinerate_loop.wav");
	g_snd.SPELL_INCINERATE_END           = createEffectSample("magic_spell_incinerate_end.wav");
	g_snd.SPELL_IGNITE                   = createEffectSample("magic_spell_ignite.wav");
	g_snd.SPELL_INVISIBILITY_START       = createEffectSample("magic_spell_invisibilityon.wav");
	g_snd.SPELL_INVISIBILITY_END         = createEffectSample("magic_spell_invisibilityoff.wav");
	g_snd.SPELL_LEVITATE_START           = createEffectSample("magic_spell_levitate_start.wav");
	g_snd.SPELL_LEVITATE_LOOP            = createEffectSample("magic_spell_levitate_loop.wav");
	g_snd.SPELL_LEVITATE_END             = createEffectSample("magic_spell_levitate_end.wav");
	g_snd.SPELL_LIGHTNING_START          = createEffectSample("magic_spell_lightning_start.wav");
	g_snd.SPELL_LIGHTNING_LOOP           = createEffectSample("magic_spell_lightning_loop.wav");
	g_snd.SPELL_LIGHTNING_END            = createEffectSample("magic_spell_lightning_end.wav");
	g_snd.SPELL_MAGICAL_HIT              = createEffectSample("magic_spell_magicalhit.wav");
	
	g_snd.SPELL_FIRE_FIELD_START         = createEffectSample("magic_spell_fire_field.wav");
	g_snd.SPELL_FIRE_FIELD_LOOP          = createEffectSample("magic_spell_fire_field_loop.wav");
	g_snd.SPELL_FIRE_FIELD_END           = createEffectSample("magic_spell_fire_field_end.wav");
	
	g_snd.SPELL_MAGICAL_SHIELD_LOOP      = createEffectSample("magic_spell_magicalshield.wav");
	g_snd.SPELL_MASS_INCINERATE          = createEffectSample("magic_spell_mass_incinerate.wav");
	g_snd.SPELL_MASS_PARALYSE            = createEffectSample("magic_spell_mass_paralyse.wav");
	g_snd.SPELL_MM_CREATE                = createEffectSample("magic_spell_missilecreate.wav");
	g_snd.SPELL_MM_HIT                   = createEffectSample("magic_spell_missilehit.wav");
	g_snd.SPELL_MM_LAUNCH                = createEffectSample("magic_spell_missilelaunch.wav");
	g_snd.SPELL_MM_LOOP                  = createEffectSample("magic_spell_missileloop.wav");
	g_snd.SPELL_NEGATE_MAGIC             = createEffectSample("magic_spell_negate_magic.wav");
	g_snd.SPELL_PARALYSE                 = createEffectSample("magic_spell_paralyse.wav");
	g_snd.SPELL_PARALYSE_END             = createEffectSample("magic_spell_paralyse_end.wav");
	g_snd.SPELL_POISON_PROJECTILE_LAUNCH = createEffectSample("magic_spell_poison_projectile_launch.wav");
	g_snd.SPELL_RAISE_DEAD               = createEffectSample("magic_spell_raise_dead.wav");
	g_snd.SPELL_REPEL_UNDEAD             = createEffectSample("magic_spell_repel_undead.wav");
	g_snd.SPELL_REPEL_UNDEAD_LOOP        = createEffectSample("magic_spell_repell_loop.wav");
	g_snd.SPELL_RUNE_OF_GUARDING         = createEffectSample("magic_spell_rune_of_guarding.wav");
	g_snd.SPELL_RUNE_OF_GUARDING_END     = createEffectSample("magic_spell_rune_of_guarding_explode.wav");
	g_snd.SPELL_SLOW_DOWN                = createEffectSample("magic_spell_slow_down.wav");
	g_snd.SPELL_SLOW_DOWN_END            = createEffectSample("magic_spell_slow_down_end.wav");
	g_snd.SPELL_SPARK                    = createEffectSample("sfx_spark.wav");
	g_snd.SPELL_SPEED_START              = createEffectSample("magic_spell_speedstart.wav");
	g_snd.SPELL_SPEED_LOOP               = createEffectSample("magic_spell_speed.wav");
	g_snd.SPELL_SPEED_END                = createEffectSample("magic_spell_speedend.wav");
	g_snd.SPELL_SUMMON_CREATURE          = createEffectSample("magic_spell_summon_creature.wav");
	g_snd.SPELL_TELEKINESIS_START        = createEffectSample("magic_spell_telekinesison.wav");
	g_snd.SPELL_TELEKINESIS_END          = createEffectSample("magic_spell_telekinesisoff.wav");
	g_snd.SPELL_VISION_START             = createEffectSample("magic_spell_vision2.wav");
	g_snd.SPELL_VISION_LOOP              = createEffectSample("magic_spell_vision.wav");
}

// Reset each static sample to audio::SourcedSample()
// Those samples are freed from memory when by audio::clean() is deleted
static void ARX_SOUND_ReleaseStaticSamples() {
	g_snd = StaticSamples();
}

const char * ARX_MATERIAL_GetNameById(Material id) {
	
	switch(id) {
		case MATERIAL_NONE:   return "none";
		case MATERIAL_WEAPON: return "weapon";
		case MATERIAL_FLESH:  return "flesh";
		case MATERIAL_METAL:  return "metal";
		case MATERIAL_GLASS:  return "glass";
		case MATERIAL_CLOTH:  return "cloth";
		case MATERIAL_WOOD:   return "wood";
		case MATERIAL_EARTH:  return "earth";
		case MATERIAL_WATER:  return "water";
		case MATERIAL_ICE:    return "ice";
		case MATERIAL_GRAVEL: return "gravel";
		case MATERIAL_STONE:  return "stone";
		case MATERIAL_FOOT_LARGE:   return "foot_large";
		case MATERIAL_FOOT_BARE:    return "foot_bare";
		case MATERIAL_FOOT_SHOE:    return "foot_shoe";
		case MATERIAL_FOOT_METAL:   return "foot_metal";
		case MATERIAL_FOOT_STEALTH: return "foot_stealth";
		case MAX_MATERIALS: arx_unreachable();
	}
	return "none";
}

static void ARX_SOUND_CreateCollisionMaps() {
	
	collisionMaps.clear();
	
	for(size_t i = 0; i < ARRAY_SIZE(ARX_SOUND_COLLISION_MAP_NAMES); i++) {
		
		res::path file = ARX_SOUND_PATH_INI / ARX_SOUND_COLLISION_MAP_NAMES[i];
		file.set_ext(ARX_SOUND_FILE_EXTENSION_INI);
		
		std::string data = g_resources->read(file);
		if(data.empty()) {
			LogWarning << "Could not read collision map " << file;
			return;
		}
		
		std::istringstream iss(data);
		
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
					
					std::ostringstream oss;
					oss << boost::to_lower_copy(key.getValue());
					if(mi) {
						oss << mi;
					}
					oss << ARX_SOUND_FILE_EXTENSION_WAV;
					audio::SampleHandle sample = audio::createSample(sample_path / oss.str());
					
					if(sample == audio::SampleHandle()) {
						std::ostringstream oss2;
						oss2 << boost::to_lower_copy(key.getValue()) << '_' << mi << ARX_SOUND_FILE_EXTENSION_WAV;
						sample = audio::createSample(sample_path / oss2.str());
					}
					
					if(sample != audio::SampleHandle()) {
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

static void ARX_SOUND_CreateMaterials() {
	
	for(size_t i = 0; i < size_t(MAX_MATERIALS); i++) {
		for(size_t j = 0; j < size_t(MAX_MATERIALS); j++) {
			g_soundMaterials[i][j] = audio::SampleHandle();
		}
	}
	
	std::ostringstream oss;
	for(Material i = MATERIAL_WEAPON; i <= MATERIAL_STONE; i = Material(i + 1)) {
		for(Material j = i; j <= MATERIAL_STONE; j = Material(j + 1)) {
			oss.str(std::string());
			oss << ARX_MATERIAL_GetNameById(i) << "_on_" << ARX_MATERIAL_GetNameById(j) << "_1.wav";
			g_soundMaterials[j][i] = g_soundMaterials[i][j] = audio::createSample(sample_path / oss.str());
		}
	}
	
}


static void ARX_SOUND_CreatePresenceMap() {
	
	g_presenceFactors.clear();
	
	res::path file = (ARX_SOUND_PATH_INI / ARX_SOUND_PRESENCE_NAME).set_ext(ARX_SOUND_FILE_EXTENSION_INI);
	
	std::string data = g_resources->read(file);
	if(data.empty()) {
		LogWarning << "Could not read presence map " << file;
		return;
	}
	
	std::istringstream iss(data);
	
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
		g_presenceFactors[res::path::load(i->getName()).set_ext(ARX_SOUND_FILE_EXTENSION_WAV)] = factor;
	}
	
}

static float GetSamplePresenceFactor(const res::path & name) {
	
	arx_assert_msg(name.string().find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == std::string::npos,
	               "bad sample name: \"%s\"", name.string().c_str());
	
	PresenceFactors::const_iterator it = g_presenceFactors.find(name);
	if(it != g_presenceFactors.end()) {
		return it->second;
	}
	
	return 1.f;
}
