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

#include "io/IniSection.h"

#include <sstream>
#include <ios>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "io/log/Logger.h"

int IniKey::getValue(int defaultValue) const {
	
	if(value == "false")
		return 0;
	else if(value == "true")
		return 1;
	
	std::istringstream iss(value);
	
	int val = defaultValue;
	if((iss >> val).fail()) {
		return defaultValue;
	}
	
	return val;
}

float IniKey::getValue(float defaultValue) const {
	
	try {
		return boost::lexical_cast<float>(value);
	} catch(boost::bad_lexical_cast &) {
		return defaultValue;
	}
}

bool IniKey::getValue(bool defaultValue) const {
	
	if("false" == value || "0" == value)
		return false;
	else if("true" == value || "1" == value)
		return true;
	else
		return defaultValue;
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
	
	IniKey k = IniKey(key, value);
	boost::to_lower(k.name);
	
	LogDebug("added key " << key << "=\"" << value << "\"");
	keys.push_back(k);
}

void IniSection::setKey(const std::string & key, const std::string & value) {
	
	IniKey k = IniKey(key, value);
	boost::to_lower(k.name);
	
	BOOST_FOREACH(IniKey & old, keys) {
		if(old.name == k.name) {
			LogDebug("replaced key " << k.name << "=\"" << k.value << "\"" << " old: " << "\"" << old.value << "\"");
			old = k;
			return;
		}
	}
	
	LogDebug("added key " << key << "=\"" << value << "\"");
	keys.push_back(k);
}

