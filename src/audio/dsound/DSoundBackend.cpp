/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include "audio/dsound/DSoundBackend.h"

#include <windows.h>

#include "core/Application.h"
#include "audio/AudioEnvironment.h"
#include "audio/AudioGlobal.h"
#include "audio/dsound/DSoundSource.h"
#include "audio/dsound/eax.h"
#include "io/log/Logger.h"
#include "window/RenderWindow.h"

namespace audio {

static const PCMFormat globalFormat = { 22050, 16, 2 };

DSoundBackend::DSoundBackend() : device(NULL), primary(NULL), listener(NULL), environment(NULL) {
	
	CoInitialize(NULL);
	
}

DSoundBackend::~DSoundBackend() {
	
	sources.clear();
	
	if(environment) {
		environment->Release(), environment = NULL;
	}
	
	if(listener) {
		listener->Release(), listener = NULL;
	}
	
	if(primary) {
		primary->Release(), primary = NULL;
	}
	
	if(device) {
		device->Release(), device = NULL;
	}
	
	CoUninitialize();
	
}


aalError DSoundBackend::init(bool enableEax) {
	
	if(device) {
		return AAL_ERROR_INIT;
	}
	
	HRESULT h;
	
	//Create DirectSound device interface
	hasEAX = enableEax;
	if(!enableEax || FAILED(CoCreateInstance(CLSID_EAXDirectSound, NULL, CLSCTX_INPROC_SERVER, IID_IDirectSound, (void**)&device))) {
		if(FAILED(h = CoCreateInstance(CLSID_DirectSound, NULL, CLSCTX_INPROC_SERVER, IID_IDirectSound, (void**)&device))) {
			LogError << "error creating DirectSound instance: " << h;
			return AAL_ERROR_SYSTEM;
		}
		hasEAX = false;
	}
	
	if(FAILED(h = device->Initialize(NULL))) {
		LogError << "error initializing DirectSound: " << h;
		return AAL_ERROR_SYSTEM;
	}
	
	if(FAILED(h = device->SetCooperativeLevel((HWND)mainApp->GetWindow()->GetHandle(), DSSCL_PRIORITY))) {
		LogError << "error setting cooperative level: " << h;
		return AAL_ERROR_SYSTEM;
	}
	
	// Create the primary buffer.
	DSBUFFERDESC desc;
	memset(&desc, 0, sizeof(DSBUFFERDESC));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRL3D;
	if(FAILED(h = device->CreateSoundBuffer(&desc, &primary, NULL))) {
		LogError << "error creating primary buffer: " << h;
		return AAL_ERROR_SYSTEM;
	}
	if(FAILED(h = primary->Play(0, 0, DSBPLAY_LOOPING))) {
		LogError << "error playing primary buffer: " << h;
		return AAL_ERROR_SYSTEM;
	}
	WAVEFORMATEX formatex;
	memset(&formatex, 0, sizeof(WAVEFORMATEX));
	formatex.wFormatTag = WAVE_FORMAT_PCM;
	formatex.nChannels = (WORD)(globalFormat.channels);
	formatex.nSamplesPerSec = globalFormat.frequency;
	formatex.wBitsPerSample = (WORD)(globalFormat.quality);
	formatex.nBlockAlign = (WORD)(globalFormat.channels * globalFormat.quality / 8);
	formatex.nAvgBytesPerSec = formatex.nBlockAlign * globalFormat.frequency;
	if(FAILED(h = primary->SetFormat(&formatex))) {
		LogError << "error setting output format: " << h;
		return AAL_ERROR_SYSTEM;
	}
	
	if(FAILED(h = primary->QueryInterface(IID_IDirectSound3DListener, (void **)&listener))) {
		LogError << "error getting listener object: " << h;
		return AAL_ERROR_SYSTEM;
	}
	
	LogInfo << "Using DirectSound " << (hasEAX ? "with" : "without") << " EAX";
	
	return AAL_OK;
}

aalError DSoundBackend::updateDeferred() {
	
	// Update output buffer with new 3D positional settings
	if(FAILED(listener->CommitDeferredSettings())) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

Source * DSoundBackend::createSource(SampleId sampleId, const Channel & channel) {
	
	SampleId s_id = getSampleId(sampleId);
	
	if(!_sample.isValid(s_id)) {
		return NULL;
	}
	
	Sample * sample = _sample[s_id];
	
	DSoundSource * orig = NULL;
	for(size_t i = 0; i < sources.size(); i++) {
		if(sources[i] && sources[i]->getSample() == sample) {
			orig = (DSoundSource*)sources[i];
			break;
		}
	}
	
	DSoundSource * source = new DSoundSource(sample, this);
	
	size_t index = sources.add(source);
	if(index == (size_t)INVALID_ID) {
		delete source;
		return NULL;
	}
	
	SourceId id = (index << 16) | s_id;
	if(orig ? source->init(id, orig, channel) : source->init(id, channel)) {
		sources.remove(index);
		return NULL;
	}
	
	return source;
}

Source * DSoundBackend::getSource(SourceId sourceId) {
	
	size_t index = ((sourceId >> 16) & 0x0000ffff);
	if(!sources.isValid(index)) {
		return NULL;
	}
	
	Source * source = sources[index];
	
	SampleId sample = getSampleId(sourceId);
	if(!_sample.isValid(sample) || source->getSample() != _sample[sample]) {
		return NULL;
	}
	
	arx_assert(source->getId() == sourceId);
	
	return source;
}

aalError DSoundBackend::setReverbEnabled(bool enable) {
	
	if(!enable) {
		if(environment) {
			environment->Release(), environment = NULL;
		}
		return AAL_OK;
	}
	
	if(environment) {
		return AAL_OK;
	}
	
	WAVEFORMATEX formatex;
	memset(&formatex, 0, sizeof(WAVEFORMATEX));
	formatex.wFormatTag = WAVE_FORMAT_PCM;
	formatex.nChannels = (WORD)(globalFormat.channels);
	formatex.nSamplesPerSec = globalFormat.frequency;
	formatex.wBitsPerSample = (WORD)(globalFormat.quality);
	formatex.nBlockAlign = (WORD)(globalFormat.channels * globalFormat.quality / 8);
	formatex.nAvgBytesPerSec = formatex.nBlockAlign * globalFormat.frequency;
	
	DSBUFFERDESC desc;
	memset(&desc, 0, sizeof(DSBUFFERDESC));
	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwBufferBytes = 10000;
	desc.dwFlags = DSBCAPS_CTRL3D;
	desc.lpwfxFormat = &formatex;
	
	LPDIRECTSOUNDBUFFER lpdsbtmp = NULL;
	if(FAILED(device->CreateSoundBuffer(&desc, &lpdsbtmp, NULL))) {
		return AAL_ERROR_SYSTEM;
	}
	
	LPDIRECTSOUND3DBUFFER lpds3dbtmp = NULL;
	if(FAILED(lpdsbtmp->QueryInterface(IID_IDirectSound3DBuffer, (void **)&lpds3dbtmp))) {
		lpdsbtmp->Release();
		return AAL_ERROR_SYSTEM;
	}
	
	if(FAILED(lpds3dbtmp->QueryInterface(IID_IKsPropertySet, (void **)&environment))) {
		lpdsbtmp->Release();
		lpds3dbtmp->Release();
		return AAL_ERROR_SYSTEM;
	}
	
	lpdsbtmp->Release();
	lpds3dbtmp->Release();
	
	ULONG support = 0;
	if(environment->QuerySupport(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS, &support) || ((support & (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)) != (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET))) {
		environment->Release(), environment = NULL;
		return AAL_ERROR_SYSTEM;
	}
	
	if(environment->QuerySupport(DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_ALLPARAMETERS, &support) || ((support & (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)) != (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET))) {
		environment->Release(), environment = NULL;
		return AAL_ERROR_SYSTEM;
	}
	
	EAXLISTENERPROPERTIES props;
	props.dwEnvironment = 0;
	props.dwFlags = EAXLISTENERFLAGS_DECAYHFLIMIT;
	props.flRoomRolloffFactor = 1.f;
	props.lRoom = 0;
	props.lRoomHF = 0;
	props.flEnvironmentSize = DEFAULT_ENVIRONMENT_SIZE;
	props.flEnvironmentDiffusion = DEFAULT_ENVIRONMENT_DIFFUSION;
	props.flAirAbsorptionHF = DEFAULT_ENVIRONMENT_ABSORPTION * -100.f;
	props.lReflections = (long)(2000 * log10(DEFAULT_ENVIRONMENT_REFLECTION_VOLUME));
	props.flReflectionsDelay = float(DEFAULT_ENVIRONMENT_REFLECTION_DELAY) * 0.001f;
	props.lReverb = (long)(2000 * log10(DEFAULT_ENVIRONMENT_REVERBERATION_VOLUME));
	props.flReverbDelay = DEFAULT_ENVIRONMENT_REVERBERATION_DELAY * 0.001f;
	props.flDecayTime = DEFAULT_ENVIRONMENT_REVERBERATION_DECAY * 0.001f;
	props.flDecayHFRatio = DEFAULT_ENVIRONMENT_REVERBERATION_HFDECAY / DEFAULT_ENVIRONMENT_REVERBERATION_DECAY;
	if(environment->Set(DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS | DSPROPERTY_EAXLISTENER_DEFERRED, NULL, 0, &props, sizeof(EAXLISTENERPROPERTIES))) {
		environment->Release(), environment = NULL;
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundBackend::setUnitFactor(float factor) {
	
	if(FAILED(listener->SetDistanceFactor(factor, DS3D_DEFERRED))) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundBackend::setRolloffFactor(float factor) {
	
	if(FAILED(listener->SetRolloffFactor(factor, DS3D_DEFERRED))) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundBackend::setListenerPosition(const Vec3f & position) {
	
	if(FAILED(listener->SetPosition(position.x, position.y, position.z, DS3D_DEFERRED))) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundBackend::setListenerOrientation(const Vec3f & front, const Vec3f & up) {
	
	if(FAILED(listener->SetOrientation(front.x, front.y, front.z, up.x, up.y, up.z, DS3D_DEFERRED))) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundBackend::setListenerEnvironment(const Environment & env) {
	
	if(!environment) {
		return AAL_ERROR_INIT;
	}
	
	EAXLISTENERPROPERTIES props;
	
	props.dwEnvironment = 0;
	props.dwFlags = EAXLISTENERFLAGS_DECAYHFLIMIT;
	props.flRoomRolloffFactor = 1.0F;
	props.lRoom = 0;
	props.lRoomHF = 0;
	props.flEnvironmentSize = env.size;
	props.flEnvironmentDiffusion = env.diffusion;
	props.flAirAbsorptionHF = env.absorption * -100.f;
	
	if(env.reverb_volume <= 0.f) {
		props.lReverb = -10000;
	} else if(env.reverb_volume >= 10.f) {
		props.lReverb = 2000;
	} else {
		props.lReverb = (long)(2000 * log10(env.reverb_volume));
	}
	
	if(env.reverb_delay >= 100.f) {
		props.flReverbDelay = 0.1f;
	} else {
		props.flReverbDelay = env.reverb_delay * 0.001f;
	}
	
	if(env.reverb_decay <= 100.f) {
		props.flDecayTime = 0.1f;
	} else if(env.reverb_decay >= 20000.f) {
		props.flDecayTime = 20.f;
	} else {
		props.flDecayTime = float(env.reverb_decay) * 0.001f;
	}
	
	props.flDecayHFRatio = env.reverb_hf_decay / env.reverb_decay;
	if(props.flDecayHFRatio <= 0.1f) {
		props.flDecayHFRatio = 0.1f;
	} else if(props.flDecayHFRatio >= 2.f) {
		props.flDecayHFRatio = 2.f;
	}
	
	if(env.reflect_volume <= 0.f) {
		props.lReflections = -10000;
	} else if(env.reflect_volume >= 3.162f) {
		props.lReflections = 1000;
	} else {
		props.lReflections = (long)(2000 * log10(env.reflect_volume));
	}
	
	if(env.reflect_delay >= 300.f) {
		props.flReflectionsDelay = 0.3f;
	} else {
		props.flReflectionsDelay = float(env.reflect_delay) * 0.001f;
	}
	
	if(FAILED(environment->Set(DSPROPSETID_EAX_ListenerProperties,
	                           DSPROPERTY_EAXLISTENER_ALLPARAMETERS | DSPROPERTY_EAXLISTENER_DEFERRED,
	                           NULL, 0, &props, sizeof(EAXLISTENERPROPERTIES)))) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

aalError DSoundBackend::setRoomRolloffFactor(float factor) {
	
	if(!environment) {
		return AAL_ERROR_INIT;
	}
	
	float rolloff = factor < 0.f ? 0.f  : factor > 10.f ? 10.f : factor;
	
	if(FAILED(environment->Set(DSPROPSETID_EAX_ListenerProperties,
			                       DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR | DSPROPERTY_EAXLISTENER_DEFERRED,
			                       NULL, 0, &rolloff, sizeof(float)))) {
		return AAL_ERROR_SYSTEM;
	}
	
	return AAL_OK;
}

Backend::source_iterator DSoundBackend::sourcesBegin() {
	return (source_iterator)sources.begin();
}

Backend::source_iterator DSoundBackend::sourcesEnd() {
	return (source_iterator)sources.end();
}

Backend::source_iterator DSoundBackend::deleteSource(source_iterator it) {
	arx_assert(it >= sourcesBegin() && it < sourcesEnd());
	return (source_iterator)sources.remove((ResourceList<DSoundSource>::iterator)it);
}

} // namespace audio
