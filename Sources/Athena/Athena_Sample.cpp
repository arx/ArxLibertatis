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
#include "Athena_Sample.h"
#include "Athena_Global.h"
#include "Athena_Stream.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

namespace ATHENA
{

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Constrcutor and destructor                                                //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	Sample::Sample() : ResourceHandle(),
		name(NULL),
		data(NULL),
		callb_c(0), callb(NULL),
		length(0)
	{
	}

	Sample::~Sample()
	{
		for (aalULong i(0); i < _inst.Size(); i++)
			if (_inst[i] && _inst[i]->sample == this) _inst.Delete(i);

		free(name);
		free(callb);
		free(data);
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// File I/O                                                                  //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError Sample::Load(const char * _name)
	{
		Stream * stream = CreateStream(_name);

		if (!stream) return AAL_ERROR_FILEIO;

		stream->GetFormat(format);
		stream->GetLength(length);
		DeleteStream(stream);

		aalVoid * ptr = realloc(name, strlen(_name) + 1);

		if (!ptr) return AAL_ERROR_MEMORY;

		name = (char *)ptr;
		strcpy(name, _name);

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Setup                                                                     //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////

	aalError Sample::SetCallback(aalSampleCallback func, aalVoid * _data, const aalULong & time, const aalUnit & unit)
	{
		aalVoid * ptr;

		ptr = realloc(callb, sizeof(Callback) * (callb_c + 1));

		if (!ptr) return AAL_ERROR_MEMORY;

		callb = (Callback *)ptr;

		callb[callb_c].func = func;
		callb[callb_c].data = _data;
		callb[callb_c].time = UnitsToBytes(time, format, unit);

		if (callb[callb_c].time > length) callb[callb_c].time = length;

		callb_c++;

		return AAL_OK;
	}

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Status                                                                    //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	aalError Sample::GetName(char * _name, const aalULong & max_char)
	{
		strncpy(_name, name, max_char);

		return AAL_OK;
	}

	aalError Sample::GetLength(aalULong & _length, const aalUnit & unit)
	{
		_length = BytesToUnits(length, format, unit);

		return AAL_OK;
	}

}//ATHENA::
