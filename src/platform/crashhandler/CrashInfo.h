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
#include "platform/Process.h"

#pragma pack(push,1)

struct CrashInfoBase {
	
	CrashInfoBase()
		: architecture(0)
		, nbFilesAttached(0)
		, nbVariables(0)
		, processId(0)
		, memoryUsage(0)
		, runningTime(0)
		, crashId(0)
		, signal(0)
		, code(0)
		, hasAddress(0)
		, hasMemory(0)
		, hasStack(0)
		, hasFrame(0)
		, address(0)
		, memory(0)
		, stack(0)
		, frame(0)
		, processorProcessId(0)
		, processorDone(0)
		, reporterStarted(0)
		, exitLock(0)
	{ }
	
	enum Constants {
		MaxNbFiles = 32,
		MaxFilenameLen = 256,
		MaxNbVariables = 64,
		MaxVariableNameLen = 64,
		MaxVariableValueLen = 128,
		MaxDetailCrashInfoLen = 128 * 1024,
		MaxCallstackDepth = 256,
		MaxCrashTitleLen = 256,
	};
	
	char executablePath[MaxFilenameLen];
	char executableVersion[MaxVariableNameLen];

	u32 architecture;
	
	// Files to attach to the report.
	u32 nbFilesAttached;
	char attachedFiles[MaxNbFiles][MaxFilenameLen];
	
	// Variables to add to the report.
	u32 nbVariables;
	struct Variable {
		char name[MaxVariableNameLen];
		char value[MaxVariableValueLen];
	} variables[MaxNbVariables];
	
	// ID of the crashed process & thread
	platform::process_id processId;
	
	u64 memoryUsage;
	double runningTime;
	
	// Where the crash reports should be written.
	char crashReportFolder[MaxFilenameLen];
	
	// On-line crash description
	char title[MaxCrashTitleLen];
	
	// Detailed crash info (messages, registers, whatever).
	char description[MaxDetailCrashInfoLen];
	
	u32 crashId;
	
	int signal;
	int code;
	
	bool hasAddress;
	bool hasMemory;
	bool hasStack;
	bool hasFrame;
	u64 address;
	u64 memory;
	u64 stack;
	u64 frame;
	
	platform::process_id processorProcessId;
	boost::interprocess::interprocess_semaphore processorDone;
	
	boost::interprocess::interprocess_semaphore reporterStarted;
	
	// Once released, this lock will allow the crashed application to terminate.
	boost::interprocess::interprocess_semaphore exitLock;
	
};

#pragma pack(pop)

#if ARX_PLATFORM != ARX_PLATFORM_WIN32

struct CrashInfo : public CrashInfoBase {
	
	void * backtrace[100];
	
	char coreDumpFile[MaxFilenameLen];
	
};

#else

#include <windows.h>

enum CrashType {
	USER_CRASH,
	SEH_EXCEPTION,
	TERMINATE_CALL,
	UNEXPECTED_CALL,
	PURE_CALL,
	NEW_OPERATOR_ERROR,
	INVALID_PARAMETER,
	SIGNAL_SIGABRT,
	SIGNAL_SIGFPE,
	SIGNAL_SIGILL,
	SIGNAL_SIGINT,
	SIGNAL_SIGSEGV,
	SIGNAL_SIGTERM,
	SIGNAL_UNKNOWN
};

#pragma pack(push,1)

struct CrashInfo : public CrashInfoBase {
	
	char contextRecord[1232];
	WCHAR miniDumpTmpFile[MAX_PATH + 64];
	u32 threadId;
	u32 exceptionCode;
	
};

#pragma pack(pop)

#endif

#endif // ARX_PLATFORM_CRASHHANDLER_CRASHINFO_H
