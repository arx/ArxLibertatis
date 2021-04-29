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

#ifndef ARX_AUDIO_OPENAL_OPENALBACKEND_H
#define ARX_AUDIO_OPENAL_OPENALBACKEND_H

#include "Configure.h"

#include <vector>

#include <al.h>
#include <alc.h>
#if ARX_HAVE_OPENAL_EFX
#include <efx.h>
#endif

#include "audio/AudioBackend.h"
#include "audio/AudioTypes.h"
#include "audio/AudioResource.h"
#include "math/Types.h"

namespace audio {

class OpenALSource;

class OpenALBackend : public Backend {
	
public:
	
	OpenALBackend();
	~OpenALBackend();
	
	aalError init(const char * device = NULL);
	
	std::vector<std::string> getDevices();
	
	Source * createSource(SampleId sampleId, const Channel & channel);
	
	Source * getSource(SourceId sourceId);
	
	aalError setReverbEnabled(bool enable);
	bool isReverbSupported();
	
	aalError setUnitFactor(float factor);
	aalError setRolloffFactor(float factor);
	
	aalError setListenerPosition(const Vec3f & position);
	aalError setListenerOrientation(const Vec3f & front, const Vec3f & up);
	
	aalError setListenerEnvironment(const Environment & env);
	
	source_iterator sourcesBegin();
	source_iterator sourcesEnd();
	source_iterator deleteSource(source_iterator it);
	
private:
	
	static const char * shortenDeviceName(const char * deviceName);
	
	ALCdevice * device;
	ALCcontext * context;
	
	#if ARX_HAVE_OPENAL_EFX
	
	bool hasEFX;
	
	LPALGENEFFECTS alGenEffects;
	LPALDELETEEFFECTS alDeleteEffects;
	LPALEFFECTI alEffecti;
	LPALEFFECTF alEffectf;
	
	LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
	LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
	LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
	
	bool effectEnabled;
	ALuint effect;
	ALuint effectSlot;
	
	#endif
	
	ResourceList<OpenALSource> sources;
	
	float rolloffFactor;
	
	friend class OpenALSource;
};

} // namespace audio

#endif // ARX_AUDIO_OPENAL_OPENALBACKEND_H
