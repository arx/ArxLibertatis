/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_CRASHINFO_H
#define ARX_PLATFORM_CRASHINFO_H

#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#if ARX_PLATFORM != ARX_PLATFORM_WIN32

#else

#include "platform/Platform.h"

#include <windows.h>
#include <dbghelp.h>

struct CrashInfo {

	CrashInfo() : exitLock(0) {
	}
	
	enum Constants {
		MaxNbFiles = 32,
		MaxFilenameLen = 256,
		MaxNbVariables = 64,
		MaxVariableNameLen = 64,
		MaxVariableValueLen = 128,
		MaxDetailCrashInfoLen = 4096,
		MaxCallstackDepth = 256
	};

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
	boost::interprocess::detail::OS_process_id_t processId;
	boost::interprocess::detail::OS_thread_id_t threadId;

	// Windows specific - necessary for the crash dump generation.
	PEXCEPTION_POINTERS pExceptionPointers;
	MINIDUMP_TYPE miniDumpType;

	// Detailed crash info (messages, registers, whatever).
	char detailedCrashInfo[MaxDetailCrashInfoLen];
	
	// Callstack of the crash.
	u64 callstack[MaxCallstackDepth];

	// Where the crash reports should be written.
	char crashReportFolder[MaxFilenameLen];

	// Once released, this lock will allow the crashed application to terminate.
	boost::interprocess::interprocess_semaphore	exitLock;
};

#endif // #if ARX_PLATFORM != ARX_PLATFORM_WIN32


#endif // ARX_PLATFORM_CRASHINFO_H
