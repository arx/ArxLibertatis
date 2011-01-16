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
#include "Athena_Global.h"
#include <time.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

namespace ATHENA
{

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Internal globals                                                          //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Audio device interface                                                    //
	LPDIRECTSOUND device(NULL);
	LPDIRECTSOUNDBUFFER primary(NULL);
	LPDIRECTSOUND3DLISTENER listener(NULL);
	LPKSPROPERTYSET environment(NULL);
	aalUBool is_reverb_present(AAL_UFALSE);
	aalSLong environment_id(AAL_SFALSE);

	// Global settings                                                           //
	char * root_path = NULL;
	char * sample_path = NULL;
	char * ambiance_path = NULL;
	char * environment_path = NULL;
	FILE * debug_log = NULL;
	aalULong stream_limit_ms(AAL_DEFAULT_STREAMLIMIT);
	aalULong stream_limit_bytes = 0;
	aalULong session_start(0);
	aalULong session_time(0);
	aalULong global_status(0);
	aalFormat global_format = { 0, 0, 0 };

	// Resources                                                                 //
	ResourceList<Mixer> _mixer;
	ResourceList<Sample> _sample;
	ResourceList<Ambiance> _amb;
	ResourceList<Environment> _env;
	ResourceList<Instance> _inst;

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Internal functions                                                        //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Random number generator                                                   //
	static const aalULong SEED = 43;
	static const aalULong MODULO = 2147483647;
	static const aalULong FACTOR = 16807;
	static const aalULong SHIFT = 91;

	static aalULong __current(SEED);

	aalULong Random()
	{
		return __current = (__current * FACTOR + SHIFT) % MODULO;
	}

	aalFloat FRandom()
	{
		__current = (__current * FACTOR + SHIFT) % MODULO;
		return aalFloat(__current) / aalFloat(MODULO);
	}

	aalULong InitSeed()
	{
		__current = (aalULong)time(NULL);
		return Random();
	}

	// Convert a value from time units to bytes                                  //
	aalULong UnitsToBytes(const aalULong & v, const aalFormat & _format, const aalUnit & unit)
	{
		switch (unit)
		{
			case AAL_UNIT_MS      :
				return aalULong(aalFloat(v) * 0.001F * _format.frequency * _format.channels * (_format.quality >> 3));

			case AAL_UNIT_SAMPLES :
				return v * _format.channels * (_format.quality >> 3);
		}

		return v;
	}

	// Convert a value from bytes to time units                                  //
	aalULong BytesToUnits(const aalULong & v, const aalFormat & _format, const aalUnit & unit)
	{
		switch (unit)
		{
			case AAL_UNIT_MS      :
				return aalULong(aalFloat(v) * 1000.0F / (_format.frequency * _format.channels * (_format.quality >> 3)));

			case AAL_UNIT_SAMPLES :
				return v / (_format.frequency * _format.channels * (_format.quality >> 3));
		}

		return v;
	}

	aalVoid DebugLog(const char * text)
	{
		fprintf(debug_log, text);
		fflush(debug_log);
	}

}//ATHENA::