/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

using std::string;
using std::transform;

void makeLowercase(string & str) {
	transform(str.begin(), str.end(), str.begin(), ::tolower);
}

string toLowercase(const string & str) {
	string copy = str;
	makeLowercase(copy);
	return copy;
}

string safestring(const char * data, size_t maxLength) {
	return string(data, std::find(data, data + maxLength, '\0'));
}

bool IsIn(const string & strin, const string & str) {
	return (strin.find( str ) != string::npos);
}

long specialstrcmp(const string & text, const string & seek) {
	return text.compare(0, seek.length(), seek) ? 1 : 0;
}

int atoi( const std::string& str )
{
	std::stringstream ss( str );
	int out;
	ss >> out;
	return out;
}

std::string itoa( int i )
{
	std::stringstream ss;
	ss << i;
	std::string out;
	ss >> out;
	return out;
}

bool atob( const std::string& str )
{
	std::stringstream ss( str );
	bool out;
	ss >> out;
	return out;
}

std::string btoa( bool i )
{
	std::stringstream ss;
	ss << i;
	std::string out;
	ss >> out;
	return out;
}

void SAFEstrcpy(char * dest, const char * src, unsigned long max) {
	if(strlen(src) > max) {
		memcpy(dest, src, max);
	} else {
		strcpy(dest, src);
	}
}
