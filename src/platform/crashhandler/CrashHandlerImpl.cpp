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

#include "platform/crashhandler/CrashHandlerImpl.h"

#include <iostream>
#include <sstream>

#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/date_time/time_duration.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include "core/Version.h"

#include "io/fs/Filesystem.h"
#include "io/fs/FilePath.h"
#include "io/log/Logger.h"

#include "math/Random.h"

#include "platform/Architecture.h"
#include "platform/Environment.h"
#include "platform/Process.h"

#include "util/String.h"

namespace bip = boost::interprocess;

CrashHandlerImpl::CrashHandlerImpl()
	: m_pCrashInfo(0) {
}

CrashHandlerImpl::~CrashHandlerImpl() {
}

void CrashHandlerImpl::processCrash(const std::string & sharedMemoryName) {
	
	m_SharedMemoryName = sharedMemoryName;
	
	// Create a shared memory object.
	m_SharedMemory = bip::shared_memory_object(bip::open_only, m_SharedMemoryName.c_str(),
	                                           bip::read_write);
	
	// Map the whole shared memory in this process
	m_MemoryMappedRegion = bip::mapped_region(m_SharedMemory, bip::read_write);
	
	// Our SharedCrashInfo will be stored in this shared memory.
	m_pCrashInfo = reinterpret_cast<CrashInfo *>(m_MemoryMappedRegion.get_address());
	
	processCrash();
	
	std::exit(0);
}

void CrashHandlerImpl::processCrash() {
	
	if(m_pCrashInfo) {
		m_pCrashInfo->processorProcessId = platform::getProcessId();
	}
	
	// Launch crash reporter GUI
	platform::process_handle reporter = 0;
	{
		fs::path executable = platform::getHelperExecutable("arxcrashreporter");
		char argument[256];
		strcpy(argument, "--crashinfo=");
		strcat(argument, m_SharedMemoryName.c_str());
		const char * args[] = { executable.string().c_str(), argument, NULL };
		reporter = platform::runAsync(args);
	}
	
	// Wait for the crash reporter to start
	while(reporter) {
		if(platform::getProcessExitCode(reporter, false) != platform::StillRunning) {
			reporter = 0;
			break;
		}
		boost::posix_time::ptime timeout
		 = boost::posix_time::microsec_clock::universal_time()
		 + boost::posix_time::milliseconds(100);
		if(m_pCrashInfo->reporterStarted.timed_wait(timeout)) {
			break;
		}
	}
	
	// Tell the crash reporter we are done
	if(m_pCrashInfo) {
		m_pCrashInfo->processorDone.post();
	}
	
	// If the crash reporter started successfully, we are done here
	if(reporter) {
		// TODO The reporter still needs the original process
		(void)platform::getProcessExitCode(reporter);
		return;
	}
	
	// Something went wrong - the crash reporter failed to start!
	
	std::cerr << "Arx Libertatis crashed, but arxcrashreporter could not be run.\n";
	std::cerr << "arx.log might contain more information.\n";
	std::cerr << "Please install arxcrashreporter to generate a detailed bug report!\n";
	std::cerr.flush();
	
	// Terminate the crashed process
	if(m_pCrashInfo) {
		m_pCrashInfo->exitLock.post();
		platform::killProcess(m_pCrashInfo->processId);
	}
	
}

bool CrashHandlerImpl::initialize() {
	Autolock autoLock(&m_Lock);
	
	bool initialized = true;
	
	m_executable = platform::getExecutablePath();
	
	if(!createSharedMemory()) {
		return false;
	}
	
	fillBasicCrashInfo();
	
	if(!registerCrashHandlers()) {
		destroySharedMemory();
		initialized = false;
	}

	return initialized;
}

void CrashHandlerImpl::shutdown() {
	Autolock autoLock(&m_Lock);
	
	unregisterCrashHandlers();
	destroySharedMemory();
}

bool CrashHandlerImpl::createSharedMemory() {
	
	try {
		
		// Generate a random name for our shared memory object
		std::ostringstream oss;
		oss << "arxcrash-" << platform::getProcessId() << "-" << Random::get<u32>();
		m_SharedMemoryName = oss.str();
		
		// Create a shared memory object.
		m_SharedMemory = boost::interprocess::shared_memory_object(boost::interprocess::create_only, m_SharedMemoryName.c_str(), boost::interprocess::read_write);
		
		// Resize to fit the CrashInfo structure
		m_SharedMemory.truncate(sizeof(CrashInfo));
		
		// Map the whole shared memory in this process
		m_MemoryMappedRegion = boost::interprocess::mapped_region(m_SharedMemory, boost::interprocess::read_write);
		
		// Our CrashInfo will be stored in this shared memory.
		m_pCrashInfo = new (m_MemoryMappedRegion.get_address()) CrashInfo;
		
	} catch(boost::interprocess::interprocess_exception) {
		return false;
	}
	
	return true;
}

void CrashHandlerImpl::destroySharedMemory() {
	m_MemoryMappedRegion = boost::interprocess::mapped_region();
	m_SharedMemory = boost::interprocess::shared_memory_object();
	m_pCrashInfo = 0;
}

void CrashHandlerImpl::fillBasicCrashInfo() {
	
	m_pCrashInfo->architecture = ARX_ARCH;
	m_pCrashInfo->processId = platform::getProcessId();

	strcpy(m_pCrashInfo->crashReportFolder, "crashes");
	
	std::string exe = platform::getExecutablePath().string();
	strncpy(m_pCrashInfo->executablePath, exe.c_str(), sizeof(m_pCrashInfo->executablePath));
	m_pCrashInfo->executablePath[sizeof(m_pCrashInfo->executablePath)-1] = 0; // Make sure our string is null terminated

	strncpy(m_pCrashInfo->executableVersion, arx_version.c_str(), sizeof(m_pCrashInfo->executableVersion));
	m_pCrashInfo->executableVersion[sizeof(m_pCrashInfo->executableVersion)-1] = 0; // Make sure our string is null terminated
	
	m_pCrashInfo->processorProcessId = 0;
	
}

bool CrashHandlerImpl::addAttachedFile(const fs::path & file) {
	
	if(file.is_relative()) {
		fs::path absolute = fs::current_path() / file;
		if(absolute.is_relative()) {
			return false;
		}
		return addAttachedFile(absolute);
	}
	
	Autolock autoLock(&m_Lock);

	if(m_pCrashInfo->nbFilesAttached == CrashInfo::MaxNbFiles) {
		LogError << "Too much files already attached to the crash report (" << m_pCrashInfo->nbFilesAttached << ").";
		return false;
	}

	if(file.string().size() >= CrashInfo::MaxFilenameLen) {
		LogError << "File name is too long.";
		return false;
	}

	for(u32 i = 0; i < m_pCrashInfo->nbFilesAttached; i++) {
		if(strcmp(m_pCrashInfo->attachedFiles[i], file.string().c_str()) == 0) {
			LogWarning << "File \"" << file << "\" is already attached.";
			return false;
		}
	}
	
	util::storeStringTerminated(m_pCrashInfo->attachedFiles[m_pCrashInfo->nbFilesAttached], file.string());
	m_pCrashInfo->nbFilesAttached++;

	return true;
}

bool CrashHandlerImpl::setNamedVariable(const std::string& name, const std::string& value) {
	Autolock autoLock(&m_Lock);

	if(name.size() >= CrashInfo::MaxVariableNameLen) {
		LogError << "Variable name is too long.";
		return false;
	}

	if(value.size() >= CrashInfo::MaxVariableValueLen) {
		LogError << "Variable description is too long.";
		return false;
	}

	// Check if our array already contains this variable.
	for(u32 i = 0; i < m_pCrashInfo->nbVariables; i++) {
		if(strcmp(m_pCrashInfo->variables[i].name, name.c_str()) == 0) {
			util::storeStringTerminated(m_pCrashInfo->variables[i].value, value);
			return true;
		}
	}

	// Not found, must add a new one.
	if(m_pCrashInfo->nbVariables == CrashInfo::MaxNbVariables) {
		LogError << "Too much variables already added to the crash report (" << m_pCrashInfo->nbVariables << ").";
		return false;
	}

	util::storeStringTerminated(m_pCrashInfo->variables[m_pCrashInfo->nbVariables].name, name);
	util::storeStringTerminated(m_pCrashInfo->variables[m_pCrashInfo->nbVariables].value, value);
	m_pCrashInfo->nbVariables++;

	return true;
}

bool CrashHandlerImpl::setReportLocation(const fs::path & location) {
	
	if(location.is_relative()) {
		fs::path absolute = fs::current_path() / location;
		if(absolute.is_relative()) {
			return false;
		}
		return setReportLocation(absolute);
	}
	
	Autolock autoLock(&m_Lock);

	if(location.string().size() >= CrashInfo::MaxFilenameLen) {
		LogError << "Report location path is too long.";
		return false;
	}
	
	util::storeStringTerminated(m_pCrashInfo->crashReportFolder, location.string());
	
	return true;
}

bool CrashHandlerImpl::deleteOldReports(size_t nbReportsToKeep)
{
	Autolock autoLock(&m_Lock);

    if(strlen(m_pCrashInfo->crashReportFolder) == 0) {
		LogError << "Report location has not been specified";
		return false;
	}
	
	// Exit if there is no crash report folder yet...
	fs::path location(m_pCrashInfo->crashReportFolder);
	if(!fs::is_directory(location))
		return true;

	typedef std::map<std::string, fs::path> CrashReportMap;
	CrashReportMap oldCrashes;
	
	for(fs::directory_iterator it(location); !it.end(); ++it) {
		fs::path folderPath = location / it.name();
		if(fs::is_directory(folderPath)) {
			// Ensure this directory really contains a crash report...
			fs::path crashManifestPath = folderPath / "crash.xml";
			if(fs::is_regular_file(crashManifestPath)) {
				oldCrashes[it.name()] = folderPath;
			}
		}
	}

	// Nothing to delete
	if(nbReportsToKeep >= oldCrashes.size())
		return true;

	int nbReportsToDelete = oldCrashes.size() - nbReportsToKeep; 

	// std::map will return the oldest reports first as folders are named "yyyy.MM.dd hh.mm.ss"
	CrashReportMap::const_iterator it = oldCrashes.begin();
	for(int i = 0; i < nbReportsToDelete; ++i, ++it) {
		fs::remove_all(it->second);
	}
	
	return true;
}

void CrashHandlerImpl::registerCrashCallback(CrashHandler::CrashCallback crashCallback) {
	Autolock autoLock(&m_Lock);

	m_crashCallbacks.push_back(crashCallback);
}

void CrashHandlerImpl::unregisterCrashCallback(CrashHandler::CrashCallback crashCallback) {
	Autolock autoLock(&m_Lock);

	m_crashCallbacks.erase(std::remove(m_crashCallbacks.begin(),
	                                   m_crashCallbacks.end(),
	                                   crashCallback),
	                       m_crashCallbacks.end());
}
