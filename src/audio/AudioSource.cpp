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

#include "audio/AudioSource.h"

#include <algorithm>

#include "audio/AudioGlobal.h"
#include "audio/Sample.h"

namespace audio {

Source::Source(Sample * sample)
	: m_channel(MixerId())
	, m_sample(sample)
	, status(Idle)
	, time(0)
	, callback_i(0)
{
	m_sample->reference();
}

Source::~Source() {
	m_sample->dereference();
}

void Source::addCallback(Callback * callback, size_t position) {
	
	size_t pos = std::min(position, m_sample->getLength());
	
	size_t i = 0;
	while(i != callbacks.size() && callbacks[i].second <= pos) {
		i++;
	}
	
	if(i <= callback_i && !callbacks.empty()) {
		callback_i++;
	}
	
	callbacks.insert(callbacks.begin() + i, std::make_pair(callback, pos));
	
}

aalError Source::update() {
	
	if(status != Playing || updateCulling()) {
		return AAL_OK;
	}
	
	aalError ret = updateBuffers();
	
	updateCallbacks();
	
	return ret;
}

void Source::updateCallbacks() {
	
	while(true) {
		
		// Check if it's time to launch a callback
		for(; callback_i != callbacks.size() && callbacks[callback_i].second <= time; callback_i++) {
			callbacks[callback_i].first->onSamplePosition(*this, callbacks[callback_i].second);
		}
		
		if(time < m_sample->getLength()) {
			break;
		}
		
		time -= m_sample->getLength();
		callback_i = 0;
		
		if(!time && status != Playing) {
			// Prevent callback for time==0 being called again after playing.
			break;
		} else {
			// TODO race condition in OpenALSource if sample completed between alGetSourcei(AL_BUFFERS_QUEUED) and alGetSourcei(AL_BYTE_OFFSET)? can alGetSourcei(AL_BYTE_OFFSET) ever be at the end?
		}
		
	}
	
}

aalError Source::setVolume(float volume) {
	
	if(!(m_channel.flags & FLAG_VOLUME)) {
		return AAL_ERROR_INIT;
	}
	
	m_channel.volume = glm::clamp(volume, 0.f, 1.f);
	
	return updateVolume();
}

aalError Source::setMixer(MixerId mixer) {
	
	m_channel.mixer = mixer;
	
	return updateVolume();
}

} // namespace audio
