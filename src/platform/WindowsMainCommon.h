/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_WINDOWSMAINCOMMON_H
#define ARX_PLATFORM_WINDOWSMAINCOMMON_H

#include <clocale>

#include <windows.h>
#include <shellapi.h>

class WindowsMain {
	
public:
	
	int argc;
	char ** argv;
	
	WindowsMain() {
		
		// Convert the UTF-16 command-line parameters to UTF-8
		
		wchar_t ** wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
		
		argv = new char *[argc + 1];
		
		for(int i = 0; i < argc; i++) {
			int n = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, NULL, 0,  NULL, NULL);
			argv[i] = new char[n];
			WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, argv[i], n, NULL, NULL);
		}
		
		argv[argc] = NULL;
		
	}
	
	~WindowsMain() {
		
		for(int i = 0; i < argc; i++) {
			delete[] argv[i];
		}
		
		delete argv;
		
	}
	
};

#endif // ARX_PLATFORM_WINDOWSMAINCOMMON_H
