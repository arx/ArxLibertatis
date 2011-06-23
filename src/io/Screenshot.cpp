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


SnapShot * pSnapShot;
SNAPSHOTINFO snapshotdata;
long CURRENTSNAPNUM = 0;
 
struct TargaHeader {
	BYTE IDLength;
	BYTE ColormapType;
	BYTE ImageType;
	BYTE ColormapSpecification[5];
	WORD XOrigin;
	WORD YOrigin;
	WORD ImageWidth;
	WORD ImageHeight;
	BYTE PixelDepth;
	BYTE ImageDescriptor;
};

struct MEMORYSNAP
{
	char name[256];
	unsigned char buffer[640*480*2];
};
MEMORYSNAP * snaps = NULL;
long MAXSNAPS = 0;
long InitMemorySnaps()
{
	if (snaps) free(snaps);

	snaps = NULL;
	long number = 1600;
	long cool = 1;

	while ((cool) && (number))
	{
		cool = 0;

		if (snaps) free(snaps);

		snaps = (MEMORYSNAP *)malloc(sizeof(MEMORYSNAP) * number);

		if (snaps == NULL) cool = 1;

		number -= 4;
	}

	number -= 8;

	if (snaps) free(snaps);

	snaps = NULL;

	if (number > 0)
		snaps = (MEMORYSNAP *)malloc(sizeof(MEMORYSNAP) * number);

	MAXSNAPS = number;
	return number;
}

// TODO replace with DevIL code
void FlushMemorySnaps(long flag)
{
	if (flag == 0)
	{
		if (snaps) free(snaps);

		snaps = NULL;
		MAXSNAPS = 0;
		return;
	}

	static char buff[640*2];
 
	TargaHeader tga;
	
	long targetsizeX = snapshotdata.xsize;
	long targetsizeY = snapshotdata.ysize;
	float xratio = 640.f / (float)targetsizeX;
	float yratio = 480.f / (float)targetsizeY;
	tga.IDLength = sizeof(TargaHeader);
	tga.ColormapType = 0;
	tga.ColormapSpecification[0] = 0;
	tga.ColormapSpecification[1] = 0;
	tga.ColormapSpecification[2] = 0;
	tga.ColormapSpecification[3] = 0;
	tga.ColormapSpecification[4] = 0;
	tga.XOrigin = 0;
	tga.YOrigin = 0;
	tga.ImageType = 0x02;
	tga.ImageWidth = (unsigned short)targetsizeX;
	tga.ImageHeight = (unsigned short)targetsizeY;
	tga.PixelDepth = 16;
	tga.ImageDescriptor =  32;


	danaeApp.Lock();
	DWORD dwRMask = danaeApp.ddsd.ddpfPixelFormat.dwRBitMask;
	DWORD dwGMask = danaeApp.ddsd.ddpfPixelFormat.dwGBitMask;
	DWORD dwBMask = danaeApp.ddsd.ddpfPixelFormat.dwBBitMask;
	DWORD dwAMask = danaeApp.ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
	danaeApp.Unlock();

	DWORD dwRShiftL = 8, dwRShiftR = 0;
	DWORD dwGShiftL = 8, dwGShiftR = 0;
	DWORD dwBShiftL = 8, dwBShiftR = 0;
	DWORD dwAShiftL = 8, dwAShiftR = 0;

	DWORD dwMask;

	for (dwMask = dwRMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwRShiftR++;

	for (; dwMask; dwMask >>= 1) dwRShiftL--;

	for (dwMask = dwGMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwGShiftR++;

	for (; dwMask; dwMask >>= 1) dwGShiftL--;

	for (dwMask = dwBMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwBShiftR++;

	for (; dwMask; dwMask >>= 1) dwBShiftL--;

	for (dwMask = dwAMask; dwMask && !(dwMask & 0x1); dwMask >>= 1) dwAShiftR++;

	for (; dwMask; dwMask >>= 1) dwAShiftL--;


	for (long i = 0; i < CURRENTSNAPNUM; i++)
	{
		FILE * file = fopen(snaps[i].name, "wb");

		if (NULL == file)
		{
			return;
		}

 
		size_t written = fwrite(&tga, sizeof(TargaHeader), 1, file);
		assert(written == sizeof(TargaHeader));
		long n;
		unsigned short col;
		unsigned char * v = (unsigned char *)snaps[i].buffer;
		written = fwrite(&tga, 1, 20, file);
		assert(written == 20);
		DWORD dwOffset;
		DWORD x;

		for (long yy = 1; yy <= targetsizeY; yy++)
		{
			dwOffset = (long)((float)yy * yratio) * 640;
			long pos = 0;

			for (long xx = 0; xx < targetsizeX; xx++)
			{
				x = (long)((float)xx * xratio);
				n = (dwOffset + x + 1) << 1;
				memcpy(&col, v + n, 2);
				unsigned short r, g, b;
				b = (unsigned short)((col & dwBMask) >> dwBShiftR) << dwBShiftL;
				g = (unsigned short)((col & dwGMask) >> dwGShiftR) << dwGShiftL;
				r = (unsigned short)((col & dwRMask) >> dwRShiftR) << dwRShiftL;
				col = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
				memcpy(buff + pos, &col, 2);
				pos += 2;
			}

			written = fwrite(buff, pos, 1, file);
			assert(written == (size_t)pos);
		}

		fclose(file);
	}

	if (snaps) free(snaps);

	snaps = NULL;
	MAXSNAPS = 0;
}
//-----------------------------------------------------------------------------
SnapShot::SnapShot(const char * _pDir, const char * _pName, bool _bReplace)
{
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
