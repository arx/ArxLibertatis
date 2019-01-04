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

#include "gui/debug/DebugPanel.h"

#include <algorithm>
#include <iomanip>

#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "math/Angle.h"


namespace arx {
namespace debug {

std::ostream & operator<<(std::ostream & ss, Vec2i value) {
	ss << boost::format("(%d, %d)") % value.x % value.y;
	return ss;
}

std::ostream & operator<<(std::ostream & ss, Vec3f value) {
	ss << boost::format("%4.2f %4.2f %4.2f") % value.x % value.y % value.z;
	return ss;
}

std::ostream & operator<<(std::ostream & ss, Anglef value) {
	ss << boost::format("%4.2f %4.2f %4.2f") % value.getPitch() % value.getYaw() % value.getRoll();
	return ss;
}

std::ostream & operator<<(std::ostream & ss, ResourcePool value) {
	ss << boost::format("%4.2f/%4.2f") % value.current % value.max;
	return ss;
}

std::ostream & operator<<(std::ostream & s, audio::SourceStatus val) {
	switch(val) {
		case audio::Idle:
			s << "idle";
			break;
		case audio::Playing:
			s << "playing";
			break;
		case audio::Paused:
			s << "paused";
			break;
		default:
			break;
	}
	return s;
}

} // namespace debug
} // namespace arx

DebugBox::DebugBox(const Vec2i & pos, const std::string & title)
	: m_pos(pos)
	, m_chars(0)
	, m_size(0)
	, m_title(title)
{ }

void DebugBox::calcSizes() {
	
	colums.clear();
	
	bool first = true;
	BOOST_FOREACH(const Row & row, m_elements) {
		colums.resize(std::max(colums.size(), row.fields.size()));
		int i = 0;
		BOOST_FOREACH(const std::string & field, row.fields){
			colums[i].width = std::max(colums[i].width, int(field.size()));
			
			if(!first) {
				bool numeric = true;
				for(size_t u = 0; u < field.size(); ++u) {
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
	
	int maxRowChars = 2;
	for(size_t i = 0; i < colums.size(); ++i) {
		maxRowChars += colums[i].width + 1;
	}
	
	m_chars = Vec2i(maxRowChars, m_elements.size() + 2);
	
	int lineHeight = hFontDebug->getLineHeight();
	int maxAdvance = hFontDebug->getMaxAdvance();
	
	m_size = Vec2i(m_chars.x * maxAdvance, m_chars.y * lineHeight);
}

void DebugBox::print() {
	calcSizes();
	printCommon();
}

void DebugBox::print(Vec2i parent) {
	calcSizes();
	m_pos = parent - m_size - Vec2i(40, 80);
	printCommon();
}

void DebugBox::printCommon() {
	
	int lineHeight = hFontDebug->getLineHeight();
	Vec2i lineOffset = m_pos;
	
	std::stringstream top;
	top << "╭─ " << m_title << " ";
	for(int i = 0; i < m_chars.x - 3 - int(m_title.size()); i++)
		top << "─";
	top << "┐";
	
	hFontDebug->draw(lineOffset, top.str(), Color::white);
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
	
	hFontDebug->draw(lineOffset, std::string("╰─"), Color::white);
}

Vec2i DebugBox::size() {
	return m_pos + m_size;
}
