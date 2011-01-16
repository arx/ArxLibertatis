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
#include <stdio.h>
#include <eax.h>
#include <math.h>
#include "Athena_Environment.h"
#include "Athena_Global.h"
#include "Athena_FileIO.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

namespace ATHENA
{

	static const aalReflection DEFAULT_REFLECTION = { 0.8F, 7 };
	static const aalReverberation DEFAULT_REVERBERATION = { 1.02F, 11, 1490, 1236 };

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Constructor and destructor                                                //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	Environment::Environment() :
		name(NULL),
		size(AAL_DEFAULT_ENVIRONMENT_SIZE),
		rolloff(0.0F),
		diffusion(AAL_DEFAULT_ENVIRONMENT_DIFFUSION),
		absorption(AAL_DEFAULT_ENVIRONMENT_ABSORPTION),
		reflect_volume(AAL_DEFAULT_ENVIRONMENT_REFLECTION_VOLUME),
		reflect_delay(aalFloat(AAL_DEFAULT_ENVIRONMENT_REFLECTION_DELAY)),
		reverb_volume(AAL_DEFAULT_ENVIRONMENT_REVERBERATION_VOLUME),
		reverb_delay(aalFloat(AAL_DEFAULT_ENVIRONMENT_REVERBERATION_DELAY)),
		reverb_decay(aalFloat(AAL_DEFAULT_ENVIRONMENT_REVERBERATION_DECAY)),
		reverb_hf_decay(aalFloat(AAL_DEFAULT_ENVIRONMENT_REVERBERATION_HFDECAY)),
		callback(NULL),
		lpksps(NULL)
	{
	}

	Environment::~Environment()
	{
		free(name);
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// File I/O                                                                  //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError Environment::Load(const char * _name)
	{
		FILE * file = OpenResource(_name, environment_path);

		if (!file) return AAL_ERROR_FILEIO;

		if (!FileRead(&size, 4, 1, file) ||
		        !FileRead(&diffusion, 4, 1, file) ||
		        !FileRead(&absorption, 4, 1, file) ||
		        !FileRead(&reflect_volume, 4, 1, file) ||
		        !FileRead(&reflect_delay, 4, 1, file) ||
		        !FileRead(&reverb_volume, 4, 1, file) ||
		        !FileRead(&reverb_delay, 4, 1, file) ||
		        !FileRead(&reverb_decay, 4, 1, file) ||
		        !FileRead(&reverb_hf_decay, 4, 1, file))
		{
			FileClose(file);
			return AAL_ERROR_FILEIO;
		}

		FileClose(file);

		SetName(_name);

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Setup                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError Environment::SetName(const char * _name)
	{
		if (_name)
		{
			aalULong length(strlen(_name) + 1);
			aalVoid * ptr = realloc(name, length);

			if (!ptr) return AAL_ERROR_MEMORY;

			name = (char *)ptr;

			memcpy(name, _name, length);
		}
		else free(name), name = NULL;

		return AAL_OK;
	}

	aalError Environment::SetRolloffFactor(const aalFloat & _factor)
	{
		rolloff = _factor < 0.0F ? 0.0F  : _factor > 10.0F ? 10.0F : _factor;

		if (lpksps)
		{
			if (lpksps->Set(DSPROPSETID_EAX_ListenerProperties,
			                DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR | DSPROPERTY_EAXLISTENER_DEFERRED,
			                NULL, 0, &rolloff, sizeof(aalFloat)))
				return AAL_ERROR_SYSTEM;
		}

		return AAL_OK;
	}

}//ATHENA::
