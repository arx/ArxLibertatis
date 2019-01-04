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

#ifndef ARX_AUDIO_AUDIOBACKEND_H
#define ARX_AUDIO_AUDIOBACKEND_H

#include <vector>

#include "audio/AudioTypes.h"

namespace audio {

class Source;
class Environment;
class Mixer;
class Sample;

/*!
 * An audio source that can play one sample.
 */
class Backend {
	
public:
	
	virtual ~Backend() { }
	
	virtual std::vector<std::string> getDevices() = 0;
	
	/*!
	 * Create a new source for the given sample and channel properties.
	 * The source is managed by the backend and should not be deleted directly.
	 * Use deleteSource to remove sources.
	 * \param sampleId The sample to be played by the new source.
	 */
	virtual Source * createSource(SampleHandle sampleId, const Channel & channel) = 0;
	
	/*!
	 * Get the source for the given id.
	 * \return the source for the given id or NULL if it doesn't exist.
	 */
	virtual Source * getSource(SourcedSample sourceId) = 0;
	
	/*!
	 * Enable or disable effects.
	 */
	virtual aalError setReverbEnabled(bool enable) = 0;
	
	/*!
	 * Check if the backend supports reverb.
	 * \return true if \ref setReverbEnabled will always fail.
	 */
	virtual bool isReverbSupported() = 0;
	
	/*!
	 * Enable or disable HRTF filter.
	 */
	virtual aalError setHRTFEnabled(HRTFAttribute enable) = 0;
	
	/*!
	 * Check if HRTF is currently enabled.
	 */
	virtual HRTFStatus getHRTFStatus() = 0;
	
	/*!
	 * Set a unit factor to scale all other distance or velocity parameters.
	 * \param factor The unit factor in meters per unit.
	 */
	virtual aalError setUnitFactor(float factor) = 0;
	
	virtual aalError setRolloffFactor(float factor) = 0;
	
	virtual aalError setListenerPosition(const Vec3f & position) = 0;
	virtual aalError setListenerOrientation(const Vec3f & front, const Vec3f & up) = 0;
	
	virtual aalError setListenerEnvironment(const Environment & env) = 0;
	
	typedef Source * const * source_iterator;
	virtual source_iterator sourcesBegin() = 0;
	virtual source_iterator sourcesEnd() = 0;
	virtual source_iterator deleteSource(source_iterator it) = 0;
};

} // namespace audio

#endif // ARX_AUDIO_AUDIOBACKEND_H
