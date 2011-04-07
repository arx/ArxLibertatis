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
// todo remover les strIcmp

#include "core/ConfigHashMap.h"

#include <list>
#include <sstream>

#include "io/String.h"
#include "io/Logger.h"

//-----------------------------------------------------------------------------
ConfigHashMap::ConfigHashMap( unsigned int init_size )
{
	pTab = new ConfigSection* [init_size];

	size = init_size;
	fill = 0;
	iMask = size - 1;

	for (unsigned long i = 0; i < size; i++)
	{
		pTab[i] = NULL;
	}

	iNbCollisions = iNbNoInsert = 0;
}

ConfigHashMap::ConfigHashMap( unsigned int init_size, std::istream& input )
{
	pTab = new ConfigSection* [init_size];

	size = init_size;
	fill = 0;
	iMask = size - 1;

	for (unsigned long i = 0; i < size; i++)
	{
		pTab[i] = NULL;
	}

	iNbCollisions = iNbNoInsert = 0;

	parse_stream( input );
}

//-----------------------------------------------------------------------------
ConfigHashMap::~ConfigHashMap()
{
	while (size--)
	{
		if (pTab[size] != NULL)
		{
			delete pTab[size];
			pTab[size] = NULL;
		}
	}

	delete [] pTab;
	pTab = NULL;
}

//-----------------------------------------------------------------------------
int ConfigHashMap::FuncH1(int _iKey)
{
	return _iKey;
}

//-----------------------------------------------------------------------------
int	ConfigHashMap::FuncH2(int _iKey)
{
	return ((_iKey >> 1) | 1);
}

//-----------------------------------------------------------------------------
int	ConfigHashMap::GetKey(const std::string& _lpszUText)
{
	int iKey = 0;
	int iLenght = _lpszUText.length();
	int iLenght2 = iLenght;

	while (iLenght--)
	{
		iKey += _lpszUText[iLenght] * (iLenght + 1) + _lpszUText[iLenght] * iLenght2;
	}

	return iKey;
}

//-----------------------------------------------------------------------------
void ConfigHashMap::ReHash()
{
	unsigned long	iNewSize = size << 1;
	long	iNewMask = iNewSize - 1;

	ConfigSection ** pTab2 = new ConfigSection *[iNewSize];

	for (unsigned long i = 0 ; i < iNewSize ; i++)
	{
		pTab2[i] = NULL;
	}


	for (unsigned int i = 0 ; i < size ; i++)
	{
		if (pTab[i] != NULL)
		{
			int iKey = GetKey( pTab[i]->section );
			int	iH1	 = FuncH1(iKey);
			int	iH2  = FuncH2(iKey);

			unsigned int iNbSolution = 0;

			while (iNbSolution < iNewSize)
			{
				iH1 &= iNewMask;

				if (pTab2[iH1] == NULL)
				{
					pTab2[iH1]	= pTab[i];
					pTab[i]		= NULL;
					//fill ++;
				}

				iNbCollisions ++;
				iH1 += iH2;

				iNbSolution ++;
			}

			iNbNoInsert ++;
		}
	}

	size = iNewSize;
	iMask = iNewMask;

	delete [] pTab;
	pTab = pTab2;
}

//-----------------------------------------------------------------------------
bool ConfigHashMap::AddElement(ConfigSection * _pLoc)
{
	if ( section_map.find( _pLoc->section ) == section_map.end() )
		section_map.insert( std::pair<std::string, ConfigSection>( _pLoc->section, *_pLoc ) );

	if (fill >= size * 0.75)
	{
		ReHash();
	}

	if ( !(_pLoc && !_pLoc->section.empty()) ) return false;

	int iKey = GetKey(_pLoc->section);
	int	iH1 = FuncH1(iKey);
	int	iH2 = FuncH2(iKey);

	unsigned long iNbSolution = 0;

	while (iNbSolution < size)
	{
		iH1 &= iMask;

		if (pTab[iH1] == NULL)
		{
			pTab[iH1] = _pLoc;
			fill++;
			return true;
		}

		iNbCollisions ++;
		iH1 += iH2;

		iNbSolution ++;
	}

	iNbNoInsert ++;
	return false;
}

const ConfigSection* ConfigHashMap::getConfigSection( const std::string& section ) const
{
	std::map<std::string, ConfigSection>::const_iterator iter = section_map.find( section );

	if ( iter != section_map.end() )
		return &(iter->second);
	else
		return 0;
}

std::string* ConfigHashMap::GetPtrWithString(const std::string& _lpszUText)
{
	std::map<std::string, ConfigSection>::iterator iter = section_map.find( _lpszUText );

	int iKey = GetKey(_lpszUText);
	int	iH1 = FuncH1(iKey);
	int	iH2 = FuncH2(iKey);

	unsigned long iNbSolution = 0;

	while (iNbSolution < size)
	{
		iH1 &= iMask;

		if ( pTab[iH1] )
		{
			if ( !strcasecmp( _lpszUText, pTab[iH1]->section ) )
			{
				if (pTab[iH1]->keys.size() > 0)
				{
					return &pTab[iH1]->keys[0];
				}

				return NULL;
			}
		}

		iH1 += iH2;
		iNbSolution++;
	}

	return NULL;
}

unsigned long ConfigHashMap::GetKeyCount(const std::string& _lpszUText)
{
	int iKey = GetKey(_lpszUText);
	int	iH1 = FuncH1(iKey);
	int	iH2 = FuncH2(iKey);

	unsigned long iNbSolution = 0;

	while (iNbSolution < size)
	{
		iH1 &= iMask;

		if (pTab[iH1])
		{
			if ( !strcasecmp( _lpszUText, pTab[iH1]->section ) )
			{
				return pTab[iH1]->keys.size();

				return 0;
			}
		}

		iH1 += iH2;
		iNbSolution++;
	}

	return 0;
}

/**
 * Gets the specified configuration value from the map of ConfigSections
 * @param section The section to search in
 * @param default_value The default value to return if anything doesn't match
 * @param key The key to look for in the section
 * @return The value of the key found or the default value otherwise
 */
const std::string& ConfigHashMap::getConfigValue( const std::string& section, const std::string& default_value, const std::string& key ) const
{
	// Look for a section
	const ConfigSection* config = getConfigSection( section );

	// If the section was not found, return default
	if ( !config )
	{
		return default_value;
	}

	// If the section has no keys, return default
	if ( config->_keys.empty() )
		return default_value;

	// If the key is not specified, return the first ones value( to avoid breakage with legacy assets )
	if ( key.empty() )
	{
		return config->_keys[0].value;
	}

	// Otherwise try to match the key to one in the section
	for ( size_t i = 0 ; i < config->_keys.size() ; i++ )
		if ( config->_keys[i].name == key ) // If the key name matches that specified
			return config->_keys[i].value; // Return the value of the requested key

	// If the key was not found, return default
	return default_value;
}

/**
 * Parses an input stream for configuration section and respective keys.
 * Stores them all in a section map as ConfigSection objects.
 * @param is The input stream with the configuration info.
 */
void ConfigHashMap::parse_stream( std::istream& is )
{
	std::list<std::string> input_strings; // Resulting list of lines extracted
	
	while ( is.good() ) // While lines remain to be extracted
	{
		// Get a line to process
		std::string str;
		std::getline( is, str );

		// Remove any commented bits until the line end
		size_t comment_start = std::string::npos;
		comment_start = str.find("//");

		// Whole line was commented, no need to do anything with it. Continue getting the next line
		if ( comment_start == 0 ) continue;

		// Remove any commented bits from the line
		if ( comment_start != std::string::npos )
			str = str.substr(0, comment_start );

		if ( str.empty() ) continue;
		// Push back the line
		input_strings.push_back( str );
	}

	// Iterate over all the lines entered into the list earlier
	std::list<std::string>::iterator iter = input_strings.begin();
	while( iter != input_strings.end() )
	{
		// If a section string is found, make en entry for it
		if ( ConfigSection::isSection( *iter ) )
		{
			// Create a localisation entry for this section
			ConfigSection* loc = new ConfigSection();

			// Set the section name as the cleaned section string
			loc->SetSection( *iter );

			// Advance to the line after the section string to start looking for keys
			iter++;

			// Iterate over more strings until a section is encountered or no more strings remain
			while ( ( iter != input_strings.end() ) && ( !ConfigSection::isSection( *iter ) ) )
			{
				// If a key is found, add it to the localisation entry
				if ( ConfigSection::isKey( *iter ) )
					loc->AddKey( *iter );

				iter++; // Continue looking for more keys
			}

			AddElement(loc);
			continue; // Continue loop, the next section or end of input was encountered
		}

		// Only get here if a non-content string managed to escape the earlier culling, advance to the next line
		iter++;
	}
}

/**
 * Updates the specified key in the specifified section
 * with the value given. If the section or key does not
 * exist in the map, it is created.
 * @param section The section to update
 * @param key The key in the section to update
 * @param value The value to update the key with
 */
void ConfigHashMap::updateConfigValue( const std::string& section, const std::string& key, const std::string value )
{
	std::map<std::string, ConfigSection>::iterator iter = section_map.find( section );

	// If the section exists, set the key value
	if ( iter != section_map.end() )
		iter->second.set_key( key, value );
	else // Otherwise, create the section and set the key in that section
	{
		section_map[section].section = section;
		section_map[section].set_key( key, value );
	}
}

void ConfigHashMap::save_all( std::ostream& out ) const
{
	// Iterate over the sections and output them into the stream
	std::map<std::string, ConfigSection>::const_iterator iter;
	for ( iter = section_map.begin() ; iter != section_map.end() ; iter++ )
		output_section( iter->second, out );
}

void ConfigHashMap::output_section( const ConfigSection& section, std::ostream& out ) const
{
	// output section name with included surrounding brackets
	out << '[' << section.section << ']' << std::endl;

	// Iterate over all the keys and output them into the stream
	std::vector<ConfigSection::Key>::const_iterator iter;
	for ( iter = section._keys.begin() ; iter != section._keys.end() ; iter++ )
		out << iter->name << '=' << '\"' << iter->value << '\"' << std::endl;
}
