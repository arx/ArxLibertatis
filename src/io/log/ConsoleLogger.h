/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_IO_LOG_CONSOLELOGGER_H
#define ARX_IO_LOG_CONSOLELOGGER_H

#include "io/log/LogBackend.h"

namespace logger {

/*!
 * Simple logger that prints plain text to standard output.
 */
class Console : public Backend {
	
public:
	
	~Console();
	
	void log(const Source & file, int line, Logger::LogLevel level, const std::string & str);
	
	void flush();
	
	/*!
	 * \return a ColorLogger instance if colors are supported or a ConsoleLogger otherwise.
	 *         May return null if standard output / error is discarded.
	 */
	static Backend * get();
	
};

} // namespace logger

#endif // ARX_IO_LOG_CONSOLELOGGER_H
