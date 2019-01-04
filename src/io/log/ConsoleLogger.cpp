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

#include "io/log/ConsoleLogger.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Configure.h"

#if ARX_HAVE_ISATTY
#include <unistd.h>
#endif

#include "io/log/ColorLogger.h"
#include "platform/Environment.h"

namespace logger {

Console::~Console() {
	// nothing to clean up
}

void Console::log(const Source & file, int line, Logger::LogLevel level, const std::string & str) {
	std::ostream & os = (level == Logger::Warning || level == Logger::Error) ? std::cerr : std::cout;
	format(os, file, line, level, str);
}

void Console::flush() {
	std::cout.flush(), std::cerr.flush();
}

Backend * Console::get() {
	
	bool hasStdout = platform::hasStdOut();
	bool hasStdErr = platform::hasStdErr();
	if(!hasStdout && !hasStdErr) {
		return NULL;
	}
	
	#if ARX_HAVE_ISATTY
	if((!hasStdout || isatty(1)) && (!hasStdErr || isatty(2)) && !std::getenv("NO_COLOR")) {
		char * term = std::getenv("TERM");
		if(term && std::strcmp(term, "dumb") != 0) {
			return new ColorConsole;
		}
	}
	#endif
	
	return new Console;
}

} // namespace logger
