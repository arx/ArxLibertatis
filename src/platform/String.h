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

#ifndef ARX_PLATFORM_STRING_H
#define ARX_PLATFORM_STRING_H

#include <string>

void makeLowercase(std::string & str);
std::string toLowercase(const std::string & str);

/**
 * Load an std::string from a const char * that may not be null-terminated.
 */
std::string safestring(const char * data, size_t maxLength);

template <size_t N>
std::string safestring(const char (&data)[N]) {
	return safestring(data, N);
}

bool IsIn(const std::string & strin, const std::string & str);

/**
 * Converts a given string to an integer using stringstream
 * @param str The string to be converted
 * @return The integer created by stringstream
 */
int atoi( const std::string& str );

/**
 * Converts a given integer to a string using stringstream
 * @param i The integer to be converted
 * @return The string created by stringstream
 */
std::string itoa( int i );

/**
 * Converts a given bool value to a string using stringstream
 * @param b The bool to be converted
 * @return The string string created by stringstream
 */
std::string btoa( bool b );

/**
 * Converts a given string to a bool using stringstream
 * @param str The string to be converted
 * @return The bool created by stringstream
 */
bool atob( const std::string& str );

/**
 * Checks if a string (seek) is at the start of another string (text)
 * returns 0 if "seek" is at the start of "text"
 * else returns 1
 */
long specialstrcmp(const std::string & text, const std::string & seek);

void SAFEstrcpy(char * dest, const char * src, unsigned long max);

#endif // ARX_PLATFORM_STRING_H
