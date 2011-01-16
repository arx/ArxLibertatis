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
#ifndef __ATHENA_CODEC_H__
#define __ATHENA_CODEC_H__

#include <stdio.h>
#include <Athena_Types.h>

namespace ATHENA
{

	///////////////////////////////////////////////////////////////////////////////
	//                                                                           //
	// Class ATHENA::Codec                                                       //
	//                                                                           //
	///////////////////////////////////////////////////////////////////////////////
	class Codec
	{
		public:
			// Destructor
			virtual ~Codec() {};
			// Setup
			virtual aalError SetHeader(aalVoid * header) = 0;
			virtual aalError SetStream(FILE * stream) = 0;
			virtual aalError SetPosition(const aalULong & position) = 0;
			// Status
			virtual aalError GetHeader(aalVoid *&header) = 0;
			virtual aalError GetStream(FILE *&stream) = 0;
			virtual aalError GetPosition(aalULong & position) = 0;
			// File I/O
			virtual aalError Read(aalVoid * buffer, const aalULong & to_read, aalULong & read) = 0;
			virtual aalError Write(aalVoid * buffer, const aalULong & to_write, aalULong & write) = 0;
	};

}//ATHENA::

#endif//__ATHENA_CODEC_H__
