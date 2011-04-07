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

#include "core/ConfigSection.h"

#include <list>
#include <sstream>

#include "io/Logger.h"

/**
 * Sets the section that this ConfigSection represents.
 * Checks that the section string provided is actually
 * a valid section before cleaning the string of extra
 * characters and setting it as the new section.
 * @param _section The new section for this ConfigSection
 */
void ConfigSection::SetSection( const std::string& _section )
{
	if ( isSection( _section ) )
		section = CleanSection( _section );
	LogDebug << "Set section to " << section;
};

/**
 * Adds an information key to this section. Checks that a key
 * is actually a key and cleans it of extra characters before
 * adding it.
 * @param _key The new key to be added to this ConfigSection
 */
void ConfigSection::AddKey( const std::string& _key )
{
	if ( isKey( _key ) )
		keys.push_back( CleanKey( _key ) );

	_keys.push_back( Key( KeyName( _key ), KeyValue( _key ) ) );
};

/*****************************************************************
 * Checks a str for square brackets and makes sure they appear
 * left first, right second. This confirms str as a section line.
 *****************************************************************/
bool ConfigSection::isSection( const std::string& str )
{
	// Location of square brackets in str
	size_t first_bracket, last_bracket;

	// Get location of '[' and ']' in str
	first_bracket = str.find_first_of('[');
	last_bracket = str.find_last_of(']');

	// If either of the brackets are not present, not a section
	if ( first_bracket == std::string::npos ) return false;
	if ( last_bracket == std::string::npos ) return false;

	// '[' must be before ']'
	if ( first_bracket > last_bracket ) return false;

	// It is a section
	return true;
}

/*****************************************************************
 * Checks whether or not str is a valid key line.
 * Looks for an alphanumeric key followed by
 * at least one '=' character.
 ****************************************************************/
bool ConfigSection::isKey( const std::string& str )
{
	// Iterate through str until alphanumeric characters are found
	for ( size_t i = 0 ; i < str.length() ; i++ )
	{
		// If we hit an '=' before other alphanumeric characters, return false
		if ( str[i] == '=' ) return false;

		// Just one alphanumeric is enough for a key, break out of the loop
		if ( isalnum(str[i]) )
			break;
	}

	// Now find the location of the first equals sign
	size_t equals_loc = str.find('=');

	// No equals sign present means not a key, return false
	if ( equals_loc == std::string::npos ) return false;

	// Value can be empty, no need to check for one. Return true
	return true;
}
	
/**************************************************************
 * Removes all characters preceding the first '[' and
 * proceeding the first ']' in a copy of _str and returns
 * the result.
 *************************************************************/
std::string ConfigSection::CleanSection( std::string str ) const
{
	// Find the first cutoff point
	size_t first_bracket = str.find_first_of('[');

	// Erase all characters up to and including the '['
	str.erase( 0, first_bracket + 1 );

	// Find the last cutoff point
	size_t last_bracket = str.find_last_of(']');

	// Erase all characters following and including the ']'
	str.erase( last_bracket );

	// Return the processed string
	return str;
}

/********************************************************************
 * Cleans up a key string by removing the type identifier before
 * '=' appears and extracting the string between the first two ""
 * marks.
 *******************************************************************/
std::string ConfigSection::CleanKey( std::string str ) const
{
	// The equals sign seperates the type identifier from the value
	size_t equals_loc = str.find_first_of('=');

	// Find the beginning of the value after the equals sign
	size_t first_quot_mark = str.find( '"', equals_loc );

	// Cut the string until the beginning of the value
	str = str.substr( first_quot_mark + 1 );

	// Find the next " mark that shows the end of the value
	size_t last_quot_mark = str.find_last_of('"');

	// Cut the string at the end of the value
	str = str.substr( 0, last_quot_mark );

	// Return the processed string
	return str;
}

/**
 * Finds the first string of non-whitespace characters
 * and returns it as the key name
 * @param str The string containing the Key name
 * @return The extracted name
 */
std::string ConfigSection::KeyName( std::string str ) const
{
	// Find the beginning of the name, the first non-whitespace character
	size_t name_begin = 0;
	for ( size_t i = 0 ; i < str.size() ; i++ )
	{
		name_begin = i;

		// If non-whitespace is hit, that's the beginning of the name
		if ( !isspace( str[i] ) )
			break; // Break out and continue
	}

	// Find the end of the name, the first whitespace or '=' character encountered
	size_t name_end = name_begin;
	for ( size_t i = name_begin ; i < str.size() ; i++ )
	{
		name_end = i;

		// If a whitespace character or an '=' character is hit, that's the end of the name
		if ( isspace( str[i] ) || str[i] == '=' )
			break;
	}

	// Return the processed string
	return str.substr( name_begin, name_end - name_begin );
}

/**
 * Finds the value of the key string given and returns it.
 * If no quotation marks surround the value, everything
 * after the equals sign is considered the value.
 * If only one quotation mark is found, it marks the
 * beginning of the value until the end of the string.
 * @param str The key string to extract the value from
 * @return The extracted value from the string
 */
std::string ConfigSection::KeyValue( std::string str ) const
{
	// The equals sign seperates the name from the value
	size_t equals_loc = str.find_first_of('=');

	// Cut away everything before and including the equals sign
	str = str.substr( equals_loc + 1 );

	// Find the beginning of the value as the first quotation mark
	size_t first_quot_mark = str.find_first_of( '"' );

	// Cut the string until the beginning of the value
	str = str.substr( first_quot_mark + 1 );

	// Find the last " mark that shows the end of the value
	size_t last_quot_mark = str.find_last_of('"');

	// Cut the string at the end of the value
	str = str.substr( 0, last_quot_mark );

	// Return the processed string
	return str;
}

/**
 * Returns the first Key in this ConfigSection
 * will probably return an invalid reference if
 * not checked for the presence of keys, beware.
 * @return The first Key in the list of Keys
 */
const Key& ConfigSection::first_key() const
{
	return _keys.front();
}

/**
 * Sets a key name and pair to the specified values.
 * Will update an existing Key if the name matches
 * one already present.
 * @param key The Key to be set
 * @param value The value to be given to the Key
 */
void ConfigSection::set_key( const std::string& key, const std::string& value )
{
	// Check if the key already exists
	std::vector<Key>::iterator iter;
	for ( iter = _keys.begin() ; iter != _keys.end() ; iter++ )
	{
		// If found, update the value and return
		if ( iter->name == key )
		{
			iter->value = value;
			return;
		}
	}

	// If the key wasn't found, add it
	_keys.push_back( Key( key, value ) );
}
