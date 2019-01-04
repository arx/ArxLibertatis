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

#ifndef ARX_IO_INISECTION_H
#define ARX_IO_INISECTION_H

#include <string>
#include <vector>

class IniKey {
	
	std::string name;
	std::string value;
	
	friend class IniSection;
	
public:
	
	IniKey(const std::string & _name, const std::string & _value)
		: name(_name)
		, value(_value)
	{}
	
	const std::string & getName() const { return name; }
	const std::string & getValue() const { return value; }
	
	int getValue(int defaultValue) const;
	
	float getValue(float defaultValue) const;
	
	//! Support either boolean specified as strings (true, false) or 0, 1
	bool getValue(bool defaultValue) const;
	
};

class IniSection {
	
	typedef std::vector<IniKey> Keys;
	Keys keys;
	
	/*!
	 * Add a key in the ini format (name=value or name="value")
	 * All preceding space and trailing space / commens must already be removed.
	 */
	void addKey(const std::string & key, const std::string & value);
	/*!
	 * Set a key in the ini format (name=value or name="value")
	 * All preceding space and trailing space / commens must already be removed.
	 */
	void setKey(const std::string & key, const std::string & value);
	
	friend class IniReader;
	
public:
	
	typedef Keys::const_iterator iterator;
	
	iterator begin() const { return keys.begin(); }
	iterator end() const { return keys.end(); }
	bool empty() const { return keys.empty(); }
	size_t size() const { return keys.size(); }
	
	const IniKey * getKey(const std::string & name) const;
	
};

#endif // ARX_IO_INISECTION_H
