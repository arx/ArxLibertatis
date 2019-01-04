/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <map>
#include <stddef.h>
#include <sstream>
#include <cstdlib>
#include <iterator>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>

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

void loadLocalisation(PakDirectory * dir, const std::string & name) {
	
	PakFile * file = dir->getFile(name);
	arx_assert(file);
	
	std::string buffer = file->read();
	if(buffer.empty()) {
		LogWarning << "Error reading localisation file localisation/" << name;
		return;
	}
	
	if(buffer.size() >= 2 && buffer[0] == '\xFF' && buffer[1] == '\xFE') {
		LogWarning << "UTF16le character encoding is unsupported for new localizations "
		              "please use UTF8 for file localisation/" << name;
		return;
	}
	
	LogInfo << "Loading: " << name;
	
	std::istringstream iss(buffer);
	if(!g_localisation.read(iss, true)) {
		LogWarning << "Error parsing localisation file localisation/" << name;
	}
}

void loadLocalisations() {
	
	const std::string suffix = ".ini";
	const std::string fallbackPrefix = "xtext_english_";
	const std::string localizedPrefix = "xtext_" + config.language + "_";
	
	typedef std::map<std::string, bool> LocalizationFiles;
	
	LocalizationFiles localizationFiles;
	
	PakDirectory * dir = g_resources->getDirectory("localisation");
	PakDirectory::files_iterator fileIter = dir->files_begin();
	
	for(; fileIter != dir->files_end(); ++fileIter) {
		const std::string & name = fileIter->first;
		
		if(boost::ends_with(name, suffix)) {
			if(boost::starts_with(name, fallbackPrefix)) {
				localizationFiles[name.substr(fallbackPrefix.length())];
			}
			if(boost::starts_with(name, localizedPrefix)) {
				localizationFiles[name.substr(localizedPrefix.length())] = true;
			}
		}
	}
	
	BOOST_FOREACH(const LocalizationFiles::value_type & i, localizationFiles) {
		
		loadLocalisation(dir, fallbackPrefix + i.first);
		
		if(i.second) {
			loadLocalisation(dir, localizedPrefix + i.first);
		}
	}
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
	
	std::string buffer = file->read();
	if(buffer.empty()) {
		return false;
	}
	
	LogDebug("Loaded localisation file of size " << file->size());
	buffer = util::convert<util::UTF16LE, util::UTF8>(buffer);
	LogDebug("Converted to UTF8 string of length " << buffer.size());
	
	if(!buffer.empty()) {
		LogDebug("Preparing to parse localisation file");
		std::istringstream iss(buffer);
		if(!g_localisation.read(iss)) {
			LogWarning << "Error parsing localisation file localisation/utext_"
			           << config.language << ".ini";
		}
	}
	
	loadLocalisations();
	
	return true;
}

long getLocalisedKeyCount(const std::string & sectionname) {
	return g_localisation.getKeyCount(sectionname);
}

std::string getLocalised(const std::string & name) {
	
	arx_assert(name.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ[]") == std::string::npos);
	
	return g_localisation.getKey(name, std::string(), name);
}

std::string getLocalised(const std::string & name, const std::string & default_value) {
	
	arx_assert(name.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ[]") == std::string::npos);
	
	return g_localisation.getKey(name, std::string(), default_value);
}
