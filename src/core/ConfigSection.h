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
// Code: Didier P�dreno

#ifndef CONFIGSECTION_H
#define CONFIGSECTION_H

#include <string>
#include <vector>

class ConfigSection
{
public:

	struct Key
	{
		Key() {}
		Key( const std::string& _name, const std::string& _value )
		{
			name = _name;
			value = _value;
		}

		std::string name;
		std::string value;
	};
	
	std::string section;
	std::vector<Key> _keys;
	
public:
	
	void SetSection( const std::string& _section );

/**
 * Sets a key name and pair to the specified values.
 * Will update an existing Key if the name matches
 * one already present.
 * @param key The Key to be set
 * @param value The value to be given to the Key
 */
	void set_key( const std::string& key, const std::string& value );

	/**
	 * Returns the first Key in this ConfigSection
	 * will probably return an invalid reference if
	 * not checked for the presence of keys, beware.
	 * @return The first Key in the list of Keys
	 */
	const Key& first_key() const;

	void AddKey( const std::string& _key );

	static bool isKey( const std::string& str );
	static bool isSection( const std::string& str );

private:


	std::string KeyName( std::string str ) const;

	/**
	 * Finds the value of the key string given and returns it.
	 * If no quotation marks surround the value, everything
	 * after the equals sign is considered the value.
	 * If only one quotation mark is found, it marks the
	 * beginning of the value until the end of the string.
	 * @param str The key string to extract the value from
	 * @return The extracted value from the string
	 */
	std::string KeyValue( std::string str ) const;

	std::string CleanKey( std::string str ) const;

	std::string CleanSection( std::string str ) const;
};


#endif // CONFIGSECTION_H
