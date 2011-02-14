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
#include <windows.h>

#include "ARX_Loc.h"
#include "ARX_Text.h"
#include "Danae.h"


#include "EERIEDraw.h"
#include "HERMESMain.h"

#include <assert.h>

//-----------------------------------------------------------------------------
_TCHAR * lpszFontMenu = NULL;
_TCHAR * lpszFontIngame = NULL;

_TCHAR tUText[8192];

CARXTextManager * pTextManageFlyingOver = NULL;

//-----------------------------------------------------------------------------

extern long CHINESE_VERSION;
extern long EAST_EUROPE;

//-----------------------------------------------------------------------------
// Exported global variables

CARXTextManager * pTextManage = NULL;

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

HMODULE hUnicodeLibrary = NULL;

// These are only valid if hUnicodeLibrary is not NULL
_TCHAR tFontResource[256] = {0};
typedef int (APIENTRY* ptrUnicodeFunc_AddFontResourceW)(const wchar_t *);
ptrUnicodeFunc_AddFontResourceW Unicows_AddFontResourceW = NULL;
typedef HFONT (APIENTRY* ptrUnicodeFunc_CreateFontW)(int,int,int,int,int,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPWSTR);
ptrUnicodeFunc_CreateFontW Unicows_CreateFontW = NULL;
typedef ATOM (APIENTRY* ptrUnicodeFunc_RemoveResourceW)(const wchar_t *);
ptrUnicodeFunc_RemoveResourceW Unicows_RemoveResourceW = NULL;
typedef bool (APIENTRY* ptrUnicodeFunc_TextOutW)(HDC,int,int,const _TCHAR*,int);
ptrUnicodeFunc_TextOutW Unicows_TextOutW = NULL;

} // \namespace

//-----------------------------------------------------------------------------
// Local utility classes

enum ARX_TEXT_TYPE
{
	ARX_TEXT_ONCE,
	ARX_TEXT_STAY
};

class ARXText
{
public:
	ARXText(HFONT font, const _TCHAR* str, const RECT& rect, long fgcolor, long bgcolor,
		long timeout, long scrolltime, float scrollspeed, unsigned maxlines);

public:
	void Update();

public:
	ARX_TEXT_TYPE	eType;
	HFONT			hFont;
	RECT			rRect;
	RECT			rRectClipp;
	std::wstring	str;
	long			fgcolor;
	long			bgcolor;
	long			height;
	long			timeout;		// dynamic
	float			offsety;		// dynamic
	float			scrollspeed;
	long			scrolltime;

private:
	// no copy
	ARXText(const ARXText&);
	ARXText& operator=(const ARXText&);
};

//-----------------------------------------------------------------------------
// Local utility functions

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

long ARX_UNICODE_FormattingInRect(HDC hdc, const _TCHAR* str, int _iSpacingY, RECT& _rRect)
{
	if (!hUnicodeLibrary)
		return 0;

	int	len = _tcslen(str);
	int iHeight = 0;
	bool bWrite = false;

	SIZE sSize;
	int prev_i;
	sSize.cx = sSize.cy = 0;

	int i = 0;

	for (;;)
	{
		bWrite = true;
		int iLenghtCurr = _rRect.left;
		prev_i = i;

		for (; i < len; ++i)
		{
			GetTextExtentPoint32(hdc, &str[i], 1, &sSize);

			if (str[i] == _T('\n') || str[i] == _T('*'))
			{
				iHeight += _iSpacingY + sSize.cy;
				bWrite = false;

				Unicows_TextOutW(hdc, _rRect.left, _rRect.top, &str[prev_i], i-prev_i);
				_rRect.top += _iSpacingY + sSize.cy;
				i++;
				break;
			}

			iLenghtCurr += sSize.cx;

			if (iLenghtCurr > _rRect.right)
			{
				iHeight += _iSpacingY + sSize.cy;

				if (CHINESE_VERSION)
				{
					_TCHAR * ptexttemp = (_TCHAR *)malloc((i - prev_i + 1) << 1);
					_tcsncpy(ptexttemp, &str[prev_i], i - prev_i);
					ptexttemp[i-prev_i] = _T('\0');

					Unicows_TextOutW(hdc, _rRect.left, _rRect.top, ptexttemp, _tcslen(ptexttemp));
					free((void *)ptexttemp);
					--i;
				}
				else
				{
					while (str[i] != _T(' ') && i > 0)
						--i;
					Unicows_TextOutW(hdc, _rRect.left, _rRect.top, &str[prev_i], i-prev_i);
				}

				bWrite = false;
				_rRect.top += _iSpacingY + sSize.cy;
				i++;
				break;
			}
		}

		if (i == len || i == prev_i)
			break;
	}


	if (bWrite)
	{
		iHeight += _iSpacingY + sSize.cy;

		if (!Unicows_TextOutW(hdc, _rRect.left, _rRect.top, &str[prev_i], _tcslen(&str[prev_i])))
			FontError();

		_rRect.top += _iSpacingY + sSize.cy;
	}

	return iHeight;
}

long ARX_UNICODE_DrawTextInRect(float x, float y, float maxx, float maxy, const _TCHAR* text,
								COLORREF col, COLORREF bcol, HFONT font, HRGN hRgn = NULL, HDC hHDC = NULL)
{
	if (!danaeApp.m_pddsRenderTarget)
		return 0;

	HDC hDC = hHDC;
	if (hHDC || SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
	{
		if (hRgn)
			SelectClipRgn(hDC, hRgn);

		if (bcol == 0x00FF00FF)
			SetBkMode(hDC, TRANSPARENT);
		else
		{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, bcol);
		}

		SetTextColor(hDC, col);

		SelectObject(hDC, font);

		RECT rect;
		rect.top	= (long)y;
		rect.left	= (long)x;
		rect.right	= (long)maxx;
		long n		= ARX_UNICODE_FormattingInRect(hDC, text, 0, rect);

		SelectClipRgn(hDC, NULL);

		if (!hHDC)
			danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);

		return n;
	}

	return 0;
}

} // \namespace

//-----------------------------------------------------------------------------
long ARX_UNICODE_ForceFormattingInRect(HFONT font, const _TCHAR* text, int linespacing, RECT rect)
{
	if (!danaeApp.m_pddsRenderTarget)
		return 0;

	int i = 0;
	HDC hDC;
	if (SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
	{
		int		len	= _tcslen(text);
		SIZE	size;

		size.cx = size.cy = 0 ;

		SelectObject(hDC, font);

		for (;;)
		{
			int width = rect.left;

			for (; i < len; ++i)
			{
				GetTextExtentPoint32(hDC, &text[i], 1, &size);

				if (text[i] == _T('\n') || text[i] == _T('*'))
				{
					rect.top += linespacing + size.cy;
					++i;
					break;
				}

				width += size.cx;

				if (width > rect.right)
				{
					if (CHINESE_VERSION)
						--i;
					else
						while (text[i] != _T(' ') && i > 0)
							--i;

					rect.top += linespacing + size.cy;
					++i;
					break;
				}
			}

			if (i == len || (rect.top + size.cy) > rect.bottom)
				break;
		}

		danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
	}

	return i;
}

//-----------------------------------------------------------------------------
void ARX_TEXT_Draw(HFONT ef, float x, float y, const _TCHAR* car, COLORREF colo, COLORREF bcol)
{
	if (!car || car[0] == _T('\0'))
		return;

	ARX_UNICODE_DrawTextInRect(x, y, 9999.f, 999.f, car, colo, bcol, ef);
}

long ARX_TEXT_DrawRect(HFONT ef, float x, float y, float maxx, float maxy, const _TCHAR* car,
					   COLORREF fgcolor, HRGN _hRgn, COLORREF bgcolor)
{
	bgcolor = RGB((bgcolor >> 16) & 255, (bgcolor >> 8) & 255, (bgcolor) & 255);
	fgcolor = RGB((fgcolor >> 16) & 255, (fgcolor >> 8) & 255, (fgcolor) & 255);

	return ARX_UNICODE_DrawTextInRect(x, y, maxx, maxy, car, fgcolor, bgcolor, ef, _hRgn);
}

//-----------------------------------------------------------------------------
void DrawBookTextInRect(HFONT font, float x, float y, float maxx, float maxy, const _TCHAR* text, COLORREF fgcolor, COLORREF bgcolor)
{
	ARX_TEXT_DrawRect(font, (BOOKDECX + x) * Xratio, (BOOKDECY + y) * Yratio, (BOOKDECX + maxx) * Xratio,
					  (BOOKDECY + maxy) * Yratio, text, fgcolor, NULL, bgcolor);
}

//-----------------------------------------------------------------------------
void DrawBookTextCenter(HFONT font, float x, float y, const _TCHAR* str, COLORREF fgcol, COLORREF bgcol)
{
	UNICODE_ARXDrawTextCenter(font, (BOOKDECX + x) * Xratio, (BOOKDECY + y) * Yratio, str, fgcol, bgcol);
}

//-----------------------------------------------------------------------------

void UNICODE_ARXDrawTextCenter(HFONT font, float x, float y, const _TCHAR* str, COLORREF fgcol, COLORREF bgcol)
{
	HDC hDC;

	if (danaeApp.m_pddsRenderTarget && SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
	{
		if (bgcol == 0x00FF00FF)
			SetBkMode(hDC, TRANSPARENT);
		else
		{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, bgcol);
		}

		SetTextColor(hDC, fgcol);

		SelectObject(hDC, font);

		SIZE siz;
		GetTextExtentPoint32(hDC, str, _tcslen(str), &siz);

		TextOut(hDC, static_cast<int>(x - siz.cx / 2), static_cast<int>(y), str, _tcslen(str));

		danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
	}
}

void UNICODE_ARXDrawTextCenteredScroll(HFONT font, float x, float y, float x2, const _TCHAR* str, COLORREF col, COLORREF bcol, int iTimeScroll, float fSpeed, int iNbLigne, int iTimeOut)
{
	RECT rRect;
	ARX_CHECK_LONG(y);
	ARX_CHECK_LONG(x + x2);   //IF OK, x - x2 cannot overflow
	rRect.left	=	ARX_CLEAN_WARN_CAST_LONG(x - x2);
	rRect.top	=	ARX_CLEAN_WARN_CAST_LONG(y);
	rRect.right	=	ARX_CLEAN_WARN_CAST_LONG(x + x2);

	if (pTextManage)
		pTextManage->AddText(font, str, rRect, col, bcol, iTimeOut, iTimeScroll, fSpeed, iNbLigne);
}

//-----------------------------------------------------------------------------
void ARX_Allocate_Text(_TCHAR*& dest, const _TCHAR* id_string)
{
	free(dest);

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
	return (MAKELONG(MAKEWORD(HIBYTE(HIWORD(ulValue)), LOBYTE(HIWORD(ulValue))),
					 MAKEWORD(HIBYTE(LOWORD(ulValue)), LOBYTE(LOWORD(ulValue)))));
}

//-----------------------------------------------------------------------------
USHORT LilEndianShort(USHORT ulValue)
{
	return (MAKEWORD(HIBYTE(ulValue), LOBYTE(ulValue)));
}

//-----------------------------------------------------------------------------
namespace
{

_TCHAR* GetFontName(const char* fontFile)
{
	DWORD dwSize;
	DWORD dwRead;
	int   iResult;

	HANDLE hFile = CreateFile(fontFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, "FontName :: File not Found", fontFile, MB_OK);
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
				MessageBox(NULL, "FontName :: Not read!", "", MB_OK);

			FNH.usNbNameRecords = LilEndianShort(FNH.usNbNameRecords);
			FNH.usOffsetStorage = LilEndianShort(FNH.usOffsetStorage);

			for (int j = 0; j < FNH.usNbNameRecords; j++)
			{
				_FONT_NAMING_NAMERECORD FNN;
				iResult = ReadFile(hFile, &FNN, sizeof(FNN), &dwRead, NULL);

				if (iResult == 0)
					MessageBox(NULL, "FontName :: Not read!", "", MB_OK);

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

} // \namespace

void _ShowText(const char* text)
{
	if (GDevice)
	{
		GDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0L);

		if (danaeApp.DANAEStartRender())
		{
			if (danaeApp.m_pddsRenderTarget)
			{
				HDC hDC;
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
// ARXText
//-----------------------------------------------------------------------------

ARXText::ARXText(HFONT font, const _TCHAR* _str, const RECT& rect, long fgcolor, long bgcolor,
				 long timeout, long scrolltime, float scrollspeed, unsigned maxlines)
: eType(ARX_TEXT_STAY)
, hFont(font)
, rRect(rect)
, rRectClipp(rect)
, str(_str)
, fgcolor(fgcolor)
, bgcolor(bgcolor)
, timeout(timeout)
, offsety(0.f)
, scrollspeed(scrollspeed)
, scrolltime(scrolltime)
{
	if (maxlines)
	{
		HDC hDC;
		if (SUCCEEDED(danaeApp.m_pddsRenderTarget->GetDC(&hDC)))
		{
			SIZE sSize;
			SelectObject(hDC, hFont);
			GetTextExtentPoint32W(hDC, str.c_str(), str.length(), &sSize);
			danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
			rRectClipp.bottom = rRectClipp.top + sSize.cy * maxlines;
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

void CARXTextManager::AddText(HFONT font, const _TCHAR* str, const RECT& rect, long fgcolor, long bgcolor, long timeout, long scrolltime, float scrollspeed, int maxlines)
{
	if (!str || !font)
		return;

	ARXText* newText = new ARXText(font, str, rect, fgcolor, bgcolor, timeout, scrolltime, scrollspeed, maxlines);
	vText_.push_back(newText);
}

void CARXTextManager::AddText(HFONT font, const _TCHAR* str, long x, long y, long fgcolor)
{
	RECT r;
	r.left = x;
	r.top = y;
	r.right = 9999;
	r.bottom = 9999;
	AddText(font, str, r, fgcolor, 0x00FF00FF);
}

void CARXTextManager::Update(float dt)
{
	ARX_CHECK_INT(dt);
	int delta = ARX_CLEAN_WARN_CAST_INT(dt);

	for (std::vector<ARXText*>::iterator it = vText_.begin(); it != vText_.end(); )
	{
		ARXText * text = *it;
		assert(text);

		if (text->timeout < 0)
		{
			delete text;
			it = vText_.erase(it);
			continue;
		}

		text->timeout -= delta;

		if (text->scrolltime < 0 && text->offsety < (text->rRect.bottom - text->rRectClipp.bottom))
		{
			text->offsety += text->scrollspeed * (float)delta;
			if (text->offsety >= (text->rRect.bottom - text->rRectClipp.bottom))
				text->offsety = ARX_CLEAN_WARN_CAST_FLOAT(text->rRect.bottom - text->rRectClipp.bottom);
		}
		else
			text->scrolltime -= delta;

		++it;
	}
}

//-----------------------------------------------------------------------------
void CARXTextManager::Render()
{
	HDC hDC = NULL;

	if (danaeApp.m_pddsRenderTarget && !vText_.empty())
		danaeApp.m_pddsRenderTarget->GetDC(&hDC);

	for (std::vector<ARXText*>::const_iterator it = vText_.begin(); it != vText_.end(); )
	{
		ARXText* text = *it;
		assert(text);

		HRGN hRgn = NULL;
		hRgn = CreateRectRgn(text->rRectClipp.left, text->rRectClipp.top,
							 text->rRectClipp.right, text->rRectClipp.bottom);

		long text_height = ARX_UNICODE_DrawTextInRect(ARX_CLEAN_WARN_CAST_FLOAT(text->rRect.left),
			text->rRect.top - text->offsety,
			ARX_CLEAN_WARN_CAST_FLOAT(text->rRect.right),
			0,
			text->str.c_str(),
			text->fgcolor,
			text->bgcolor,
			text->hFont,
			hRgn,
			hDC);
		text->rRect.bottom = text->rRect.top + text_height;

		if (hRgn)
			DeleteObject(hRgn);

		if (text->eType == ARX_TEXT_ONCE)
		{
			delete text;
			it = vText_.erase(it);
			continue;
		}

		++it;
	}

	if (hDC)
		danaeApp.m_pddsRenderTarget->ReleaseDC(hDC);
}

void CARXTextManager::Clear()
{
	for (std::vector<ARXText*>::iterator it = vText_.begin(), it_end = vText_.end(); it != it_end; ++it)
		delete *it;

	vText_.clear();
}

bool CARXTextManager::HasText() const
{
	return !vText_.empty();
}

//-----------------------------------------------------------------------------

namespace
{

HFONT CreateFontFromProfile(_TCHAR* faceName, const _TCHAR* section, const _TCHAR* defaultFontSize, bool antialias)
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
									antialias ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY,
									VARIABLE_PITCH, faceName);

	if (!ret)
		FontError();

	return ret;
}

} // \namespace

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

	sprintf(tx, "%smisc\\%s", Project.workingdir, "ARX.ttf"); // Full path

	if (!FileExist(tx))
	{
		sprintf(tx, "%smisc\\%s", Project.workingdir, "ARX_default.ttf"); // Full path
	}

	MultiByteToWideChar(CP_ACP, 0, tx , -1, wtx, 256);		// XS : We need to pass a unicode string to AddFontResourceW

	wchar_t wtx[256];
	typedef int (APIENTRY * AddFontRessourceW)(const wchar_t *);

	AddFontRessourceW Unicows_AddFontResourceW = (AddFontRessourceW)GetProcAddress(hUnicodeLibrary, "AddFontResourceW");

	lpszFontIngame = GetFontName(tx);

	if (Unicows_AddFontResourceW(wtx) == 0)
	{
		FontError();
	}

	sprintf(tx, "%smisc\\%s", Project.workingdir, "ARX.ttf");

	if (!FileExist(tx))
	{
		sprintf(tx, "%smisc\\%s", Project.workingdir, "ARX_default.ttf"); // Full path
	}

	MultiByteToWideChar(CP_ACP, 0, tx , -1, wtx, 256);		// XS : We need to pass an unicode string to AddFontResourceW

	lpszFontMenu = GetFontName(tx);

	if (Unicows_AddFontResourceW(wtx) == 0)
	{
		FontError();
	}


	pTextManage = new CARXTextManager();

	hFontMainMenu   = CreateFontFromProfile(lpszFont, _T("system_font_mainmenu_size"),     _T("58"), true);
	hFontMenu       = CreateFontFromProfile(lpszFont, _T("system_font_menu_size"),         _T("32"), true);
	hFontControls   = CreateFontFromProfile(lpszFont, _T("system_font_menucontrols_size"), _T("22"), true);
	hFontCredits    = CreateFontFromProfile(lpszFont, _T("system_font_menucredits_size"),  _T("36"), true);
	hFontRedist     = CreateFontFromProfile(lpszFont, _T("system_font_redist_size"),       _T("18"), true);
	if (Yratio > 1.f) Yratio *= .8f; // NEW QUEST
	hFontInGame     = CreateFontFromProfile(lpszFont, _T("system_font_book_size"),         _T("18"), false);
	hFontInGameNote = CreateFontFromProfile(lpszFont, _T("system_font_note_size"),         _T("18"), false);
	hFontInBook     = CreateFontFromProfile(lpszFont, _T("system_font_book_size"),         _T("18"), false);

	delete[] lpszFont;
}

void ARX_Text_Close()
{
	if (!hUnicodeLibrary)
	{
		hUnicodeLibrary = LoadLibraryA("unicows.dll");
	}

	if (lpszFontIngame)
	{
		delete [] lpszFontIngame;
		lpszFontIngame = NULL;
	}

	if (lpszFontMenu)
	{
		delete [] lpszFontMenu;
		lpszFontMenu = NULL;
	}

	typedef ATOM(APIENTRY * RemoveRessourceW)(const wchar_t *);

	RemoveRessourceW Unicows_RemoveRessourceW = (RemoveRessourceW)GetProcAddress(hUnicodeLibrary, "RemoveFontResourceW");

	_TCHAR wtx[256];
	char tx[256];
	sprintf(tx, "%smisc\\%s", Project.workingdir, "ARX.ttf"); // Full path

	if (!FileExist(tx))
	{
		sprintf(tx, "%smisc\\%s", Project.workingdir, "ARX_default.ttf"); // Full path
	}

	MultiByteToWideChar(CP_ACP, 0, tx , -1, wtx, 256);		// XS : We need to pass a unicode string to RemoveRessourceW

	lpszFontIngame = GetFontName(tx);


	if (Unicows_RemoveRessourceW(wtx) == 0)
	{
		//	FontError(); // XS : Annoying popup, uncomment if you really want to track something down.
	}

	sprintf(tx, "%smisc\\%s", Project.workingdir, "ARX.ttf"); // Full path

	if (!FileExist(tx))
	{
		sprintf(tx, "%smisc\\%s", Project.workingdir, "ARX_default.ttf"); // Full path
	}

	MultiByteToWideChar(CP_ACP, 0, tx , -1, wtx, 256);		// XS : We need to pass a unicode string to RemoveRessourceW
	lpszFontMenu = GetFontName(tx);

	if (Unicows_RemoveRessourceW(wtx) == 0)
	{
		//	FontError();// XS : Annoying popup, uncomment if you really want to track something down.
	}

	ARX_Localisation_Close();

	delete pTextManage;
	pTextManage = NULL;

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
