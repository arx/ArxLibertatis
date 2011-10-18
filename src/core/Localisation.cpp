/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/Localisation.h"

#include <stddef.h>
#include <sstream>
#include <cstdlib>
#include <iterator>

#include "core/Unicode.hpp"
#include "core/Config.h"

#include "io/FilePath.h"
#include "io/PakReader.h"
#include "io/IniReader.h"
#include "io/log/Logger.h"

#include "platform/Platform.h"

using std::string;

namespace {
IniReader localisation;
}

bool InitLocalisation() {
	
	LogDebug("Starting localization");
	
	localisation.clear();

	// Generate the filename for the localization strings
	std::string tx = "localisation/utext_" + config.language + ".ini";
	
	size_t loc_file_size = 0; // Used to report how large the loaded file is

	// Attempt loading the selected locale file
	u16 * Localisation = (u16*)resources->readAlloc(tx, loc_file_size);

	// if no file was loaded
	if ( !Localisation )
	{
		// Default to english locale
		config.language = "english";
		tx = "localisation/utext_" + config.language + ".ini";

		// Load the default english locale file
		Localisation = (u16*)resources->readAlloc(tx, loc_file_size);
	}
	
	u16 * toFree = Localisation;

	if ( !Localisation ) {
		LogError << "Could not load localisation file " << tx;
		return false;
	}
	
	
	LogDebug("Loaded localisation file " << tx);
	
	// Scale the loaded size to new stride of uint16_t vs char
	loc_file_size /= sizeof(*Localisation);
	
	if(loc_file_size >= 1 && *Localisation == 0xfeff) {
		// Ignore the byte order mark.
		loc_file_size--, Localisation++;
	}

	LogDebug("Loaded localisation file: " << tx << " of size " << loc_file_size);
	size_t nchars = GetUTF16Length(Localisation, &Localisation[loc_file_size]);
	ARX_UNUSED(nchars);
	LogDebug("UTF-16 size is " << nchars);
	std::string out;
	out.reserve(loc_file_size);
	UTF16ToUTF8( Localisation, &Localisation[loc_file_size], std::back_inserter(out) );
	LogDebug("Converted to UTF8 string of length " << out.size());

	if ( Localisation && loc_file_size)
	{
		LogDebug("Preparing to parse localisation file");
		std::istringstream iss( out );
		if(!localisation.read(iss)) {
			LogWarning << "errors while parsing localisation file " << tx;
		}
		
	}
	
	free(toFree);
	
	return true;
}

long getLocalisedKeyCount(const string & sectionname) {
	return localisation.getKeyCount(sectionname);
}

static const string BADNAMECHAR = "ABCDEFGHIJKLMNOPQRSTUVWXYZ[]"; // TODO(case-sensitive) remove

/**
 * Returns the localized string for the given key name
 * @param name The string to be looked up
 * @return The localized string based on the currently loaded locale file
 */
string getLocalised(const string & name, const string & default_value) {
	
	arx_assert(name.find_first_of(BADNAMECHAR) == string::npos); ARX_UNUSED(BADNAMECHAR); // TODO(case-sensitive) remove
	
	return localisation.getKey(name, string(), default_value);
}
