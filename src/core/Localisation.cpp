/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include <set>
#include <stddef.h>
#include <sstream>
#include <cstdlib>
#include <iterator>

#include <boost/algorithm/string/predicate.hpp>

#include "core/Config.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/IniReader.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/log/Logger.h"

#include "platform/Environment.h"
#include "platform/Platform.h"

#include "util/String.h"
#include "util/Unicode.h"

namespace {

IniReader g_localisation;

Language getLanguageInfo(std::string_view id) {
	
	IniReader reader(g_resources->read((std::string("localisation/languages/") += id) += ".ini"));
	
	Language result;
	result.name = reader.getKey("language", "name", id);
	result.locale = util::toLowercase(reader.getKey("language", "locale", id));
	
	return result;
}

std::string_view selectPreferredLanguage(const Languages & languages) {
	
	std::vector<std::string> locales = platform::getPreferredLocales();
	
	// Select language based on system language
	for(std::string & locale : locales) {
		for(const Languages::value_type & language : languages) {
			if(language.second.locale == locale) {
				return language.first;
			}
		}
		for(const Languages::value_type & language : languages) {
			if(boost::starts_with(language.second.locale, locale)) {
				return language.first;
			}
		}
	}
	
	return std::string_view();
}

std::string_view selectDefaultLanguage(const Languages & languages) {
	
	// Select language from AF config file
	// For AF 1.22 this will match language selected by the user in the Steam / GOG / Bethesda launcher
	const char * cfgFiles[] = { "cfg.ini", "cfg_default.ini" };
	for(const fs::path & datadir : fs::getDataDirs()) {
		for(const char * cfgFile : cfgFiles) {
			std::string data = fs::read(datadir / cfgFile);
			if(!data.empty()) {
				std::string language = util::toLowercase(IniReader(data).getKey("language", "string", std::string()));
				if(auto it = languages.find(language); it != languages.end()) {
					return it->first;
				}
			}
		}
	}
	
	return std::string_view();
}

void autodetectTextLanguage() {
	
	Languages languages = getAvailableTextLanguages();
	if(languages.empty()) {
		LogCritical << "Could not find any localisation file. (localisation/utext_*.ini)";
		config.interface.language.clear();
		return;
	}
	
	if(languages.size() == 1) {
		// Only one language available so just use that
		config.interface.language = languages.begin()->first;
		return;
	}
	
	std::ostringstream list;
	for(const Languages::value_type & language : languages) {
		if(list.tellp()) {
			list << ", ";
		}
		list << language.first;
	}
	LogInfo << "Multiple text languages available: " << list.str();
	
	config.interface.language = selectPreferredLanguage(languages);
	if(!config.interface.language.empty()) {
		return;
	}
	
	config.interface.language = selectDefaultLanguage(languages);
	if(!config.interface.language.empty()) {
		return;
	}
	
	// Otherwise, prefer english if english audio is available
	Languages::iterator english = languages.find("english");
	if(english != languages.end() && g_resources->getDirectory(res::path("speech") / english->first)) {
		config.interface.language = english->first;
		return;
	}
	
	// Otherwise, prefer the first language with audio available
	for(const Languages::value_type & language : languages) {
		if(g_resources->getDirectory(res::path("speech") / language.first)) {
			config.interface.language = language.first;
			return;
		}
	}
	
	// Otherwise, prefer english
	if(english != languages.end()) {
		config.interface.language = english->first;
		return;
	}
	
	// Finally just select the first available language
	config.interface.language = languages.begin()->first;
}

void autodetectAudioLanguage() {
	
	Languages languages = getAvailableAudioLanguages();
	if(languages.empty()) {
		LogCritical << "Could not find any localisation dir. (speech/*/)";
		config.audio.language.clear();
		return;
	}
	
	if(languages.size() == 1) {
		// Only one language available so just use that
		config.audio.language = languages.begin()->first;
		return;
	}
	
	std::ostringstream list;
	for(const Languages::value_type & language : languages) {
		if(list.tellp()) {
			list << ", ";
		}
		list << language.first;
	}
	LogInfo << "Multiple audio languages available: " << list.str();
	
	config.audio.language = selectPreferredLanguage(languages);
	if(!config.audio.language.empty()) {
		return;
	}
	
	config.audio.language = selectDefaultLanguage(languages);
	if(!config.audio.language.empty()) {
		return;
	}
	
	// Use text language for audio if that exists
	Languages::iterator text = languages.find(config.interface.language);
	if(text != languages.end()) {
		config.audio.language = text->first;
		return;
	}
	
	// Otherwise, prefer english
	Languages::iterator english = languages.find("english");
	if(english != languages.end()) {
		config.audio.language = english->first;
		return;
	}
	
	// Finally just select the first available language
	config.audio.language = languages.begin()->first;
}

void loadLocalisation(PakDirectory * dir, std::string_view name) {
	
	PakFile * file = dir->getFile(name);
	if(!file) {
		return;
	}
	
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
	
	if(!g_localisation.read(buffer, true)) {
		LogWarning << "Error parsing localisation file localisation/" << name;
	}
	
}

void loadLocalisations() {
	
	const std::string_view suffix = ".ini";
	const std::string_view fallbackPrefix = "xtext_default_";
	const std::string localizedPrefix = "xtext_" + config.interface.language + "_";
	
	PakDirectory * dir = g_resources->getDirectory("localisation");
	if(!dir) {
		return;
	}
	
	typedef std::set<std::string_view> LocalizationFiles;
	
	LocalizationFiles localizationFiles;
	
	for(const std::string_view name : dir->files()) {
		if(boost::ends_with(name, suffix)
		   && (boost::starts_with(name, fallbackPrefix) || boost::starts_with(name, localizedPrefix))) {
			localizationFiles.insert(name.substr(fallbackPrefix.length()));
		}
	}
	
	for(std::string_view name : localizationFiles) {
		loadLocalisation(dir, std::string(fallbackPrefix) += name);
		loadLocalisation(dir, std::string(localizedPrefix) += name);
	}
	
}

} // anonymous namespace

Languages getAvailableTextLanguages() {
	
	Languages result;
	
	PakDirectory * localisation = g_resources->getDirectory("localisation");
	if(!localisation) {
		LogCritical << "Missing 'localisation' directory. Is 'loc.pak' present?";
		return result;
	}
	for(const std::string_view name : localisation->files()) {
		
		const std::string_view prefix = "utext_";
		const std::string_view suffix = ".ini";
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
		std::string_view id = name.substr(prefix.length(), length);
		
		if(id.find_first_not_of("abcdefghijklmnopqrstuvwxyz_") != std::string_view::npos) {
			LogWarning << "Ignoring localisation/" << name;
			continue;
		}
		
		result.insert(Languages::value_type(id, getLanguageInfo(id)));
	}
	
	return result;
}

Languages getAvailableAudioLanguages() {
	
	Languages result;
	
	PakDirectory * localisation = g_resources->getDirectory("speech");
	if(!localisation) {
		LogCritical << "Missing 'speech' directory. Is 'speech.pak' present?";
		return result;
	}
	for(const std::string_view name : localisation->dirs()) {
		
		if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz_") != std::string::npos) {
			LogWarning << "Ignoring speech/" << name << "/";
			continue;
		}
		
		result.insert(Languages::value_type(name, getLanguageInfo(name)));
	}
	
	return result;
}

bool initLocalisation() {
	
	LogDebug("Starting localization");
	
	g_localisation.clear();
	
	PakFile * file = nullptr;
	if(!config.interface.language.empty()) {
		LogInfo << "Using text language from config: " << config.interface.language;
		std::string filename = "localisation/utext_" + config.interface.language + ".ini";
		file = g_resources->getFile(filename);
		if(!file) {
			LogWarning << "Localisation file " << filename << " not found";
		}
	}
	if(!file) {
		autodetectTextLanguage();
		if(!config.interface.language.empty()) {
			LogInfo << "Autodetected text language: " << config.interface.language;
			file = g_resources->getFile("localisation/utext_" + config.interface.language + ".ini");
		}
	}
	
	PakDirectory * dir = nullptr;
	if(!config.audio.language.empty()) {
		LogInfo << "Using audio language from config: " << config.audio.language;
		std::string dirname = "speech/" + config.audio.language;
		dir = g_resources->getDirectory(dirname);
		if(!dir) {
			LogWarning << "Localisation dir " << dirname << "/ not found";
		}
	}
	if(!dir) {
		autodetectAudioLanguage();
		if(!config.audio.language.empty()) {
			LogInfo << "Autodetected audio language: " << config.audio.language;
		}
	}
	
	if(!file) {
		return false;
	}
	
	arx_assert(!config.interface.language.empty());
	
	std::string buffer = file->read();
	if(buffer.empty()) {
		return false;
	}
	
	LogDebug("Loaded localisation file of size " << buffer.size());
	buffer = util::convert<util::UTF16LE, util::UTF8>(buffer);
	LogDebug("Converted to UTF8 string of length " << buffer.size());
	
	if(!buffer.empty()) {
		LogDebug("Preparing to parse localisation file");
		if(!g_localisation.read(buffer)) {
			LogWarning << "Error parsing localisation file localisation/utext_"
			           << config.interface.language << ".ini";
		}
	}
	
	loadLocalisations();
	
	return true;
}

long getLocalisedKeyCount(std::string_view sectionname) {
	return g_localisation.getKeyCount(sectionname);
}

std::string_view getLocalised(std::string_view name) {
	return g_localisation.getKey(name, std::string_view(), name);
}

std::string_view getLocalised(std::string_view name, const std::string_view default_value) {
	return g_localisation.getKey(name, std::string_view(), default_value);
}
