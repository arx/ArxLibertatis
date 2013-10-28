/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#include <iostream>

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
#include <boost/foreach.hpp>

#include "util/String.h"


namespace dialog {

enum DialogType {
	DialogInfo,
	DialogWarning,
	DialogError,
	DialogYesNo,
	DialogOkCancel
};

#if ARX_PLATFORM == ARX_PLATFORM_WIN32

static bool showDialog(DialogType type, const std::string & message,
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

// See Dialog.mm for the implementation of showDialog
bool showDialog(DialogType type, const std::string & message,
                const std::string & dialogTitle);

#else

static std::string escape(const std::string & input) {
	return util::escapeString(input, "\\\"$");
}

static bool isAllowedInUrl(char c) {
	return !isspace(c) && c != '"' && c != '\'' && c != ')';
}

static void closeLink(std::stringstream & oss, size_t start) {
	size_t end = oss.tellp();
	std::vector<char> url(end - start);
	oss.seekg(start).read(&url.front(), end - start);
	oss << "\">";
	oss.write(&url.front(), end - start);
	oss << "</a>";
}

/*!
 * Minimal HTML formatter for error messages
 *
 * Features:
 * ' * ' => html link or nicer bullet point
 * 'http://' / 'https://' => link
 * "..." => "<b>...</b>"
 *
 * @param newline Keep don't convert newlines to &lt;br&gr; tags.
 * @param ul      Use HTML lists.
 */
static std::string formatAsHtml(const std::string & text, bool newline, bool ul = false) {
	
	std::stringstream oss;
	std::istringstream iss(text);
	
	bool list = false, first = true;
	
	std::string line;
	while(!std::getline(iss, line).fail()) {
		
		size_t i = 0;
		
		if(line.length() >= 3 && line.compare(0, 3, " * ", 3) == 0) {
			i += 3;
			
			if(ul && !list) {
				oss << "<ul>";
				list = true;
			} else if(!ul && !first) {
				oss << (newline ? "\n" : "<br>");
			}
			
			oss << (ul ? "<li>" : " &#8226; ");
			
		} else if(list) {
			oss << "</ul>";
			list = false;
		} else if(!first) {
			oss << (newline ? "\n" : "<br>");
		}
		first = false;
		
		bool quote = false, link = false;
		
		size_t link_start;
		
		for(; i < line.length(); i++) {
			
			if(link && !isAllowedInUrl(line[i])) {
				closeLink(oss, link_start);
				link = false;
			}
			
			if(line[i] == '<') {
				oss << "&lt;";
			} else if(line[i] == '>') {
				oss << "&gt;";
			} else if(line[i] == '"') {
				if(!quote) {
					oss << "\"<b>";
				} else {
					oss << "</b>\"";
				}
				quote = !quote;
			} else if(!link && line.compare(i, 7, "http://", 7) == 0) {
				oss << "<a href=\"";
				link_start = oss.tellp(), link = true;
				oss << "http://";
				i += 6;
			} else if(!link && line.compare(i, 8, "https://", 8) == 0) {
				oss << "<a href=";
				link_start = oss.tellp(), link = true;
				oss << "https://";
				i += 7;
			} else {
				oss << line[i];
			}
			
		}
		
		if(link) {
			closeLink(oss, link_start);
		}
		
		if(quote) {
			oss << "</code>";
		}
	}
	
	return oss.str();
}

static int zenityCommand(DialogType type, const std::string & message,
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
	
	boost::format command("zenity %1% --no-wrap --text=\"%2%\" --title=\"%3%\"");
	command = command % options;
	command = command % escape(formatAsHtml(message, true));
	command = command % escape(dialogTitle);
	
	return system(command.str().c_str());
}

static int kdialogCommand(DialogType type, const std::string & message,
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
	command = command % escape(formatAsHtml(message, false));
	command = command % escape(dialogTitle);
	
	return system(command.str().c_str());
}

static int gxmessageCommand(DialogType type, const std::string & message,
                            const std::string & dialogTitle) {
	
	const char * options = "";
	switch(type) {
		default:             options = "-buttons OK"; break;
		case DialogYesNo:    options = "-buttons Yes:0,No:1"; break;
		case DialogOkCancel: options = "-buttons OK:0,Cancel:1"; break;
	}
	
	boost::format command("gxmessage -center %1% -title \"%2%\" \"%3%\"");
	command = command % options;
	command = command % escape(dialogTitle);
	command = command % escape(message);
	
	return system(command.str().c_str());
}

static int xdialogCommand(DialogType type, const std::string & message,
                          const std::string & dialogTitle) {
	
	const char * options = "";
	switch(type) {
		default:             options = "--msgbox"; break;
		case DialogYesNo:    options = "--yesno"; break;
		case DialogOkCancel: options = "--ok-label OK --cancel-label Cancel --yesno"; break;
	}
	
	boost::format command("Xdialog --left --title \"%3%\""
	                      " %1% \"%2%\" 0 0");
	command = command % options;
	command = command % escape(message);
	command = command % escape(dialogTitle);
	
	return system(command.str().c_str());
}

static int xmessageCommand(DialogType type, const std::string & message,
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

static bool showDialog(DialogType type, const std::string & message,
                       const std::string & dialogTitle) {
	
	typedef int (*dialogCommand_t)(DialogType type, const std::string & message,
	                               const std::string & dialogTitle);
	
	// This may not be the best way
	const char * session = getenv("DESKTOP_SESSION");
	bool usingKDE = (session != NULL) && !strcasecmp(session, "kde");
	usingKDE = usingKDE || (getenv("KDE_FULL_SESSION") != NULL);
	usingKDE = usingKDE || (getenv("KDE_SESSION_UID") != NULL);
	usingKDE = usingKDE || (getenv("KDE_SESSION_VERSION") != NULL);
	
	dialogCommand_t commands[] = {
		usingKDE ? &kdialogCommand : &zenityCommand,
		usingKDE ? &zenityCommand : &kdialogCommand,
		&gxmessageCommand,
		&xdialogCommand,
		&xmessageCommand
	};
	
	BOOST_FOREACH(dialogCommand_t command, commands) {
		int exitCode = command(type, message, dialogTitle);
		if(WIFEXITED(exitCode) && WEXITSTATUS(exitCode) >= 0
		   && WEXITSTATUS(exitCode) < 127) {
			return WEXITSTATUS(exitCode) == 0;
		}
	}
	
	std::cerr << "Failed to show a dialog: " << dialogTitle << ": " << message << std::endl;
	return true;
}
#endif

void showInfo(const std::string & message, const std::string & dialogTitle) {
	showDialog(DialogInfo, message, dialogTitle);
}

void showWarning(const std::string & message, const std::string & dialogTitle) {
	showDialog(DialogWarning, message, dialogTitle);
}

void showError(const std::string & message, const std::string & dialogTitle) {
	showDialog(DialogError, message, dialogTitle);
}

bool askYesNo(const std::string & question, const std::string & dialogTitle) {
	return showDialog(DialogYesNo, question, dialogTitle);
}

bool askOkCancel(const std::string & question, const std::string & dialogTitle) {
	return showDialog(DialogOkCancel, question, dialogTitle);
}

}
