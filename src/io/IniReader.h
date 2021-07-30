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

#ifndef ARX_IO_INIREADER_H
#define ARX_IO_INIREADER_H

#include <map>
#include <string>
#include <string_view>

#include "io/IniSection.h"

/*!
 * Reader for .ini files.
 */
class IniReader {
	
	typedef std::map<std::string, IniSection, std::less<>> Sections;
	Sections sections;
	
public:
	
	typedef Sections::const_iterator iterator;
	
	IniReader() = default;
	
	explicit IniReader(std::string_view data) {
		read(data);
	}
	
	/*!
	 * Parses an input stream for configuration section and respective keys.
	 * Stores them all in a section map as IniSection objects.
	 * \param data The ini data.
	 * \param overrideValues override values if key already exists
	 * \return false if there were problems (some data may have been read)
	 */
	bool read(std::string_view data, bool overrideValues = false);
	
	void clear();
	
	/*!
	 * Gets the specified configuration value from the map of ConfigSections
	 * \param sectionName The section to search in
	 * \param keyName The key to look for in the section. Retuns the value of the first key if this is empty.
	 * \param defaultValue The default value to return if anything doesn't match
	 * \return The value of the key found or the default value otherwise
	 */
	std::string_view getKey(std::string_view sectionName, std::string_view keyName,
	                        std::string_view defaultValue) const;
	
	/*!
	 * Gets the specified configuration value from the map of ConfigSections
	 * \param sectionName The section to search in
	 * \param keyName The key to look for in the section. Retuns the value of the first key if this is empty.
	 * \param defaultValue The default value to return if anything doesn't match
	 * \return The value of the key found or the default value otherwise
	 */
	const std::string & getKey(std::string_view sectionName, std::string_view keyName,
	                           const std::string & defaultValue) const;
	
	/*!
	 * Reads an int from the ini and returns its converted int value,
	 * return the default value if an empty string is found.
	 * \param sectionName The section to read from
	 * \param keyName The key in the section to return
	 * \param defaultValue The default value to return in the case of an empty string
	 */
	int getKey(std::string_view sectionName, std::string_view keyName, int defaultValue) const;
	
	/*!
	 * Reads a float from the ini and returns its converted int value,
	 * return the default value if an empty string is found.
	 * \param sectionName The section to read from
	 * \param keyName The key in the section to return
	 * \param defaultValue The default value to return in the case of an empty string
	 */
	float getKey(std::string_view sectionName, std::string_view keyName, float defaultValue) const;
	
	/*!
	 * Reads a bool from the ini and returns its converted bool value,
	 * return the default value if an empty string is found.
	 * \param sectionName The section to read from
	 * \param keyName The key in the section to return
	 * \param defaultValue The default value to return in the case of an empty string
	 */
	bool getKey(std::string_view sectionName, std::string_view keyName, bool defaultValue) const;
	
	/*!
	 * Get the value at the specified key in the specified section.
	 * \return the value string or nullptr if no such value is set.
	 */
	const IniKey * getKey(std::string_view sectionName, std::string_view keyName) const;
	
	const IniSection * getSection(std::string_view sectionName) const;
	
	size_t getKeyCount(std::string_view sectionName) const;
	
	iterator begin() const { return sections.begin(); }
	iterator end() const { return sections.end(); }
	
};

#endif // ARX_IO_INIREADER_H
