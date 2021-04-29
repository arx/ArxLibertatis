/*
 * Copyright 2011-2014 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_IO_LOG_CRITICALLOGGER_H
#define ARX_IO_LOG_CRITICALLOGGER_H

#include "io/fs/FileStream.h"
#include "io/log/LogBackend.h"
#include "platform/CrashHandler.h"

namespace logger {

/*!
 * Logger that displays an error dialog on shutdown for the first critical error.
 */
class CriticalErrorDialog : public Backend {
	
	std::string errorString;
	
public:
	
	typedef void(*ExitCommand)();
	
	/*!
	 * \brief Set a question to ask and command to run on shutdown
	 *
	 * The question will be merged with the critical error if there was any.
	 *
	 * \param question the question string to ask. Must be statically allocated.
	 * \param command  the command to execute.
	 */
	static void setExitQuestion(const char * question, ExitCommand command);
	
	~CriticalErrorDialog();
	
	void log(const Source & file, int line, Logger::LogLevel level, const std::string & str);
};

} // namespace logger

#endif // ARX_IO_LOG_CRITICALLOGGER_H
