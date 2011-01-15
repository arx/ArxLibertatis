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
#ifndef HACHAGE_H
#define HACHAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <ARX_Common.h>

typedef struct
{
	char	*lpszName;
	void	*pMem;
}T_HACHAGE_DATAS;

class CHachageString
{
private:
	int				iSize;
	int				iMask;
	int				iFill;
	T_HACHAGE_DATAS	*tTab;
public:
	int		iNbCollisions;
	int		iNbNoInsert;
private:
	int FuncH1(int);
	int	FuncH2(int);
	int	GetKey(char*);
public:
	CHachageString(int _iSize=256);
	~CHachageString();

	bool AddString(char*,void *_pMem=NULL);
 
	void* GetPtrWithString(char *_lpszText);
};

#endif
