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

#include "io/log/LogBackend.h"

#include <ios>
#include <iomanip>

#include "platform/Platform.h"

void logger::Backend::format(std::ostream & os, const Source & file,
                             int line, Logger::LogLevel level, const std::string & str) {
	
	size_t length = 3;
	switch(level) {
		case Logger::Debug:    os << "[D]"; break;
		case Logger::Info:     os << "[I]"; break;
		case Logger::Console:  os << "[C]"; break;
		case Logger::Warning:  os << "[W]"; break;
		case Logger::Error:    os << "[E]"; break;
		case Logger::Critical: os << "[CRITICAL]", length = 10; break;
		case Logger::None: arx_unreachable();
	}
	os << ' ' << file.name << ':';
	
	std::ostream::fmtflags flags = os.flags();
	
	length += 1 + file.name.length() + 1;
	if(length < alignment) {
		os << std::left << std::setfill(' ') << std::setw(alignment - length);
	}
	
	os << line << "  " << str << std::endl;
	
	os.flags(flags);
}
