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

#ifndef LOC_HASH_H
#define LOC_HASH_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <vector>
#include <ARX_Common.h>

using namespace std;

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
class CLocalisation
{
	public:
		_TCHAR		*	 lpszUSection;
		vector<_TCHAR *> vUKeys;
	
	public:
		CLocalisation()
		{
			lpszUSection = NULL;
		};
		~CLocalisation()
		{
			if (lpszUSection)
			{
				free((void *)lpszUSection);
				lpszUSection = NULL;
			}
			
			for (UINT i = 0 ; i < vUKeys.size() ; i++)
			{
				free((void *) vUKeys[i]);
				vUKeys[i] = NULL;
			}
		};

		void SetSection(_TCHAR * _lpszUSection)
		{
			if (lpszUSection)
			{
				free(lpszUSection);
				lpszUSection = NULL;
			}

			lpszUSection = (_TCHAR *) malloc((_tcslen(_lpszUSection) + 2) * sizeof(_TCHAR));
			memset(lpszUSection, 0, (_tcslen(_lpszUSection) + 2) * sizeof(_TCHAR));
			_tcscpy(lpszUSection, _lpszUSection);
		};
		void AddKey(_TCHAR * _lpszUText)
		{
			_TCHAR * lpszT = (_TCHAR *) malloc((_tcslen(_lpszUText) + 2) * sizeof(_TCHAR));
			memset(lpszT, 0, (_tcslen(_lpszUText) + 2) * sizeof(_TCHAR));
			_tcscpy(lpszT, _lpszUText);

			vUKeys.push_back(lpszT);
		};
};

//-----------------------------------------------------------------------------
class CLocalisationHash
{
	public:
		unsigned long	iSize;
		long			iMask;
		unsigned long	iFill;
		CLocalisation	** pTab;
	public:
		unsigned long	iNbCollisions;
		unsigned long	iNbNoInsert;

	private:
		int				FuncH1(int);
		int				FuncH2(int);
		int				GetKey(const _TCHAR *);

	public:
		CLocalisationHash(int _iSize = 1024);
		~CLocalisationHash();

		void ReHash();
		bool AddElement(CLocalisation * _pLoc);
 
		_TCHAR * GetPtrWithString(const _TCHAR *);
		unsigned long GetKeyCount(const _TCHAR *);
};

#endif
