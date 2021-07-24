/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/lexical_cast.hpp>

#include "io/log/Logger.h"

#include "util/String.h"

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

const IniKey * IniSection::getKey(std::string_view name) const {
	
	// Try to match the key to one in the section
	for(iterator i = begin(); i != end(); ++i) {
		if(i->getName() == name) { // If the key name matches that specified
			return &*i;
		}
	}
	
	// If the key was not found, return nullptr
	return nullptr;
}

void IniSection::addKey(std::string key, std::string value) {
	
	IniKey k(util::toLowercase(std::move(key)), std::move(value));
	
	LogDebug("added key " << key << "=\"" << value << "\"");
	keys.emplace_back(std::move(k));
}

void IniSection::setKey(std::string key, std::string value) {
	
	IniKey k(util::toLowercase(std::move(key)), std::move(value));
	
	for(IniKey & old : keys) {
		if(old.name == k.name) {
			LogDebug("replaced key " << k.name << "=\"" << k.value << "\"" << " old: " << "\"" << old.value << "\"");
			old = std::move(k);
			return;
		}
	}
	
	LogDebug("added key " << key << "=\"" << value << "\"");
	keys.emplace_back(std::move(k));
}

