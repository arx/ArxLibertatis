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
#include <tchar.h>
#include <stdint.h>
#include <SFML/System.hpp>
#include <list>

#include "core/LocalizationHash.h"
#include "core/Localization.h"
#include "gui/MenuWidgets.h"
#include "core/Application.h"
#include "graphics/data/Texture.h"
#include "io/IO.h"
#include "io/PakManager.h"
#include "io/Logger.h"

using std::sprintf;

extern long GERMAN_VERSION;
extern long FRENCH_VERSION;
extern long CHINESE_VERSION;
extern long FINAL_COMMERCIAL_GAME;

extern PROJECT Project;
extern long FINAL_COMMERCIAL_DEMO;
using namespace std;
extern CMenuConfig * pMenuConfig;
#define MAX_LINE_SIZE 8096

CLocalisationHash * pHashLocalisation;

//-----------------------------------------------------------------------------
bool isSection( const _TCHAR * _lpszUText)
{
	ULONG i			 = 0;
	unsigned long ulTextSize = _tcslen(_lpszUText);
	bool bFirst = false;
	bool bLast  = false;

	while (i < ulTextSize)
	{
		if (_lpszUText[i] == _T('['))
		{
			if (bFirst)
				return false;
			else
				bFirst = true;
		}
		else if (_lpszUText[i] == _T(']'))
		{
			if (bFirst)
				if (bLast)
					return false;
				else
					bLast = true;
			else
				return false;
		}
		else if (isalnum(_lpszUText[i]))
		{
			if (!bFirst)
				return false;
			else if (bFirst && bLast)
				return false;
		}

		i++;
	}

	if (bFirst && bLast) return true;

	return false;
}

bool isSection( const std::string& str )
{
	size_t first_bracket, last_bracket;

	first_bracket = str.find_first_of('[');
	last_bracket = str.find_last_of(']');

	if ( first_bracket == std::string::npos ) return false;
	if ( last_bracket == std::string::npos ) return false;

	if ( first_bracket > last_bracket ) return false;

	return true;
}
 

//-----------------------------------------------------------------------------
bool isKey(const _TCHAR * _lpszUText)
{
	unsigned long i = 0;
	unsigned long ulTextSize = _tcslen(_lpszUText);
 
 
	bool bSpace = false;
	bool bAlpha = false;

	while (i < ulTextSize)
	{
		if (_lpszUText[i] == _T('='))
		{
			if (bSpace)
				return false;
			else
				bSpace = true;
		}
		else if (isalnum(_lpszUText[i]))
		{
			if (!bAlpha)
				bAlpha = true;
		}

		i++;
	}

	if (bSpace && bAlpha) return true;

	return false;
}

//-----------------------------------------------------------------------------
bool isNotEmpty(_TCHAR * _lpszUText)
{
	ULONG i			 = 0;
	unsigned long ulTextSize = _tcslen(_lpszUText);

	while (i < ulTextSize)
	{
		if (isalnum(_lpszUText[i]))
		{
			return true;
		}

		i++;
	}

	return false;
}

//-----------------------------------------------------------------------------
_TCHAR * CleanSection(const _TCHAR * _lpszUText)
{
	_TCHAR * lpszUText = (_TCHAR *) malloc((_tcslen(_lpszUText) + 2) * sizeof(_TCHAR));
	ZeroMemory(lpszUText, (_tcslen(_lpszUText) + 2)*sizeof(_TCHAR));

	unsigned long ulPos = 0;
	bool bFirst = false;
	bool bLast = false;

	for (unsigned long ul = 0; ul < _tcslen(_lpszUText); ul++)
	{
		if (_lpszUText[ul] == _T('['))
		{
			bFirst = true;
		}
		else if (_lpszUText[ul] == _T(']'))
		{
			bLast = true;
		}

		if (bFirst)
		{
			lpszUText[ulPos] = _lpszUText[ul];
			ulPos ++;

			if (bLast)
			{
				break;
			}
		}
	}

	return lpszUText;
}

//-----------------------------------------------------------------------------
_TCHAR * CleanKey(const _TCHAR * _lpszUText)
{
	_TCHAR * lpszUText = (_TCHAR *) malloc((_tcslen(_lpszUText) + 2) * sizeof(_TCHAR));
	ZeroMemory(lpszUText, (_tcslen(_lpszUText) + 2)*sizeof(_TCHAR));

	unsigned long ulPos = 0;
	unsigned long ulTextSize = _tcslen(_lpszUText);
	bool bAlpha = false;
	bool bEqual = false;

	for (unsigned long ul = 0; ul < ulTextSize; ul++)
	{
		if (_lpszUText[ul] == _T('='))
		{
			bEqual = true;
		}
		else if (bEqual && (isalnum(_lpszUText[ul]) ||
							((_lpszUText[ul] != _T(' ')) &&
							 (_lpszUText[ul] != _T('"')))
						   ))
		{
			bAlpha = true;
		}
		else if ((_lpszUText[ul] == _T('\"')) && (!bAlpha))
		{
			continue;
		}

		if (bEqual && bAlpha)
		{
			lpszUText[ulPos] = _lpszUText[ul];
			ulPos ++;
		}
	}

	while (ulPos--)
	{
		if (isalnum(lpszUText[ulPos]))
		{
			break;
		}
		else if (lpszUText[ulPos] == _T('"'))
		{
			lpszUText[ulPos] = 0;
		}
	}

	return lpszUText;
}

//-----------------------------------------------------------------------------
void ParseFile( const std::string& _lpszUTextFile, const unsigned long _ulFileSize)
{
	std::string temp = _lpszUTextFile;
	temp.erase( 0, 1 ); // TODO This apparently removes a unicode header?
	unsigned long ulFileSize = _ulFileSize - 1;


	std::list<std::string> input_strings;
	std::stringstream ss( _lpszUTextFile );
	
	while ( ss.good() )
	{
		// Get a line to process
		std::string str;
		std::getline( ss, str );

		// Remove any commented bits until the line end
		size_t comment_start = std::string::npos;
		comment_start = str.find("/");

		if ( comment_start != std::string::npos )

		// Whole line was commented, no need to do anything with it. Continue getting the next line
		if ( comment_start == 0 ) continue;

		// Remove any commented bits from the line
		if ( comment_start != std::string::npos )
			str = str.substr(0, comment_start );

		if ( str.empty() ) continue;
		// Push back the line
		input_strings.push_back( str );
	}

	

/*
	// Remove all comments from the input string
	while ( true )
	{
		// Find comment start
		size_t comment_start = temp.find( "//" );

		// No comments found, break
		if ( comment_start == std::string::npos ) break;

		// Find comment end after comment start
		size_t line_end = temp.find_first_of( "\r\n", comment_start );

		// Remove comments until end of line or end if file
		if ( line_end != std::string::npos )
			temp.erase( comment_start, line_end - comment_start );
		else
			temp.erase( comment_start );
	}
*/
/*
	for (unsigned long i = 0; i < (ulFileSize - 1); i++)
	{
		if ((pFile[i] == _T('/')) && (pFile[i+1] == _T('/')))
		{
			unsigned long j = i;

			while ((j < (ulFileSize - 1)) && (pFile[j] != _T('\r')) && (pFile[j+1] != _T('\n')))
			{
				pFile[j] = _T(' ');
				j++;
			}

			i = j;
		}
	}
*/
	/*list< std::string > strings;

	// Find lines and put them into the vector
	while ( !temp.empty() )
	{
		size_t line_end = temp.find_first_of( "\r\n" );
		strings.push_back( temp.substr( 0, line_end ) );
		temp.erase( 0, line_end );
	}*/
/*		
	
	list<_TCHAR *> lUText;
	pULine = _tcstok(pFile, _T("\r\n"));

	long t = 0;

	while (pULine != NULL)
	{
		if (isNotEmpty(pULine))
		{
			t++;
			lUText.insert(lUText.end(), (pULine)); //_tcsdup
		}

		pULine = _tcstok(NULL, _T("\r\n"));
	}
*/

	std::list<std::string>::iterator iter = input_strings.begin();

	while( iter != input_strings.end() )
	{
		if ( isSection( *iter ) )
		{
			CLocalisation* loc = new CLocalisation();
			std::string section_str = CleanSection( iter->c_str() );
			loc->SetSection( iter->c_str() );

			iter++;

			while ( ( iter != input_strings.end() ) && ( !isSection( iter->c_str() ) ) )
			{
				if ( isKey( iter->c_str() ) )
				{
					loc->AddKey( CleanKey( iter->c_str() ) );
					iter++;
				}
				else
					break; // Done reading the keys for this section, break the loop and continue with the finished loc object
			}

			pHashLocalisation->AddElement(loc);
			continue; // Continue loop, the next section or end of input was encountered
		}

		iter++;
	}
/*
	list< std::string >::iterator iter = strings.begin();

	while ( iter != strings.end() )
	{
		if ( isSection((*iter).c_str()) )
		{
			CLocalisation* loc = new CLocalisation();
			std::string section_str = CleanSection( (*iter).c_str() );
			loc->SetSection( (*iter).c_str() );

			iter++;

			while ((iter != strings.end()) && (!isSection((*iter).c_str())))
			{
				if(isKey((*iter).c_str()))
					loc->AddKey( CleanKey((*iter).c_str() ) );
				else
					break;
			}

			pHashLocalisation->AddElement(loc);
			continue;
		}

		iter++;
	}*/
		
	//list<_TCHAR *>::iterator it;
/*
	//-------------------------------------------------------------------------
	// look up for sections and associated keys
	it = lUText.begin();
	t = 0;

	while (it != lUText.end())
	{
		if (isSection(*it))
		{
			CLocalisation * pLoc = new CLocalisation();
			_TCHAR * lpszUT = CleanSection(*it);
			pLoc->SetSection(lpszUT);

			free(lpszUT);

			it++;
			t++;

			while ((it != lUText.end()) && (!isSection(*it)))
			{
				if (isKey(*it))
				{
					_TCHAR * lpszUTk = CleanKey(*it);
					pLoc->AddKey(lpszUTk);
					free(lpszUTk);
					it++;
					t++;
				}
				else
				{
					break;
				}
			}

			pHashLocalisation->AddElement(pLoc);
			continue;
		}

		it++;
		t++;
	}*/
}

//-----------------------------------------------------------------------------
char LocalisationLanguage = -1;

//-----------------------------------------------------------------------------
// TODO parameter not really used.
void ARX_Localisation_Init(const char * _lpszExtension) 
{
	if (_lpszExtension == NULL)
		return;

	// nettoyage
	if (pHashLocalisation)
	{
		ARX_Localisation_Close();
	}

	std::string tx = "localisation\\utext_" + Project.localisationpath + ".ini";
	
	std::string temp = tx;
	size_t LocalisationSize = 0;

	uint16_t* Localisation = (uint16_t*)PAK_FileLoadMallocZero(tx, LocalisationSize);
	
	if ( Localisation)
	{
		if (GERMAN_VERSION || FRENCH_VERSION)
		{
			delete pMenuConfig;
			pMenuConfig = NULL;
			exit(0);
		}

		Project.localisationpath = "english";
		tx = "localisation\\utext_" + Project.localisationpath + ".ini";
		Localisation = (uint16_t*)PAK_FileLoadMallocZero(tx, LocalisationSize);
	}

	// Scale size to new stride
	LocalisationSize *= ( 1.0 * sizeof(char)/sizeof(uint16_t) );

	LogDebug << "Loaded localisation file: " << tx << " of size " << LocalisationSize;
	LogDebug << "UTF-16 size is " << sf::Unicode::GetUTF16Length( Localisation, &Localisation[LocalisationSize] );
	std::string out;
	out.resize( sf::Unicode::GetUTF16Length( Localisation, &Localisation[LocalisationSize] ) );
	LogDebug << "Resized to " << out.length();
	sf::Unicode::UTF16ToUTF8( Localisation, &Localisation[LocalisationSize], out.begin() );
	LogDebug << "Converted to UTF8";

	if ( Localisation && LocalisationSize)
	{
		LogDebug << "Preparing to parse localisation file";
		pHashLocalisation = new CLocalisationHash(1 << 13);

		LogDebug << "Converting loaded localistation file:";
		ParseFile( out, out.length() );
	}

	//CD Check
	if (FINAL_COMMERCIAL_DEMO)
	{
		std::string szMenuText;
		PAK_UNICODE_GetPrivateProfileString( "system_menus_main_cdnotfound", "", szMenuText, 256);

		if (!szMenuText[0]) //warez
		{
			ARX_Localisation_Init();
		}
	}

	if (FINAL_COMMERCIAL_GAME)
	{
		std::string szMenuText;
		PAK_UNICODE_GetPrivateProfileString( "unicode", "", szMenuText, 256);

		if (szMenuText[0]) //warez
		{
			if (!szMenuText.compare( "chinese" ) )
			{
				CHINESE_VERSION = 1;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void ARX_Localisation_Close()
{
	LocalisationLanguage = -1;

	delete pHashLocalisation;
	pHashLocalisation = NULL;
}

//-----------------------------------------------------------------------------
long HERMES_UNICODE_GetProfileString(   const std::string&  sectionname,
										const std::string&  defaultstring,
										std::string&        destination,
										unsigned long       maxsize )
{

	destination.clear();

	if (pHashLocalisation)
	{
		std::string* t = pHashLocalisation->GetPtrWithString(sectionname);

		if (t)
			destination = *t;
		else
			destination = defaultstring;
	}
	else
		destination = defaultstring;

	//TODO(lubosz): Hardcode string
	destination = "foo";
	return 0;
}

//-----------------------------------------------------------------------------
long HERMES_UNICODE_GetProfileSectionKeyCount(const std::string& sectionname)
{
	if (pHashLocalisation)
		return pHashLocalisation->GetKeyCount(sectionname);

	return 0;
}

static long ltNum = 0;

//-----------------------------------------------------------------------------
int PAK_UNICODE_GetPrivateProfileString(    const std::string&  _lpszSection,
											const std::string&  _lpszDefault,
											std::string&        _lpszBuffer,
											unsigned long       _lBufferSize )
{
	ltNum ++;
	_lpszBuffer.clear();

	if (_lpszSection.empty())	{
		LogError <<  _lpszDefault << " not found";
		_lpszBuffer = _lpszDefault + ":NOT FOUND";
		return 0;
	}

	std::string szSection = "[" + _lpszSection + "]";

	HERMES_UNICODE_GetProfileString( szSection,
									_lpszDefault,
									_lpszBuffer,
									_lBufferSize );

	return 1;
}

