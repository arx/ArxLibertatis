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
#ifndef EVE_SAVE
#define EVE_SAVE

class HashMap;

#include <cstddef>

class PakFile {
	
public:
	
	const char * name;
	std::size_t taille;
	PakFile * fprev;
	PakFile * fnext;
	unsigned int param;
	unsigned int param2;
	unsigned int param3;
	
public:
	
	PakFile(const char * n = NULL);
	~PakFile();
	
};

class PakDirectory {
	
public:
	
	const char * name;
	unsigned int nbsousreps;
	unsigned int nbfiles;
	PakDirectory * parent;
	PakDirectory * fils;
	PakDirectory * brotherprev;
	PakDirectory * brothernext;
	PakFile * fichiers;
	HashMap * pHachage;
	unsigned int param;
	
public:
	
	PakDirectory(PakDirectory * p, const char * n);
	~PakDirectory();

	PakDirectory * AddSousRepertoire(const char * sname);
	bool DelSousRepertoire(const char * sname);
	PakFile * AddFileToSousRepertoire(const char * sname, const char * name);
 
	PakDirectory * GetSousRepertoire(const char * sname);
 
	void ConstructFullNameRepertoire(char * );
	friend void Kill(PakDirectory * r);
};

char * EVEF_GetDirName(const char * dirplusname);
char * EVEF_GetFileName(const char * dirplusname);

#endif
