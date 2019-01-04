/*
 * Copyright 2016-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_DEBUG_DEBUGPANEL_H
#define ARX_GUI_DEBUG_DEBUGPANEL_H

#include <sstream>
#include <string>
#include <vector>

#include <boost/mpl/size.hpp>

#include "audio/AudioTypes.h"
#include "game/GameTypes.h"
#include "gui/hud/HudCommon.h"
#include "math/Angle.h"
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

namespace arx {
namespace debug {

std::ostream & operator<<(std::ostream & ss, Vec2i value);
std::ostream & operator<<(std::ostream & ss, Vec3f value);
std::ostream & operator<<(std::ostream & ss, Anglef value);
std::ostream & operator<<(std::ostream & ss, ResourcePool value);
std::ostream & operator<<(std::ostream & s, audio::SourceStatus val);

} // namespace debug
} // namespace arx

class DebugBox {
	
	struct Row {
		std::vector<std::string> fields;
	};
	
	struct ColInfo {
		int width;
		bool numeric;
		
		ColInfo()
			: width(0)
			, numeric(true)
		{ }
	};
	
	Vec2i m_pos;
	Vec2i m_chars;
	Vec2i m_size;
	std::string m_title;
	
	std::vector<Row> m_elements;
	std::vector<ColInfo> colums;
	
	void calcSizes();
	void printCommon();
	
	template <typename FIELD_T>
	void put(Row & row, FIELD_T val) {
		using arx::debug::operator<<;
		std::stringstream ss;
		ss << val;
		row.fields.push_back(ss.str());
	}
	
public:
	DebugBox(const Vec2i & pos, const std::string & title);
	
	template <typename F1, typename F2>
	void add(F1 f1, F2 f2) {
		Row row;
		put(row, f1);
		put(row, f2);
		m_elements.push_back(row);
	}
	
	template <typename F1, typename F2, typename F3>
	void add(F1 f1, F2 f2, F3 f3) {
		Row row;
		put(row, f1);
		put(row, f2);
		put(row, f3);
		m_elements.push_back(row);
	}
	
	template <typename F1, typename F2, typename F3, typename F4>
	void add(F1 f1, F2 f2, F3 f3, F4 f4) {
		Row row;
		put(row, f1);
		put(row, f2);
		put(row, f3);
		put(row, f4);
		m_elements.push_back(row);
	}
	
	void print();
	void print(Vec2i parent);
	
	Vec2i size();
};

#endif // ARX_GUI_DEBUG_DEBUGPANEL_H
