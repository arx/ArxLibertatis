/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <algorithm>
#include <sstream>
#include <cstring>
#include <ctime>

#include <boost/range/size.hpp>

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
	: m_pCrashInfo(0)
	, m_textLength(0)
{ }

CrashHandlerImpl::~CrashHandlerImpl() {
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
		m_SharedMemory = bip::shared_memory_object(bip::create_only, m_SharedMemoryName.c_str(), bip::read_write);
		
		// Resize to fit the CrashInfo structure
		m_SharedMemory.truncate(sizeof(CrashInfo));
		
		// Map the whole shared memory in this process
		m_MemoryMappedRegion = bip::mapped_region(m_SharedMemory, bip::read_write);
		
		// Our CrashInfo will be stored in this shared memory.
		m_pCrashInfo = new(m_MemoryMappedRegion.get_address()) CrashInfo;
		m_textLength = 0;
		
	} catch(const bip::interprocess_exception &) {
		return false;
	}
	
	return true;
}

void CrashHandlerImpl::destroySharedMemory() {
	m_MemoryMappedRegion = bip::mapped_region();
	m_SharedMemory = bip::shared_memory_object();
	m_pCrashInfo = 0;
}

void CrashHandlerImpl::fillBasicCrashInfo() {
	
	m_pCrashInfo->architecture = ARX_ARCH;
	m_pCrashInfo->processId = platform::getProcessId();
	m_pCrashInfo->memoryUsage = 0;
	m_pCrashInfo->runningTime = 0.0;

	util::storeStringTerminated(m_pCrashInfo->crashReportFolder, "crashes");
	
	std::string exe = platform::getExecutablePath().string();
	util::storeStringTerminated(m_pCrashInfo->executablePath, exe);
	util::storeStringTerminated(m_pCrashInfo->executableVersion, arx_name + " " + arx_version);
	
	m_pCrashInfo->window = 0;
	
	m_pCrashInfo->title[0] = '\0';
	
	m_pCrashInfo->description[0] = '\0';
	
	m_pCrashInfo->crashId = 0;
	
	m_pCrashInfo->signal = 0;
	m_pCrashInfo->code = 0;
	
	m_pCrashInfo->hasAddress = false;
	m_pCrashInfo->address = 0;
	m_pCrashInfo->hasMemory = false;
	m_pCrashInfo->memory = 0;
	m_pCrashInfo->hasStack = false;
	m_pCrashInfo->stack = 0;
	m_pCrashInfo->hasFrame = false;
	m_pCrashInfo->frame = 0;
	
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

bool CrashHandlerImpl::setVariable(const std::string & name, const std::string & value) {
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

void CrashHandlerImpl::setWindow(u64 window) {
	Autolock autoLock(&m_Lock);
	
	m_pCrashInfo->window = window;
}

bool CrashHandlerImpl::addText(const char * text) {
	
	Autolock autoLock(&m_Lock);
	
	if(!m_pCrashInfo) {
		return false;
	}
	
	size_t length = std::strlen(text);
	size_t remaining = boost::size(m_pCrashInfo->description) - m_textLength - 1;
	
	size_t n = std::min(length, remaining);
	std::memcpy(&m_pCrashInfo->description[m_textLength], text, n);
	m_textLength += n;
	
	m_pCrashInfo->description[m_textLength] = '\0';
	
	return n == length;
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
	
	fs::create_directories(location);
	
	util::storeStringTerminated(m_pCrashInfo->crashReportFolder, location.string());
	
	return true;
}

bool CrashHandlerImpl::deleteOldReports(size_t nbReportsToKeep) {
	
	Autolock autoLock(&m_Lock);
	
	if(strlen(m_pCrashInfo->crashReportFolder) == 0) {
		LogError << "Report location has not been specified";
		return false;
	}
	
	// Exit if there is no crash report folder yet...
	fs::path location(m_pCrashInfo->crashReportFolder);
	if(!fs::is_directory(location)) {
		return true;
	}
	
	typedef std::multimap<std::time_t, fs::path> CrashReportMap;
	CrashReportMap oldCrashes;
	
	for(fs::directory_iterator it(location); !it.end(); ++it) {
		fs::path path = location / it.name();
		if(fs::is_directory(path)) {
			oldCrashes.insert(CrashReportMap::value_type(fs::last_write_time(path), path));
		} else {
			fs::remove(path);
		}
	}
	
	// Nothing to delete
	if(nbReportsToKeep >= oldCrashes.size()) {
		return true;
	}
	
	int nbReportsToDelete = oldCrashes.size() - nbReportsToKeep;
	
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
