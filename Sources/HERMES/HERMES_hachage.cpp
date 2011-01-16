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
#include "HERMES_hachage.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
CHachageString::CHachageString(int _iSize)
{
	tTab = (T_HACHAGE_DATAS *)malloc(_iSize * sizeof(T_HACHAGE_DATAS));
	iNbCollisions = iNbNoInsert = 0;

	iSize = _iSize;
	iFill = 0;

	while (_iSize--)
	{
		tTab[_iSize].lpszName = NULL;
	}

	iMask = iSize - 1;
}

//-----------------------------------------------------------------------------
CHachageString::~CHachageString()
{
	while (iSize--)
	{
		if (tTab[iSize].lpszName)
		{
			free((void *)tTab[iSize].lpszName);
			tTab[iSize].lpszName = NULL;
		}
	}

	free((void *)tTab);
}

//-----------------------------------------------------------------------------
bool CHachageString::AddString(char * _lpszText, void * _pMem)
{
	char * lpszTextLow = _strlwr(_lpszText);

	if (iFill >= iSize * 0.75)
	{
		//TO DO: recrée toute la table!!!!
		iSize <<= 1;
		iMask = iSize - 1;
		tTab = (T_HACHAGE_DATAS *)realloc(tTab, iSize * sizeof(T_HACHAGE_DATAS));
	}

	int iKey = GetKey(lpszTextLow);
	int	iH1 = FuncH1(iKey);
	int	iH2 = FuncH2(iKey);

	int	iNbSolution = 0;

	while (iNbSolution < iSize)
	{
		iH1 &= iMask;

		if (!tTab[iH1].lpszName)
		{
			tTab[iH1].lpszName = strdup(lpszTextLow);
			tTab[iH1].pMem = _pMem;
			iFill++;
			return true;
		}

		iNbCollisions++;
		iH1 += iH2;

		iNbSolution++;
	}

	iNbNoInsert++;
	return false;
}

//-----------------------------------------------------------------------------

void * CHachageString::GetPtrWithString(char * _lpszText)
{
	char * lpszTextLow = _strlwr(_lpszText);

	int iKey = GetKey(lpszTextLow);
	int	iH1 = FuncH1(iKey);
	int	iH2 = FuncH2(iKey);

	int	iNbSolution = 0;

	while (iNbSolution < iSize)
	{
		iH1 &= iMask;

		if (tTab[iH1].lpszName)
		{
			if (!strcmp((const char *)lpszTextLow, (const char *)tTab[iH1].lpszName))
			{
				return tTab[iH1].pMem;
			}
		}

		iH1 += iH2;
		iNbSolution++;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
int CHachageString::FuncH1(int _iKey)
{
	return _iKey;
}

//-----------------------------------------------------------------------------
int	CHachageString::FuncH2(int _iKey)
{
	return ((_iKey >> 1) | 1);
}

//-----------------------------------------------------------------------------
int	CHachageString::GetKey(char * _lpszText)
{
	int iKey = 0;
	int iLenght = strlen((const char *)_lpszText);
	int iLenght2 = iLenght;

	while (iLenght--)
	{
		iKey += _lpszText[iLenght] * (iLenght + 1) + _lpszText[iLenght] * iLenght2;
	}

	return iKey;
}
