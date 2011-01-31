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
// Code: Didier Pédreno
// todo remover les strIcmp

// Nuky - 30-01-11 - refactored most of this file

#include "ARX_LocHash.h"

#include <cstring>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

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
	for (unsigned long i = 0; i < iSize_ ; ++i)
		delete pTab_[i];
	delete [] pTab_;
}

/// Increase container capacity (*= 2) and rehash all elements
void CLocalisationHash::ReHash()
{
	unsigned long	iNewSize = iSize_ << 1;
	long			iNewMask = iNewSize - 1;

	CLocalisation** pNewTab = new CLocalisation*[iNewSize];
	memset(pNewTab, NULL, iNewSize * sizeof(*pNewTab));

	for (unsigned long i = 0; i < iSize_; i++)
	{
		if (pTab_[i] != NULL)
		{
			int iKey = GetHash(pTab_[i]->section);
			int	iH1	 = FuncH1(iKey);
			int	iH2  = FuncH2(iKey);

			for (unsigned long j = 0; j < iNewSize; ++j)
			{
				iH1 &= iNewMask;
				if (pNewTab[iH1] == NULL)
				{
					pNewTab[iH1] = pTab_[i];
					pTab_[i] = NULL;
					//++iFill;
				}
				iH1 += iH2;
			}
		}
	}

	delete [] pTab_;
	iSize_ = iNewSize;
	iMask_ = iNewMask;
	pTab_ = pNewTab;
}

bool CLocalisationHash::AddElement(CLocalisation* loc)
{
	if (!loc || !loc->section)
		return false;

	if (iFill_ >= iSize_ * 0.75)
		ReHash();

	int iHash = GetHash(loc->section);
	int	iH1 = FuncH1(iHash);
	int	iH2 = FuncH2(iHash);

	for (unsigned long i = 0; i < iSize_; ++i)
	{
		iH1 &= iMask_;
		if (pTab_[iH1] == NULL)
		{
			pTab_[iH1] = loc;
			++iFill_;
			return true;
		}
		iH1 += iH2;
	}

	return false;
}

const CLocalisation* CLocalisationHash::GetElement(const _TCHAR* name) const
{
	int iHash = GetHash(name);
	int	iH1 = FuncH1(iHash);
	int	iH2 = FuncH2(iHash);

	for (unsigned long i = 0; i < iSize_; ++i)
	{
		iH1 &= iMask_;
		CLocalisation* loc = pTab_[iH1];
		if (loc && !_tcsicmp(name, loc->section))
			return loc;
		iH1 += iH2;
	}

	return NULL;
}
