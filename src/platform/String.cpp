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

#include "platform/String.h"

#include <cstring>
#include <algorithm>
#include <sstream>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

void makeLowercase(std::string & str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

std::string safestring(const char * data, size_t maxLength) {
	return std::string(data, std::find(data, data + maxLength, '\0'));
}

int atoi(const std::string & str) {
	std::stringstream ss( str );
	int out;
	ss >> out;
	return out;
}

std::string itoa(int i) {
	std::stringstream ss;
	ss << i;
	std::string out;
	ss >> out;
	return out;
}

bool atob(const std::string & str) {
	std::stringstream ss( str );
	bool out;
	ss >> out;
	return out;
}

std::string btoa(bool i) {
	std::stringstream ss;
	ss << i;
	std::string out;
	ss >> out;
	return out;
}

struct character_escaper {
	template<typename FinderT>
	std::string operator()(const FinderT & match) const {
		std::string s;
		for(typename FinderT::const_iterator i = match.begin(); i != match.end(); i++) {
			s += std::string("\\") + *i;
		}
		return s;
	}
};

std::string escapeString(const std::string & str, const char * escapeChars) {
	std::string escapedStr(str);
	boost::find_format_all(escapedStr,
	                       boost::token_finder(boost::is_any_of(escapeChars)),
	                                           character_escaper());
	return escapedStr;
}
