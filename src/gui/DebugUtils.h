/*
 * Copyright 2016 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_DEBUGUTILS_H
#define ARX_GUI_DEBUGUTILS_H

#include <sstream>
#include <string>
#include <vector>

#include <boost/mpl/size.hpp>

#include "game/GameTypes.h"
#include "math/Types.h"

template <typename T>
struct FlagName {
	T flag;
	const char * name;
};

template <typename T, size_t N>
std::string flagNames(const FlagName<T> (&names)[N], const T flags) {
	std::stringstream ss;
	for(size_t i = 0; i < N; i++) {
		if(names[i].flag & flags) {
			ss << names[i].name << " ";
		}
	}
	return ss.str();
}

class DebugBox {
public:
	DebugBox(const Vec2i & pos, const std::string & title);
	
	void add(const std::string & key, const std::string & value);
	void add(const std::string & key, const long value);
	void add(const std::string & key, const float value);
	void add(const std::string & key, const Vec2i value);
	void add(const std::string & key, const Vec3f value);
	void add(const std::string & key, const Anglef value);
	void add(const std::string & key, const ResourcePool value);
	
	void print();
	
	Vec2i size();
	
private:
	Vec2i m_pos;
	std::string m_title;
	size_t m_maxKeyLen;
	Vec2i m_size;
	
	
	std::vector<std::pair<std::string, std::string> > m_elements;
};

#endif // ARX_GUI_DEBUGUTILS_H
