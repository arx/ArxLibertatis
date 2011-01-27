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
#ifndef __MINOS_COMMON_H__
#define __MINOS_COMMON_H__

#define __MINOS_COMMON_VERSION__ "0000"

#include <ARX_Common.h>

namespace MINOS
{
	typedef void Void;
	typedef unsigned char UBool;
	typedef signed char SBool;
	typedef unsigned short UWord;
	typedef signed short SWord;
	typedef unsigned long ULong;
	typedef signed long SLong;
	typedef unsigned long UInt;
	typedef float Float;

	const UBool UFALSE(0);
	const UBool UTRUE(1);
	const SBool SFALSE(-1);
	const SBool STRUE(0);

}//MINOS::

#endif//__MINOS_COMMON_H__
