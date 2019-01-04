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

#include "io/log/CriticalLogger.h"

#include <sstream>

#include "core/Version.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "platform/Dialog.h"
#include "platform/Platform.h"

namespace logger {

static const char * g_exitQuestion = NULL;
static CriticalErrorDialog::ExitCommand g_exitCommand = NULL;

CriticalErrorDialog::~CriticalErrorDialog() {
	
	bool runExitCommand = false;
	
	if(!errorString.empty()) {
		
		std::ostringstream message;
		message << errorString;
		fs::path logfile = fs::getUserDir() / "arx.log";
		if(fs::exists(logfile)) {
			message << "\n\nYou might want to look at the log for more details:\n";
			message << logfile.string();
			if(g_exitQuestion && g_exitCommand) {
				message << "\n";
			}
		}
		
		if(g_exitQuestion && g_exitCommand) {
			message << "\n\n-> " << g_exitQuestion;
			runExitCommand = platform::askYesNoWarning(message.str(), "Error - " + arx_name);
		} else {
			platform::showErrorDialog(message.str(), "Critical Error - " + arx_name);
		}
		
	} else if(g_exitQuestion && g_exitCommand) {
		runExitCommand = platform::askYesNo(g_exitQuestion, arx_name);
	}
	
	if(runExitCommand) {
		g_exitCommand();
	}
	
}

void CriticalErrorDialog::setExitQuestion(const char * question, ExitCommand command) {
	if(!g_exitQuestion || !g_exitCommand) {
		g_exitQuestion = question;
		g_exitCommand = command;
	}
}

void CriticalErrorDialog::log(const Source & file, int line, Logger::LogLevel level,
                              const std::string & str) {
	ARX_UNUSED(file), ARX_UNUSED(line);
	
	if(level == Logger::Critical && errorString.empty()) {
		errorString = str;
	}
}

} // namespace logger
