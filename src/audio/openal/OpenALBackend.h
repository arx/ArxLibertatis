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

#ifndef ARX_AUDIO_OPENAL_OPENALBACKEND_H
#define ARX_AUDIO_OPENAL_OPENALBACKEND_H

#include "Configure.h"

#include <string_view>
#include <vector>

#include <al.h>
#include <alc.h>
#if ARX_HAVE_OPENAL_EFX
#include <efx.h>
#endif
#if ARX_HAVE_OPENAL_HRTF
#include <alext.h>
#endif

#include "audio/AudioBackend.h"
#include "audio/AudioTypes.h"
#include "audio/AudioResource.h"
#include "math/Types.h"
#include "platform/Platform.h"

namespace audio {

class OpenALSource;

class OpenALBackend final : public Backend {
	
public:
	
	OpenALBackend();
	~OpenALBackend() override;
	
	aalError init(std::string_view requestedDeviceName = std::string_view(), HRTFAttribute hrtf = HRTFDefault);
	
	std::vector<std::string> getDevices() override;
	
	Source * createSource(SampleHandle sampleId, const Channel & channel) override;
	
	Source * getSource(SourcedSample sourceId) override;
	
	aalError setReverbEnabled(bool enable) override;
	bool isReverbSupported() override;
	
	aalError setHRTFEnabled(HRTFAttribute enable) override;
	HRTFStatus getHRTFStatus() override;
	
	aalError setUnitFactor(float factor) override;
	aalError setRolloffFactor(float factor) override;
	
	aalError setListenerPosition(const Vec3f & position) override;
	aalError setListenerOrientation(const Vec3f & front, const Vec3f & up) override;
	
	aalError setListenerEnvironment(const Environment & env) override;
	
	source_iterator sourcesBegin() override;
	source_iterator sourcesEnd() override;
	source_iterator deleteSource(source_iterator it) override;
	
private:
	
	static const char * shortenDeviceName(const char * deviceName);
	
	void fillDeviceAttributes(ALCint (&attrs)[3]);
	
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
	
	#if ARX_HAVE_OPENAL_HRTF
	
	bool m_hasHRTF;
	
	LPALCRESETDEVICESOFT alcResetDeviceSOFT;
	
	HRTFAttribute m_HRTFAttribute;
	
	#endif
	
	typedef ResourceList<OpenALSource, SourceHandle> SourceList;
	SourceList sources;
	
	float rolloffFactor;
	
	friend class OpenALSource;
};

} // namespace audio

#endif // ARX_AUDIO_OPENAL_OPENALBACKEND_H
