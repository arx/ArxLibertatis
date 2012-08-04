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
 
#include "platform/Dialog.h"

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#else
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <sys/wait.h>
#endif

#include <boost/format.hpp>

#include "io/log/Logger.h"
#include "platform/String.h"

namespace dialog {
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32

bool showDialog(DialogType type, const std::string & message,
                const std::string & dialogTitle) {
	
	UINT flags;
	switch(type) {
		case DialogInfo:     flags = MB_ICONINFORMATION | MB_OK; break;
		case DialogWarning:  flags = MB_ICONWARNING | MB_OK; break;
		case DialogError:    flags = MB_ICONERROR | MB_OK; break;
		case DialogYesNo:    flags = MB_ICONQUESTION | MB_YESNO; break;
		case DialogOkCancel: flags = MB_ICONQUESTION | MB_OKCANCEL; break;
	}
	
	int ret = MessageBoxA(NULL, message.c_str(), dialogTitle.c_str(),
	                      flags | MB_SETFOREGROUND | MB_TOPMOST);
	
	switch(ret) {
		case IDCANCEL:
		case IDNO:
			return false;
		case IDYES:
		case IDOK:
			return true;
	}
	
	return false;
}

#elif ARX_PLATFORM == ARX_PLATFORM_MACOSX
	// SEE Dialog.mm for the implementation of showDialog
    bool showDialog(DialogType type, const std::string & message, 
                    const std::string & dialogTitle);
#else

std::string escape(const std::string & input) {
	return escapeString(input, "\\\"$");
}

int zenityCommand(DialogType type, const std::string & message,
                  const std::string & dialogTitle) {
	
	const char * options = "";
	switch(type) {
		case DialogInfo:     options = "--info"; break;
		case DialogWarning:  options = "--warning"; break;
		case DialogError:    options = "--error"; break;
		case DialogYesNo:    options = "--question --ok-label=\"Yes\""
		                               " --cancel-label=\"No\""; break;
		case DialogOkCancel: options = "--question --ok-label=\"OK\""
		                               " --cancel-label=\"Cancel\""; break;
	}
	
	boost::format command("zenity %1% --text=\"%2%\" --title=\"%3%\"");
	command = command % options;
	command = command % escape(message);
	command = command % escape(dialogTitle);
	
	return system(command.str().c_str());
}

int kdialogCommand(DialogType type, const std::string & message,
                   const std::string & dialogTitle) {
	
	const char * options = "";
	switch(type) {
		case DialogInfo:     options = "--msgbox"; break;
		case DialogWarning:  options = "--sorry"; break;
		case DialogError:    options = "--error"; break;
		case DialogYesNo:    options = "--warningyesno"; break;
		case DialogOkCancel: options = "--warningcontinuecancel"; break;
	}
	
	boost::format command("kdialog %1% \"%2%\" --title \"%3%\""
	                      " --icon arx-libertatis");
	command = command % options;
	command = command % escape(message);
	command = command % escape(dialogTitle);
	
	return system(command.str().c_str());
}

int xmessageCommand(DialogType type, const std::string & message,
                    const std::string & dialogTitle) {
	
	ARX_UNUSED(dialogTitle);
	
	const char * options = "";
	switch(type) {
		default:             options = "-buttons OK"; break;
		case DialogYesNo:    options = "-buttons Yes:0,No:1"; break;
		case DialogOkCancel: options = "-buttons OK:0,Cancel:1"; break;
	}
	
	boost::format command("xmessage -center %1% \"%2%\"");
	command = command % options;
	command = command % escape(message);
	
	return system(command.str().c_str());
}

bool showDialog(DialogType type, const std::string & message,
                const std::string & dialogTitle) {
	
	typedef int (*dialogCommand_t)(DialogType type, const std::string & message,
	                               const std::string & dialogTitle);
	std::vector<dialogCommand_t> commands;
	
	// This may not be the best way
	const char * session = getenv("DESKTOP_SESSION");
	bool usingKDE = (session != NULL) && !strcasecmp(session, "kde");
	usingKDE = usingKDE || (getenv("KDE_FULL_SESSION") != NULL);
	usingKDE = usingKDE || (getenv("KDE_SESSION_UID") != NULL);
	usingKDE = usingKDE || (getenv("KDE_SESSION_VERSION") != NULL);
	
	if(usingKDE) {
		commands.push_back(&kdialogCommand);
		commands.push_back(&zenityCommand);
	} else {
		commands.push_back(&zenityCommand);
		commands.push_back(&kdialogCommand);
	}
	commands.push_back(&xmessageCommand);
	
	for(std::vector<dialogCommand_t>::const_iterator it = commands.begin();
			it != commands.end(); ++it) {
		int exitCode = (*it)(type, message, dialogTitle);
		if(WIFEXITED(exitCode) && WEXITSTATUS(exitCode) >= 0
		   && WEXITSTATUS(exitCode) < 127) {
			return WEXITSTATUS(exitCode) == 0;
		}
	}
	
	LogWarning << "Failed to show a dialog: [" << dialogTitle << "] " << message;
	return true;
}
#endif
	
void showInfo(const std::string& message, const std::string& dialogTitle) {
	showDialog(DialogInfo, message, dialogTitle);
}

void showWarning(const std::string& message, const std::string& dialogTitle) {
	showDialog(DialogWarning, message, dialogTitle);
}

void showError(const std::string& message, const std::string& dialogTitle) {
	showDialog(DialogError, message, dialogTitle);
}

bool askYesNo(const std::string& question, const std::string& dialogTitle) {
	return showDialog(DialogYesNo, question, dialogTitle);
}

bool askOkCancel(const std::string& question, const std::string& dialogTitle) {
	return showDialog(DialogOkCancel, question, dialogTitle);
}

}
