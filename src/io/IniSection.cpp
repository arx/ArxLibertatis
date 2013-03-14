/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include "io/IniSection.h"

#include <sstream>
#include <ios>

#include <boost/algorithm/string/case_conv.hpp>

#include "io/log/Logger.h"

int IniKey::getValue(int defaultValue) const {
	
	std::istringstream iss(value);
	
	int val = defaultValue;
	if((iss >> val).fail()) {
		return defaultValue;
	}
	
	return val;
}

float IniKey::getValue(float defaultValue) const {
	
	std::istringstream iss(value);
	
	float val;
	if((iss >> val).fail()) {
		return defaultValue;
	}
	
	return val;
}

bool IniKey::getValue(bool defaultValue) const {
	
	std::istringstream iss(value);
	
	// Support either boolean specified as strings (true, false) or 0, 1
	bool val;
	if((iss >> std::boolalpha >> val).fail()) {
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

void IniSection::addKey(const std::string & key, const std::string & value) {
	
	keys.resize(keys.size() + 1);
	
	keys.back().name = key;
	keys.back().value = value;
	
	boost::to_lower(keys.back().name);
	
	LogDebug("found key " << key << "=\"" << value << "\"");
};
