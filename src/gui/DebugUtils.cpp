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

#include "gui/DebugUtils.h"

#include <algorithm>
#include <iomanip>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "math/Angle.h"


namespace arx {
namespace debug {

std::stringstream &operator <<(std::stringstream &ss, const Vec2i value) {
	ss << boost::str(boost::format("(%d, %d)") % value.x % value.y);
	return ss;
}

std::stringstream &operator <<(std::stringstream &ss, const Vec3f value) {
	ss << boost::str(boost::format("%4.2f %4.2f %4.2f") % value.x % value.y % value.z);
	return ss;
}

std::stringstream &operator <<(std::stringstream &ss, const Anglef value) {
	ss << boost::str(boost::format("%4.2f %4.2f %4.2f") % value.getPitch() % value.getYaw() % value.getRoll());
	return ss;
}

std::stringstream &operator <<(std::stringstream &ss, const ResourcePool value) {
	ss << boost::str(boost::format("%4.2f/%4.2f") % value.current % value.max);
	return ss;
}

} // namespace debug
} // namespace arx


DebugBox::DebugBox(const Vec2i & pos, const std::string & title)
	: m_pos(pos)
	, m_title(title)
{ }

void DebugBox::print() {
	
	struct ColInfo {
		int width;
		bool numeric;
		
		ColInfo()
			: width(0)
			, numeric(true)
		{ }
	};
	
	std::vector<ColInfo> colums;
	bool first = true;
	BOOST_FOREACH(const Row & row, m_elements) {
		colums.resize(std::max(colums.size(), row.fields.size()));
		int i = 0;
		BOOST_FOREACH(const std::string & field, row.fields){
			colums[i].width = std::max(colums[i].width, int(field.size()));
			
			if(!first) {
				bool numeric = true;
				for(size_t u=0; u<field.size(); ++u) {
					if(!(field[u] >= 48 && field[u] <= 57)) {
						numeric = false;
						break;
					}
				}
				
				if(!numeric) {
					colums[i].numeric = false;
				}
			}
			
			i++;
		}
		first = false;
	}
	
	int lineHeight = hFontDebug->getLineHeight();
	Vec2i lineOffset = m_pos;
	
	hFontDebug->draw(lineOffset, std::string("╭─ ") + m_title, Color::white);
	lineOffset.y += lineHeight;
	
	BOOST_FOREACH(const Row & row, m_elements) {
		std::stringstream out;
		out << "│ ";
		for(size_t i = 0; i < row.fields.size(); ++i) {
			if(colums[i].numeric) {
				out << std::right;
			} else {
				out << std::left;
			}
			out << std::setw(colums[i].width) << std::setfill(' ');
			out << row.fields[i] << ' ';
		}
		hFontDebug->draw(lineOffset, out.str(), Color::white);
		lineOffset.y += lineHeight;
	}
	
	hFontDebug->draw(lineOffset, std::string("╰─ "), Color::white);
	lineOffset.y += lineHeight;
	
	m_size = lineOffset;
}

Vec2i DebugBox::size() {
	return m_size;
}
