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

#ifndef ARX_PLATFORM_CRASHHANDLER_H
#define ARX_PLATFORM_CRASHHANDLER_H

#include <string>
#include "platform/Platform.h"

#if ARX_PLATFORM != ARX_PLATFORM_WIN32

void initCrashHandler();

#elif 0 // TODO

#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/uuid/uuid.hpp>

#include "platform/CrashInfo.h"
#include "platform/Lock.h"

class CrashHandler {

public:
	CrashHandler();
	~CrashHandler();

	static CrashHandler& getInstance();

	// Initialize the crash handler.
	bool init();

	// Add a file to attach to the crash report.
	bool addAttachedFile(const std::string& filename);
	
	// Add a variable to the crash report.
	template <class T>
	void addVariable(const std::string & name, const T & value) {
		std::stringstream ss;
		ss << value;
		addNamedVariable(name, ss.str());
	}

	bool registerThreadCrashHandlers();
	void unregisterThreadCrashHandlers();

	void handleCrash(int crashType, void* crashExtraInfo = 0, int fpeCode = 0);

private:
	bool initSharedMemory();
	void fillBasicCrashInfo();

	bool registerCrashHandlers();
	void unregisterCrashHandlers();
	
	bool addNamedVariable(const std::string& name, const std::string& value);

	void shutdown();

private:
	// Memory shared to the crash reporter.
	boost::interprocess::shared_memory_object m_SharedMemory;
	boost::interprocess::mapped_region m_MemoryMappedRegion;
	
	// Name of the shared memory.
	std::string m_SharedMemoryName;

	// Crash information (pointer to shared memory)
	CrashInfo* m_pCrashInfo;

	// Crash handlers to restore.
	struct PlatformCrashHandlers* m_pPreviousCrashHandlers;

	// Protect against multiple accesses.
	Lock m_Lock;

	static CrashHandler* m_sInstance;
};

#endif // #if ARX_PLATFORM != ARX_PLATFORM_WIN32

#endif // ARX_PLATFORM_CRASHHANDLER_H
