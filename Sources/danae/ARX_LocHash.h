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

//-----------------------------------------------------------------------------
class CLocalisation
{
public:
	_TCHAR*					section;
	std::vector<_TCHAR*>	keys;

public:
	CLocalisation();
	~CLocalisation();

public:
	/// Set the section name to \a str.
	void SetSection(const _TCHAR* str);
	/// Adds a \a key.
	void AddKey(const _TCHAR* key);

private:
	// No copy
	CLocalisation(const CLocalisation&);
	CLocalisation& operator=(const CLocalisation&);
};

//-----------------------------------------------------------------------------
/// Hash map container for CLocalisation's. Supposedly fast lookup by section name
class CLocalisationHash
{
public:
	explicit CLocalisationHash(int reservedSize = 1024);
	~CLocalisationHash();

public:
	/// Add \a loc in the container.
	/// \note Takes ownership of \a loc if it returns true
	bool AddElement(CLocalisation* loc);
	/// Get element whose section matches \a name
	/// \note case insensitive
	const CLocalisation* GetElement(const _TCHAR* name) const;

private:
	void ReHash();

private:
	unsigned long	iSize_;
	long			iMask_;
	unsigned long	iFill_;
	CLocalisation	** pTab_;

	// No copy
	CLocalisationHash(const CLocalisationHash&);
	CLocalisationHash& operator=(const CLocalisationHash&);
};

#endif
