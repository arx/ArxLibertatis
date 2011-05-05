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
// Code: Didier Pédreno
// todo remover les strIcmp

#include "io/IniReader.h"

#include <sstream>

#include "io/Logger.h"

using std::string;
using std::istream;
using std::getline;

const IniSection* IniReader::getSection( const std::string& section ) const {
	
	std::map<std::string, IniSection>::const_iterator iter = sections.find(section);
	
	if(iter != sections.end()) {
		return &(iter->second);
	} else {
		return NULL;
	}
}

size_t IniReader::getKeyCount(const string & sectionName) const {
	
	const IniSection * section = getSection(sectionName);
	if(section) {
		return section->keys.size();
	} else {
		return 0;
	}
}

const string & IniReader::getKey(const string & section, const string & key, const string & default_value) const {
	
	const string * value = getKey(section, key);
	if(value) {
		return *value;
	} else {
		// If the value was not found, return default
		return default_value;
	}
	
}

const string * IniReader::getKey(const string & section, const string & key) const {
	
	// Look for a section
	const IniSection * config = getSection(section);
	
	// If the section was not found, return NULL
	if(!config) {
		return NULL;
	}
	
	// If the section has no keys, return NULL
	if(config->keys.empty()) {
		return NULL;
	}
	
	// If the key is not specified, return the first ones value( to avoid breakage with legacy assets )
	if(key.empty()) {
		return &config->keys.front().value;
	}
	
	// Otherwise try to match the key to one in the section
	for(size_t i = 0; i < config->keys.size(); i++) {
		if(config->keys[i].name == key) { // If the key name matches that specified
			return &config->keys[i].value; // Return the value of the requested key
		}
	}
	
	// If the key was not found, return NULL
	return NULL;
}

const string WHITESPACE = " \t\r\n";
const string ALPHANUM = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";

bool IniReader::read(istream & is) {
	
	// The current section.
	IniSection * section = NULL;
	
	bool ok = true;
	
	// While lines remain to be extracted
	for(size_t line = 1; is.good(); line++) {
		
		// Get a line to process
		string str;
		getline(is, str);
		
		size_t start = str.find_first_not_of(WHITESPACE);
		if(start == string::npos) {
			// Empty line (only whitespace).
			continue;
		}
		
		if(str[start] == '#' || (start + 1 < str.length() && str[start] == '/' && str[start+1] == '/')) {
			// Whole line was commented, no need to do anything with it. Continue getting the next line.
			continue;
		}
		
		// Section header.
		if(str[start] == '[') {
			
			size_t end = str.find(']', start + 1);
			if(end == string::npos) {
				LogWarning << "invalid header @ line " << line << ": " << str;
				section = NULL;
				ok = false;
				continue;
			}
			
			LogDebug << "found section: \"" << str.substr(start + 1, end - start - 1) << "\"";
			section = &sections[str.substr(start + 1, end - start - 1)];
			
			// Ignoring rest of the line, not verifying that it's only whitespace / comment
			
			continue;
		}
		
		if(!section) {
			LogWarning << "ignoring non-empty line " << line << " outside a section: " << str;
			ok = false;
			continue;
		}
		
		size_t nameEnd = str.find_first_not_of(ALPHANUM, start);
		if(nameEnd == string::npos) {
			ok = false;
			LogWarning << "missing '=' separator @ line " << line << ": " << str;
			continue;
		} else if(nameEnd == start) {
			ok = false;
			LogWarning << "empty key name @ line " << line << ": " << str;
			continue;
		}
		
		size_t separator = str.find_first_not_of(WHITESPACE, nameEnd);
		if(separator == string::npos || str[separator] != '=') {
			ok = false;
			LogWarning << "missing '=' separator @ line " << line << ": " << str;
			continue;
		}
		
		size_t valueStart = str.find_first_not_of(WHITESPACE, separator + 1);
		if(valueStart == string::npos) {
			// Empty value.
			section->addKey(str.substr(start, nameEnd - start), string());
		}
		
		size_t valueEnd;
		if(str[valueStart] == '"') {
			valueStart++;
			valueEnd = str.find_last_of('"');
			arx_assert(valueEnd != string::npos);
			if(valueEnd < valueStart) {
				// The localisation files are broken (missing ending quote), so ignore this error.
				LogDebug << "invalid quoted value @ line " << line << ": " << str;
				valueEnd = str.length();
			}
		} else {
			valueEnd = str.find_last_not_of(WHITESPACE) + 1;
			arx_assert(valueEnd != string::npos);
		}
		arx_assert(valueEnd >= valueStart);
		
		section->addKey(str.substr(start, nameEnd - start), str.substr(valueStart, valueEnd - valueStart));
		
		// Ignoring rest of the line, not verifying that it's only whitespace / comment
		
	}
	
	return ok;
}

void IniReader::clear() {
	sections.clear();
}
