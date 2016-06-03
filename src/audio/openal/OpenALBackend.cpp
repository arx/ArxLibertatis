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

#include "audio/openal/OpenALBackend.h"

#include <stddef.h>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <sstream>

#if ARX_HAVE_SETENV || ARX_HAVE_UNSETENV
#include <stdlib.h>
#endif

#include <boost/scope_exit.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#include "audio/openal/OpenALSource.h"
#include "audio/openal/OpenALUtils.h"
#include "audio/AudioEnvironment.h"
#include "audio/AudioGlobal.h"
#include "audio/AudioSource.h"
#include "core/Version.h"
#include "gui/Credits.h"
#include "io/log/Logger.h"
#include "math/Vector.h"
#include "platform/Platform.h"
#include "platform/CrashHandler.h"

namespace audio {

class Sample;

#undef ALError
#define ALError LogError

OpenALBackend::OpenALBackend()
	: device(NULL)
	, context(NULL)
	#if ARX_HAVE_OPENAL_EFX
	, hasEFX(false)
	, alGenEffects(NULL)
	, alDeleteEffects(NULL)
	, alEffecti(NULL)
	, alEffectf(NULL)
	, alGenAuxiliaryEffectSlots(NULL)
	, alDeleteAuxiliaryEffectSlots(NULL)
	, alAuxiliaryEffectSloti(NULL)
	, effectEnabled(false)
	, effect(AL_EFFECT_NULL)
	, effectSlot(AL_EFFECTSLOT_NULL)
	#endif
	, rolloffFactor(1.f)
{}

OpenALBackend::~OpenALBackend() {
	
	sources.clear();
	
	if(context) {
		
		alcDestroyContext(context);
		
		ALenum error = alcGetError(device);
		if(error != AL_NO_ERROR) {
			LogError << "Error destroying OpenAL context: " << error << " = " << getAlcErrorString(error);
		}
	}
	
	if(device) {
		if(alcCloseDevice(device) == ALC_FALSE) {
			LogError << "Error closing device";
		}
	}
}

#if ARX_HAVE_OPENAL_EFX
namespace {
class al_function_ptr {
	void * m_func;
public:
	explicit al_function_ptr(void * func) : m_func(func) { }
	template <typename T>
	operator T() { return (T)m_func; }
};
} // anonymous namespace
#endif

static const char * const deviceNamePrefixOpenALSoft = "OpenAL Soft on ";

const char * OpenALBackend::shortenDeviceName(const char * deviceName) {
	
	if(deviceName && boost::starts_with(deviceName, deviceNamePrefixOpenALSoft)) {
		// TODO do this for the name displayed in the menu as well?
		deviceName += std::strlen(deviceNamePrefixOpenALSoft);
	}
	
	return deviceName;
}

aalError OpenALBackend::init(const char * requestedDeviceName) {
	
	if(device) {
		return AAL_ERROR_INIT;
	}
	
	#if ARX_PLATFORM != ARX_PLATFORM_WIN32 && ARX_HAVE_SETENV && ARX_HAVE_UNSETENV
	/*
	 * OpenAL Soft does not provide a way to pass through these properties, so use
	 * environment variables.
	 * Unfortunately it also always clears out PA_PROP_MEDIA_ROLE :(
	 */
	const char * oldClass = std::getenv("PULSE_PROP_application.icon_name");
	if(!oldClass) {
		setenv("PULSE_PROP_application.icon_name", arx_icon_name.c_str(), 1);
	}
	const char * oldName = std::getenv("PULSE_PROP_application.name");
	if(!oldName) {
		setenv("PULSE_PROP_application.name", arx_name.c_str(), 1);
	}
	BOOST_SCOPE_EXIT((oldClass)(oldName)) {
		// Don't the properties of pulse child processes
		if(!oldClass) {
			unsetenv("PULSE_PROP_application.icon_name");
		}
		if(!oldName) {
			unsetenv("PULSE_PROP_application.name");
		}
	} BOOST_SCOPE_EXIT_END
	#endif
	
	// Clear error
	ALenum error = alGetError();
	ARX_UNUSED(error);
	
	// Create OpenAL interface
	device = alcOpenDevice(requestedDeviceName);
	if(!device && requestedDeviceName) {
		std::string fullDeviceName = deviceNamePrefixOpenALSoft;
		fullDeviceName += requestedDeviceName;
		device = alcOpenDevice(fullDeviceName.c_str());
	}
	if(!device) {
		ALenum error = alcGetError(NULL);
		if(error != ALC_INVALID_VALUE) {
			LogError << "Error opening device: " << error << " = " << getAlcErrorString(error);
		}
		return AAL_ERROR_SYSTEM;
	}
	
	context = alcCreateContext(device, NULL);
	if(!context) {
		ALenum error = alcGetError(device);
		LogError << "Error creating OpenAL context: " << error << " = " << getAlcErrorString(error);
		return AAL_ERROR_SYSTEM;
	}
	alcMakeContextCurrent(context);
	
	#if ARX_HAVE_OPENAL_EFX
	hasEFX = (alcIsExtensionPresent(device, "ALC_EXT_EFX") != ALC_FALSE);
	if(hasEFX) {
		#define ARX_AL_LOAD_FUNC(Name) \
			Name = al_function_ptr(alGetProcAddress(ARX_STR(Name))); \
			hasEFX = hasEFX && Name != NULL
		ARX_AL_LOAD_FUNC(alGenEffects);
		ARX_AL_LOAD_FUNC(alDeleteEffects);
		ARX_AL_LOAD_FUNC(alEffecti);
		ARX_AL_LOAD_FUNC(alEffectf);
		ARX_AL_LOAD_FUNC(alGenAuxiliaryEffectSlots);
		ARX_AL_LOAD_FUNC(alDeleteAuxiliaryEffectSlots);
		ARX_AL_LOAD_FUNC(alAuxiliaryEffectSloti);
		#undef ARX_AL_LOAD_FUNC
	}
	#endif
	
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	
	AL_CHECK_ERROR("initializing")
	
	const ALchar * version = alGetString(AL_VERSION);
	#if ARX_HAVE_OPENAL_EFX
	if(hasEFX) {
		ALCint major = 0, minor = 0;
		alcGetIntegerv(device, ALC_EFX_MAJOR_VERSION, 1, &major);
		alcGetIntegerv(device, ALC_EFX_MINOR_VERSION, 1, &minor);
		LogInfo << "Using OpenAL " << version << " with EFX " << major << '.' << minor;
	}
	else
	#endif
	{
		LogInfo << "Using OpenAL " << version << " without EFX";
	}
	CrashHandler::setVariable("OpenAL version", version);
	{
		const char * start, * end, * type;
		for(int i = 0; i < 2; i++) {
			if(i == 0) {
				start = std::strstr(version, "ALSOFT");
				if(!start) {
					continue;
				}
				start += 6;
				type = "OpenAL Soft ";
			} else {
				start = version;
				type = "OpenAL ";
			}
			while(*start == ' ') {
				start++;
			}
			end = start;
			while(*end != '\0' && *end != ' ') {
				end++;
			}
			if(start != end) {
				break;
			}
		}
		std::ostringstream oss;
		oss << type;
		oss.write(start, end - start);
		credits::setLibraryCredits("audio", oss.str());
	}
	
	const char * vendor = alGetString(AL_VENDOR);
	LogInfo << " ├─ Vendor: " << vendor;
	CrashHandler::setVariable("OpenAL vendor", vendor);
	
	const char * renderer = alGetString(AL_RENDERER);
	LogInfo << " ├─ Renderer: " << renderer;
	CrashHandler::setVariable("OpenAL renderer", renderer);
	
	const char * deviceName = alcGetString(device, ALC_DEVICE_SPECIFIER);
	#ifdef ALC_ENUMERATE_ALL_EXT
	ALCboolean hasDetailedDevices = alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT");
	if(hasDetailedDevices != ALC_FALSE && !std::strcmp(deviceName, "OpenAL Soft")) {
		/*
		 * OpenAL Soft hides the extended device name since version 1.14.
		 * Instead, queries for ALC_ALL_DEVICES_SPECIFIER with a valid device
		 * will return the extended name of that device. Both old OpenAL Soft
		 * and Creative OpenAL return the extended device name in ALC_DEVICE_SPECIFIER
		 * and always return a list of all devices for ALC_ALL_DEVICES_SPECIFIER
		 * even if a valid device is given. Since the only specification I can find for
		 * ALC_ENUMERATE_ALL_EXT [1] doesn't say anything about using a device
		 * with ALC_ALL_DEVICES_SPECIFIER, only do that if ALC_DEVICE_SPECIFIER is useless.
		 *  [1] http://icculus.org/alextreg/wiki/ALC_ENUMERATE_ALL_EXT
		 */
		deviceName = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
	}
	#endif
	deviceName = shortenDeviceName(deviceName);
	if(!deviceName || *deviceName == '\0') {
		deviceName = "(unknown)";
	}
	LogInfo << " └─ Device: " << deviceName;
	CrashHandler::setVariable("OpenAL device", deviceName);
	
	LogDebug("AL extensions: " << alGetString(AL_EXTENSIONS));
	LogDebug("ALC extensions: " << alcGetString(device, ALC_EXTENSIONS));
	
	return AAL_OK;
}

std::vector<std::string> OpenALBackend::getDevices() {
	
	std::vector<std::string> result;
	
	const char * devices = NULL;
	
	#ifdef ALC_ENUMERATE_ALL_EXT
	ALCboolean hasDetailedDevices = alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT");
	if(hasDetailedDevices != ALC_FALSE) {
		devices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
	}
	#endif
	
	if(!devices) {
		devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
	}
	
	while(devices && *devices) {
		devices = shortenDeviceName(devices);
		result.push_back(devices);
		devices += result.back().length() + 1;
	}
	
	return result;
}

Source * OpenALBackend::createSource(SampleId sampleId, const Channel & channel) {
	
	SampleId s_id = getSampleId(sampleId);
	
	if(!_sample.isValid(s_id)) {
		return NULL;
	}
	
	Sample * sample = _sample[s_id];
	
	OpenALSource * orig = NULL;
	for(size_t i = 0; i < sources.size(); i++) {
		if(sources[i] && sources[i]->getSample() == sample) {
			orig = (OpenALSource*)sources[i];
			break;
		}
	}
	
	OpenALSource * source = new OpenALSource(sample);
	
	size_t index = sources.add(source);
	if(index == (size_t)INVALID_ID) {
		delete source;
		return NULL;
	}
	
	SourceId id = (index << 16) | s_id;
	if(source->init(id, orig, channel)) {
		sources.remove(index);
		return NULL;
	}
	
	source->setRolloffFactor(rolloffFactor);
	
	#if ARX_HAVE_OPENAL_EFX
	if(effectSlot != AL_EFFECTSLOT_NULL) {
		source->setEffectSlot(effectSlot);
	}
	#endif
	
	return source;
}

Source * OpenALBackend::getSource(SourceId sourceId) {
	
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

aalError OpenALBackend::setRolloffFactor(float factor) {
	
	rolloffFactor = factor;
	for(size_t i = 0; i < sources.size(); i++) {
		if(sources[i]) {
			sources[i]->setRolloffFactor(rolloffFactor);
		}
	}
	
	return AAL_OK;
}

aalError OpenALBackend::setListenerPosition(const Vec3f & position) {
	
	arx_assert(isallfinite(position));
	
	if(!isallfinite(position)) {
		return AAL_ERROR; // OpenAL soft will lock up if given NaN or +-Inf here
	}
	
	alListener3f(AL_POSITION, position.x, position.y, position.z);
	AL_CHECK_ERROR("setting listener posiotion")
	
	return AAL_OK;
}

aalError OpenALBackend::setListenerOrientation(const Vec3f & front, const Vec3f & up) {
	
	arx_assert(isallfinite(front) && isallfinite(up));
	
	if(!isallfinite(front) || !isallfinite(up)) {
		return AAL_ERROR; // OpenAL soft will lock up if given NaN or +-Inf here
	}
	
	ALfloat orientation[] = {front.x, front.y, front.z, -up.x, -up.y, -up.z};
	alListenerfv(AL_ORIENTATION, orientation);
	AL_CHECK_ERROR("setting listener orientation")
	
	return AAL_OK;
}

Backend::source_iterator OpenALBackend::sourcesBegin() {
	return (source_iterator)sources.begin();
}

Backend::source_iterator OpenALBackend::sourcesEnd() {
	return (source_iterator)sources.end();
}

Backend::source_iterator OpenALBackend::deleteSource(source_iterator it) {
	arx_assert(it >= sourcesBegin() && it < sourcesEnd());
	return (source_iterator)sources.remove((ResourceList<OpenALSource>::iterator)it);
}

aalError OpenALBackend::setUnitFactor(float factor) {
	
#if ARX_HAVE_OPENAL_EFX
	if(hasEFX) {
		alListenerf(AL_METERS_PER_UNIT, factor);
		AL_CHECK_ERROR("setting unit factor")
	}
#endif
	
	const float speedOfSoundInMetersPerSecond = 343.3f; // Default for OpenAL
	
	float speedOfSoundInUnits = speedOfSoundInMetersPerSecond / factor;
	
	if(!(boost::math::isfinite)(speedOfSoundInUnits)) {
		return AAL_ERROR; // OpenAL soft will lock up if given NaN or +-Inf here
	}
	
	alSpeedOfSound(speedOfSoundInUnits);
	AL_CHECK_ERROR("scaling speed of sound to unit factor")
	
	return AAL_OK;
}

#if ARX_HAVE_OPENAL_EFX

aalError OpenALBackend::setReverbEnabled(bool enable) {
	
	if(effectEnabled == enable) {
		return AAL_OK;
	}
	
	if(!hasEFX) {
		LogWarning << "Cannot enable effects, missing the EFX extension";
		return AAL_ERROR_SYSTEM;
	}
	
	if(enable) {
		alGenEffects(1, &effect);
		alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
		alGenAuxiliaryEffectSlots(1, &effectSlot);
		AL_CHECK_ERROR_N("creating effect",
			enable = false;
		);
	}
	
	for(size_t i = 0; i < sources.size(); i++) {
		if(sources[i]) {
			sources[i]->setEffectSlot(enable ? effectSlot : AL_EFFECTSLOT_NULL);
		}
	}
	
	if(!enable) {
		alDeleteEffects(1, &effect);
		effect = AL_EFFECT_NULL;
		alDeleteAuxiliaryEffectSlots(1, &effectSlot);
		effectSlot = AL_EFFECTSLOT_NULL;
		AL_CHECK_ERROR("deleting effect");
	}
	
	
	effectEnabled = enable;
	
	return AAL_OK;
}

bool OpenALBackend::isReverbSupported() {
	
	if(!hasEFX) {
		return false;
	} else if(effectEnabled) {
		return true;
	}
	
	// Clear error state
	(void)alGetError();
	
	ALuint dummy;
	alGenEffects(1, &dummy);
	alEffecti(dummy, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
	alDeleteEffects(1, &dummy);
	
	return alGetError() == AL_NO_ERROR;
}

aalError OpenALBackend::setListenerEnvironment(const Environment & env) {
	
	if(!effectEnabled) {
		return AAL_ERROR_INIT;
	}
	
	LogDebug("Using environment " << env.name << ":"
		<< "\nsize = " << env.size
		<< "\ndiffusion = " << env.diffusion
		<< "\nabsorption = " << env.absorption
		<< "\nreflect_volume = " << env.reflect_volume
		<< "\nreflect_delay = " << env.reflect_delay
		<< "\nreverb_volume = " << env.reverb_volume
		<< "\nreverb_delay = " << env.reverb_delay
		<< "\nreverb_decay = " << env.reverb_decay
		<< "\nreverb_hf_decay = " << env.reverb_hf_decay
	);
	
	#define ARX_AL_REVERB_SET(Property, Value) \
		float raw ## Property = (Value); \
		float al ## Property = glm::clamp(raw ## Property, AL_REVERB_MIN_ ## Property, \
		                                                   AL_REVERB_MAX_ ## Property); \
		if(al ## Property != raw ## Property) { \
			LogWarning << "Clamping REVERB_" << ARX_STR(Property) << " from " \
			           << raw ## Property << " to " << al ## Property; \
		} \
		alEffectf(effect, AL_REVERB_ ## Property, al ## Property); \
		AL_CHECK_ERROR_N("setting REVERB_" << ARX_STR(Property) << " to " << al ## Property,)
	
	ARX_AL_REVERB_SET(ROOM_ROLLOFF_FACTOR, rolloffFactor);
	ARX_AL_REVERB_SET(DENSITY, 1.f);
	ARX_AL_REVERB_SET(GAIN, 1.f);
	ARX_AL_REVERB_SET(GAINHF, 0.8f);
	ARX_AL_REVERB_SET(DIFFUSION, env.diffusion);
	ARX_AL_REVERB_SET(AIR_ABSORPTION_GAINHF, std::pow(10.f, env.absorption * -0.05f));
	ARX_AL_REVERB_SET(REFLECTIONS_GAIN, env.reflect_volume);
	ARX_AL_REVERB_SET(REFLECTIONS_DELAY, env.reflect_delay * 0.001f);
	ARX_AL_REVERB_SET(LATE_REVERB_GAIN, env.reverb_volume);
	ARX_AL_REVERB_SET(LATE_REVERB_DELAY, env.reverb_delay * 0.001f);
	ARX_AL_REVERB_SET(DECAY_TIME, env.reverb_decay * 0.001f);
	ARX_AL_REVERB_SET(DECAY_HFRATIO, env.reverb_hf_decay / env.reverb_decay);
	
	#undef ARX_AL_REVERB_SET
	
	/*
	 * With OpenAL Soft this call must come *after* setting up all properties on
	 * the effect object.
	 */
	alAuxiliaryEffectSloti(effectSlot, AL_EFFECTSLOT_EFFECT, effect);
	
	return AAL_OK;
}

#else // !ARX_HAVE_OPENAL_EFX

aalError OpenALBackend::setReverbEnabled(bool enable) {
	return enable ? AAL_ERROR_SYSTEM : AAL_OK;
}

bool OpenALBackend::isReverbSupported() {
	return false;
}

aalError OpenALBackend::setListenerEnvironment(const Environment & env) {
	ARX_UNUSED(env);
	return AAL_ERROR_INIT;
}

#endif // !ARX_HAVE_OPENAL_EFX

} // namespace audio
