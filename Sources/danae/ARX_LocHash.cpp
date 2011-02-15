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
// Code: Didier P�dreno
// todo remover les strIcmp

// Nuky - 30-01-11 - refactored most of this file

#include "ARX_LocHash.h"

#include <cstring>

//----------------------------------------------------------------------------
// CLocalisation
//----------------------------------------------------------------------------

namespace
{

_TCHAR* tchardup(const _TCHAR* str)
{
	// Nuky - TODO: replace by strdup ? but why the + 2 here ? some other code
	//        might be buggy and depend on it ..
	const size_t size = (_tcslen(str) + 2) * sizeof(_TCHAR);

	_TCHAR* result = (_TCHAR *)malloc(size);
	memset(result, 0, size);	//superfluous
	_tcscpy(result, str);

	return result;
}

} // \namespace

CLocalisation::CLocalisation()
: section(NULL)
, keys()
{
}

CLocalisation::~CLocalisation()
{
	free(section);

	for (size_t i = 0, i_end = keys.size(); i != i_end; ++i)
		free(keys[i]);
}

void CLocalisation::SetSection(const _TCHAR* newsection)
{
	free(section);
	section = tchardup(newsection);
};

void CLocalisation::AddKey(const _TCHAR* key)
{
	keys.push_back(tchardup(key));
};


//----------------------------------------------------------------------------
// CLocalisationHash
//----------------------------------------------------------------------------

namespace
{

int	GetHash(const _TCHAR* str)
{
	const size_t len = _tcslen(str);
	int result = 0;

	for (size_t i = 0; i != len; ++i)
		result += str[i] * (i + 1) + str[i] * len;

	return result;
}

int FuncH1(int k)
{
	return k;
}

int	FuncH2(int k)
{
	return ((k >> 1) | 1);
}

} // \namespace

CLocalisationHash::CLocalisationHash(int reservedSize)
: iSize_(reservedSize)
, iMask_(reservedSize - 1)
, iFill_(0)
, pTab_(new CLocalisation*[reservedSize])
//, iNbCollisions_(0)
//, iNbNoInsert_(0)
{
	memset(pTab_, NULL, reservedSize * sizeof(*pTab_));
}

CLocalisationHash::~CLocalisationHash()
{
	while (iSize--)
	{
		if (pTab[iSize] != NULL)
		{
			delete pTab[iSize];
			pTab[iSize] = NULL;
		}
	}

	delete [] pTab;
	pTab = NULL;
}

//-----------------------------------------------------------------------------
int CLocalisationHash::FuncH1(int _iKey)
{
	return _iKey;
}

//-----------------------------------------------------------------------------
int	CLocalisationHash::FuncH2(int _iKey)
{
	return ((_iKey >> 1) | 1);
}

//-----------------------------------------------------------------------------
int	CLocalisationHash::GetKey(const _TCHAR * _lpszUText)
{
	int iKey = 0;
	int iLenght = _tcslen((const _TCHAR *)_lpszUText);
	int iLenght2 = iLenght;

	while (iLenght--)
	{
		iKey += _lpszUText[iLenght] * (iLenght + 1) + _lpszUText[iLenght] * iLenght2;
	}

	return iKey;
}

//-----------------------------------------------------------------------------
void CLocalisationHash::ReHash()
{
	ULONG	iNewSize = iSize << 1;
	long	iNewMask = iNewSize - 1;

	CLocalisation ** pTab2 = new CLocalisation *[iNewSize];

	for (unsigned long i = 0 ; i < iNewSize ; i++)
	{
		pTab2[i] = NULL;
	}


	for (UINT i = 0 ; i < iSize ; i++)
	{
		if (pTab[i] != NULL)
		{
			int iKey = GetKey(pTab[i]->lpszUSection);
			int	iH1	 = FuncH1(iKey);
			int	iH2  = FuncH2(iKey);

			UINT iNbSolution = 0;

			while (iNbSolution < iNewSize)
			{
				iH1 &= iNewMask;

				if (pTab2[iH1] == NULL)
				{
					pTab2[iH1]	= pTab[i];
					pTab[i]		= NULL;
					//iFill ++;
				}

				iNbCollisions ++;
				iH1 += iH2;

				iNbSolution ++;
			}

			iNbNoInsert ++;
		}
	}

	iSize = iNewSize;
	iMask = iNewMask;

	delete [] pTab;
	pTab = pTab2;
}

//-----------------------------------------------------------------------------
bool CLocalisationHash::AddElement(CLocalisation * _pLoc)
{
	if (iFill >= iSize * 0.75)
	{
		ReHash();
	}

	if (!(_pLoc && _pLoc->lpszUSection)) return false;

	int iKey = GetKey(_pLoc->lpszUSection);
	int	iH1 = FuncH1(iKey);
	int	iH2 = FuncH2(iKey);

	unsigned long iNbSolution = 0;

	while (iNbSolution < iSize)
	{
		iH1 &= iMask;

		if (pTab[iH1] == NULL)
		{
			pTab[iH1] = _pLoc;
			iFill ++;
			return true;
		}

		iNbCollisions ++;
		iH1 += iH2;

		iNbSolution ++;
	}

	iNbNoInsert ++;
	return false;
}

//-----------------------------------------------------------------------------
_TCHAR * CLocalisationHash::GetPtrWithString(const _TCHAR * _lpszUText)
{
	int iKey = GetKey(_lpszUText);
	int	iH1 = FuncH1(iKey);
	int	iH2 = FuncH2(iKey);

	unsigned long iNbSolution = 0;

	while (iNbSolution < iSize)
	{
		iH1 &= iMask;

		if (pTab[iH1])
		{
			if (!_tcsicmp(_lpszUText, pTab[iH1]->lpszUSection))
			{
				if (pTab[iH1]->vUKeys.size() > 0)
				{
					return pTab[iH1]->vUKeys[0];
				}

				return NULL;
			}
		}

		iH1 += iH2;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
unsigned long CLocalisationHash::GetKeyCount(const _TCHAR * _lpszUText)
{
	int iKey = GetKey(_lpszUText);
	int	iH1 = FuncH1(iKey);
	int	iH2 = FuncH2(iKey);

	unsigned long iNbSolution = 0;

	while (iNbSolution < iSize)
	{
		iH1 &= iMask;

		if (pTab[iH1])
		{
			if (!_tcsicmp(_lpszUText, pTab[iH1]->lpszUSection))
			{
				return pTab[iH1]->vUKeys.size();

				return 0;
			}
		}

		iH1 += iH2;
		iNbSolution++;
	}

	return 0;
}
