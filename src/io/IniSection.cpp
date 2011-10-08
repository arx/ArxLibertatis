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

#include "io/IniSection.h"

#include <sstream>
#include <algorithm>

#include "io/log/Logger.h"

using std::string;
using std::transform;
using std::istringstream;
using std::boolalpha;

int IniKey::getValue(int defaultValue) const {
	
	istringstream iss(value);
	
	int val = defaultValue;
	if((iss >> val).fail()) {
		return defaultValue;
	}
	
	return val;
}

float IniKey::getValue(float defaultValue) const {
	
	istringstream iss(value);
	
	float val;
	if((iss >> val).fail()) {
		return defaultValue;
	}
	
	return val;
}

bool IniKey::getValue(bool defaultValue) const {
	
	istringstream iss(value);
	
	// Support either boolean specified as strings (true, false) or 0, 1
	bool val;
	if((iss >> boolalpha >> val).fail()) {
		iss.clear();
		int intVal;
		if((iss >> intVal).fail()) {
			return defaultValue;
		}
		val = (intVal != 0);
	}
	
	return val;
}

const IniKey * IniSection::getKey(const std::string & name) const {
	
	// Try to match the key to one in the section
	for(iterator i = begin(); i != end(); ++i) {
		if(i->getName() == name) { // If the key name matches that specified
			return &*i;
		}
	}
	
	// If the key was not found, return NULL
	return NULL;
}

void IniSection::addKey(const string & key, const string & value) {
	
	keys.resize(keys.size() + 1);
	
	keys.back().name = key;
	keys.back().value = value;
	
	transform(keys.back().name.begin(), keys.back().name.end(), keys.back().name.begin(), ::tolower);
	
	LogDebug("found key " << key << "=\"" << value << "\"");
};
