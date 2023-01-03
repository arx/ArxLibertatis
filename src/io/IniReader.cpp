/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "io/IniReader.h"

#include <algorithm>
#include <utility>

#include "io/log/Logger.h"
#include "platform/Platform.h"
#include "util/String.h"

const IniSection * IniReader::getSection(std::string_view sectionName) const {
	
	iterator iter = sections.find(sectionName);
	
	if(iter != sections.end()) {
		return &(iter->second);
	} else {
		return nullptr;
	}
	
}

size_t IniReader::getKeyCount(std::string_view sectionName) const {
	
	const IniSection * section = getSection(sectionName);
	if(section) {
		return section->size();
	}
	
	return 0;
}

std::string_view IniReader::getKey(std::string_view sectionName, std::string_view keyName,
                                   std::string_view defaultValue) const {
	
	const IniKey * key = getKey(sectionName, keyName);
	if(!key) {
		return defaultValue;
	}
	
	return key->getValue();
}

const std::string & IniReader::getKey(std::string_view sectionName, std::string_view keyName,
                                      const std::string & defaultValue) const {
	
	const IniKey * key = getKey(sectionName, keyName);
	if(!key) {
		return defaultValue;
	}
	
	return key->getValue();
}

int IniReader::getKey(std::string_view sectionName, std::string_view keyName,
                      int defaultValue) const {
	
	const IniKey * key = getKey(sectionName, keyName);
	if(!key) {
		return defaultValue;
	}
	
	return key->getValue(defaultValue);
}

float IniReader::getKey(std::string_view sectionName, std::string_view keyName,
                        float defaultValue) const {
	
	const IniKey * key = getKey(sectionName, keyName);
	if(!key) {
		return defaultValue;
	}
	
	return key->getValue(defaultValue);
}


bool IniReader::getKey(std::string_view sectionName, std::string_view keyName,
                       bool defaultValue) const {
	
	const IniKey * key = getKey(sectionName, keyName);
	if(!key) {
		return defaultValue;
	}
	
	return key->getValue(defaultValue);
}

const IniKey * IniReader::getKey(std::string_view sectionName, std::string_view keyName) const {
	
	// Look for a section
	const IniSection * section = getSection(sectionName);
	
	// If the section was not found, return nullptr
	if(!section) {
		return nullptr;
	}
	
	// If the section has no keys, return nullptr
	if(section->empty()) {
		return nullptr;
	}
	
	// If the key is not specified, return the first ones value( to avoid breakage with legacy assets )
	if(keyName.empty()) {
		return &*section->begin();
	}
	
	return section->getKey(keyName);
}

static constexpr const std::string_view ALPHANUM = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";

bool IniReader::read(std::string_view data, bool overrideValues) {
	
	// The current section
	IniSection * section = nullptr;
	IniKey * key = nullptr;
	
	bool ok = true;
	
	// While lines remain to be extracted
	size_t lineNumber = 0;
	for(std::string_view line : util::split(data, '\n')) {
		lineNumber++;
		
		std::string_view str = util::trimLeft(line);
		if(str.empty()) {
			// Empty line (only whitespace)
			key = nullptr;
			continue;
		}
		
		if(str[0] == '#' || (str.length() > 1 && str[0] == '/' && str[1] == '/')) {
			// Whole line was commented, no need to do anything with it. Continue getting the next line
			key = nullptr;
			continue;
		}
		
		// Section header
		if(str[0] == '[') {
			
			key = nullptr;
			
			size_t end = str.find(']', 1);
			if(end == std::string_view::npos) {
				LogDebug("invalid header @ line " << lineNumber << ": " << line);
				end = str.find_first_not_of(ALPHANUM, 1);
				if(end == std::string_view::npos) {
					end = str.length();
				}
			}
			
			std::string sectionName = util::toLowercase(str.substr(1, end - 1));
			LogDebug("found section: \"" << sectionName << "\"");
			section = &sections[std::move(sectionName)];
			
			// Ignoring rest of the line, not verifying that it's only whitespace / comment
			
			continue;
		}
		
		if(!section) {
			LogWarning << "Ignoring non-empty line " << lineNumber << " outside a section: " << line;
			ok = false;
			continue;
		}
		
		std::string_view name;
		bool quoted = false;
		
		size_t nameEnd = str.find_first_not_of(ALPHANUM);
		if(nameEnd != std::string_view::npos) {
			std::string_view right = util::trimLeft(str.substr(nameEnd));
			if(!right.empty() && right[0] == '=') {
				name = str.substr(0, nameEnd);
				str = right.substr(1);
			} else if(right.length() > 1 && right[0] == '"' && right[ 1] == '=') {
				LogDebug("found '\"=' instead of '=\"' @ line " << lineNumber << ": " << line);
				quoted = true;
				name = str.substr(0, nameEnd);
				str = right.substr(1);
			}
		}
		
		if(name.empty()) {
			if(key) {
				// Replace newlines with spaces
				key->append(" ");
				size_t newValueEnd = str.find_last_of('"');
				if(newValueEnd != std::string_view::npos) {
					// End of multi-line value
					key->append(str.substr(0, newValueEnd));
					key = nullptr;
				} else {
					key->append(util::trimRight(str));
				}
			} else {
				ok = false;
				LogWarning << "Invalid key @ line " << lineNumber << ": " << line;
			}
			continue;
		}
		
		str = util::trimLeft(str);
		
		bool valueIncomplete = false;
		std::string_view value;
		
		if(quoted || (!str.empty() && str[0] == '"')) {
			str.remove_prefix(1);
			size_t valueEnd = str.find_last_of('"');
			
			if(valueEnd == std::string_view::npos) {
				
				// The localisation files are broken (missing ending quote)
				// But the spanish localisation files have erroneous newlines in some values
				LogDebug("invalid quoted value @ line " << lineNumber << ": " << line);
				
				value = util::trimRight(str);
				
				// Add following lines until we find either a terminating quote,
				// an empty or commented line, a new section or a new key
				valueIncomplete = true;
				
			} else {
				value = str.substr(0, valueEnd);
			}
			
		} else {
			value = util::trimRight(str);
		}
		
		IniKey * addedKey = &section->addKey(name, value, overrideValues);
		
		key = valueIncomplete ? addedKey : nullptr;
		
		// Ignoring rest of the line, not verifying that it's only whitespace / comment
		
	}
	
	return ok;
}

void IniReader::clear() {
	sections.clear();
}
