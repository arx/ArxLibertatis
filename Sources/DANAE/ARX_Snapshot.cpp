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
#include <stdio.h>
#include <danae.h>
#include "ARX_SnapShot.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

SnapShot * pSnapShot;

SNAPSHOTINFO snapshotdata;
long CURRENTSNAPNUM = 0;
 
struct TargaHeader
{
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
} tga;

typedef struct
{
	char name[256];
	unsigned char buffer[640*480*2];
} MEMORYSNAP;
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
 
	long pitch = danaeApp.ddsd.lPitch ;
	danaeApp.Unlock();
	pitch = 640;
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
		FILE * file;
		file = fopen(snaps[i].name, "wb");

		if (NULL == file)
		{
			return;
		}

 
		fwrite(&tga, sizeof(TargaHeader), 1, file);
		long n;
		unsigned short col;
		unsigned char * v = (unsigned char *)snaps[i].buffer;
		fwrite(&tga, 1, 20, file);
		DWORD dwOffset;
		DWORD x;

		for (long yy = 1; yy <= targetsizeY; yy++)
		{
			dwOffset = (long)((float)yy * yratio) * pitch; //640;
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

			fwrite(buff, pos, 1, file);
		}

		fclose(file);
	}

	if (snaps) free(snaps);

	snaps = NULL;
	MAXSNAPS = 0;
}
//-----------------------------------------------------------------------------
SnapShot::SnapShot(char * _pDir, char * _pName, bool _bReplace)
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
		strupr(pName);

		strcpy(tTxt, Project.workingdir);

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
					strupr(tTxt);

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
bool SnapShot::GetSnapShot()
{
	DDSURFACEDESC2 ddsd2;
	memset((void *)&ddsd2, 0, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);

	if (danaeApp.m_pFramework->m_pddsBackBuffer->Lock(NULL, &ddsd2, DDLOCK_SURFACEMEMORYPTR, 0) != DD_OK)
	{
		return false;
	}

	unsigned long * pulMemorySnapShot = new unsigned long[ddsd2.dwWidth*ddsd2.dwHeight];

	if (!pulMemorySnapShot)
	{
		danaeApp.m_pFramework->m_pddsBackBuffer->Unlock(NULL);
		return false;
	}

	unsigned long * pulMemorySnapShotClone = pulMemorySnapShot;

	unsigned char * pMemoryBase = (unsigned char *)ddsd2.lpSurface;
	int iPitch = ddsd2.lPitch * (ddsd2.dwHeight - 1);
 

	switch (ddsd2.ddpfPixelFormat.dwRGBBitCount)
	{
		case 16:
			{
			    int iDecalR = 0;
			    int iDecalG = 0;
			    int iDecalB = 0;
			    int iDec;
			    iDec = ddsd2.ddpfPixelFormat.dwRBitMask;

			    while (!(iDec & 0x800000))
		{
			iDecalR++;
			iDec <<= 1;
		}

	iDec = ddsd2.ddpfPixelFormat.dwGBitMask;

	while (!(iDec & 0x8000))
	{
		iDec <<= 1;
		iDecalG++;
	}

	iDec = ddsd2.ddpfPixelFormat.dwBBitMask;

	while (!(iDec & 0x80))
	{
		iDec <<= 1;
		iDecalB++;
	}

	for (unsigned int iY = 0; iY < ddsd2.dwHeight; iY++)
	{
		unsigned short * pMemory = (unsigned short *)(pMemoryBase + iPitch);

			for (unsigned int iX = 0; iX < ddsd2.dwWidth; iX++)
			{
				int iR = (pMemory[iX] & ddsd2.ddpfPixelFormat.dwRBitMask) << iDecalR;
				int iG = (pMemory[iX] & ddsd2.ddpfPixelFormat.dwGBitMask) << iDecalG;
				int iB = (pMemory[iX] & ddsd2.ddpfPixelFormat.dwBBitMask) << iDecalB;
				*pulMemorySnapShotClone++ = iR | iG | iB;
			}

			iPitch -= ddsd2.lPitch;
		}
	}
		break;
		case 24:
			{
			    for (unsigned int iY = 0; iY < ddsd2.dwHeight; iY++)
		{
			unsigned char * pMemory = (unsigned char *)(pMemoryBase + iPitch);

				for (unsigned int iX = 0; iX < ddsd2.dwWidth; iX++)
				{
					int iR = *pMemory++;
					int iG = *pMemory++;
					int iB = *pMemory++;
					*pulMemorySnapShotClone++ = (iR << 16) | (iG << 8) | iB;
				}

				iPitch -= ddsd2.lPitch;
			}
	}
		break;
		case 32:
			{
			    for (unsigned int iY = 0; iY < ddsd2.dwHeight; iY++)
		{
			unsigned long * pMemory = (unsigned long *)(pMemoryBase + iPitch);
				memcpy(pulMemorySnapShotClone, pMemory, ddsd2.dwWidth << 2);
				pulMemorySnapShotClone += ddsd2.dwWidth;
				iPitch -= ddsd2.lPitch;
			}
	}
		break;
	}

	danaeApp.m_pFramework->m_pddsBackBuffer->Unlock(NULL);

	//sauvegarde bmp
	char tTxt[256];
	sprintf(tTxt, "%s%s_%d.bmp", Project.workingdir, pName, ulNum);
	FILE * fFile = fopen(tTxt, "wb");

	if (!fFile)
	{
		delete pulMemorySnapShot;
		return false;
	}

	ulNum++;

	BITMAPFILEHEADER bfhBitmapFileHeader;
	bfhBitmapFileHeader.bfType = ('M' << 8) | ('B');
	bfhBitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + ((ddsd2.dwWidth * ddsd2.dwHeight) << 2);
	bfhBitmapFileHeader.bfReserved1 = bfhBitmapFileHeader.bfReserved2 = 0;
	bfhBitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	fwrite(&bfhBitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, fFile);

	BITMAPINFOHEADER bihBitmapInfoHeader;
	bihBitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bihBitmapInfoHeader.biWidth = ddsd2.dwWidth;
	bihBitmapInfoHeader.biHeight = ddsd2.dwHeight;
	bihBitmapInfoHeader.biPlanes = 1;
	bihBitmapInfoHeader.biBitCount = 32;
	bihBitmapInfoHeader.biCompression = BI_RGB;
	bihBitmapInfoHeader.biSizeImage = 0;
	bihBitmapInfoHeader.biXPelsPerMeter = 0;
	bihBitmapInfoHeader.biYPelsPerMeter = 0;
	bihBitmapInfoHeader.biClrUsed = 0;
	bihBitmapInfoHeader.biClrImportant = 0;
	fwrite(&bihBitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, fFile);

	fwrite(pulMemorySnapShot, (ddsd2.dwWidth * ddsd2.dwHeight) << 2, 1, fFile);

	fclose(fFile);
	delete pulMemorySnapShot;
	return true;
}

//-----------------------------------------------------------------------------
//Sauvegarde en BMP 24bits
bool SnapShot::GetSnapShotDim(int _iWith, int _iHeight)
{
	DDSURFACEDESC2 ddsd2;
	memset((void *)&ddsd2, 0, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);

	danaeApp.m_pFramework->m_pddsBackBuffer->GetSurfaceDesc(&ddsd2);

	LPDIRECTDRAW7        pDD;
	LPDIRECTDRAWSURFACE7 pddsRender;
	GDevice->GetRenderTarget(&pddsRender);
	pddsRender->GetDDInterface((VOID **)&pDD);
	pddsRender->Release();

	LPDIRECTDRAWSURFACE7 m_pddsSurface = NULL;
	ddsd2.dwSize = sizeof(ddsd2);
	ddsd2.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
	ddsd2.dwWidth = _iWith;
	ddsd2.dwHeight = _iHeight;
	ddsd2.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	pDD->CreateSurface(&ddsd2, &m_pddsSurface, NULL);

	pDD->Release();

	m_pddsSurface->Blt(NULL, danaeApp.m_pFramework->m_pddsBackBuffer, NULL, DDBLT_WAIT, NULL);
	memset((void *)&ddsd2, 0, sizeof(ddsd2));
	ddsd2.dwSize = sizeof(ddsd2);
	m_pddsSurface->GetSurfaceDesc(&ddsd2);

	if (m_pddsSurface->Lock(NULL, &ddsd2, DDLOCK_SURFACEMEMORYPTR, 0) != DD_OK)
	{
		return false;
	}

	unsigned char * pulMemorySnapShot = new unsigned char[ddsd2.dwWidth*ddsd2.dwHeight*3];

	if (!pulMemorySnapShot)
	{
		danaeApp.m_pFramework->m_pddsBackBuffer->Unlock(NULL);
		return false;
	}

	unsigned char * pulMemorySnapShotClone = pulMemorySnapShot;

	unsigned char * pMemoryBase = (unsigned char *)ddsd2.lpSurface;
	int iPitch = ddsd2.lPitch * (ddsd2.dwHeight - 1);
 

	switch (ddsd2.ddpfPixelFormat.dwRGBBitCount)
	{
		case 16:
			{
			    int iDecalR = 0;
			    int iDecalG = 0;
			    int iDecalB = 0;
			    int iDec;
			    iDec = ddsd2.ddpfPixelFormat.dwRBitMask;

			    while (!(iDec & 0x800000))
		{
			iDecalR++;
			iDec <<= 1;
		}

	iDec = ddsd2.ddpfPixelFormat.dwGBitMask;

	while (!(iDec & 0x8000))
	{
		iDec <<= 1;
		iDecalG++;
	}

	iDec = ddsd2.ddpfPixelFormat.dwBBitMask;

	while (!(iDec & 0x80))
	{
		iDec <<= 1;
		iDecalB++;
	}

	for (unsigned int iY = 0; iY < ddsd2.dwHeight; iY++)
	{
		unsigned short * pMemory = (unsigned short *)(pMemoryBase + iPitch);

			for (unsigned int iX = 0; iX < ddsd2.dwWidth; iX++)
			{
				unsigned int iColor = pMemory[iX];
				int iR = (iColor & ddsd2.ddpfPixelFormat.dwRBitMask) << iDecalR;
				int iG = (iColor & ddsd2.ddpfPixelFormat.dwGBitMask) << iDecalG;
				int iB = (iColor & ddsd2.ddpfPixelFormat.dwBBitMask) << iDecalB;


				ARX_CHECK_UCHAR(iB);
				*pulMemorySnapShotClone++ = ARX_CLEAN_WARN_CAST_UCHAR(iB);
				ARX_CHECK_UCHAR(iG >> 8);
				*pulMemorySnapShotClone++ = ARX_CLEAN_WARN_CAST_UCHAR(iG >> 8);
				ARX_CHECK_UCHAR(iR >> 16);
				*pulMemorySnapShotClone++ = ARX_CLEAN_WARN_CAST_UCHAR(iR >> 16);

			}

			iPitch -= ddsd2.lPitch;
		}
	}
		break;
		case 24:
			{
			    for (unsigned int iY = 0; iY < ddsd2.dwHeight; iY++)
		{
			unsigned char * pMemory = (unsigned char *)(pMemoryBase + iPitch);

				for (unsigned int iX = 0; iX < ddsd2.dwWidth; iX++)
				{
					int iR = *pMemory++;
					int iG = *pMemory++;
					int iB = *pMemory++;


					ARX_CHECK_UCHAR(iR);
					*pulMemorySnapShotClone++ = ARX_CLEAN_WARN_CAST_UCHAR(iR);
					ARX_CHECK_UCHAR(iG);
					*pulMemorySnapShotClone++ = ARX_CLEAN_WARN_CAST_UCHAR(iG);
					ARX_CHECK_UCHAR(iB);
					*pulMemorySnapShotClone++ = ARX_CLEAN_WARN_CAST_UCHAR(iB);

				}

				iPitch -= ddsd2.lPitch;
			}
	}
		break;
		case 32:
			{
			    for (unsigned int iY = 0; iY < ddsd2.dwHeight; iY++)
		{
			unsigned char * pMemory = (unsigned char *)(pMemoryBase + iPitch);

				for (unsigned int iX = 0; iX < ddsd2.dwWidth; iX++)
				{
					int iR = *pMemory++;
					int iG = *pMemory++;
					int iB = *pMemory++;
					pMemory++;


					ARX_CHECK_UCHAR(iR);
					*pulMemorySnapShotClone++ = ARX_CLEAN_WARN_CAST_UCHAR(iR);
					ARX_CHECK_UCHAR(iG);
					*pulMemorySnapShotClone++ = ARX_CLEAN_WARN_CAST_UCHAR(iG);
					ARX_CHECK_UCHAR(iB);
					*pulMemorySnapShotClone++ = ARX_CLEAN_WARN_CAST_UCHAR(iB);

				}

				iPitch -= ddsd2.lPitch;
			}
	}
		break;
	}

	m_pddsSurface->Unlock(NULL);
	m_pddsSurface->Release();

	//sauvegarde bmp
	char tTxt[256];
	sprintf(tTxt, "%s%s_%d.bmp", Project.workingdir, pName, ulNum);
	FILE * fFile = fopen(tTxt, "wb");

	if (!fFile)
	{
		delete pulMemorySnapShot;
		return false;
	}

	ulNum++;

	BITMAPFILEHEADER bfhBitmapFileHeader;
	bfhBitmapFileHeader.bfType = ('M' << 8) | ('B');
	bfhBitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + ((ddsd2.dwWidth * ddsd2.dwHeight) * 3);
	bfhBitmapFileHeader.bfReserved1 = bfhBitmapFileHeader.bfReserved2 = 0;
	bfhBitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	fwrite(&bfhBitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, fFile);

	BITMAPINFOHEADER bihBitmapInfoHeader;
	bihBitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bihBitmapInfoHeader.biWidth = ddsd2.dwWidth;
	bihBitmapInfoHeader.biHeight = ddsd2.dwHeight;
	bihBitmapInfoHeader.biPlanes = 1;
	bihBitmapInfoHeader.biBitCount = 24;
	bihBitmapInfoHeader.biCompression = BI_RGB;
	bihBitmapInfoHeader.biSizeImage = ((ddsd2.dwWidth * ddsd2.dwHeight) * 3);
	bihBitmapInfoHeader.biXPelsPerMeter = 0;
	bihBitmapInfoHeader.biYPelsPerMeter = 0;
	bihBitmapInfoHeader.biClrUsed = 0;
	bihBitmapInfoHeader.biClrImportant = 0;
	fwrite(&bihBitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, fFile);

	fwrite(pulMemorySnapShot, (ddsd2.dwWidth * ddsd2.dwHeight) * 3, 1, fFile);

	fclose(fFile);
	delete pulMemorySnapShot;
	return true;
}

//-----------------------------------------------------------------------------
void InitSnapShot(char * _pDir, char * _pName)
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
