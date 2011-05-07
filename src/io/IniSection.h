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

#ifndef ARX_IO_INISECTION_H
#define ARX_IO_INISECTION_H

#include <string>
#include <vector>

struct IniKey {
	
	inline const std::string & getName() const { return name; }
	inline const std::string & getValue() const { return value; }
	
	int getValue(int defaultValue) const;
	
	float getValue(float defaultValue) const;
	
	bool getValue(bool defaultValue) const;
	
private:
	
	std::string name;
	std::string value;
	
	friend class IniSection;
};

class IniSection {
	
private:
	
	typedef std::vector<IniKey> Keys;
	Keys keys;
	
	/*!
	 * Add a key in the ini format (name=value or name="value")
	 * All preceding space and trailing space / commens must already be removed.
	 */
	void addKey(const std::string & key, const std::string & value);
	
	friend class IniReader;
	
public:
	
	typedef Keys::const_iterator iterator;
	
	inline iterator begin() const { return keys.begin(); }
	inline iterator end() const { return keys.end(); }
	inline bool empty() const { return keys.empty(); }
	inline bool size() const { return keys.size(); }
	
	const IniKey * getKey(const std::string & name) const;
	
};


#endif // ARX_IO_INISECTION_H
