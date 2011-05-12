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

#include "core/Localisation.h"

#include <list>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "core/Application.h"
#include "core/Unicode.hpp"
#include "core/Config.h"

#include "io/PakManager.h"
#include "io/Logger.h"
#include "io/IniReader.h"

#include "platform/Platform.h"

using std::string;
using std::transform;

namespace {
IniReader localisation;
}

extern long FINAL_COMMERCIAL_GAME;
extern long FINAL_COMMERCIAL_DEMO;

void LocalisationInit() {
	
	LogDebug << "Starting localization";
	
	localisation.clear();

	// Generate the filename for the localization strings
	std::string tx = "localisation\\utext_" + config.language + ".ini";
	
	size_t loc_file_size = 0; // Used to report how large the loaded file is

	// Attempt loading the selected locale file
	u16 * Localisation = (u16*)PAK_FileLoadMalloc( tx, loc_file_size );

	// if no file was loaded
	if ( !Localisation )
	{
		// Default to english locale
		config.language = "english";
		tx = "localisation\\utext_" + config.language + ".ini";

		// Load the default english locale file
		Localisation = (u16*)PAK_FileLoadMalloc( tx, loc_file_size );
	}
	
	u16 * toFree = Localisation;

	if ( !Localisation )
		LogFatal << "Could not load localisation file " << tx;
	
	
	LogDebug << "Loaded localisation file " << tx;
	
	// Scale the loaded size to new stride of uint16_t vs char
	loc_file_size *= ( 1.0 * sizeof(char)/sizeof(*Localisation) );
	
	if(loc_file_size >= 1 && *Localisation == 0xfeff) {
		// Ignore the byte order mark.
		loc_file_size--, Localisation++;
	}

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
		if(!localisation.read(iss)) {
			LogWarning << "errors while parsing localisation file " << tx;
		}
		
	}
	
	free(toFree);
	
}

long getLocalisedKeyCount(const string & sectionname) {
	return localisation.getKeyCount(sectionname);
}

static const string UPCASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // TODO remove

/**
 * Returns the localized string for the given key name
 * @param name The string to be looked up
 * @return The localized string based on the currently loaded locale file
 */
string getLocalised(const string & name, const string & default_value) {
	
	if(name.find_first_of(UPCASE) != string::npos || (!name.empty() && name[0] == '[')) {
		LogWarning << "upcase char in \"" << name << "\""; // TODO remove
	}
	
	return localisation.getKey(name, string(), default_value);
}

// TODO remove
long MakeLocalised(const std::string & text, std::string & output) {
	
	if(text.empty()) {
		output = "ERROR";
		return 0;
	}
	
	std::string section = text;

	// if the section name has the qualifying brackets "[]", cut the back one then the front off
	if(section[0] == '[' && section[section.length() - 1] == ']') {
		section = section.substr(1, section.length() - 2);
	}

	transform(section.begin(), section.end(), section.begin(), ::tolower);
	
	// TODO move to caller
	output = getLocalised(section, "error");
	return 0;
}
