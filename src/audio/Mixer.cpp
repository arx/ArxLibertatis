/*
 * Copyright 2011-2015 Arx Libertatis Team (see the AUTHORS file)
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

#include "audio/AudioGlobal.h"
#include "audio/AudioBackend.h"
#include "audio/AudioResource.h"
#include "audio/AudioSource.h"
#include "audio/Ambiance.h"

namespace audio {

Mixer::Mixer() :
	paused(false),
	volume(DEFAULT_VOLUME),
	parent(NULL),
	finalVolume(DEFAULT_VOLUME) {
}

Mixer::~Mixer() {
	
	for(size_t i = 0; i < g_mixers.size(); i++) {
		if(g_mixers[i] && g_mixers[i]->parent == this) {
			g_mixers.remove(i);
		}
	}
	
	clear(true);
}

void Mixer::clear(bool force) {
	
	for(size_t i = 0; i < g_ambiances.size(); i++) {
		Ambiance * ambiance = g_ambiances[i];
		if(ambiance && g_mixers[ambiance->getChannel().mixer.handleData()] == this) {
			if(force || ambiance->getChannel().flags & FLAG_AUTOFREE) {
				g_ambiances.remove(i);
			} else {
				ambiance->stop();
			}
		}
	}
	
	// Delete sources referencing this mixer.
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd();) {
		if(*p && g_mixers.isValid((*p)->getChannel().mixer.handleData())
		   && g_mixers[(*p)->getChannel().mixer.handleData()] == this) {
			p = backend->deleteSource(p);
		} else {
			++p;
		}
	}
	
}

aalError Mixer::setVolume(float v) {
	
	float vol = glm::clamp(v, 0.f, 1.f);
	if(volume == vol) {
		return AAL_OK;
	}
	volume = vol;
	
	updateVolume();
	
	return AAL_OK;
}

void Mixer::updateVolume() {
	
	float vol = parent ? parent->finalVolume * volume : volume;
	if(finalVolume == vol) {
		return;
	}
	finalVolume = vol;
	
	for(size_t i = 0; i < g_mixers.size(); i++) {
		if(g_mixers[i] && g_mixers[i]->parent == this) {
			g_mixers[i]->updateVolume();
		}
	}
	
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd(); ++p) {
		if(*p && g_mixers.isValid((*p)->getChannel().mixer.handleData()) && g_mixers[(*p)->getChannel().mixer.handleData()] == this) {
			(*p)->updateVolume();
		}
	}
	
}

aalError Mixer::setParent(const Mixer * _mixer) {
	
	if(parent == _mixer) {
		return AAL_OK;
	}
	
	// Check for cyles.
	const Mixer * mixer = _mixer;
	while(mixer) {
		if(mixer == this) {
			return AAL_ERROR;
		}
		mixer = mixer->parent;
	}
	
	parent = _mixer;
	
	updateVolume();
	
	if(parent && !paused && parent->paused) {
		pause();
	}
	
	return AAL_OK;
}

aalError Mixer::stop() {
	
	for(size_t i = 0; i < g_mixers.size(); i++) {
		Mixer * mixer = g_mixers[i];
		if(mixer && mixer->parent == this) {
			mixer->stop();
		}
	}
	
	clear(false);
	
	paused = false;
	
	return AAL_OK;
}

aalError Mixer::pause() {
	
	for(size_t i = 0; i < g_mixers.size(); i++) {
		if(g_mixers[i] && g_mixers[i]->parent == this) {
			g_mixers[i]->pause();
		}
	}
	
	for(size_t i = 0; i < g_ambiances.size(); i++) {
		if(g_ambiances[i] && g_mixers[g_ambiances[i]->getChannel().mixer.handleData()] == this) {
			g_ambiances[i]->pause();
		}
	}
	
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd(); ++p) {
		if(*p && g_mixers.isValid((*p)->getChannel().mixer.handleData())
		   && g_mixers[(*p)->getChannel().mixer.handleData()] == this) {
			(*p)->pause();
		}
	}
	
	paused = true;
	
	return AAL_OK;
}

aalError Mixer::resume() {
	
	if(!paused) {
		return AAL_OK;
	}
	
	for(size_t i = 0; i < g_mixers.size(); i++) {
		if(g_mixers[i] && g_mixers[i]->parent == this) {
			g_mixers[i]->resume();
		}
	}
	
	for(size_t i = 0; i < g_ambiances.size(); i++) {
		if(g_ambiances[i] && g_mixers[g_ambiances[i]->getChannel().mixer.handleData()] == this) {
			g_ambiances[i]->resume();
		}
	}
	
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd(); ++p) {
		if(*p && g_mixers.isValid((*p)->getChannel().mixer.handleData())
		   && g_mixers[(*p)->getChannel().mixer.handleData()] == this) {
			(*p)->resume();
		}
	}
	
	paused = false;
	
	return AAL_OK;
}

} // namespace audio
