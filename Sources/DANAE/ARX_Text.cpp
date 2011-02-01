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
// ARX_Text
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Text Management
//
// Updates: (date) (person) (update)
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
// Nuky - 10-02-11 - cleaned whole file

#include "ARX_Text.h"

#include "ARX_Loc.h"
#include "danae.h"

#include "EERIEDraw.h"

#include "HERMESmain.h"

#include <windows.h>
#include <cassert>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//-----------------------------------------------------------------------------
// Imported global variables

extern long CHINESE_VERSION;
extern long EAST_EUROPE;

//-----------------------------------------------------------------------------
// Exported global variables

CARXTextManager * pTextManage = NULL;

HMODULE hUnicodeLibrary = NULL;

HFONT hFontMainMenu		= NULL;
HFONT hFontMenu			= NULL;
HFONT hFontControls		= NULL;
HFONT hFontCredits		= NULL;
HFONT hFontInBook		= NULL;
HFONT hFontRedist		= NULL;
HFONT hFontInGame		= NULL;
HFONT hFontInGameNote	= NULL;

//-----------------------------------------------------------------------------
// Static local variables

namespace
{

// These are only valid if hUnicodeLibrary is not NULL
_TCHAR tFontResource[256] = {0};
typedef int (APIENTRY* ptrUnicodeFunc_AddFontResourceW)(const wchar_t *);
ptrUnicodeFunc_AddFontResourceW Unicows_AddFontResourceW = NULL;
typedef HFONT (APIENTRY* ptrUnicodeFunc_CreateFontW)(int,int,int,int,int,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPWSTR);
ptrUnicodeFunc_CreateFontW Unicows_CreateFontW = NULL;
typedef ATOM (APIENTRY* ptrUnicodeFunc_RemoveResourceW)(const wchar_t *);
ptrUnicodeFunc_RemoveResourceW Unicows_RemoveResourceW = NULL;
typedef bool (APIENTRY* ptrUnicodeFunc_TextOutW)(HDC,int,int,_TCHAR*,int);
ptrUnicodeFunc_TextOutW Unicows_TextOutW = NULL;

_TCHAR tUText[8192];

CARXTextManager * pTextManageFlyingOver = NULL;

} // \namespace

//-----------------------------------------------------------------------------
// Local utility functions
//
namespace
{

void FontError()
// XS : Lil' wrapper around FormatMessage & GetLastError
{
#ifdef _DEBUG
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		0, // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);

	MessageBox(NULL, (LPCSTR)lpMsgBuf, (LPCSTR)"Font Error", MB_OK | MB_ICONINFORMATION);
#endif
}

} // \namespace

//-----------------------------------------------------------------------------

long ARX_UNICODE_ForceFormattingInRect(HFONT _hFont, _TCHAR * _lpszUText, int _iSpacingY, RECT _rRect)
{
	int iTemp = 0;

	if (danaeApp.m_pddsRenderTarget)
	{
		HDC hDC;

		if (SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
		{
			int		iLenght	= _tcslen(_lpszUText);
			int		iHeight	= 0;
			SIZE	sSize;
			int		iOldTemp;
			bool	bWrite;

			sSize.cx = sSize.cy = 0 ;

			SelectObject(hDC, _hFont);

			for (;;)
			{
				bWrite			= true;
				int iLenghtCurr	= _rRect.left;
				iOldTemp		= iTemp;

				ARX_CHECK(iTemp < iLenght);

				for (; iTemp < iLenght ; iTemp++)
				{
					GetTextExtentPoint32W(hDC,
										  &_lpszUText[iTemp],
										  1,
										  &sSize);
					{
						if ((_lpszUText[iTemp] == _T('\n')) ||
								(_lpszUText[iTemp] == _T('*')))
						{
							iHeight		+= _iSpacingY + sSize.cy;
							bWrite		 = false;
							_rRect.top	+= _iSpacingY + sSize.cy;
							iTemp++;
							break;
						}
					}


					iLenghtCurr	+= sSize.cx;

					if (iLenghtCurr > _rRect.right)
					{
						iHeight += _iSpacingY + sSize.cy;

						if (CHINESE_VERSION)
						{
							iTemp--;
						}
						else
						{
							while ((_lpszUText[iTemp] != _T(' ')) && (iTemp > 0)) iTemp--;
						}

						bWrite		 = false;
						_rRect.top	+= _iSpacingY + sSize.cy;
						iTemp++;
						break;
					}
				}

				if ((iTemp == iLenght) ||
						((_rRect.top + sSize.cy) > _rRect.bottom))
				{
					break;
				}

			}

			danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
		}
	}

	return iTemp;
}

//-----------------------------------------------------------------------------
long ARX_UNICODE_FormattingInRect(HDC _hDC, _TCHAR * _lpszUText, int _iSpacingY, RECT & _rRect)
{
	if (!hUnicodeLibrary)
		return 0;

	int	iLenght = _tcslen(_lpszUText);
	int iHeight = 0;
	SIZE sSize;
	int iOldTemp;
	bool bWrite;
	sSize.cx = sSize.cy = 0;

	int iTemp = 0;

	for (;;)
	{
		bWrite = true;
		int iLenghtCurr = _rRect.left;
		iOldTemp = iTemp;

		for (; iTemp < iLenght; iTemp++)
		{
			GetTextExtentPoint32W(_hDC,
								  &_lpszUText[iTemp],
								  1,
								  &sSize);

			if ((_lpszUText[iTemp] == _T('\n')) ||
					(_lpszUText[iTemp] == _T('*')))
			{
				iHeight += _iSpacingY + sSize.cy;
				_lpszUText[iTemp] = _T('\0');
				bWrite = false;

				Unicows_TextOutW(_hDC, _rRect.left, _rRect.top, &_lpszUText[iOldTemp], _tcslen(&_lpszUText[iOldTemp]));
				_rRect.top += _iSpacingY + sSize.cy;
				iTemp++;
				break;
			}

			iLenghtCurr += sSize.cx;

			if (iLenghtCurr > _rRect.right)
			{
				iHeight += _iSpacingY + sSize.cy;

				if (CHINESE_VERSION)
				{
					_TCHAR * ptexttemp = (_TCHAR *)malloc((iTemp - iOldTemp + 1) << 1);
					_tcsncpy(ptexttemp, &_lpszUText[iOldTemp], iTemp - iOldTemp);
					ptexttemp[iTemp-iOldTemp] = _T('\0');

					Unicows_TextOutW(_hDC, _rRect.left, _rRect.top, ptexttemp, _tcslen(ptexttemp));
					free((void *)ptexttemp);
					ptexttemp = NULL;
					iTemp--;
				}
				else
				{
					while ((_lpszUText[iTemp] != _T(' ')) && (iTemp > 0)) iTemp--;

					_lpszUText[iTemp] = _T('\0');

					Unicows_TextOutW(_hDC, _rRect.left, _rRect.top, &_lpszUText[iOldTemp], _tcslen(&_lpszUText[iOldTemp]));
				}

				bWrite = false;
				_rRect.top += _iSpacingY + sSize.cy;
				iTemp++;
				break;
			}


		}

		if (iTemp == iLenght) break;

		if (iTemp == iOldTemp) break;
	}

	if (bWrite)
	{
		iHeight += _iSpacingY + sSize.cy;

		if (!Unicows_TextOutW(_hDC, _rRect.left, _rRect.top, &_lpszUText[iOldTemp], _tcslen(&_lpszUText[iOldTemp])))
		{
			FontError();
		}

		_rRect.top += _iSpacingY + sSize.cy;
	}

	return iHeight;
}

//-----------------------------------------------------------------------------
long ARX_UNICODE_DrawTextInRect(float x, float y,
								float maxx, float maxy,
								_TCHAR * _lpszUText,
								COLORREF col,
								COLORREF bcol,
								HFONT font,
								HRGN hRgn,
								HDC hHDC
							   )
{
	HDC hDC = NULL;

	// Get a DC for the surface. Then, write out the buffer
	if (danaeApp.m_pddsRenderTarget)
	{
		if (hHDC)
		{
			hDC = hHDC;
		}

		if (hHDC || SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
		{

			_tcscpy(tUText, _lpszUText);

			if (hRgn)
			{
				SelectClipRgn(hDC,
							  hRgn);
			}

			if (bcol == 0x00FF00FF) SetBkMode(hDC, TRANSPARENT);
			else
			{
				SetBkMode(hDC, OPAQUE);
				SetBkColor(hDC, bcol);
			}

			SetTextColor(hDC, col);

			SelectObject(hDC,  font);

			RECT rect;
			rect.top	= (long)y;
			rect.left	= (long)x;
			rect.right	= (long)maxx;
			long n		= ARX_UNICODE_FormattingInRect(hDC, tUText, 0, rect);
			rect.top	= (long)y;
			rect.bottom	= ((long)y) + n;

			SelectClipRgn(hDC,
						  NULL);

			if (!hHDC)
			{
				danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
			}

			return n;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
long ARX_TEXT_Draw(LPDIRECT3DDEVICE7 pd3dDevice,
				   HFONT ef,
				   float x, float y,
				   long spacingx, long spacingy,
				   _TCHAR * car,
				   COLORREF colo, COLORREF bcol)
{
	if (car == NULL) return 0;

	if (car[0] == 0) return 0;

	//ArxFont
	ARX_UNICODE_DrawTextInRect(x, y, 9999.f, 999.f, car, colo, bcol, ef);
	return 15 + spacingy;
}

long ARX_TEXT_DrawRect(LPDIRECT3DDEVICE7 pd3dDevice,
					   HFONT ef,
					   float x, float y,
					   long spacingx, long spacingy,
					   float maxx, float maxy,
					   _TCHAR * car,
					   COLORREF colo,
					   HRGN _hRgn,
					   COLORREF bcol,
					   long flags)
{

	bcol = RGB((bcol >> 16) & 255, (bcol >> 8) & 255, (bcol) & 255);

	colo = RGB((colo >> 16) & 255, (colo >> 8) & 255, (colo) & 255);
	return ARX_UNICODE_DrawTextInRect(x, y, maxx, maxy, car, colo, bcol, ef, _hRgn);
}


//-----------------------------------------------------------------------------
float DrawBookTextInRect(float x, float y, float maxx, float maxy, _TCHAR * text, COLORREF col, COLORREF col2, HFONT font)
{
	return (float)ARX_TEXT_DrawRect(GDevice, font,
									(BOOKDECX + x) * Xratio, (BOOKDECY + y) * Yratio, -3, 0,
									(BOOKDECX + maxx) * Xratio, (BOOKDECY + maxy) * Yratio, text, col, NULL, col2);
}

//-----------------------------------------------------------------------------
void DrawBookTextCenter(float x, float y, _TCHAR * text, COLORREF col, COLORREF col2, HFONT font)
{
	UNICODE_ARXDrawTextCenter((BOOKDECX + x)*Xratio, (BOOKDECY + y)*Yratio, text, col, col2, font);
}

//-----------------------------------------------------------------------------

long UNICODE_ARXDrawTextCenter(float x, float y, _TCHAR * str, COLORREF col, COLORREF bcol, HFONT font)
{
	HDC hDC;

	// Get a DC for the surface. Then, write out the buffer
	if (danaeApp.m_pddsRenderTarget)
	{
		if (SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
		{
			if (bcol == 0x00FF00FF) SetBkMode(hDC, TRANSPARENT);
			else
			{
				SetBkMode(hDC, OPAQUE);
				SetBkColor(hDC, bcol);
			}

			SetTextColor(hDC, col);

			SelectObject(hDC,  font);


			SIZE siz;
			GetTextExtentPoint32W(hDC,         // handle to DC
									str,           // character string
									_tcslen(str),   // number of characters
									&siz          // size
								   );
			RECT rect;
			rect.top = (long)y;
			rect.bottom = (long)999;
			rect.left = (long)x - (siz.cx >> 1);
			rect.right = (long)999;

			TextOutW(hDC, rect.left, rect.top, str, _tcslen(str));

			danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
			return siz.cx;
		}
	}

	return 0;
}



long UNICODE_ARXDrawTextCenteredScroll(float x, float y, float x2, _TCHAR * str, COLORREF col, COLORREF bcol, HFONT font, int iTimeScroll, float fSpeed, int iNbLigne, int iTimeOut)
{

	RECT rRect;
	ARX_CHECK_LONG(y);
	ARX_CHECK_LONG(x + x2);   //IF OK, x - x2 cannot overflow
	rRect.left	=	ARX_CLEAN_WARN_CAST_LONG(x - x2);
	rRect.top	=	ARX_CLEAN_WARN_CAST_LONG(y);
	rRect.right	=	ARX_CLEAN_WARN_CAST_LONG(x + x2);


	if (pTextManage)
	{
		pTextManage->AddText(font,
							 str,
							 rRect,
							 col,
							 bcol,
							 iTimeOut,
							 iTimeScroll,
							 fSpeed,
							 iNbLigne
							);

		return ARX_CLEAN_WARN_CAST_LONG(x2);
	}

	return 0;
}

//-----------------------------------------------------------------------------
void ARX_Allocate_Text(_TCHAR *&dest, _TCHAR * id_string)
{
	if (dest != NULL)
	{
		free(dest);
		dest = NULL;
	}

	_TCHAR output[4096];
	PAK_UNICODE_GetPrivateProfileString(id_string, _T("string"), _T("default"), output, 4096, NULL);
	dest = (_TCHAR *)malloc((_tcslen(output) + 1) * sizeof(_TCHAR));
	_tcscpy(dest, output);
}

//-----------------------------------------------------------------------------
struct _FONT_HEADER
{
	ULONG	ulVersion;
	USHORT 	usNumTables;
	USHORT 	usSearchRange;
	USHORT 	usEntrySelector;
	USHORT 	usRangeShift;
};

//-----------------------------------------------------------------------------
struct _FONT_TABLE_HEADER
{
	ULONG	ulTag;
	ULONG	ulCheckSum;
	ULONG	ulOffset;
	ULONG	ulLength;
};

//-----------------------------------------------------------------------------
struct _FONT_NAMING_HEADER
{
	USHORT	usFormat;
	USHORT	usNbNameRecords;
	USHORT	usOffsetStorage;	//(from start of table)
};

//-----------------------------------------------------------------------------
struct _FONT_NAMING_NAMERECORD
{
	USHORT	usPlatformID;
	USHORT	usPlatformSpecificEncodingID;
	USHORT	usLanguageID;
	USHORT	usNameID;
	USHORT	usStringLength;
	USHORT	usStringOffset;		//from start of storage area (in bytes)
};

//-----------------------------------------------------------------------------
ULONG LilEndianLong(ULONG ulValue)
{
	return (
			   MAKELONG(
				   MAKEWORD(HIBYTE(HIWORD(ulValue)), LOBYTE(HIWORD(ulValue))),
				   MAKEWORD(HIBYTE(LOWORD(ulValue)), LOBYTE(LOWORD(ulValue)))
			   )
		   );
}

//-----------------------------------------------------------------------------
USHORT LilEndianShort(USHORT ulValue)
{
	return (
			   MAKEWORD(HIBYTE(ulValue), LOBYTE(ulValue))//,
		   );
}

//-----------------------------------------------------------------------------
_TCHAR * GetFontName(char * _lpszFileName)
{
	DWORD dwSize;
	DWORD dwRead;
	int   iResult;

	HANDLE hFile = CreateFile(_lpszFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "FontName :: File not Found", _lpszFileName, MB_OK);
		return NULL;
	}

	dwSize = GetFileSize(hFile, NULL);

	// HEADER
	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
	_FONT_HEADER FH;
	iResult = ReadFile(hFile, &FH, sizeof(FH), &dwRead, NULL);

	if (iResult == 0)
	{
		MessageBox(NULL, "FontName :: Not read!", "", MB_OK);
	}

	// TABLE HEADERS
	for (int i = 0; i < FH.usNumTables; i++)
	{
		_FONT_TABLE_HEADER FTH;
		iResult = ReadFile(hFile, &FTH, sizeof(FTH), &dwRead, NULL);

		if (iResult == 0)
		{
			MessageBox(NULL, "FontName :: Not read!", "", MB_OK);
		}

		char szName[5];
		szName[0] = LOBYTE(LOWORD(FTH.ulTag));
		szName[1] = HIBYTE(LOWORD(FTH.ulTag));
		szName[2] = LOBYTE(HIWORD(FTH.ulTag));
		szName[3] = HIBYTE(HIWORD(FTH.ulTag));
		szName[4] = 0;



		if (strcmp(szName, "name") == 0)
		{
			FTH.ulOffset = LilEndianLong(FTH.ulOffset);
			SetFilePointer(hFile, FTH.ulOffset, NULL, FILE_BEGIN);

			_FONT_NAMING_HEADER FNH;
			iResult = ReadFile(hFile, &FNH, sizeof(FNH), &dwRead, NULL);

			if (iResult == 0)
			{
				MessageBox(NULL, "FontName :: Not read!", "", MB_OK);
			}

			FNH.usNbNameRecords = LilEndianShort(FNH.usNbNameRecords);
			FNH.usOffsetStorage = LilEndianShort(FNH.usOffsetStorage);

			for (int j = 0; j < FNH.usNbNameRecords; j++)
			{
				_FONT_NAMING_NAMERECORD FNN;
				iResult = ReadFile(hFile, &FNN, sizeof(FNN), &dwRead, NULL);

				if (iResult == 0)
				{
					MessageBox(NULL, "FontName :: Not read!", "", MB_OK);
				}

				FNN.usNameID = LilEndianShort(FNN.usNameID);
				FNN.usPlatformID = LilEndianShort(FNN.usPlatformID);
				FNN.usStringLength = LilEndianShort(FNN.usStringLength);
				FNN.usStringOffset = LilEndianShort(FNN.usStringOffset);
				FNN.usLanguageID = LilEndianShort(FNN.usLanguageID);

				if (FNN.usLanguageID == 1033)
					if (FNN.usNameID == 1)
					{
						SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
						SetFilePointer(hFile, FTH.ulOffset + FNH.usOffsetStorage + FNN.usStringOffset, NULL, FILE_BEGIN);


						wchar_t szName[256];



						ZeroMemory(szName, 256);
						assert(FNN.usStringLength < 256);

						iResult = ReadFile(hFile, szName, FNN.usStringLength, &dwRead, NULL);

						if (iResult == 0)
						{
							MessageBox(NULL, "FontName :: Not read!", "", MB_OK);
						}

						for (int k = 0; k<(FNN.usStringLength >> 1); k++)
						{
							szName[k] = LilEndianShort(szName[k]);
						}


						_TCHAR * szText = new _TCHAR[(FNN.usStringLength>>1)+1]; //@HACK
						_tcscpy(szText, szName);

						CloseHandle(hFile);
						return szText;
					}
			}

		}
	}

	CloseHandle(hFile);

	return NULL;
}

void _ShowText(char * text)
{
	if (GDevice)
	{
		GDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0L);

		if (danaeApp.DANAEStartRender())
		{

			HDC hDC;

			if (danaeApp.m_pddsRenderTarget)
			{
				if (SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
				{
					SetTextColor(hDC, RGB(0, 255, 0));
					SetBkMode(hDC, TRANSPARENT);
					ExtTextOut(hDC, 0, 0, 0, NULL, text, lstrlen(text), NULL);
					danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
				}
			}

			danaeApp.DANAEEndRender();

			danaeApp.m_pFramework->ShowFrame();
		}
	}
}

//-----------------------------------------------------------------------------
// CARXTextManager
//-----------------------------------------------------------------------------
CARXTextManager::CARXTextManager()
{
}

CARXTextManager::~CARXTextManager()
{
	Clear();
}

bool CARXTextManager::AddText(HFONT font, _TCHAR* str, const RECT& rect, long fgcolor, long bgcolor, long timeout, long scrolltime, float scrollspeed, int iNbLigneClipp)
{
	if (!str || !font)
		return false;

	ARX_TEXT* newText = (ARX_TEXT*)malloc(sizeof(ARX_TEXT));
	if (!newText)
		return false;

	newText->lpszUText = new _TCHAR[_tcsclen(str)+1];
	if (!newText->lpszUText)
	{
		free((void*)newText);
		return false;
	}

	newText->hFont = font;
	_tcscpy(newText->lpszUText, str);
	newText->eType			= ARX_TEXT_STAY;
	newText->rRect			= rect;
	newText->lCol			= fgcolor;
	newText->lBkgCol		= bgcolor;
	newText->lTimeScroll	= scrolltime;
	newText->fDeltaY		= 0.f;
	newText->fSpeedScrollY	= scrollspeed;
	newText->lTimeOut		= timeout;

	if (iNbLigneClipp)
	{
		HDC hDC;
		SIZE sSize;

		if (SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
		{
			SelectObject(hDC, newText->hFont);
			GetTextExtentPoint32W(hDC,
								  newText->lpszUText,
								  _tcslen(newText->lpszUText),
								  &sSize);
			danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
			newText->lTailleLigne = sSize.cy * iNbLigneClipp;
		}
		else
		{
			sSize.cy = rect.bottom - rect.top;
			newText->lTailleLigne = sSize.cy;
		}

		SetRect(&newText->rRectClipp,
				newText->rRect.left,
				newText->rRect.top,
				newText->rRect.right,
				newText->rRect.top + sSize.cy);
	}
	else
	{
		newText->rRectClipp = newText->rRect;
	}

	vText_.push_back(newText);
	return true;
}

void CARXTextManager::Update(float dt)
{
	ARX_CHECK_INT(dt);
	int delta = ARX_CLEAN_WARN_CAST_INT(dt);

	for (std::vector<ARX_TEXT*>::iterator it = vText_.begin(); it != vText_.end(); )
	{
		ARX_TEXT * pArxText = *it;

		if (pArxText)
		{
			if (pArxText->lTimeOut < 0)
			{
				delete[] pArxText->lpszUText;
				pArxText->lpszUText = NULL;
				free((void *)pArxText);
				pArxText = NULL;
				it = vText_.erase(it);
				continue;
			}

			pArxText->lTimeOut -= delta;

			if (pArxText->lTimeScroll < 0 && pArxText->fDeltaY < (pArxText->rRect.bottom - pArxText->rRectClipp.bottom))
			{
				pArxText->fDeltaY += pArxText->fSpeedScrollY * (float)delta;
				if (pArxText->fDeltaY >= (pArxText->rRect.bottom - pArxText->rRectClipp.bottom))
					pArxText->fDeltaY = ARX_CLEAN_WARN_CAST_FLOAT(pArxText->rRect.bottom - pArxText->rRectClipp.bottom);
			}
			else
				pArxText->lTimeScroll -= delta;
		}

		++it;
	}
}

//-----------------------------------------------------------------------------
void CARXTextManager::Render()
{
	std::vector<ARX_TEXT *>::iterator itManage;

	itManage = vText_.begin();

	HDC hDC = NULL;

	if (danaeApp.m_pddsRenderTarget && vText_.size())
	{
		danaeApp.m_pddsRenderTarget->GetDC(&hDC);
	}

	while (itManage != vText_.end())
	{
		ARX_TEXT * pArxText = *itManage;

		if (pArxText)
		{
			HRGN hRgn = NULL;
			hRgn = CreateRectRgn(pArxText->rRectClipp.left,
								 pArxText->rRectClipp.top,
								 pArxText->rRectClipp.right,
								 pArxText->rRectClipp.bottom);


			pArxText->rRect.bottom = pArxText->rRect.top + ARX_UNICODE_DrawTextInRect(ARX_CLEAN_WARN_CAST_FLOAT(pArxText->rRect.left),
									 pArxText->rRect.top - pArxText->fDeltaY,
									 ARX_CLEAN_WARN_CAST_FLOAT(pArxText->rRect.right),
									 0,
									 pArxText->lpszUText,
									 pArxText->lCol,
									 pArxText->lBkgCol,
									 pArxText->hFont,
									 hRgn,
									 hDC);


			if (hRgn)
			{
				DeleteObject(hRgn);
			}

			if (pArxText->eType == ARX_TEXT_ONCE)
			{
				delete[] pArxText->lpszUText;
				pArxText->lpszUText = NULL;
				free((void *)pArxText);
				pArxText = NULL;
				itManage = vText_.erase(itManage);
				continue;
			}
		}

		itManage++;
	}

	if (hDC)
	{
		danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
	}
}

void CARXTextManager::Clear()
{
	for (std::vector<ARX_TEXT*>::iterator it = vText_.begin(), it_end = vText_.end(); it != it_end; ++it)
	{
		if (ARX_TEXT* text = *it)
		{
			if (text->lpszUText)
				delete[] text->lpszUText;
			free((void*)text);
		}
	}

	vText_.clear();
}

bool CARXTextManager::HasText() const
{
	return !vText_.empty();
}

//-----------------------------------------------------------------------------

HFONT CreateFontFromProfile(_TCHAR* faceName, const _TCHAR* section, _TCHAR* defaultFontSize)
{
	if (!hUnicodeLibrary)
		return NULL;

	_TCHAR szUT[256];
	PAK_UNICODE_GetPrivateProfileString(section, _T("string"), defaultFontSize, szUT, 256, NULL);
	int iFontSize = _ttoi(szUT);

	// Apply fixes to iFontSize
	iFontSize = (int)(float)(iFontSize * Yratio);
	if (CHINESE_VERSION)
	{
		if      (iFontSize < 14)	iFontSize = 12;
		else if (iFontSize < 15)	iFontSize = 14;
		else if (iFontSize < 18)	iFontSize = 15;
		else if (iFontSize < 30)	iFontSize = 18;
		else						iFontSize = 30;
	}

	HFONT ret = Unicows_CreateFontW(iFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
									EAST_EUROPE ? CHINESEBIG5_CHARSET : DEFAULT_CHARSET,
									OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
									ANTIALIASED_QUALITY, VARIABLE_PITCH, faceName);

	if (!ret)
		FontError();

	return ret;
}

//-----------------------------------------------------------------------------

void ARX_Text_Init()
{
	ARX_Text_Close();

	// Load unicows
	hUnicodeLibrary = LoadLibraryA("unicows.dll");
	if (!hUnicodeLibrary)
		exit(0);
	Unicows_AddFontResourceW = (ptrUnicodeFunc_AddFontResourceW)GetProcAddress(hUnicodeLibrary, "AddFontResourceW");
	Unicows_CreateFontW      = (ptrUnicodeFunc_CreateFontW)     GetProcAddress(hUnicodeLibrary, "CreateFontW");
	Unicows_RemoveResourceW  = (ptrUnicodeFunc_RemoveResourceW) GetProcAddress(hUnicodeLibrary, "RemoveFontResourceW");
	Unicows_TextOutW         = (ptrUnicodeFunc_TextOutW)        GetProcAddress(hUnicodeLibrary, "TextOutW");

	ARX_Localisation_Init();

	// Add the font file to the font resources
	char tx[256];
	sprintf(tx, "%smisc\\%s", Project.workingdir, "arx.ttf"); // Full path
	if (!FileExist(tx))
		sprintf(tx, "%smisc\\%s", Project.workingdir, "arx_default.ttf"); // Full path
	MultiByteToWideChar(CP_ACP, 0, tx , -1, tFontResource, 256);
	if (Unicows_AddFontResourceW(tFontResource) == 0)
		FontError();
	_TCHAR* lpszFont = GetFontName(tx);

	pTextManage = new CARXTextManager();
	pTextManageFlyingOver = new CARXTextManager();

	hFontMainMenu   = CreateFontFromProfile(lpszFont, _T("system_font_mainmenu_size"),     _T("58"));
	hFontMenu       = CreateFontFromProfile(lpszFont, _T("system_font_menu_size"),         _T("32"));
	hFontControls   = CreateFontFromProfile(lpszFont, _T("system_font_menucontrols_size"), _T("22"));
	hFontCredits    = CreateFontFromProfile(lpszFont, _T("system_font_menucredits_size"),  _T("36"));
	hFontRedist     = CreateFontFromProfile(lpszFont, _T("system_font_redist_size"),       _T("18"));
	if (Yratio > 1.f) Yratio *= .8f; // NEW QUEST
	hFontInGame     = CreateFontFromProfile(lpszFont, _T("system_font_book_size"),         _T("18"));
	hFontInGameNote = CreateFontFromProfile(lpszFont, _T("system_font_note_size"),         _T("18"));
	hFontInBook     = CreateFontFromProfile(lpszFont, _T("system_font_book_size"),         _T("18"));

	delete[] lpszFont;
}

//-----------------------------------------------------------------------------
void ARX_Text_Close()
{
	if (hUnicodeLibrary)
	{
		if (Unicows_RemoveResourceW(tFontResource) == 0)
			FontError();
		tFontResource[0] = _T('\0');
		Unicows_AddFontResourceW = NULL;
		Unicows_CreateFontW = NULL;
		Unicows_RemoveResourceW = NULL;
	}

	ARX_Localisation_Close();

	delete pTextManage;
	pTextManage = NULL;
	delete pTextManageFlyingOver;
	pTextManageFlyingOver = NULL;

	DeleteObject(hFontInBook);
	hFontInBook = NULL;
	DeleteObject(hFontRedist);
	hFontRedist = NULL;
	DeleteObject(hFontMainMenu);
	hFontMainMenu = NULL;
	DeleteObject(hFontMenu);
	hFontMenu = NULL;
	DeleteObject(hFontControls);
	hFontControls = NULL;
	DeleteObject(hFontCredits);
	hFontCredits = NULL;
	DeleteObject(hFontInGame);
	hFontInGame = NULL;
	DeleteObject(hFontInGameNote);
	hFontInGameNote = NULL;

	FreeLibrary(hUnicodeLibrary);
	hUnicodeLibrary = NULL;
}
