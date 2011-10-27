
#ifndef ARX_PLATFORM_CRASHHANDLER_H
#define ARX_PLATFORM_CRASHHANDLER_H

#include <string>

#if ARX_PLATFORM != ARX_PLATFORM_WIN32

void initCrashHandler();

#else

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
	void addVariable(const std::string& name, const T& value) {
		std::stringstream ss;
		ss << i;
		addNamedVariable(ss.str());
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
