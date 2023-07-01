/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERIMPL_H
#define ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERIMPL_H

#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_BSD || ARX_PLATFORM == ARX_PLATFORM_MACOS
// Some versions of boost/interprocess/shared_memory_object.hpp are
// missing includes needed for BSD-kernel-specific code:
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>

#include "io/fs/FilePath.h"
#include "platform/CrashHandler.h"
#include "platform/crashhandler/CrashInfo.h"

class CrashHandlerImpl {
	
public:
	
	CrashHandlerImpl();
	virtual ~CrashHandlerImpl() = default;
	
	virtual bool initialize();
	virtual void shutdown();
	
	bool addAttachedFile(const fs::path & file);
	bool setVariable(std::string_view name, std::string_view value);
	void setWindow(u64 window);
	
	bool addText(std::string_view text);
	
	bool setReportLocation(const fs::path & location);
	bool deleteOldReports(size_t nbReportsToKeep);
	
	void registerCrashCallback(CrashHandler::CrashCallback crashCallback);
	void unregisterCrashCallback(CrashHandler::CrashCallback crashCallback);
	
	virtual bool registerThreadCrashHandlers() = 0;
	virtual void unregisterThreadCrashHandlers() = 0;
	
	void processCrash(std::string_view sharedMemoryName);
	
private:
	
	virtual bool registerCrashHandlers() = 0;
	virtual void unregisterCrashHandlers() = 0;
	
	bool createSharedMemory();
	void destroySharedMemory();
	void fillBasicCrashInfo();
	
	virtual void processCrashInfo() { }
	virtual void processCrashSignal() { }
	void processCrashRegisters();
	virtual void processCrashTrace() { }
	virtual void processCrashDump() { }
	
protected:
	
	void processCrash();
	
	fs::path m_executable;
	
	fs::path m_crashReportDir;
	
	// Memory shared to the crash reporter.
	boost::interprocess::shared_memory_object m_SharedMemory;
	boost::interprocess::mapped_region m_MemoryMappedRegion;
	
	// Name of the shared memory.
	std::string m_SharedMemoryName;
	
	// Crash information (pointer to shared memory)
	CrashInfo * m_pCrashInfo;
	size_t m_textLength;
	
	// Protect against multiple accesses.
	std::mutex m_mutex;
	
	// Crash callbacks
	std::vector<CrashHandler::CrashCallback> m_crashCallbacks;
	
};

#endif // ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERIMPL_H
