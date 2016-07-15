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

#include <iomanip>

#include <boost/format.hpp>

#include "graphics/font/Font.h"
#include "gui/Text.h"
#include "math/Angle.h"

DebugBox::DebugBox(const Vec2i &pos, const std::string &title)
	: m_pos(pos)
	, m_title(title)
	, m_maxKeyLen(0)
{}

void DebugBox::add(std::string key, const std::string value) {
	m_maxKeyLen = std::max(m_maxKeyLen, key.length());
	m_elements.push_back(std::pair<std::string, std::string>(key, value));
}

void DebugBox::add(std::string key, const long value) {
	m_maxKeyLen = std::max(m_maxKeyLen, key.length());
	std::string valueStr = boost::str(boost::format("%ld") % value);
	m_elements.push_back(std::pair<std::string, std::string>(key, valueStr));
}

void DebugBox::add(std::string key, const float value) {
	m_maxKeyLen = std::max(m_maxKeyLen, key.length());
	std::string valueStr = boost::str(boost::format("%4.2f") % value);
	m_elements.push_back(std::pair<std::string, std::string>(key, valueStr));
}

void DebugBox::add(std::string key, const Vec2i value) {
	m_maxKeyLen = std::max(m_maxKeyLen, key.length());
	std::string valueStr = boost::str(boost::format("(%d, %d)") % value.x % value.y);
	m_elements.push_back(std::pair<std::string, std::string>(key, valueStr));
}

void DebugBox::add(std::string key, const Vec3f value) {
	m_maxKeyLen = std::max(m_maxKeyLen, key.length());
	std::string valueStr = boost::str(boost::format("%4.2f %4.2f %4.2f") % value.x % value.y % value.z);
	m_elements.push_back(std::pair<std::string, std::string>(key, valueStr));
}

void DebugBox::add(std::string key, const Anglef value) {
	m_maxKeyLen = std::max(m_maxKeyLen, key.length());
	std::string valueStr = boost::str(boost::format("%4.2f %4.2f %4.2f") % value.getYaw() % value.getPitch() % value.getRoll());
	m_elements.push_back(std::pair<std::string, std::string>(key, valueStr));
}

void DebugBox::add(std::string key, const ResourcePool value) {
	m_maxKeyLen = std::max(m_maxKeyLen, key.length());
	std::string valueStr = boost::str(boost::format("%4.2f/%4.2f") % value.current % value.max);
	m_elements.push_back(std::pair<std::string, std::string>(key, valueStr));
}

void DebugBox::print() {
	int lineHeight = hFontDebug->getLineHeight();
	Vec2i lineOffset = m_pos;
	
	hFontDebug->draw(lineOffset, std::string("╭─ ") + m_title, Color::white);
	lineOffset.y += lineHeight;
	
	std::vector<std::pair<std::string, std::string> >::const_iterator itr;
	for(itr = m_elements.begin(); itr != m_elements.end(); ++itr) {
		std::stringstream out;
		out << "│ " << std::left << std::setw(int(m_maxKeyLen)) << std::setfill(' ') << itr->first << " " << itr->second;
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
