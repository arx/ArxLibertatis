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

#else

#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/uuid/uuid.hpp>

#include "platform/CrashInfo.h"
#include "platform/Lock.h"

/**
 * Handle crashes and collect as much info as possible in order to ease bug fixing.
 * 
 * This class relies on a crash reporter, which is second process that gets started
 * when a crash occurs. The crash reporter has two purpose:
 *    * Collect information on the crashed process in a safer environment.
 *    * Report the crash to the user or developpers (console, network, db, etc.)
 *
 * You can attach files to the crash report using addAttachedFile(). Those will be
 * included in the crash report. Note that some of these files might still be
 * opened when a crash occurs. To handle this case, you can also register callbacks
 * using registerCrashCallback() to be notified of a crash and take the appropriate
 * actions (for example, close a log file).
 *
 * Another nice feature allows you to add variables to the crash report.
 * For example, you might want to set a variable to know the last loaded level, the
 * amount of health, the last spell casted... see setVariable() for more details.
 */
class CrashHandler {

public:
	/// Crash callback type that can be used with registerCrashCallback().
	typedef void (*CrashCallback)(void);

public:
	/**
	 * Constructor.
	 */
	CrashHandler();

	/**
	 * Destructor.
	 */
	~CrashHandler();

	/**
	 * Get the singleton object.
	 * @return The singleton CrashHandler object.
	 */
	static CrashHandler& getInstance();

	/**
	 * Initialize the crash handler.
	 * Will register the necessary platform specific handlers to trap all kind of crash scenarios.
	 * This method can fail for a variety of reasons... See the log for more information.
	 * This method should be called by the main thread of the process.
	 * @return True if initialized correctly, false otherwise.
	 */
	bool init();

	/**
	 * Add a file that will be included in the crash report.
	 * Upon a crash, if this file is not found, it will simply be ignored.
	 * You can attach up to CrashInfo::MaxNbFiles files to the report.
	 * @param file Path to the file (relative to this executable).
	 * @return True if the file could be attached, false otherwise.
	 */
	bool addAttachedFile(const fs::path& file);
	
	/**
	 * Set a variable value, which will be included in the crash report.
	 * You can set up to CrashInfo::MaxNbVariables variables.
	 * If called multiple times with the same name, only the last value will be kept.
	 * @param name Name of the variable
	 * @param value Value of the variable, which will be converted to string.
	 * @return True if the variable could be set, false otherwise.
	 */
	template <class T>
	bool setVariable(const std::string & name, const T & value) {
		std::stringstream ss;
		ss << value;
		return setNamedVariable(name, ss.str());
	}

	/**
	 * Register platform specific crash handlers for the current thread.
	 * Depending on the platform, this call might not be needed.
	 * It's not necessary to call this method for the main thread.
	 * @return True if the crash handlers could be registered, or false otherwise.
	 */
	bool registerThreadCrashHandlers();

	/**
	 * Unregister platform specific crash handlers for the current thread.
	 * Depending on the platform, this call might not be needed.
	 * It's not necessary to call this method for the main thread.
	 */
	void unregisterThreadCrashHandlers();

	/**
	 * Register a callback in order to react in case a crash occurs.
	 * The main use is probably to close opened files that needs to be
	 * attached to the report.
	 * @note In these callbacks, you must make sure to keep processing and 
	 * memory accesses to a minimum as the process is in an unstable state.
	 */
	void registerCrashCallback(CrashCallback crashCallback);

	/**
	 * Unregister a previously registed crash callback.
	 */
	void unregisterCrashCallback(CrashCallback crashCallback);

	/**
	 * Handle a crash and trigger the execution of the crash reporter.
	 * This method is public in order to allow you to trigger it by yourself.
	 * It might be useful for test purpose, or to allow a user to report a
	 * defect which is not a crash.
	 */
	void handleCrash(int crashType, void* crashExtraInfo = 0, int fpeCode = 0);

private:
	bool initSharedMemory();
	void fillBasicCrashInfo();

	bool registerCrashHandlers();
	void unregisterCrashHandlers();
	
	bool setNamedVariable(const std::string& name, const std::string& value);

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

	// Crash callbacks
	std::vector<CrashCallback> m_crashCallbacks;

	static CrashHandler* m_sInstance;
};

#endif // #if ARX_PLATFORM != ARX_PLATFORM_WIN32

#endif // ARX_PLATFORM_CRASHHANDLER_H
