/*
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

#include "audio/AudioGlobal.h"
#include "audio/AudioBackend.h"
#include "audio/AudioSource.h"

namespace audio {

Mixer::Mixer() :
	paused(false),
	volume(AAL_DEFAULT_VOLUME),
	parent(NULL),
	finalVolume(AAL_DEFAULT_VOLUME)
{
}

Mixer::~Mixer() {
	
	for(aalULong i = 0; i < _mixer.Size(); i++) {
		if(_mixer[i] && _mixer[i]->parent == this) {
			_mixer.Delete(i);
		}
	}
	
	clear(true);
}

void Mixer::clear(bool force) {
	
	for(aalULong i = 0; i < _amb.Size(); i++) {
		Ambiance * ambiance = _amb[i];
		if(ambiance && _mixer[ambiance->channel.mixer] == this) {
			if(force || ambiance->channel.flags & AAL_FLAG_AUTOFREE) {
				_amb.Delete(i);
			} else {
				ambiance->Stop();
			}
		}
	}
	
	// Delete sources referencing this mixer.
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd();) {
		if(*p && _mixer.IsValid((*p)->getChannel().mixer) && _mixer[(*p)->getChannel().mixer] == this) {
			p = backend->deleteSource(p);
		} else {
			++p;
		}
	}
	
}

aalError Mixer::setVolume(float v) {
	
	float vol = clamp(v, 0.f, 1.f);
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
	
	for(aalULong i = 0; i < _mixer.Size(); i++) {
		if(_mixer[i] && _mixer[i]->parent == this) {
			_mixer[i]->updateVolume();
		}
	}
	
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd(); ++p) {
		if(*p && _mixer.IsValid((*p)->getChannel().mixer) && _mixer[(*p)->getChannel().mixer] == this) {
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
	
	for(aalULong i = 0; i < _mixer.Size(); i++) {
		Mixer * mixer = _mixer[i];
		if(mixer && mixer->parent == this) {
			mixer->stop();
		}
	}
	
	clear(false);
	
	paused = false;
	
	return AAL_OK;
}

aalError Mixer::pause() {
	
	for(aalULong i = 0; i < _mixer.Size(); i++) {
		if(_mixer[i] && _mixer[i]->parent == this) {
			_mixer[i]->pause();
		}
	}
	
	for(aalULong i = 0; i < _amb.Size(); i++) {
		if(_amb[i] && _mixer[_amb[i]->channel.mixer] == this) {
			_amb[i]->Pause();
		}
	}
	
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd(); ++p) {
		if(*p && _mixer.IsValid((*p)->getChannel().mixer) && _mixer[(*p)->getChannel().mixer] == this) {
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
	
	for(aalULong i = 0; i < _mixer.Size(); i++) {
		if(_mixer[i] && _mixer[i]->parent == this) {
			_mixer[i]->resume();
		}
	}
	
	for(aalULong i = 0; i < _amb.Size(); i++) {
		if(_amb[i] && _mixer[_amb[i]->channel.mixer] == this) {
			_amb[i]->Resume();
		}
	}
	
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd(); ++p) {
		if(*p && _mixer.IsValid((*p)->getChannel().mixer) && _mixer[(*p)->getChannel().mixer] == this) {
			(*p)->resume();
		}
	}
	
	paused = false;
	
	return AAL_OK;
}

} // namespace audio
