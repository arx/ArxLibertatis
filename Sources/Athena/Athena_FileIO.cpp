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
#include "Athena_FileIO.h"
#include "Athena_Global.h"
#include <Hermes_Pak.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

extern long CURRENT_LOADMODE;

namespace ATHENA
{

	FILE *(* FileOpen)(const char * name, const char * mode) = fopen;
	int (* FileClose)(FILE * file) = fclose;
	size_t (* FileRead)(void * buffer, size_t size, size_t count, FILE * file) = fread;
	size_t (* FileWrite)(const void * buffer, size_t size, size_t count, FILE * file) = fwrite;
	int (* FileSeek)(FILE * file, long offset, int origin) = fseek;
	long(* FileTell)(FILE * file) = ftell;

	aalVoid FileIOInit()
	{
		if (global_status & AAL_FLAG_PACKEDRESOURCES)
		{
			CURRENT_LOADMODE = LOAD_PACK;

			FileOpen = PAK_fopen;
			FileClose = PAK_fclose;
			FileRead = PAK_fread;
			FileWrite = ::fwrite;
			FileSeek = PAK_fseek;
			FileTell = PAK_ftell;
		}
		else
		{
			CURRENT_LOADMODE = LOAD_TRUEFILE;

			FileOpen =	::fopen;
			FileClose = ::fclose;
			FileRead =	::fread;
			FileWrite = ::fwrite;
			FileSeek =	::fseek;
			FileTell =	::ftell;
		}
	}

	aalVoid AddPack(const char * name)
	{
		PAK_SetLoadMode(global_status & AAL_FLAG_PACKEDRESOURCES ? LOAD_PACK : LOAD_TRUEFILE,
		                (char *)name);
	}

}//ATHENA::