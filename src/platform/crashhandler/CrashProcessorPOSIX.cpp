/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/crashhandler/CrashHandlerPOSIX.h"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>

#include <signal.h>
#include <unistd.h>

#include "Configure.h"

#if ARX_HAVE_GETRUSAGE
#include <sys/resource.h>
#include <sys/time.h>
#endif

#if ARX_HAVE_PRCTL
#include <sys/prctl.h>
#ifndef PR_SET_PTRACER
#define PR_SET_PTRACER 0x59616d61
#endif
#endif

#include <boost/crc.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/range/size.hpp>

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"

#include "platform/Process.h"
#include "platform/Thread.h"

#include "util/String.h"


static void getProcessSatus(platform::process_id pid, u64 & rss, u64 & startTicks) {
	
	rss = startTicks = 0;
	
	std::ostringstream oss;
	oss << "/proc/" << pid << "/stat";
	std::istringstream iss(fs::read(oss.str()));
	
	const size_t rssIndex = 23;
	const size_t startTicksIndex = 21;
	for(size_t i = 0; iss.good(); i++) {
		if(i == rssIndex) {
			iss >> rss;
		} else if(i == startTicksIndex) {
			iss >> startTicks;
		}
		iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
	}
	
}

void CrashHandlerPOSIX::processCrashInfo() {
	
	#if ARX_HAVE_GETRUSAGE && ARX_PLATFORM != ARX_PLATFORM_MACOS
	{
		struct rusage usage;
		if(getrusage(m_pCrashInfo->processId, &usage) == 0) {
			m_pCrashInfo->memoryUsage = usage.ru_maxrss * 1024;
		}
	}
	#endif
	
	#if ARX_HAVE_SYSCONF && (defined(_SC_PAGESIZE) || defined(_SC_CLK_TCK))
	
	u64 rss, startTicks, endTicks, dummy;
	
	getProcessSatus(m_pCrashInfo->processId, rss, startTicks);
	
	// Get the rss memory usage from /proc/pid/stat
	#ifdef _SC_PAGESIZE
	if(rss != 0) {
		long pagesize = sysconf(_SC_PAGESIZE);
		if(pagesize > 0) {
			m_pCrashInfo->memoryUsage = rss * pagesize;
		}
	}
	#endif
	
	#ifdef _SC_CLK_TCK
	
	if(startTicks == 0) {
		return;
	}
	
	pid_t child = fork();
	if(!child) {
		while(true) {
			// wait
		}
	}
	
	getProcessSatus(child, dummy, endTicks);
	kill(child, SIGTERM);
	
	if(startTicks != 0 && endTicks != 0) {
		u64 ticksPerSecond = sysconf(_SC_CLK_TCK);
		if(ticksPerSecond > 0) {
			m_pCrashInfo->runningTime = double(endTicks - startTicks) / double(ticksPerSecond);
		}
	}
	
	#endif
	
	#endif
	
}

void CrashHandlerPOSIX::processCrashSignal() {
	
	std::ostringstream description;
	
	switch(m_pCrashInfo->signal) {
		
		#ifdef SIGILL
		case SIGILL: {
			description << "Illegal instruction";
			switch(m_pCrashInfo->code) {
				#ifdef ILL_ILLOPC
				case ILL_ILLOPC: description << ": illegal opcode"; break;
				#endif
				#ifdef ILL_ILLOPN
				case ILL_ILLOPN: description << ": illegal operand"; break;
				#endif
				#ifdef ILL_ADR
				case ILL_ADR: description << ": illegal addressing mode"); break;
				#endif
				#ifdef ILL_ILLTRP
				case ILL_ILLTRP: description << ": illegal trap"; break;
				#endif
				#ifdef ILL_PRVOPC
				case ILL_PRVOPC: description << ": privileged opcode"; break;
				#endif
				#ifdef ILL_PRVREG
				case ILL_PRVREG: description << ": privileged register"; break;
				#endif
				#ifdef ILL_COPROC
				case ILL_COPROC: description << ": coprocessor error"; break;
				#endif
				#ifdef ILL_BADSTK
				case ILL_BADSTK: description << ": internal stack error"; break;
				#endif
				default: break;
			}
			break;
		}
		#endif
		
		#ifdef SIGABRT
		case SIGABRT: {
			description << "Abnormal termination";
			break;
		}
		#endif
		
		#ifdef SIGBUS
		case SIGBUS: {
			description << "Bus error";
			switch(m_pCrashInfo->code) {
				#ifdef BUS_ADRALN
				case BUS_ADRALN: description << ": invalid address alignment"; break;
				#endif
				#ifdef BUS_ADRERR
				case BUS_ADRERR: description << ": non-existent physical address"; break;
				#endif
				#ifdef BUS_OBJERR
				case BUS_OBJERR: description << ": object specific hardware error"; break;
				#endif
				default: break;
			}
			break;
		}
		#endif
		
		#ifdef SIGFPE
		case SIGFPE: {
			description << "Floating-point error";
			switch(m_pCrashInfo->code) {
				#ifdef FPE_INTDIV
				case FPE_INTDIV: description << ": integer division by zero"; break;
				#endif
				#ifdef FPE_INTOVF
				case FPE_INTOVF: description << ": integer overflow"; break;
				#endif
				#ifdef FPE_FLTDIV
				case FPE_FLTDIV: description << ": floating point division by zero"; break;
				#endif
				#ifdef FPE_FLTOVF
				case FPE_FLTOVF: description << ": floating point overflow"; break;
				#endif
				#ifdef FPE_FLTUND
				case FPE_FLTUND: description << ": floating point underflow"; break;
				#endif
				#ifdef FPE_FLTRES
				case FPE_FLTRES: description << ": floating point inexact result"; break;
				#endif
				#ifdef FPE_FLTINV
				case FPE_FLTINV: description << ": invalid floating point operation"; break;
				#endif
				#ifdef FPE_FLTSUB
				case FPE_FLTSUB: description << ": subscript out of range"; break;
				#endif
				default: break;
			}
			break;
		}
		#endif
		
		#ifdef SIGSEGV
		case SIGSEGV: {
			description << "Illegal storage access";
			switch(m_pCrashInfo->code) {
				#ifdef SEGV_MAPERR
				case SEGV_MAPERR: {
					description << ": address not mapped to object";
					break;
				}
				#endif
				#ifdef SEGV_ACCERR
				case SEGV_ACCERR: {
					description << ": invalid permissions for mapped object";
					break;
				}
				#endif
				default: break;
			}
			break;
		}
		#endif
		
		default: {
			description << "Received signal " << m_pCrashInfo->signal;
			break;
		}
		
	}
	
	description << "\n\n";
	
	addText(description.str().c_str());
}

struct MemoryRegion {
	intptr_t begin;
	intptr_t offset;
	std::string file;
};

void CrashHandlerPOSIX::processCrashTrace() {
	
	std::ostringstream description;
	
	std::string maps;
	{
		std::ostringstream oss;
		oss << "/proc/" << m_pCrashInfo->processId << "/maps";
		maps = fs::read(oss.str());
		if(!maps.empty()) {
			fs::path file = m_crashReportDir / "maps.txt";
			if(fs::write(file, maps)) {
				addAttachedFile(file);
			}
		}
	}
	
	enum FrameType { Handler = 0, System = 1, Fault = 2, Done = 3 };
	#if ARX_HAVE_BACKTRACE
	if(m_pCrashInfo->backtrace[0] != 0) {
		std::map<intptr_t, MemoryRegion> regions;
		if(!maps.empty()) {
			std::istringstream iss(maps);
			while(iss.good()) {
				MemoryRegion region;
				iss >> std::hex >> region.begin;
				if(iss.get() != '-') {
					continue;
				}
				intptr_t end;
				iss >> std::hex >> end;
				if(iss.get() != ' ') {
					continue;
				}
				iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
				iss >> std::hex >> region.offset;
				if(iss.get() != ' ') {
					continue;
				}
				iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
				iss.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
				std::getline(iss, region.file);
				boost::trim(region.file);
				if(!iss.fail() && !region.file.empty()) {
					region.file = fs::path(region.file).filename();
					regions[end] = region;
				}
			}
		}
		description << "\nCallstack:\n";
		boost::crc_32_type checksum;
		std::string exe = fs::path(util::loadString(m_pCrashInfo->executablePath)).filename();
		FrameType status = Handler;
		u64 minOffset = std::numeric_limits<u64>::max();
		for(size_t i = 0; i < size_t(boost::size(m_pCrashInfo->backtrace)); i++) {
			if(m_pCrashInfo->backtrace[i] == 0) {
				break;
			}
			std::ostringstream line;
			intptr_t value = intptr_t(m_pCrashInfo->backtrace[i]);
			// Map the frame address to a mapped executable / library
			if(!regions.empty()) {
				std::map<intptr_t, MemoryRegion>::const_iterator it = regions.lower_bound(value);
				if(it != regions.end() && value >= it->second.begin) {
					line << it->second.file;
					checksum.process_bytes(it->second.file.data(), it->second.file.length());
					value = it->second.offset + (value - it->second.begin);
					if(status == Handler && it->second.file != exe) {
						status = System;
					}
				} else {
					line << "??";
				}
				line << '!';
			}
			checksum.process_bytes(&value, sizeof(value));
			line << "0x" << std::hex << intptr_t(m_pCrashInfo->backtrace[i]);
			description << ' ' << line.str() << '\n';
			// Guess fault frame based on the offset to the fault address
			if(m_pCrashInfo->address && m_pCrashInfo->address >= u64(m_pCrashInfo->backtrace[i])) {
				u64 offset = m_pCrashInfo->address - u64(m_pCrashInfo->backtrace[i]);
				if(offset < minOffset) {
					util::storeStringTerminated(m_pCrashInfo->title, line.str());
					status = Done;
					minOffset = offset;
				}
			}
			// Guess fault frame based on the offset to the fault address
			if((i == 2 && status != Done) || status == System) {
				util::storeStringTerminated(m_pCrashInfo->title, line.str());
				status = Done;
			} else if(status == System) {
				status = Fault;
			}
		}
		m_pCrashInfo->crashId = checksum.checksum();
	}
	#else
	ARX_UNUSED(maps);
	#endif
	
	// Get a stack trace via GDB
	if(platform::isProcessRunning(m_pCrashInfo->processId)) {
		std::string pid = boost::lexical_cast<std::string>(m_pCrashInfo->processId);
		const char * args[] = {
			"gdb", "--batch", "-n",
			"-ex", "thread",
			"-ex", "set confirm off",
			"-ex", "set print frame-arguments all",
			"-ex", "set print static-members off",
			"-ex", "info threads",
			"-ex", "thread apply all bt full",
			"--pid", pid.c_str(), NULL
		};
		std::string gdbstdout = platform::getOutputOf(args, /*unlocalized=*/ true);
		if(!gdbstdout.empty()) {
			description << "\nGDB stack trace:\n";
			std::istringstream iss(gdbstdout);
			std::string line;
			FrameType status = Handler;
			while(iss.good()) {
				std::getline(iss, line);
				if(status == Handler && line.find("<signal handler called>") != std::string::npos) {
					status = Fault;
				} else if(status == Fault) {
					size_t start = line.find('#');
					if(start != std::string::npos) {
						size_t sstart = line.find(" in ", start);
						if(sstart != std::string::npos) {
							start = sstart + 4;
						}
						std::string function = line.substr(start);
						size_t pstart = function.find('(');
						if(pstart != std::string::npos) {
							if(pstart != 0 && function[pstart - 1] == ' ') {
								function.erase(pstart - 1, 1);
								pstart--;
							}
							size_t pend = function.find(')', pstart);
							if(pend != std::string::npos) {
								function.erase(pstart + 1, pend - pstart - 1);
							}
						}
						size_t fstart = function.find(") at");
						if(fstart != std::string::npos) {
							size_t fend = function.find_last_of('/');
							if(fend != std::string::npos && fend > fstart) {
								function.erase(fstart + 2, fend + 1 - fstart - 2);
							}
						}
						boost::trim(function);
						if(!function.empty()) {
							util::storeStringTerminated(m_pCrashInfo->title, function);
						}
						status = Done;
					}
				}
				description << ' ' << line << '\n';
			}
		}
	}
	
	addText(description.str().c_str());
}

void CrashHandlerPOSIX::processCrashDump() {
	
	// Generate a core dump
	fs::path coredump = m_crashReportDir / "crash.dmp";
	#if ARX_HAVE_KILL && defined(SIGABRT)
	if(m_pCrashInfo->coreDumpFile[0] != '\0'
	   && m_pCrashInfo->processId != platform::getProcessId()) {
		kill(m_pCrashInfo->processId, SIGABRT);
		for(size_t i = 1; platform::isProcessRunning(m_pCrashInfo->processId) && i < 10; i++) {
			Thread::sleep(PlatformDurationMs(100));
		}
		fs::path core = util::loadString(m_pCrashInfo->coreDumpFile);
		fs::path crashReportDir = m_crashReportDir.parent();
		if(core.is_relative() && !crashReportDir.empty() && fs::exists(crashReportDir / core)) {
			fs::rename(crashReportDir / core, coredump);
			addAttachedFile(coredump);
		} else if(fs::exists(core)) {
			if(core.is_absolute()) {
				u64 oldsize, newsize = fs::file_size(core);
				do {
					oldsize = newsize;
					Thread::sleep(PlatformDurationMs(500));
					newsize = fs::file_size(core);
				} while(newsize != oldsize);
			}
			fs::copy_file(core, coredump);
			addAttachedFile(coredump);
		}
	}
	else
	#endif
	if(platform::isProcessRunning(m_pCrashInfo->processId)) {
		std::string pid = boost::lexical_cast<std::string>(m_pCrashInfo->processId);
		const char * command[] = { "gcore", "-o", "arx-crash-core-dump", pid.c_str(), NULL };
		(void)platform::runHelper(command, true);
		if(fs::exists("arx-crash-core-dump")) {
			fs::rename("arx-crash-core-dump", coredump);
			addAttachedFile(coredump);
		} else {
			fs::path filename = "arx-crash-core-dump." + pid;
			if(fs::exists(filename)) {
				fs::rename(filename, coredump);
				addAttachedFile(coredump);
			}
		}
	}
	
}
