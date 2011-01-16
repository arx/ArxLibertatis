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
#include <string.h>
#include "Athena_Resource.h"
#include "Athena_FileIO.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

namespace ATHENA
{

	extern char * root_path;


	FILE * OpenResource(const char * name, const char * resource_path)
	{
		FILE * file = FileOpen(name, "rb");
		char text[256];

		if (!file && root_path)
		{
			strcpy(text, root_path);
			strcat(text, name);
			file = FileOpen(text, "rb");
		}

		if (!file && resource_path)
		{
			strcpy(text, resource_path);
			strcat(text, name);
			file = FileOpen(text, "rb");
		}

		if (!file && root_path && resource_path)
		{
			strcpy(text, root_path);
			strcat(text, resource_path);
			strcat(text, name);
			file = FileOpen(text, "rb");
		}

		return file;
	}

}//ATHENA::
