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

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <ARX_Common.h>
#include <cstring>

using namespace std;

//-----------------------------------------------------------------------------
class CLocalisation
{
    public:
        std::string lpszUSection;
        vector< std::string > vUKeys;
    
    public:
        void SetSection( const std::string& _lpszUSection )
        {
            lpszUSection = _lpszUSection;
        };

        void AddKey( const std::string& _lpszUText )
        {
            vUKeys.push_back( _lpszUText );
        };
};

//-----------------------------------------------------------------------------
class CLocalisationHash
{
    public:
        unsigned long   iSize;
        long            iMask;
        unsigned long   iFill;
        CLocalisation** pTab;
    public:
        unsigned long   iNbCollisions;
        unsigned long   iNbNoInsert;

    private:
        int FuncH1(int);
        int FuncH2(int);
        int GetKey( const std::string& );

    public:
        CLocalisationHash(int _iSize = 1024);
        ~CLocalisationHash();

        void ReHash();
        bool AddElement(CLocalisation * _pLoc);
 
        std::string* GetPtrWithString( const std::string& );
        unsigned long GetKeyCount(const std::string& );
};

#endif
