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

#ifndef ARX_IO_LOG_LOGBACKEND_H
#define ARX_IO_LOG_LOGBACKEND_H

#include <ostream>

#include "io/log/Logger.h"

namespace logger {

struct Source {
	
	const char * file;
	
	std::string name;
	
	Logger::LogLevel level;
	
};

class Backend {
	
public:
	
	virtual ~Backend() { }
	
	virtual void log(const Source & file, int line, Logger::LogLevel level, const std::string & str) = 0;
	
	virtual void flush() = 0;
	
	/*!
	 * Format a log entry for a text-based logging backend.
	 */
	void format(std::ostream & os, const Source & file, int line, Logger::LogLevel level, const std::string & str);
	
};

} // namespace logger

#endif // ARX_IO_LOG_LOGBACKEND_H
