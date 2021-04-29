/*
 * Copyright 2011-2014 Arx Libertatis Team (see the AUTHORS file)
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

#include "core/Config.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/IniReader.h"
#include "io/log/Logger.h"

#include "platform/Platform.h"

#include "util/Unicode.h"

namespace {

IniReader g_localisation;

PakFile * autodetectLanguage() {
	
	PakDirectory * dir = g_resources->getDirectory("localisation");
	if(!dir) {
		LogCritical << "Missing 'localisation' directory. Is 'loc.pak' present?";
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
		LogCritical << "Could not find any localisation file. (localisation/utext_*.ini)";
		return NULL;
	}
	
	if(languages.tellp()) {
		LogWarning << "Multiple localisations avalable: " << languages.rdbuf();
	}
	
	LogInfo << "Autodetected language: " << config.language;
	
	return localisation;
}

} // anonymous namespace

bool initLocalisation() {
	
	LogDebug("Starting localization");
	
	g_localisation.clear();
	
	PakFile * file;
	
	if(config.language.empty()) {
		file = autodetectLanguage();
	} else {
		
		LogInfo << "Using language from config file: " << config.language;
		
		// Attempt to load localisation for the configured language.
		std::string filename = "localisation/utext_" + config.language + ".ini";
		file = g_resources->getFile(filename);
		
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
	
	char * data = file->readAlloc();
	if(!data) {
		return false;
	}
	
	LogDebug("Loaded localisation file of size " << file->size());
	std::string out = util::convert<util::UTF16LE, util::UTF8>(data, data + file->size());
	LogDebug("Converted to UTF8 string of length " << out.size());
	
	if(!out.empty()) {
		LogDebug("Preparing to parse localisation file");
		std::istringstream iss(out);
		if(!::g_localisation.read(iss)) {
			LogWarning << "Error parsing localisation file localisation/utext_"
			           << config.language << ".ini";
		}
	}
	
	free(data);
	
	return true;
}

long getLocalisedKeyCount(const std::string & sectionname) {
	return g_localisation.getKeyCount(sectionname);
}

std::string getLocalised(const std::string & name, const std::string & default_value) {
	
	arx_assert(name.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ[]") == std::string::npos);
	
	return g_localisation.getKey(name, std::string(), default_value);
}
