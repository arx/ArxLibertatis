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

#include "audio/AudioGlobal.h"

#include <ctime>

#include "audio/Mixer.h"
#include "audio/Sample.h"
#include "audio/Ambiance.h"
#include "audio/AudioEnvironment.h"

using std::string;

namespace audio {

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Internal globals                                                          //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Audio device interface                                                    //
	Backend * backend = NULL;

	// Global settings                                                           //
	string sample_path;
	string ambiance_path;
	string environment_path;
	size_t stream_limit_bytes = DEFAULT_STREAMLIMIT;
	size_t session_time = 0;

	// Resources                                                                 //
	ResourceList<Mixer> _mixer;
	ResourceList<Sample> _sample;
	ResourceList<Ambiance> _amb;
	ResourceList<Environment> _env;

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Internal functions                                                        //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	// Random number generator                                                   //
	static const size_t SEED = 43;
	static const size_t MODULO = 2147483647;
	static const size_t FACTOR = 16807;
	static const size_t SHIFT = 91;

	static size_t __current = SEED;

	size_t Random()
	{
		return __current = (__current * FACTOR + SHIFT) % MODULO;
	}

	float FRandom()
	{
		__current = (__current * FACTOR + SHIFT) % MODULO;
		return float(__current) / float(MODULO);
	}

	size_t InitSeed()
	{
		__current = (size_t)time(NULL);
		return Random();
	}

	// Convert a value from time units to bytes                                  //
	size_t UnitsToBytes(size_t v, const PCMFormat & _format, TimeUnit unit)
	{
		switch (unit)
		{
			case UNIT_MS:
				return (size_t)(float(v) * 0.001F * _format.frequency * _format.channels * (_format.quality >> 3)) / 1000;

			case UNIT_SAMPLES:
				return v * _format.channels * (_format.quality >> 3);
			
			default:
				return v;
		}
	}

	// Convert a value from bytes to time units                                  //
	size_t BytesToUnits(size_t v, const PCMFormat & _format, TimeUnit unit)
	{
		switch (unit)
		{
			case UNIT_MS      :
				return (size_t)(float(v) * 1000.0F / (_format.frequency * _format.channels * (_format.quality >> 3)));

			case UNIT_SAMPLES :
				return v / (_format.frequency * _format.channels * (_format.quality >> 3));
			
			default:
				return v;
		}
	}

} // namespace audio
