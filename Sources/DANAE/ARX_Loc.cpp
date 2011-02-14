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
#include "EERIEApp.h"
#include "HERMESMain.h"
#include "ARX_LocHash.h"
#include "ARX_Loc.h"
#include "ARX_Config.h"
#include <tchar.h>
#include <list>
#include "ARX_Menu2.h"
#include "EERIETexture.h"

extern long GERMAN_VERSION;
extern long FRENCH_VERSION;
extern long CHINESE_VERSION;
extern long FINAL_COMMERCIAL_GAME;
extern long FINAL_COMMERCIAL_DEMO;
using namespace std;
extern CMenuConfig * pMenuConfig;

CLocalisationHash * pHashLocalisation = NULL;

namespace
{

/// Currently loaded localisation language name
std::string loadedLocalisationLanguage;

// Nuky - recoded, can't be too strict because it seems there are many mistakes
//        in the data files
/// @return whether str fully matches: "\[.*\][^\0\]:alnum:]*"
bool isSection(const _TCHAR* str)
{
	if (!str)
		return false;

	if (*str != _T('['))
		return false;
	++str;

	str = _tcschr(str, _T(']'));
	if (str == NULL)
		return false;
	++str;

	while (*str != _T('\0') && *str != _T(']') && !_istalnum(*str))
		++str;

	return *str == _T('\0');
}

// Nuky - recoded, can't be too strict because it seems there are many mistakes
//        in the data files
/// @return whether str begins by: "[:alnum: ]*="
bool isKey(const _TCHAR* str)
{
	if (!str)
		return false;

	while (_istalnum(*str) || *str == _T(' '))
		++str;

	if (*str != _T('='))
		return false;

	return true;
}

// Nuky - recoded
/// @return whether str contains at least one alphanum char
bool isNotEmpty(const _TCHAR* str)
{
	if (!str)
		return false;

	for (; *str != _T('\0'); ++str)
		if (_istalnum(*str))
			return true;

	return false;
}

// Nuky - recoded
/// @return malloc'ed string containing the content between the first [ and
/// the first following ] from str (including the [] themselves)
_TCHAR* CleanSection(const _TCHAR* str)
{
	_TCHAR* result = (_TCHAR *) malloc((_tcslen(str) + 2) * sizeof(_TCHAR));
	*result = _T('\0');

	const _TCHAR* start = _tcschr(str, _T('['));
	if (!start)
		return result;

	const _TCHAR* end = _tcschr(start, _T(']'));
	if (!end)
		return result;
	++end;

	_tcsncpy(result, start, end - start);

	result[end - start] = _T('\0');

	return result;
}

//-----------------------------------------------------------------------------
_TCHAR* CleanKey(const _TCHAR* str)
{
	_TCHAR * lpszUText = (_TCHAR *) malloc((_tcslen(str) + 2) * sizeof(_TCHAR));
	ZeroMemory(lpszUText, (_tcslen(str) + 2)*sizeof(_TCHAR));

	unsigned long ulPos = 0;
	unsigned long ulTextSize = _tcslen(str);
	bool bAlpha = false;
	bool bEqual = false;

	for (unsigned long ul = 0; ul < ulTextSize; ul++)
	{
		if (str[ul] == _T('='))
			bEqual = true;
		else if (bEqual && (_istalnum(str[ul]) || (str[ul] != _T(' ') && str[ul] != _T('"'))))
			bAlpha = true;
		else if (str[ul] == _T('\"') && !bAlpha)
			continue;

		if (bEqual && bAlpha)
		{
			lpszUText[ulPos] = str[ul];
			ulPos ++;
		}
	}

	while (ulPos--)
	{
		if (_istalnum(lpszUText[ulPos]))
			break;
		else if (lpszUText[ulPos] == _T('"'))
			lpszUText[ulPos] = 0;
	}

	return lpszUText;
}

//-----------------------------------------------------------------------------
// Nuky - Recoded - use reserved vector instead of list for performance
/// Loads sections and corresponding keys into the global variable pHashLocalisation
/// @note textFile gets modified ! (comments are replaced by spaces)
void ParseIniFile(_TCHAR* textFile, unsigned long textFileSize)
{
	// skip unicode header
	++textFile;
	--textFileSize;

	// clean up comments from textFile
	for (unsigned long i = 0; i < textFileSize - 1; ++i)
		if ((textFile[i] == _T('/')) && (textFile[i+1] == _T('/')))
			for (; i < textFileSize - 1 && textFile[i] != _T('\r') && textFile[i+1] != _T('\n'); ++i)
				textFile[i] = _T(' ');

	// get all lines
	std::vector<_TCHAR*> lines;
	lines.reserve(textFileSize / 16); // 16 chars long lines average, guesstimate
	for (_TCHAR* line = _tcstok(textFile, _T("\r\n")); line; line = _tcstok(NULL, _T("\r\n")))
		if (isNotEmpty(line))
			lines.push_back(line);

	// look up for sections and associated keys
	for (std::vector<_TCHAR*>::const_iterator it = lines.begin(), it_end = lines.end(); it != it_end; )
	{
		if (isSection(*it))
		{
			CLocalisation* loc = new CLocalisation();
			_TCHAR* section = CleanSection(*it);
			loc->SetSection(section);
			free(section);
			++it;

			while (it != it_end && !isSection(*it) && isKey(*it))
			{
				_TCHAR* key = CleanKey(*it);
				loc->AddKey(key);
				free(key);
				++it;
			}

			pHashLocalisation->AddElement(loc);
			continue;
		}

		++it;
	}
}

} // \namespace

//-----------------------------------------------------------------------------
void ARX_Localisation_Init(const char * _lpszExtension) 
{
	// Nuky - no need to reload the already loaded language
	if (loadedLocalisationLanguage == _lpszExtension)
		return;

	// cleanup before loading new one
	if (pHashLocalisation)
		ARX_Localisation_Close();

	char tx[256];
	ZeroMemory(tx, 256);
	sprintf(tx, "%slocalisation\\utext_%s.ini", Project.workingdir, Project.localisationpath);

	long LocalisationSize = 0;
	_TCHAR * Localisation = NULL;

	if (GERMAN_VERSION)
	{
		bool bOldForceInPack = bForceInPack;
		bForceInPack = true;
		Localisation = (_TCHAR *)PAK_FileLoadMallocZero(tx, &LocalisationSize);
		bForceInPack = bOldForceInPack;
	}
	else
	{
		Localisation = (_TCHAR *)PAK_FileLoadMallocZero(tx, &LocalisationSize);
	}

	if (Localisation == NULL)
	{
		if (GERMAN_VERSION || FRENCH_VERSION)
		{
			delete pMenuConfig;
			pMenuConfig = NULL;
			exit(0);
		}

		ZeroMemory(tx, 256);
		strcpy(Project.localisationpath, "english");
		sprintf(tx, "%slocalisation\\utext_%s.ini", Project.workingdir, Project.localisationpath);
		Localisation = (_TCHAR *)PAK_FileLoadMallocZero(tx, &LocalisationSize);
	}

	if (Localisation && LocalisationSize)
	{
		pHashLocalisation = new CLocalisationHash(1 << 13);
		LocalisationSize = _tcslen(Localisation);
		ParseIniFile(Localisation, LocalisationSize);
		free((void *)Localisation);
		Localisation = NULL;
		LocalisationSize = 0;
	}

	//CD Check
	if (FINAL_COMMERCIAL_DEMO && (!bForceInPack))
	{
		_TCHAR szMenuText[256];
		PAK_UNICODE_GetPrivateProfileString(_T("system_menus_main_cdnotfound"), _T("string"), _T(""), szMenuText, 256, NULL);

		if (!szMenuText[0]) //warez
		{
			bForceInPack = true;
			ARX_Localisation_Init();
			bForceInPack = false;
		}
	}

	if (FINAL_COMMERCIAL_GAME)
	{
		_TCHAR szMenuText[256] = {0};
		PAK_UNICODE_GetPrivateProfileString(_T("unicode"), _T("string"), _T(""), szMenuText, 256, NULL);

		if (szMenuText[0]) //warez
		{
			if (!_tcsicmp(_T("chinese"), szMenuText))
			{
				CHINESE_VERSION = 1;
			}
		}
	}

	loadedLocalisationLanguage = _lpszExtension;
}

//-----------------------------------------------------------------------------
void ARX_Localisation_Close()
{
	loadedLocalisationLanguage.clear();

	delete pHashLocalisation;
	pHashLocalisation = NULL;
}

//-----------------------------------------------------------------------------
long HERMES_UNICODE_GetProfileString(const _TCHAR * sectionname,
									 const _TCHAR * t_keyname,
									 const _TCHAR * defaultstring,
									 _TCHAR * destination,
									 unsigned long maxsize,
									 const _TCHAR * datastream,
									 long lastspeech)
{
	ZeroMemory(destination, maxsize * sizeof(_TCHAR));

	if (const CLocalisation* loc = pHashLocalisation ? pHashLocalisation->GetElement(sectionname) : NULL)
		if (!loc->keys.empty())
			defaultstring = loc->keys[0];

	_tcsncpy(destination, defaultstring, std::min(static_cast<size_t>(maxsize), _tcslen(defaultstring)));

	return 0;
}

//-----------------------------------------------------------------------------
long HERMES_UNICODE_GetProfileSectionKeyCount(const _TCHAR * sectionname)
{
	if (const CLocalisation* loc = pHashLocalisation ? pHashLocalisation->GetElement(sectionname) : NULL)
		return loc->keys.size();
	return 0;
}

//-----------------------------------------------------------------------------
void PAK_UNICODE_GetPrivateProfileString(const _TCHAR * _lpszSection,
										 const _TCHAR * _lpszKey,
										 const _TCHAR * _lpszDefault,
										 _TCHAR * _lpszBuffer,
										 unsigned long _lBufferSize,
										 const char * _lpszFileName)
{
	ZeroMemory(_lpszBuffer, _lBufferSize * sizeof(_TCHAR));

	if (_lpszSection[0] == _T('\0'))
	{
		_tcsncpy(_lpszBuffer, _lpszDefault, std::min(static_cast<size_t>(_lBufferSize), _tcslen(_lpszDefault)));
		_stprintf(_lpszBuffer, _T("%s: NOT FOUND"), _lpszSection);
		return;
	}

	_TCHAR szSection[256] = _T("");
	_stprintf(szSection, _T("[%s]"), _lpszSection);

	HERMES_UNICODE_GetProfileString(
	    szSection,
	    _lpszKey,
	    _lpszDefault,
	    _lpszBuffer,
	    _lBufferSize,
	    NULL,
	    -1); //lastspeechflag

}

#include <vector>
using namespace std;

extern PakManager * pPakManager;
vector<char *> mlist;

//-----------------------------------------------------------------------------
void ParseCurFile(EVE_TFILE * _e)
{
	if (!_e) return;

	if (strcasecmp((char *)_e->name, "utext") > 0)
	{
		char * t = strdup((char *)_e->name);

		for (UINT i = 0 ; i < strlen(t) ; i++)
		{

			t[i] = ARX_CLEAN_WARN_CAST_CHAR(tolower(t[i]));

		}

		mlist.insert(mlist.end(), t);
		//free (t);
	}

	ParseCurFile(_e->fnext);
}

//-----------------------------------------------------------------------------
void ParseCurRep(EVE_REPERTOIRE * _er)
{
	if (!_er) return;

	ParseCurFile(_er->fichiers);

	ParseCurRep(_er->fils);
	ParseCurRep(_er->brothernext);
}

extern HBITMAP ARX_CONFIG_hBitmap;
