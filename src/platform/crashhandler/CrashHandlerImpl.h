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

#ifndef ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERIMPL_H
#define ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERIMPL_H

// STL
#include <string>
#include <vector>

// BOOST
#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "io/log/Logger.h"
#include "platform/CrashHandler.h"
#include "platform/crashhandler/CrashInfo.h"
#include "platform/Lock.h"

namespace fs { class path; }

class CrashHandlerImpl {
public:
	CrashHandlerImpl();
	virtual ~CrashHandlerImpl();

	bool initialize();
	void shutdown();

	bool addAttachedFile(const fs::path& file);
	bool setNamedVariable(const std::string& name, const std::string& value);

	bool setReportLocation(const fs::path& location);
	bool deleteOldReports(size_t nbReportsToKeep);

	void registerCrashCallback(CrashHandler::CrashCallback crashCallback);
	void unregisterCrashCallback(CrashHandler::CrashCallback crashCallback);

	virtual bool registerThreadCrashHandlers() = 0;
	virtual void unregisterThreadCrashHandlers() = 0;

private:
	virtual bool registerCrashHandlers() = 0;
	virtual void unregisterCrashHandlers() = 0;
	
	bool createSharedMemory();
	void destroySharedMemory();
	
protected:
	virtual void fillBasicCrashInfo();

	std::string m_CrashHandlerApp;

	// Memory shared to the crash reporter.
	boost::interprocess::shared_memory_object m_SharedMemory;
	boost::interprocess::mapped_region m_MemoryMappedRegion;
	
	// Name of the shared memory.
	std::string m_SharedMemoryName;

	// Crash information (pointer to shared memory)
	CrashInfo* m_pCrashInfo;

	// Protect against multiple accesses.
	Lock m_Lock;

	// Crash callbacks
	std::vector<CrashHandler::CrashCallback> m_crashCallbacks;
};

#endif // ARX_PLATFORM_CRASHHANDLER_CRASHHANDLERIMPL_H
