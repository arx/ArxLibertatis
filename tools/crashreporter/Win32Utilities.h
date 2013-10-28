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

#ifndef ARX_TOOLS_CRASHREPORTER_WIN32UTILITIES_H
#define ARX_TOOLS_CRASHREPORTER_WIN32UTILITIES_H

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32

#include <windows.h>
#include <string>

ULONG64 ConvertSystemTimeToULONG64( const SYSTEMTIME& st );

bool GetCallStackInfo(HANDLE hProcess, HANDLE hThread, PCONTEXT pContext, std::string& callstack, std::string& callstackTop, u32& callstackCrc);
std::string GetRegisters(PCONTEXT pCtx);
std::string GetExceptionString(DWORD dwCode);

#endif // ARX_PLATFORM == ARX_PLATFORM_WIN32

#endif // ARX_TOOLS_CRASHREPORTER_WIN32UTILITIES_H
