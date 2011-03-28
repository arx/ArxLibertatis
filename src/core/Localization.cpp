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

#include "core/Localization.h"

#include <list>
#include <sstream>

#include "core/Common.h"
#include "core/LocalizationHash.h"
#include "core/Unicode.hpp"

#include "gui/MenuWidgets.h"

#include "io/PakManager.h"
#include "io/Logger.h"

extern long GERMAN_VERSION;
extern long FRENCH_VERSION;
extern long CHINESE_VERSION;
extern long FINAL_COMMERCIAL_GAME;
extern long FINAL_COMMERCIAL_DEMO;
extern CMenuConfig * pMenuConfig;

CLocalisationHash * pHashLocalisation;

bool isKey( const std::string& str );
bool isSection( const std::string& str );
std::string CleanKey( std::string str );
std::string CleanSection( std::string str );

/*****************************************************************
 * Checks a str for square brackets and makes sure they appear
 * left first, right second. This confirms str as a section line.
 *****************************************************************/
bool isSection( const std::string& str )
{
	// Location of square brackets in str
	size_t first_bracket, last_bracket;

	// Get location of '[' and ']' in str
	first_bracket = str.find_first_of('[');
	last_bracket = str.find_last_of(']');

	// If either of the brackets are not present, not a section
	if ( first_bracket == std::string::npos ) return false;
	if ( last_bracket == std::string::npos ) return false;

	// '[' must be before ']'
	if ( first_bracket > last_bracket ) return false;

	// It is a section
	return true;
}


/*****************************************************************
 * Checks whether or not str is a valid key line.
 * Looks for an alphanumeric key followed by
 * at least one '=' character.
 ****************************************************************/
bool isKey( const std::string& str )
{
	// Iterate through str until alphanumeric characters are found
	for ( size_t i = 0 ; i < str.length() ; i++ )
	{
		// If we hit an '=' before other alphanumeric characters, return false
		if ( str[i] == '=' ) return false;

		// Just one alphanumeric is enough for a key, break out of the loop
		if ( isalnum(str[i]) )
			break;
	}

	// Now find the location of the first equals sign
	size_t equals_loc = str.find('=');

	// No equals sign present means not a key, return false
	if ( equals_loc == std::string::npos ) return false;

	// Value can be empty, no need to check for one. Return true
	return true;
}
	
/**************************************************************
 * Removes all characters preceding the first '[' and
 * proceeding the first ']' in a copy of _str and returns
 * the result.
 *************************************************************/
std::string CleanSection( std::string str )
{
	// Find the cutoff points for the line trimming
	size_t first_bracket = str.find_first_of('[');
	size_t last_bracket = str.find_last_of(']');

	// Erase all characters up to but not including the '['
	str.erase( 0, first_bracket );

	// Erase all characters following but not including the ']'
	str.erase( last_bracket + 1 );

	// Return the processed string
	return str;
}

/********************************************************************
 * Cleans up a key string by removing the type identifier before
 * '=' appears and extracting the string between the first two ""
 * marks.
 *******************************************************************/
std::string CleanKey( std::string str )
{
	// The equals sign seperates the type identifier from the value
	size_t equals_loc = str.find_first_of('=');

	// Find the beginning of the value after the equals sign
	size_t first_quot_mark = str.find( '"', equals_loc );

	// Cut the string until the beginning of the value
	str = str.substr( first_quot_mark + 1 );

	// Find the next " mark that shows the end of the value
	size_t last_quot_mark = str.find_last_of('"');

	// Cut the string at the end of the value
	str = str.substr( 0, last_quot_mark );

	// Return the processed string
	return str;
}

/*******************************************************************************
 * Takes a string as a memory buffer for the contents of a file and extracts
 * extract lines from it that correspond to sections and keys. It then maps
 * these sections with corresponding keys into the localization hashmap for
 * later lookup when the strings are used to display text.
 ******************************************************************************/
void ParseFile( const std::string& file_text )
{
	std::stringstream ss( file_text ); // Input stream to extract lines from
	std::list<std::string> input_strings; // Resulting list of lines extracted
	
	while ( ss.good() ) // While lines remain to be extracted
	{
		// Get a line to process
		std::string str;
		std::getline( ss, str );

		// Remove any commented bits until the line end
		size_t comment_start = std::string::npos;
		comment_start = str.find("//");

		// Whole line was commented, no need to do anything with it. Continue getting the next line
		if ( comment_start == 0 ) continue;

		// Remove any commented bits from the line
		if ( comment_start != std::string::npos )
			str = str.substr(0, comment_start );

		if ( str.empty() ) continue;
		// Push back the line
		input_strings.push_back( str );
	}

	// Iterate over all the lines entered into the list earlier
	std::list<std::string>::iterator iter = input_strings.begin();
	while( iter != input_strings.end() )
	{
		// If a section string is found, make en entry for it
		if ( isSection( *iter ) )
		{
			// Create a localisation entry for this section
			CLocalisation* loc = new CLocalisation();

			// Set the section name as the cleaned section string
			loc->SetSection( CleanSection( *iter ) );

			// Advance to the line after the section string to start looking for keys
			iter++;

			// Iterate over more strings until a section is encountered or no more strings remain
			while ( ( iter != input_strings.end() ) && ( !isSection( *iter ) ) )
			{
				// If a key is found, add it to the localisation entry
				if ( isKey( *iter ) )
					loc->AddKey( CleanKey( *iter ) );

				iter++; // Continue looking for more keys
			}

			pHashLocalisation->AddElement(loc);
			continue; // Continue loop, the next section or end of input was encountered
		}

		// Only get here if a non-content string managed to escape the earlier culling, advance to the next line
		iter++;
	}
}


/****************************************************************************
 * Initializes the localisation hashmap based on the current chosen locale
 ***************************************************************************/
void Localisation_Init() 
{

	// If a locale is already initialized, close it
	if (pHashLocalisation)
		Localisation_Close();

	// Generate the filename for the localization strings
	std::string tx = "localisation\\utext_" + Project.localisationpath + ".ini";
	
	size_t loc_file_size = 0; // Used to report how large the loaded file is

	// Attempt loading the selected locale file
	u16* Localisation = (u16*)PAK_FileLoadMallocZero( tx, loc_file_size );

	// if no file was loaded
	if ( Localisation )
	{
		// No file loaded and locale is set to german or french
		if (GERMAN_VERSION || FRENCH_VERSION)
		{
			// Delete the menuconfig and abort the program
			delete pMenuConfig;
			pMenuConfig = NULL;
			exit(0);
		}

		// Otherwise default to english locale
		Project.localisationpath = "english";
		tx = "localisation\\utext_" + Project.localisationpath + ".ini";

		// Load the default english locale file
		Localisation = (u16*)PAK_FileLoadMallocZero( tx, loc_file_size );
	}
	
	// Scale the loaded size to new stride of uint16_t vs char
	loc_file_size *= ( 1.0 * sizeof(char)/sizeof(*Localisation) );

	LogDebug << "Loaded localisation file: " << tx << " of size " << loc_file_size;
	size_t nchars = GetUTF16Length( Localisation, &Localisation[loc_file_size] );
	LogDebug << "UTF-16 size is " << nchars;
	std::string out;
	out.reserve(loc_file_size);
	UTF16ToUTF8( Localisation, &Localisation[loc_file_size], std::back_inserter(out) );
	LogDebug << "Converted to UTF8 string of length " << out.size();

	if ( Localisation && loc_file_size)
	{
		LogDebug << "Preparing to parse localisation file";
		pHashLocalisation = new CLocalisationHash(1 << 13);

		LogDebug << "Converting loaded localistation file:";
		ParseFile( out );
	}

	//CD Check
	if (FINAL_COMMERCIAL_DEMO)
	{
		std::string szMenuText;
		PAK_UNICODE_GetPrivateProfileString( "system_menus_main_cdnotfound", "", szMenuText );

		if (szMenuText.empty()) //warez
		{
			Localisation_Init();
		}
	}

	if (FINAL_COMMERCIAL_GAME)
	{
		std::string szMenuText;
		PAK_UNICODE_GetPrivateProfileString( "unicode", "", szMenuText );

		if (!szMenuText.empty()) //warez
		{
			if (!szMenuText.compare( "chinese" ) )
			{
				CHINESE_VERSION = 1;
			}
		}
	}
}

/***************************************************************
 * Deinitialize the current locale.
 * Delete the hashmap.
 * ************************************************************/
void Localisation_Close()
{

	if ( pHashLocalisation)
		delete pHashLocalisation;

	pHashLocalisation = NULL;
}

//-----------------------------------------------------------------------------
long HERMES_UNICODE_GetProfileString( const std::string&  sectionname,
                                      const std::string&  defaultstring,
                                      std::string&        destination )
{
	// If localisation has been initialized
	if (pHashLocalisation)
	{
		// Get the string for this section
		std::string* t = pHashLocalisation->GetPtrWithString(sectionname);

		// If a string is found matching the section
		if (t)
			destination = *t; // Return that string in destination
		else
			destination = defaultstring; // Otherwise, return the default value
	}
	else
		destination = defaultstring; // No localisation, return the default string

	return 0;
}

//-----------------------------------------------------------------------------
long HERMES_UNICODE_GetProfileSectionKeyCount(const std::string& sectionname)
{
	if (pHashLocalisation)
		return pHashLocalisation->GetKeyCount(sectionname);

	return 0;
}

//-----------------------------------------------------------------------------
bool PAK_UNICODE_GetPrivateProfileString( const std::string&  _in_section,
                                         const std::string&  _default_return,
                                         std::string&        _buf )
{
	if ( _in_section.empty())
	{
		LogError <<  _default_return << " not found";
		_buf = _default_return + ":NOT FOUND";
		return false;
	}

	std::string section = "[" + _in_section + "]";

	HERMES_UNICODE_GetProfileString( section,
	                                 _default_return,
	                                 _buf );

	return true;
}

