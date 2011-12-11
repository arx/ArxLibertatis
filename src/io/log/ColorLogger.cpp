/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/Platform.h"

namespace logger {

ColorConsole::~ColorConsole() {
	// nothing to clean up
}

void ColorConsole::log(const Source & file, int line, Logger::LogLevel level, const std::string & str) {
	
	std::ostream * os;
	switch(level) {
		case Logger::Debug:   std::cout << "\e[1;36m[D]\e[0;36m", os = &std::cout; break;
		case Logger::Info:    std::cout << "\e[1;32m[I]\e[0;32m", os = &std::cout; break;
		case Logger::Warning: std::cerr << "\e[1;33m[W]\e[0;33m", os = &std::cerr; break;
		case Logger::Error:   std::cerr << "\e[1;31m[E]\e[0;31m", os = &std::cerr; break;
		case Logger::None: ARX_DEAD_CODE(); return;
	}
	
	(*os) << ' ' << file.name << "\e[m:\e[0;33m" << line << "\e[m" << "  " << str << std::endl;
}

void ColorConsole::flush() {
	std::cout.flush(), std::cerr.flush();
}

} // namespace logger
