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
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/range/size.hpp>

#include "core/URLConstants.h"
#include "core/Version.h"

#include "io/fs/Filesystem.h"
#include "io/fs/FilePath.h"
#include "io/fs/FileStream.h"

#include "platform/Architecture.h"
#include "platform/Dialog.h"
#include "platform/Environment.h"
#include "platform/OS.h"
#include "platform/Process.h"

#include "util/String.h"

namespace bip = boost::interprocess;

void CrashHandlerImpl::processCrash(const std::string & sharedMemoryName) {
	
	try {
		
		m_SharedMemoryName = sharedMemoryName;
		
		// Create a shared memory object.
		m_SharedMemory = bip::shared_memory_object(bip::open_only, m_SharedMemoryName.c_str(),
		                                           bip::read_write);
		
		// Map the whole shared memory in this process
		m_MemoryMappedRegion = bip::mapped_region(m_SharedMemory, bip::read_write);
		
		// Our SharedCrashInfo will be stored in this shared memory.
		m_pCrashInfo = reinterpret_cast<CrashInfo *>(m_MemoryMappedRegion.get_address());
		if(m_pCrashInfo) {
			m_textLength = std::find(m_pCrashInfo->description, m_pCrashInfo->description
			                         + boost::size(m_pCrashInfo->description), '\0')
			               - m_pCrashInfo->description;
		}
		
	} catch(...) {
		// Unexpected error
		m_pCrashInfo = NULL;
	}
	
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
		std::string arg = "--crashinfo=" + m_SharedMemoryName;
		const char * args[] = { executable.string().c_str(), arg.c_str(), NULL };
		reporter = platform::runAsync(args);
	}
	
	if(m_pCrashInfo) {
		
		std::time_t timestamp = std::time(NULL);
		{
			fs::path crashReportDir = util::loadString(m_pCrashInfo->crashReportFolder);
			std::tm * time = std::gmtime(&timestamp);
			for(int count = 0; ; count++) {
				std::ostringstream oss;
				oss << (time->tm_year + 1900) << '-'
				    << std::setfill('0') << std::setw(2) << (time->tm_mon + 1) << '-'
				    << std::setfill('0') << std::setw(2) << time->tm_mday << '-'
				    << std::setfill('0') << std::setw(2) << time->tm_hour << '-'
				    << std::setfill('0') << std::setw(2) << time->tm_min << '-'
				    << std::setfill('0') << std::setw(2) << time->tm_sec << '-'
				    << std::setfill('0') << std::setw(2) << count;
				m_crashReportDir = crashReportDir / oss.str();
				if(!fs::exists(m_crashReportDir)) {
					break;
				}
			}
			fs::create_directories(m_crashReportDir);
			util::storeStringTerminated(m_pCrashInfo->crashReportFolder, m_crashReportDir.string());
		}
		
		processCrashInfo();
		processCrashSignal();
		processCrashRegisters();
		processCrashTrace();
		processCrashDump();
		
		{
			size_t nfiles = std::min(size_t(m_pCrashInfo->nbFilesAttached),
			                         size_t(CrashInfo::MaxNbFiles));
			for(size_t i = 0; i < nfiles; i++) {
				fs::path file = util::loadString(m_pCrashInfo->attachedFiles[i]);
				if(file.empty() || !fs::is_regular_file(file)) {
					continue;
				}
				fs::path copy = m_crashReportDir / file.filename();
				if(copy == file || !fs::copy_file(file, copy)) {
					continue;
				}
				util::storeStringTerminated(m_pCrashInfo->attachedFiles[i], copy.string());
			}
		}
		
		// Terminate the crashed process - we collected all the information we could
		if(m_pCrashInfo->processId != m_pCrashInfo->processorProcessId) {
			m_pCrashInfo->exitLock.post();
			platform::killProcess(m_pCrashInfo->processId);
		}
		
		// Write the crash description to a file
		fs::path crashinfo = m_crashReportDir / "crash.txt";
		fs::ofstream ofs(crashinfo, fs::fstream::out | fs::fstream::trunc);
		if(ofs.is_open()) {
			
			ofs.write(m_pCrashInfo->description, m_textLength);
			
			ofs << '\n';
			
			ofs << "\nProcess information:\n";
			ofs << "- path: " << util::loadString(m_pCrashInfo->executablePath) << '\n';
			ofs << "- version: " << util::loadString(m_pCrashInfo->executableVersion) << '\n';
			if(m_pCrashInfo->memoryUsage != 0) {
				ofs << "- memory usage: " << m_pCrashInfo->memoryUsage << " bytes" << '\n';
			}
			const char * arch = platform::getArchitectureName(m_pCrashInfo->architecture);
			ofs << "- architecture: " << arch << '\n';
			if(m_pCrashInfo->runningTime > 0.0) {
				ofs << "- runnig time: " << m_pCrashInfo->runningTime << " seconds" << '\n';
			}
			std::tm * time = std::gmtime(&timestamp);
			ofs << "- crash time: " << (time->tm_year + 1900) << '-'
			    << std::setfill('0') << std::setw(2) << (time->tm_mon + 1) << '-'
			    << std::setfill('0') << std::setw(2) << time->tm_mday << ' '
			    << std::setfill('0') << std::setw(2) << time->tm_hour << ':'
			    << std::setfill('0') << std::setw(2) << time->tm_min << ':'
			    << std::setfill('0') << std::setw(2) << time->tm_sec << '\n';
			
			ofs << "\nSystem information:\n";
			std::string os = platform::getOSName();
			if(!os.empty()) {
				ofs << "- operating system: " << os << '\n';
			}
			std::string osarch = platform::getOSArchitecture();
			if(!osarch.empty()) {
				ofs << "- architecture: " << osarch << '\n';
			}
			std::string distro = platform::getOSDistribution();
			if(!distro.empty()) {
				ofs << "- distribution: " << distro << '\n';
			}
			std::string libc = platform::getCLibraryVersion();
			if(!libc.empty()) {
				ofs << "- libc: " << libc << '\n';
			}
			std::string threading = platform::getThreadLibraryVersion();
			if(!threading.empty()) {
				ofs << "- threading: " << threading << '\n';
			}
			std::string cpu = platform::getCPUName();
			if(!cpu.empty()) {
				ofs << "- cpu: " << cpu << '\n';
			}
			platform::MemoryInfo memory = platform::getMemoryInfo();
			if(memory.total) {
				ofs << "- total physical memory: " << memory.total << " bytes" << '\n';
			}
			if(memory.available) {
				ofs << "- free physical memory: " << memory.available << " bytes" << '\n';
			}
			
			ofs << "\nVariables:\n";
			size_t nbVariables = std::min(size_t(m_pCrashInfo->nbVariables),
			                              size_t(CrashInfo::MaxNbVariables));
			for(size_t i = 0; i < nbVariables; ++i) {
				ofs << "- " << util::loadString(m_pCrashInfo->variables[i].name) << ": "
				    << util::loadString(m_pCrashInfo->variables[i].value) << '\n';
			}
			
			ofs << "\nAdditional files:\n";
			size_t nfiles = std::min(size_t(m_pCrashInfo->nbFilesAttached),
			                         size_t(CrashInfo::MaxNbFiles));
			for(size_t i = 0; i < nfiles; i++) {
				fs::path file = util::loadString(m_pCrashInfo->attachedFiles[i]);
				if(fs::exists(file) && fs::file_size(file) != 0) {
					ofs << "- " << file.filename() << '\n';
				}
			}
			
			addAttachedFile(crashinfo);
		}
		
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
		if(m_pCrashInfo && m_pCrashInfo->reporterStarted.timed_wait(timeout)) {
			break;
		}
	}
	
	// Tell the crash reporter we are done
	if(m_pCrashInfo) {
		m_pCrashInfo->processorDone.post();
	}
	
	// If the crash reporter started successfully, we are done here
	if(reporter) {
		return;
	}
	
	// The crash reporter is not available or failed to start - provide our own dialog
	{
		std::ostringstream oss;
		oss << arx_name + " crashed!\n\n";
		if(m_pCrashInfo) {
			oss << "Please install arxcrashreporter or manually report the crash to "
			    << url::bug_report << "\n\n";
			oss << "Include the files contained in the following directory in your report:\n";
			oss << " " << m_crashReportDir;
		} else {
			oss << "Additionally, we encountered an unexpected error while collecting"
			       " crash information!\n\n";
		}
		std::cout << "\n\n" << oss.str() << '\n';
		if(m_pCrashInfo) {
			oss << "\n\nClick OK to open that directory now.";
		}
		platform::showErrorDialog(oss.str(), "Fatal Error - " + arx_name);
		if(m_pCrashInfo) {
			platform::launchDefaultProgram(m_crashReportDir.string());
		}
	}
	
}

void CrashHandlerImpl::processCrashRegisters() {
	
	std::ostringstream description;
	
	if(m_pCrashInfo->hasAddress || m_pCrashInfo->hasMemory || m_pCrashInfo->hasStack
	   || m_pCrashInfo->hasFrame) {
		description << '\n';
	}
	if(m_pCrashInfo->hasAddress) {
		description << " Instruction address: 0x" << std::hex << m_pCrashInfo->address << '\n';
	}
	if(m_pCrashInfo->hasMemory) {
		description << " Memory accessed: 0x" << std::hex << m_pCrashInfo->memory << '\n';
	}
	if(m_pCrashInfo->hasStack) {
		description << " Stack pointer: 0x" << std::hex << m_pCrashInfo->stack << '\n';
	}
	if(m_pCrashInfo->hasFrame) {
		description << " Frame pointer: 0x" << std::hex << m_pCrashInfo->frame << '\n';
	}
	description << std::dec;
	
	addText(description.str().c_str());
}
