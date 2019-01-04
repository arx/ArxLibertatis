/*
 * Copyright 2015-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include <math.h>

#include "platform/Architecture.h"

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
		
		// Work around a bug in Visual Studio 2013's math library
		// https://support.microsoft.com/en-us/help/3174417/fix-programs-that-are-built-in-visual-c-2013-crash-with-illegal-instru
		#if defined(_MSC_VER) && _MSC_VER >= 1800 && _MSC_VER < 1900 && ARX_ARCH == ARX_ARCH_X86_64
		bool hasAVX = false;
		HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
		if(kernel32) {
			typedef DWORD64 (WINAPI * PGETENABLEDXSTATEFEATURES)();
			PGETENABLEDXSTATEFEATURES GetEnabledXStateFeatures
				= (PGETENABLEDXSTATEFEATURES)GetProcAddress(kernel32, "GetEnabledXStateFeatures");
			if(GetEnabledXStateFeatures) {
				DWORD64 features = GetEnabledXStateFeatures();
				if(features & XSTATE_MASK_GSSE) {
					hasAVX = true;
				}
			}
		}
		if(!hasAVX) {
			_set_FMA3_enable(0);
		}
		#endif
		
	}
	
	~WindowsMain() {
		
		for(int i = 0; i < argc; i++) {
			delete[] argv[i];
		}
		
		delete argv;
		
	}
	
};

// Disable telemetry added in Visual Studio 2015
#if defined(_MSC_VER) && _MSC_VER >= 1900
extern "C" {
	void _cdecl __vcrt_initialize_telemetry_provider() { }
	void _cdecl __telemetry_main_invoke_trigger() { }
	void _cdecl __telemetry_main_return_trigger() { }
	void _cdecl __vcrt_uninitialize_telemetry_provider() { }
};
#endif

#endif // ARX_PLATFORM_WINDOWSMAINCOMMON_H
