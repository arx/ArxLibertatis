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

#include "audio/Mixer.h"

#include <stddef.h>

#include <boost/foreach.hpp>

#include "audio/AudioGlobal.h"
#include "audio/AudioBackend.h"
#include "audio/AudioResource.h"
#include "audio/AudioSource.h"
#include "audio/Ambiance.h"

namespace audio {

Mixer::Mixer(const Mixer * parent)
	: paused(false)
	, m_volume(DEFAULT_VOLUME)
	, m_parent(parent)
	, finalVolume(DEFAULT_VOLUME)
{
	updateVolume();
	
	if(m_parent && !paused && m_parent->paused) {
		pause();
	}
}

Mixer::~Mixer() {
	
	for(MixerList::iterator i = g_mixers.begin(); i != g_mixers.end();) {
		if(*i && (*i)->m_parent == this) {
			i = g_mixers.remove(i);
		} else {
			++i;
		}
	}
	
	clear(true);
}

void Mixer::clear(bool force) {
	
	for(AmbianceList::iterator i = g_ambiances.begin(); i != g_ambiances.end();) {
		Ambiance * ambiance = *i;
		if(ambiance && g_mixers[ambiance->getChannel().mixer] == this) {
			if(force || (ambiance->getChannel().flags & FLAG_AUTOFREE)) {
				i = g_ambiances.remove(i);
			} else {
				ambiance->stop();
				++i;
			}
		} else {
			++i;
		}
	}
	
	// Delete sources referencing this mixer.
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd();) {
		if(*p && g_mixers.isValid((*p)->getChannel().mixer)
		   && g_mixers[(*p)->getChannel().mixer] == this) {
			p = backend->deleteSource(p);
		} else {
			++p;
		}
	}
	
}

void Mixer::setVolume(float volume) {
	
	volume = glm::clamp(volume, 0.f, 1.f);
	if(m_volume == volume) {
		return;
	}
	m_volume = volume;
	
	updateVolume();
}

void Mixer::updateVolume() {
	
	float volume = m_parent ? m_parent->finalVolume * m_volume : m_volume;
	if(finalVolume == volume) {
		return;
	}
	finalVolume = volume;
	
	BOOST_FOREACH(Mixer * mixer, g_mixers) {
		if(mixer && mixer->m_parent == this) {
			mixer->updateVolume();
		}
	}
	
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd(); ++p) {
		if(*p && g_mixers.isValid((*p)->getChannel().mixer) && g_mixers[(*p)->getChannel().mixer] == this) {
			(*p)->updateVolume();
		}
	}
	
}

void Mixer::stop() {
	
	BOOST_FOREACH(Mixer * mixer, g_mixers) {
		if(mixer && mixer->m_parent == this) {
			mixer->stop();
		}
	}
	
	clear(false);
	
	paused = false;
}

void Mixer::pause() {
	
	BOOST_FOREACH(Mixer * mixer, g_mixers) {
		if(mixer && mixer->m_parent == this) {
			mixer->pause();
		}
	}
	
	BOOST_FOREACH(Ambiance * ambiance, g_ambiances) {
		if(ambiance && g_mixers[ambiance->getChannel().mixer] == this) {
			ambiance->pause();
		}
	}
	
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd(); ++p) {
		if(*p && g_mixers.isValid((*p)->getChannel().mixer)
		   && g_mixers[(*p)->getChannel().mixer] == this) {
			(*p)->pause();
		}
	}
	
	paused = true;
}

void Mixer::resume() {
	
	if(!paused) {
		return;
	}
	
	BOOST_FOREACH(Mixer * mixer, g_mixers) {
		if(mixer && mixer->m_parent == this) {
			mixer->resume();
		}
	}
	
	BOOST_FOREACH(Ambiance * ambiance, g_ambiances) {
		if(ambiance && g_mixers[ambiance->getChannel().mixer] == this) {
			ambiance->resume();
		}
	}
	
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd(); ++p) {
		if(*p && g_mixers.isValid((*p)->getChannel().mixer)
		   && g_mixers[(*p)->getChannel().mixer] == this) {
			(*p)->resume();
		}
	}
	
	paused = false;
}

} // namespace audio
