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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// ARX_Snapshot
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Snapshot management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "io/Screenshot.h"

#include <cstdio>
#include <string>
#include <cassert>
#include <climits>
#include <sstream>

#include "core/Core.h"

#include "io/Filesystem.h"

using std::ostringstream;

static SnapShot * pSnapShot;

SnapShot::SnapShot(const char * _pDir, const char * _pName, bool _bReplace) {
	ulNum = 0;

	if (_pName)
	{
		char tTxt[256];

		if (_pDir)
		{
			strcpy(tTxt, _pDir);
		}
		else
		{
			*tTxt = 0;
		}

		strcat(tTxt, _pName);
		pName = strdup(tTxt);
//		strupr(pName);

		tTxt[0] = '\0';

		if (_pDir)
		{
			strcpy(tTxt, _pDir);
		}

		strcat(tTxt, "*.bmp");

		if (!_bReplace)
		{
			char tTemp[sizeof(WIN32_FIND_DATA)+2];
			WIN32_FIND_DATA * w32fdata = (WIN32_FIND_DATA *)tTemp;
			HANDLE h = FindFirstFile((const char *)tTxt, w32fdata);

			if (h == INVALID_HANDLE_VALUE) return;

			do
			{
				if (!(w32fdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					strcpy(tTxt, w32fdata->cFileName);
//					strupr(tTxt);

					if (strstr(tTxt, pName)) ulNum++;
				}
			}
			while (FindNextFile(h, w32fdata));

			FindClose(h);
		}
	}
	else
	{
		pName = NULL;
	}
}

//-----------------------------------------------------------------------------
SnapShot::~SnapShot()
{
	if (pName)
	{
		free((void *)pName);
		pName = NULL;
	}
}

//-----------------------------------------------------------------------------
//Sauvegarde en BMP 32bits
bool SnapShot::GetSnapShot() {
	
	Image image;
	
	if(!GRenderer->getSnapshot(image)) {
		return false;
	}
	
	ostringstream oss;
	oss << pName << '_' << ulNum << ".bmp";
	
	image.save(oss.str());
	
	return true;
}

//-----------------------------------------------------------------------------
//Sauvegarde en BMP 24bits
bool SnapShot::GetSnapShotDim(int _iWith, int _iHeight) {
	
	Image image;
	
	if(!GRenderer->getSnapshot(image, _iWith, _iHeight)) {
		return false;
	}
	
	ostringstream oss;
	oss << pName << '_' << ulNum << ".bmp";
	
	image.save(oss.str());
	
	return true;
}

//-----------------------------------------------------------------------------
void InitSnapShot(const char * _pDir, const char * _pName)
{
	FreeSnapShot();
	pSnapShot = new SnapShot(_pDir, _pName);
}

//-----------------------------------------------------------------------------
void GetSnapShot()
{
	if (pSnapShot)
	{
		pSnapShot->GetSnapShot();
	}
}

//-----------------------------------------------------------------------------
void FreeSnapShot()
{
	if (pSnapShot)
	{
		delete pSnapShot;
		pSnapShot = NULL;
	}
}
