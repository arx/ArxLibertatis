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
#include <math.h>
#include "Athena_Mixer.h"
#include "Athena_Global.h"
#include "Athena_Instance.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

namespace ATHENA
{

	static enum MixerFlag
	{
		IS_PAUSED = 0x00000001
	};

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

	Mixer::~Mixer()
	{
		aalULong i;

		for (i = 0; i < _mixer.Size(); i++)
			if (_mixer[i] && _mixer[i]->parent == this)
				_mixer.Delete(i);

		for (i = 0; i < _inst.Size(); i++)
			if (_inst[i] &&
			        _inst[i]->IsPlaying() &&
			        _mixer.IsValid(_inst[i]->channel.mixer) &&
			        _mixer[_inst[i]->channel.mixer] == this)
				_inst.Delete(i);

		for (i = 0; i < _amb.Size(); i++)
			if (_amb[i] &&
			        _amb[i]->IsPlaying() &&
			        _mixer.IsValid(_amb[i]->channel.mixer) &&
			        _mixer[_amb[i]->channel.mixer] == this)
				_amb.Delete(i);

		free(name);
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Setup                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError Mixer::SetName(const char * _name)
	{
		aalVoid * ptr;

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

	aalError Mixer::SetVolume(const aalFloat & v)
	{
		aalULong i;

		volume = v > 1.0F ? 1.0F : v < 0.0F ? 0.0F : v;

		for (i = 0; i < _mixer.Size(); i++)
			if (_mixer[i] && _mixer[i]->parent == this)
				_mixer[i]->SetVolume(_mixer[i]->volume);

		for (i = 0; i < _inst.Size(); i++)
			if (_inst[i] && _mixer[_inst[i]->channel.mixer] == this)
				_inst[i]->SetVolume(_inst[i]->channel.volume);

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
		return status & IS_PAUSED ? AAL_UTRUE : AAL_UFALSE;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Control                                                                   //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError Mixer::Stop()
	{
		aalULong i;

		for (i = 0; i < _mixer.Size(); i++)
		{
			Mixer * mixer = _mixer[i];

			if (mixer && mixer->parent == this)
				mixer->Stop();
		}

		for (i = 0; i < _amb.Size(); i++)
		{
			Ambiance * ambiance = _amb[i];

			if (ambiance && _mixer[ambiance->channel.mixer] == this)
			{
				ambiance->Stop();

				if (ambiance->channel.flags & AAL_FLAG_AUTOFREE)
					_amb.Delete(i);
			}
		}

		for (i = 0; i < _inst.Size(); i++)
		{
			Instance * instance = _inst[i];

			if (instance && _mixer[instance->channel.mixer] == this)
				_inst.Delete(i);
		}

		status &= ~IS_PAUSED;

		return AAL_OK;
	}

	aalError Mixer::Pause()
	{
		aalULong i;

		for (i = 0; i < _mixer.Size(); i++)
			if (_mixer[i] && _mixer[i]->parent == this)
				_mixer[i]->Pause();

		for (i = 0; i < _amb.Size(); i++)
			if (_amb[i] && _mixer[_amb[i]->channel.mixer] == this)
				_amb[i]->Pause();

		for (i = 0; i < _inst.Size(); i++)
			if (_inst[i] && _mixer[_inst[i]->channel.mixer] == this)
				_inst[i]->Pause();

		status |= IS_PAUSED;
		return AAL_OK;
	}

	aalError Mixer::Resume()
	{
		if (!(status & IS_PAUSED)) return AAL_OK;

		aalULong i;

		for (i = 0; i < _mixer.Size(); i++)
			if (_mixer[i] && _mixer[i]->parent == this)
				_mixer[i]->Resume();

		for (i = 0; i < _amb.Size(); i++)
			if (_amb[i] && _mixer[_amb[i]->channel.mixer] == this)
				_amb[i]->Resume();

		for (i = 0; i < _inst.Size(); i++)
			if (_inst[i] && _mixer[_inst[i]->channel.mixer] == this)
				_inst[i]->Resume();

		status &= ~IS_PAUSED;
		return AAL_OK;
	}

}//ATHENA::
