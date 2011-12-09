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

#include <boost/algorithm/string/predicate.hpp>

#include "core/Unicode.hpp"
#include "core/Config.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/IniReader.h"
#include "io/log/Logger.h"

#include "platform/Platform.h"

using std::string;

namespace {
IniReader localisation;
}

static PakFile * autodetectLanguage() {
	
	PakDirectory * dir = resources->getDirectory("localisation");
	if(!dir) {
		LogError << "Missing 'localisation' directory. Is 'loc.pak' present?";
		return NULL;
	}
	
	std::ostringstream languages;
	PakFile * localisation = NULL;
	
	PakDirectory::files_iterator file = dir->files_begin();
	for(; file != dir->files_end(); ++file) {
		
		const std::string & name = file->first;
		
		const std::string prefix = "utext_";
		const std::string suffix = ".ini";
		if(!boost::starts_with(name, prefix) || !boost::ends_with(name, suffix)) {
			// Not a localisation file.
			continue;
		}
		
		if(name.length() <= prefix.length() + suffix.length()) {
			// Missing language name.
			continue;
		}
		
		// Extract the language name.
		size_t length = name.length() - prefix.length() - suffix.length();
		std::string language = name.substr(prefix.length(), length);
		
		if(!localisation) {
			
			localisation = file->second;
			config.language = language;
			
		} else {
			
			if(!languages.tellp()) {
				languages << config.language;
			}
			languages << ", " << language;
			
			// Prefer english if there are multiple localisations.
			if(language == "english") {
				localisation = file->second;
				config.language = language;
			}
		}
	}
	
	if(!localisation) {
		LogError << "Could not find any localisation file. (localisation/utext_*.ini)";
		return NULL;
	}
	
	if(languages.tellp()) {
		LogWarning << "Multiple localisations avalable: " << languages.rdbuf();
	}
	
	LogInfo << "Autodetected language: " << config.language;
	
	return localisation;
}

bool initLocalisation() {
	
	LogDebug("Starting localization");
	
	localisation.clear();
	
	PakFile * file;
	
	if(config.language.empty()) {
		file = autodetectLanguage();
	} else {
		
		// Attempt to load localisation for the configured language.
		std::string filename = "localisation/utext_" + config.language + ".ini";
		file = resources->getFile(filename);
		
		if(!file) {
			LogWarning << "Localisation file " << filename << " not found, autodetecting language.";
			/*
			 * TODO we might want to keep the old config.language setting as there could be
			 * localized audio for that language even if there is no localized text.
			 */
			file = autodetectLanguage();
		}
	}
	
	if(!file) {
		return false;
	}
	
	arx_assert(!config.language.empty());
	
	u16 * localisation = reinterpret_cast<u16 *>(file->readAlloc());
	u16 * toFree = localisation;
	
	// Scale the loaded size to new stride of uint16_t vs char
	size_t loc_file_size = file->size() / sizeof(*localisation);
	
	// Ignore any byte order mark.
	if(loc_file_size >= 1 && *localisation == 0xfeff) {
		loc_file_size--, localisation++;
	}

	LogDebug("Loaded localisation file of size " << loc_file_size);
	size_t nchars = GetUTF16Length(localisation, &localisation[loc_file_size]);
	ARX_UNUSED(nchars);
	LogDebug("UTF-16 size is " << nchars);
	std::string out;
	out.reserve(loc_file_size);
	UTF16ToUTF8(localisation, &localisation[loc_file_size], std::back_inserter(out));
	LogDebug("Converted to UTF8 string of length " << out.size());

	if(localisation && loc_file_size) {
		LogDebug("Preparing to parse localisation file");
		std::istringstream iss( out );
		if(!::localisation.read(iss)) {
			LogWarning << "Error parsing localisation file localisation/utext_"
			           << config.language << ".ini";
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
