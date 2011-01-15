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
#ifndef __ATHENA_SAMPLE_H__
#define __ATHENA_SAMPLE_H__

#include <Athena_Types.h>
#include "Athena_Resource.h"

namespace ATHENA
{

	typedef struct
	{
		aalSampleCallback func;
		aalVoid * data;
		aalULong time;
		aalULong done;
	} Callback;

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Class ATHENA::Sample                                                      //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	class Sample : public ResourceHandle
	{
		public:
			//Constructor and destructor
			Sample();
			~Sample();

			//File I/O
			aalError Load(const char * name);

			//Setup
			aalError SetCallback(aalSampleCallback func, aalVoid * data, const aalULong & time, const aalUnit & unit = AAL_UNIT_MS);

			//Status
			aalError GetName(char * name, const aalULong & max_char = AAL_DEFAULT_STRINGSIZE);
			aalError GetLength(aalULong & length, const aalUnit & unit = AAL_UNIT_MS);

			//Data
			char * name;
			aalULong length;
			aalFormat format;
			aalVoid * data;           //User data
			aalULong callb_c;         //User callback count
			Callback * callb;         //User callback list
			//	long			loaded;
	};

}//ATHENA::

#endif//__ATHENA_SAMPLE_H__
