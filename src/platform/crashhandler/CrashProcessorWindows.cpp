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

#include "platform/crashhandler/CrashHandlerWindows.h"

#include <cfloat>
#include <sstream>

#include <windows.h>
#include <psapi.h>


static u64 convertFileTimeToInteger(const FILETIME & ft) {
	ULARGE_INTEGER integer;
	integer.LowPart = ft.dwLowDateTime;
	integer.HighPart = ft.dwHighDateTime;
	return integer.QuadPart ;
}

void CrashHandlerWindows::processCrashInfo() {
	
	// Required for XP - for Vista+, PROCESS_QUERY_LIMITED_INFORMATION would be enough
	DWORD access = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ;
	HANDLE process = OpenProcess(access, FALSE, m_pCrashInfo->processId);
	if(!process) {
		return;
	}
	
	// Get memory usage info
	PROCESS_MEMORY_COUNTERS meminfo;
	if(GetProcessMemoryInfo(process, &meminfo, sizeof(meminfo))) {
		m_pCrashInfo->memoryUsage = meminfo.WorkingSetSize;
	}
	
	// Determine how long thre process was running
	FILETIME creation, exit, kernel, user, now;
	if(GetProcessTimes(process, &creation, &exit, &kernel, &user)) {
		SYSTEMTIME time;
		GetSystemTime(&time);
		SystemTimeToFileTime(&time, &now) ;
		u64 delta = convertFileTimeToInteger(now) - convertFileTimeToInteger(creation);
		m_pCrashInfo->runningTime = double(delta) * 10e-08;
	}
	
	CloseHandle(process);
	
}

void CrashHandlerWindows::processCrashSignal() {
	
	std::ostringstream description;
	
	switch(m_pCrashInfo->signal) {
		case USER_CRASH:         description << "Aborted"; break;
		case SEH_EXCEPTION:      description << "Unhandled exception"; break;
		case TERMINATE_CALL:     description << "terminate() was called"; break;
		case UNEXPECTED_CALL:    description << "unexpected() was called"; break;
		case PURE_CALL:          description << "Pure virtual function called"; break;
		case NEW_OPERATOR_ERROR: description << "new operator failed"; break;
		case INVALID_PARAMETER:  description << "Invalid parameter detected"; break;
		case SIGNAL_SIGABRT:     description << "Abnormal termination"; break;
		case SIGNAL_SIGFPE:      description << "Floating-point error"; break;
		case SIGNAL_SIGILL:      description << "Illegal instruction"; break;
		case SIGNAL_SIGINT:      description << "CTRL+C signal"; break;
		case SIGNAL_SIGSEGV:     description << "Illegal storage access"; break;
		case SIGNAL_SIGTERM:     description << "Termination request"; break;
		case SIGNAL_UNKNOWN:     description << "Unknown signal"; break;
		default:                 description << "Unknown error"; break;
	}
	
	// Append detailed information in case of a floating point exception
	if(m_pCrashInfo->signal == SIGNAL_SIGFPE) {
		switch(m_pCrashInfo->code) {
			case _FPE_INVALID:         description << ": Invalid result"; break;
			case _FPE_DENORMAL:        description << ": Denormal operand"; break;
			case _FPE_ZERODIVIDE:      description << ": Divide by zero"; break;
			case _FPE_OVERFLOW:        description << ": Overflow"; break;
			case _FPE_UNDERFLOW:       description << ": Underflow"; break;
			case _FPE_INEXACT:         description << ": Inexact precision"; break;
			case _FPE_UNEMULATED:      description << ": Unemulated"; break;
			case _FPE_SQRTNEG:         description << ": Negative square root"; break;
			case _FPE_STACKOVERFLOW:   description << ": Stack Overflow"; break;
			case _FPE_STACKUNDERFLOW:  description << ": Stack Underflow"; break;
			case _FPE_EXPLICITGEN:     description << ": raise( SIGFPE ) was called"; break;
			#ifdef _FPE_MULTIPLE_TRAPS // Not available on all VC++ versions
			case _FPE_MULTIPLE_TRAPS:  description << ": Multiple traps"; break;
			#endif
			#ifdef _FPE_MULTIPLE_FAULTS // Not available on all VC++ versions
			case _FPE_MULTIPLE_FAULTS: description << ": Multiple faults"; break;
			#endif
			default:                   break;
		}
	}
	
	description << "\n\n";
	
	addText(description.str().c_str());
}
