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

#ifndef ARX_PLATFORM_CRASHHANDLER_CRASHINFO_H
#define ARX_PLATFORM_CRASHHANDLER_CRASHINFO_H

#include "Configure.h"

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>

#include "platform/Platform.h"
#include "platform/Thread.h"

struct CrashInfoBase {
	
	enum Constants {
		MaxNbFiles = 32,
		MaxFilenameLen = 256,
		MaxNbVariables = 64,
		MaxVariableNameLen = 64,
		MaxVariableValueLen = 128,
		MaxDetailCrashInfoLen = 4096,
		MaxCallstackDepth = 256
	};
	
	char executablePath[MaxFilenameLen];
	char executableVersion[MaxVariableNameLen];

	u32 architecture;
	
	// Files to attach to the report.
	int	 nbFilesAttached;
	char attachedFiles[MaxNbFiles][MaxFilenameLen];
	
	// Variables to add to the report.
	int nbVariables;
	struct Variable {
		char name[MaxVariableNameLen];
		char value[MaxVariableValueLen];
	} variables[MaxNbVariables];
	
	// ID of the crashed process & thread
	process_id_type processId;
	
	// Where the crash reports should be written.
	char crashReportFolder[MaxFilenameLen];
	
};


#if ARX_PLATFORM != ARX_PLATFORM_WIN32

struct CrashInfo : public CrashInfoBase {
	
	CrashInfo()
		: signal(0)
		, code(0)
		, backtrace()
	{}
	
	int signal;
	int code;
	
	void * backtrace[100];
};

#else

#include <windows.h>
#include <dbghelp.h>

struct CrashInfo : public CrashInfoBase {
	
	CrashInfo() : exitLock(0) { }
	
	// Detailed crash info (messages, registers, whatever).
	char detailedCrashInfo[MaxDetailCrashInfoLen];
	
	CONTEXT contextRecord;
	CHAR	miniDumpTmpFile[MAX_PATH];
	HANDLE  threadHandle;
	DWORD	exceptionCode;
	
	// Once released, this lock will allow the crashed application to terminate.
	boost::interprocess::interprocess_semaphore	exitLock;
	
};

#endif

#endif // ARX_PLATFORM_CRASHHANDLER_CRASHINFO_H
