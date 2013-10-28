/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include <cstring>
#include <iostream>

#include "Configure.h"

#if ARX_HAVE_ISATTY || ARX_HAVE_READLINK
#include <unistd.h>
#include <errno.h>
#endif

#include "io/log/ColorLogger.h"
#include "platform/Platform.h"

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

static bool is_fd_disabled(int fd) {
	
	ARX_UNUSED(fd);
	
	// Disable the console log backend if output is redirected to /dev/null
#if ARX_HAVE_READLINK
	static const char * names[] = { NULL, "/proc/self/fd/1", "/proc/self/fd/2" };
	char path[64];
	ssize_t len = readlink(names[fd], path, ARRAY_SIZE(path));
	if(len == 9 && !memcmp(path, "/dev/null", 9)) {
		return true;
	} else if(len == -1 && errno == ENOENT) {
		return true;
	}
#endif
	
	return false;
}

Backend * Console::get() {
	
#if ARX_HAVE_ISATTY
	if(isatty(1) && isatty(2)) {
		return new ColorConsole;
	}
#endif
	
	if(is_fd_disabled(1) && is_fd_disabled(2)) {
		return NULL;
	}
	
	return new Console;
}

} // namespace logger
