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

#include <stdio.h>
#include <windows.h>
#include "implode.h"
#include "HERMES_pack_types.h"
#include "HERMES_hachage.h"

#define PAK	1

class EVE_TFILE{
public:
	EVE_U8		*name;
	EVE_U32		taille;
	EVE_TFILE	*fprev,*fnext;
	EVE_U32		param;
	EVE_U32		param2;
	EVE_U32		param3;
public:
	EVE_TFILE(EVE_U8 *dir=NULL,EVE_U8 *n=NULL);
	~EVE_TFILE();
};

class EVE_REPERTOIRE{
public:
	EVE_U8			*name;
	EVE_U32			nbsousreps;
	EVE_U32			nbfiles;
	EVE_REPERTOIRE	*parent;
	EVE_REPERTOIRE	*fils;
	EVE_REPERTOIRE	*brotherprev,*brothernext;
	EVE_TFILE		*fichiers;
	CHachageString	*pHachage;
	EVE_U32			param;
public:
	EVE_REPERTOIRE(EVE_REPERTOIRE *p,EVE_U8 *n);
	~EVE_REPERTOIRE();

	void AddSousRepertoire(EVE_U8 *sname);
	BOOL DelSousRepertoire(EVE_U8 *sname);
	EVE_TFILE* AddFileToSousRepertoire(EVE_U8 *sname,EVE_U8 *name);
 
	EVE_REPERTOIRE * GetSousRepertoire(EVE_U8 *sname);
 
	void ConstructFullNameRepertoire(char*);
	friend void Kill(EVE_REPERTOIRE *r);
};

EVE_U8 * EVEF_GetDirName(EVE_U8 *dirplusname);
EVE_U8 * EVEF_GetFileName(EVE_U8 *dirplusname);
#endif
