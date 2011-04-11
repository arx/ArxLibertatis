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

#ifndef CONFIG_HASHMAP_H
#define CONFIG_HASHMAP_H

#include <iostream>
#include <map>
#include <string>

#include "core/ConfigSection.h"

class ConfigHashMap
{
private:

	std::map<std::string, ConfigSection> section_map;
	
	/**
	 * Parses an input stream for configuration section and respective keys.
	 * Stores them all in a section map as ConfigSection objects.
	 * @param is The input stream with the configuration info.
	 */
	void parse_stream( std::istream& input );

	/**
	 * Writes a ConfigSection to an output stream
	 * in a .ini style fashion using "[section]"
	 * section names and "key = value" key definitions
	 */
	void output_section( const ConfigSection& section, std::ostream& output ) const;

public:
	
	ConfigHashMap() {}
	ConfigHashMap( std::istream& input );

	bool AddElement( ConfigSection * _pLoc);

	/**
	 * Gets the specified configuration value from the map of ConfigSections
	 * @param section The section to search in
	 * @param default_value The default value to return if anything doesn't match
	 * @param key The key to look for in the section
	 * @return The value of the key found or the default value otherwise
	 */
	const std::string& getConfigValue( const std::string& section, const std::string& default_value, const std::string& key ) const;

	/**
	 * Updates the specified key in the specifified section
	 * with the value given. If the section or key does not
	 * exist in the map, it is created.
	 * @param section The section to update
	 * @param key The key in the section to update
	 * @param value The value to update the key with
	 */
	void updateConfigValue( const std::string& section, const std::string& key, const std::string value );

	const ConfigSection* getConfigSection( const std::string& str ) const;

	unsigned long GetKeyCount(const std::string &);

	/**
	 * Iterates the section map and outputs
	 * the contents of each map in a format
	 * fitting the common .ini style section
	 * and key setup
	 * @param output The stream to write data to
	 */
	void save_all( std::ostream& output ) const;
};

#endif // CONFIG_HASHMAP_H
