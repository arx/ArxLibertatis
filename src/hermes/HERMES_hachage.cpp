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

#include <cstdlib>
#include <cstring>

using std::size_t;

//-----------------------------------------------------------------------------
HashMap::HashMap(size_t _iSize)
{
	tTab = (Entry *)malloc(_iSize * sizeof(Entry));
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
HashMap::~HashMap()
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
bool HashMap::AddString(const char * _lpszText, void * _pMem)
{
	//	todo string;
	const char * lpszTextLow = _lpszText;
//	char * lpszTextLow = strlwr(_lpszText);

	if (iFill >= iSize * 0.75)
	{
		//TO DO: recr�e toute la table!!!!
		iSize <<= 1;
		iMask = iSize - 1;
		tTab = (Entry *)realloc(tTab, iSize * sizeof(Entry));
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

void * HashMap::GetPtrWithString(const char * _lpszText)
{
//	todo string;
	const char * lpszTextLow = _lpszText;
//	char * lpszTextLow = _strlwr(_lpszText);

	int iKey = GetKey(lpszTextLow);
	int	iH1 = FuncH1(iKey);
	int	iH2 = FuncH2(iKey);

	int	iNbSolution = 0;

	while (iNbSolution < iSize)
	{
		iH1 &= iMask;

		if (tTab[iH1].lpszName)
		{
			if (!strcmp(lpszTextLow, tTab[iH1].lpszName))
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
int HashMap::FuncH1(int _iKey)
{
	return _iKey;
}

//-----------------------------------------------------------------------------
int	HashMap::FuncH2(int _iKey)
{
	return ((_iKey >> 1) | 1);
}

//-----------------------------------------------------------------------------
int	HashMap::GetKey(const char * _lpszText)
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
