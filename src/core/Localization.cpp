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

#include "core/Application.h"
#include "core/ConfigHashMap.h"
#include "core/Unicode.hpp"

#include "io/PakManager.h"
#include "io/Logger.h"

namespace
{
	ConfigHashMap* pHashLocalisation = 0;
	std::string empty_string = "";
}

extern long GERMAN_VERSION;
extern long FRENCH_VERSION;
extern long CHINESE_VERSION;
extern long FINAL_COMMERCIAL_GAME;
extern long FINAL_COMMERCIAL_DEMO;

/****************************************************************************
 * Initializes the localisation hashmap based on the current chosen locale
 ***************************************************************************/
void Localisation_Init() 
{
	LogDebug << "Starting localization";
	// If a locale is already initialized, close it
	if (pHashLocalisation)
		Localisation_Close();

	// Generate the filename for the localization strings
	std::string tx = "localisation\\utext_" + Project.localisationpath + ".ini";
	
	size_t loc_file_size = 0; // Used to report how large the loaded file is

	// Attempt loading the selected locale file
	u16* Localisation = (u16*)PAK_FileLoadMallocZero( tx, loc_file_size );

	// if no file was loaded
	if ( !Localisation )
	{
		// Default to english locale
		Project.localisationpath = "english";
		tx = "localisation\\utext_" + Project.localisationpath + ".ini";

		// Load the default english locale file
		Localisation = (u16*)PAK_FileLoadMallocZero( tx, loc_file_size );
	}

	LogDebug << "Loaded localisation file " << tx;
	if ( !Localisation )
		LogFatal << "Could not load localisation file " << tx;
	
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
		std::istringstream iss( out );
		pHashLocalisation = new ConfigHashMap(1 << 13, iss);

		//LogDebug << "Converting loaded localistation file:";
		//ParseFile( out );
	}

	//CD Check
	if (FINAL_COMMERCIAL_DEMO)
	{
		std::string szMenuText;
		szMenuText = getLocalized( "system_menus_main_cdnotfound" );

		if (szMenuText.empty()) //warez
		{
			Localisation_Init();
		}
	}

	if (FINAL_COMMERCIAL_GAME)
	{
		std::string szMenuText;
		szMenuText = getLocalized( "unicode" );

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

/**
 * Returns the localized string for the given key name
 * @param name The string to be looked up
 * @return The localized string based on the currently loaded locale file
 */
std::string getLocalized( const std::string& name, const std::string& default_value )
{
	if ( !pHashLocalisation )
		return default_value;

	return pHashLocalisation->getConfigValue( name, default_value, empty_string );
}
