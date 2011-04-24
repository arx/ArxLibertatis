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

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "audio/AudioGlobal.h"
#include "audio/AudioBackend.h"
#include "audio/AudioSource.h"

using namespace std;

namespace audio {

	const aalULong MIXER_PAUSED = 0x00000001;

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Constrcutor and destructor                                                //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	Mixer::Mixer() :
		name(NULL), status(0), flags(0),
		volume(AAL_DEFAULT_VOLUME),
		pitch(AAL_DEFAULT_PITCH),
		pan(AAL_DEFAULT_PAN),
		parent(NULL)
	{
	}

Mixer::~Mixer() {
	
	for(aalULong i = 0; i < _mixer.Size(); i++) {
		if(_mixer[i] && _mixer[i]->parent == this) {
			_mixer.Delete(i);
		}
	}
	
	clear(true);
	
	free(name);
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

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Setup                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError Mixer::SetName(const char * _name)
	{
		void * ptr;

		if (!_name)
		{
			free(name), name = NULL;
			return AAL_OK;
		}

		ptr = realloc(name, strlen(_name) + 1);

		if (!ptr) return AAL_ERROR_MEMORY;

		name = (char *)ptr;

		strcpy(name, _name);

		return AAL_OK;
	}

aalError Mixer::SetVolume(const aalFloat & v) {
	
	volume = v > 1.f ? 1.f : v < 0.f ? 0.f : v;
	
	for(aalULong i = 0; i < _mixer.Size(); i++) {
		if(_mixer[i] && _mixer[i]->parent == this) {
			_mixer[i]->SetVolume(_mixer[i]->volume);
		}
	}
	
	for(Backend::source_iterator p = backend->sourcesBegin(); p != backend->sourcesEnd(); ++p) {
		if(*p && _mixer.IsValid((*p)->getChannel().mixer) && _mixer[(*p)->getChannel().mixer] == this) {
			(*p)->updateVolume();
		}
	}
	
	return AAL_OK;
}

	aalError Mixer::SetParent(const Mixer * _mixer)
	{
		const Mixer * mixer = _mixer;

		while (mixer)
		{
			if (mixer == this) return AAL_ERROR;

			mixer = mixer->parent;
		}

		parent = _mixer;
		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Status                                                                    //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError Mixer::GetVolume(aalFloat & _volume) const
	{
		_volume = volume;
		return AAL_OK;
	}

	aalUBool Mixer::IsPaused() const
	{
		return status & MIXER_PAUSED ? AAL_UTRUE : AAL_UFALSE;
	}

// Control

aalError Mixer::Stop() {
	
	for(aalULong i = 0; i < _mixer.Size(); i++) {
		Mixer * mixer = _mixer[i];
		if(mixer && mixer->parent == this) {
			mixer->Stop();
		}
	}
	
	clear(false);
	
	status &= ~MIXER_PAUSED;
	
	return AAL_OK;
}

aalError Mixer::Pause() {
	
	for(aalULong i = 0; i < _mixer.Size(); i++) {
		if(_mixer[i] && _mixer[i]->parent == this) {
			_mixer[i]->Pause();
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
	
	status |= MIXER_PAUSED;
	
	return AAL_OK;
}

aalError Mixer::Resume() {
	
	if(!(status & MIXER_PAUSED)) {
		return AAL_OK;
	}
	
	for(aalULong i = 0; i < _mixer.Size(); i++) {
		if(_mixer[i] && _mixer[i]->parent == this) {
			_mixer[i]->Resume();
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
	
	status &= ~MIXER_PAUSED;
	
	return AAL_OK;
}

} // namespace audio
