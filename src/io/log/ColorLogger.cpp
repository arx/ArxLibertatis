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

#include "io/log/ColorLogger.h"

#include <iostream>
#include <iomanip>

#include "platform/Platform.h"

#if ARX_HAVE_ISATTY

namespace logger {

ColorConsole::~ColorConsole() {
	// nothing to clean up
}

void ColorConsole::log(const Source & file, int line, Logger::LogLevel level,
                       const std::string & str) {
	
	std::ostream * os = (level >= Logger::Warning) ? &std::cerr : &std::cout;
	
	const char * c = "\x1b[m";
	const char * e = "";
	size_t length = 3;
	switch(level) {
		case Logger::Debug:    std::cout << "\x1b[1;36m[D]\x1b[0;36m"; break;
		case Logger::Info:     std::cout << "\x1b[1;32m[I]\x1b[0;32m"; break;
		case Logger::Console:  std::cout << "\x1b[1;37m[C]\x1b[0;37m"; break;
		case Logger::Warning:  {
			std::cerr << "\x1b[1;33m[W]\x1b[0;33m";
			e = c, c = "\x1b[0;33m";
			break;
		}
		case Logger::Error: {
			std::cerr << "\x1b[1;31m[E]\x1b[0;31m";
			e = c, c = "\x1b[0;31m";
			break;
		}
		case Logger::Critical: {
			std::cerr << "\x1b[1;31m[CRITICAL]\x1b[0;31m";
			e = c, c = "\x1b[1;31m", length = 10;
			break;
		}
		case Logger::None: arx_unreachable();
	}
	(*os) << ' ' << file.name << "\x1b[m:\x1b[0;33m";
	
	std::ostream::fmtflags flags = os->flags();
	
	length += 1 + file.name.length() + 1;
	if(length < alignment) {
		(*os) << std::left << std::setfill(' ') << std::setw(alignment - length);
	}
	
	(*os) << line << " " << c << str << e << std::endl;
	
	os->flags(flags);
}

void ColorConsole::flush() {
	std::cout.flush(), std::cerr.flush();
}

} // namespace logger

#endif // ARX_HAVE_ISATTY
