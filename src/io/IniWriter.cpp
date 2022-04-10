/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include "io/IniWriter.h"

#include <ios>

void IniWriter::beginSection(std::string_view section) {
	output << '\n' << '[' << section << ']' << '\n';
}

void IniWriter::writeKey(std::string_view key, std::string_view value) {
	output << key << '=' << '"' << value << '"' << '\n';
}

void IniWriter::writeKey(std::string_view key, int value) {
	output << key << '=' << value << '\n';
}

void IniWriter::writeKey(std::string_view key, float value) {
	output << key << '=' << value << '\n';
}

void IniWriter::writeKey(std::string_view key, bool value) {
	output << key << '=' << std::boolalpha << value << '\n';
}
