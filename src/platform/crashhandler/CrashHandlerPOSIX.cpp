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

#include "platform/crashhandler/CrashHandlerPOSIX.h"

#include <cstring>
#include <sstream>

#include "Configure.h"

#if ARX_HAVE_BACKTRACE
#include <execinfo.h>
#endif

#if ARX_HAVE_PRCTL
#include <sys/prctl.h>
#ifndef PR_SET_PTRACER
#define PR_SET_PTRACER 0x59616d61
#endif
#endif

#if ARX_HAVE_WAITPID
#include <sys/wait.h>
#endif

#if ARX_HAVE_NANOSLEEP
#include <time.h>
#endif

#if ARX_HAVE_SETRLIMIT
#include <sys/resource.h>
#endif

#if ARX_HAVE_UNAME
#include <sys/utsname.h>
#endif

#if ARX_HAVE_SYSCTLBYNAME
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if ARX_HAVE_SIGACTION && ARX_PLATFORM == ARX_PLATFORM_LINUX
#include <ucontext.h>
#endif

#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/range/size.hpp>

#include "io/fs/FilePath.h"
#include "io/fs/Filesystem.h"

#include "platform/Architecture.h"
#include "platform/Environment.h"
#include "platform/Process.h"
#include "platform/Thread.h"

#include "util/String.h"


#if ARX_HAVE_SIGACTION

static void signalHandler(int signal, siginfo_t * info, void * context) {
	CrashHandlerPOSIX::getInstance().handleCrash(signal, info, context);
}

typedef struct sigaction signal_handler;

static void registerSignalHandler(int s, signal_handler & old_handler) {
	struct sigaction handler;
	handler.sa_flags = SA_RESETHAND | SA_SIGINFO;
	handler.sa_sigaction = signalHandler;
	sigemptyset(&handler.sa_mask);
	sigaction(s, &handler, &old_handler);
}

static void unregisterSignalHandler(int s, signal_handler & old_handler) {
	sigaction(s, &old_handler, NULL);
}

#else

static void signalHandler(int signal) {
	CrashHandlerPOSIX::getInstance().handleCrash(signal, NULL, NULL);
}

typedef void (*signal_handler)(int signal);

static void registerSignalHandler(int s, signal_handler & old_handler) {
	old_handler = signal(s, signalHandler);
}

static void unregisterSignalHandler(int s, signal_handler & old_handler) {
	signal(s, old_handler);
}

#endif

struct PlatformCrashHandlers {
	
	signal_handler illHandler;
	signal_handler abrtHandler;
	signal_handler busHandler;
	signal_handler fpeHandler;
	signal_handler segvHandler;
	
};

CrashHandlerPOSIX * CrashHandlerPOSIX::m_sInstance = 0;

CrashHandlerPOSIX::CrashHandlerPOSIX() : m_pPreviousCrashHandlers(NULL) {
	m_sInstance = this;
}

static fs::path getCoreDumpFile() {
	
	#if ARX_PLATFORM == ARX_PLATFORM_LINUX
	
	std::string pattern = fs::read("/proc/sys/kernel/core_pattern");
	std::string usesPID = fs::read("/proc/sys/kernel/core_uses_pid");
	boost::trim(pattern), boost::trim(usesPID);
	
	if(pattern.empty()) {
		if(usesPID.empty() || usesPID == "0") {
			return "core";
		} else {
			std::ostringstream oss;
			oss << "core." << platform::getProcessId();
			return oss.str();
		}
	}
	
	if(pattern[0] == '|') {
		if(pattern.find("/apport ") != std::string::npos) {
			// Ubuntu …
			std::ostringstream oss;
			oss << "/var/crash/";
			std::string exe = platform::getExecutablePath().string();
			std::replace(exe.begin(), exe.end(), '/', '_');
			oss << exe << '.' << getuid() << ".crash";
			return oss.str();
		}
		// Unknown system crash handler
		return fs::path();
	}
	
	
	bool hasPID = false;
	std::ostringstream oss;
	size_t start = 0;
	while(start < pattern.length()) {
		
		size_t end = pattern.find('%');
		if(end == std::string::npos) {
			end = pattern.length();
		}
		
		oss.write(&pattern[start], end - start);
		
		if(end + 1 >= pattern.length()) {
			break;
		}
		
		switch(pattern[end + 1]) {
			case '%': oss << '%'; break;
			case 'p': oss << platform::getProcessId(); hasPID = true; break;
			#if ARX_HAVE_GETUID
			case 'u': oss << getuid(); break;
			#endif
			#if ARX_HAVE_GETGID
			case 'g': oss << getgid(); break;
			#endif
			#ifdef SIGABRT
			case 's': oss << SIGABRT; break;
			#endif
			#if ARX_HAVE_UNAME
			case 'h': {
				utsname info;
				if(uname(&info) < 0) {
					return fs::path();
				}
				oss << info.nodename;
				break;
			}
			#endif
			case 'e': oss << platform::getExecutablePath().filename(); break;
			case 'E': {
				std::string exe = platform::getExecutablePath().string();
				std::replace(exe.begin(), exe.end(), '/', '!');
				oss << exe << '.' << getuid() << ".crash";
				break;
			}
			default: {
				// Unknown or unsupported pattern
				return fs::path();
			}
		}
		
		start = end + 2;
	}
	
	if(!hasPID && usesPID != "0") {
		oss << '.' << platform::getProcessId();
	}
	
	return oss.str();
	
	#elif ARX_PLATFORM == ARX_PLATFORM_BSD
	
	std::string pattern;
	#if ARX_HAVE_SYSCTLBYNAME && defined(PATH_MAX)
	char pathname[PATH_MAX];
	size_t size = sizeof(pathname);
	int error = sysctlbyname("kern.corefile", pathname, &size, NULL, 0);
	if(error != -1 || size > 0 || size <= sizeof(pathname)) {
		pattern = util::loadString(pathname, size);
	} else {
	#endif
		pattern = "%N.core";
	#if ARX_HAVE_SYSCTLBYNAME && defined(PATH_MAX)
	}
	#endif
	
	std::ostringstream oss;
	size_t start = 0;
	while(start < pattern.length()) {
		
		size_t end = pattern.find('%');
		if(end == std::string::npos) {
			end = pattern.length();
		}
		
		oss.write(&pattern[start], end - start);
		
		if(end + 1 >= pattern.length()) {
			break;
		}
		
		switch(pattern[end + 1]) {
			case '%': oss << '%'; break;
			#if ARX_HAVE_UNAME
			case 'H': {
				utsname info;
				if(uname(&info) < 0) {
					return fs::path();
				}
				oss << info.nodename;
				break;
			}
			#endif
			case 'N': oss << platform::getExecutablePath().filename(); break;
			case 'P': oss << platform::getProcessId();  break;
			#if ARX_HAVE_GETUID
			case 'U': oss << getuid(); break;
			#endif
			default: {
				// Unknown or unsupported pattern
				return fs::path();
			}
		}
		
		start = end + 2;
	}
	
	return oss.str();
	
	#else
	return fs::path();
	#endif
	
}

bool CrashHandlerPOSIX::initialize() {
	
	if(!CrashHandlerImpl::initialize()) {
		return false;
	}
	
	m_arg = "--crashinfo=" + m_SharedMemoryName;
	
	std::memset(m_pCrashInfo->backtrace, 0, sizeof(m_pCrashInfo->backtrace));
	
	#if ARX_HAVE_PRCTL
	// Allow all processes in the same pid namespace to PTRACE this process
	prctl(PR_SET_PTRACER, platform::getProcessId());
	#endif
	
	m_pCrashInfo->coreDumpFile[0] = '\0';
	fs::path core = getCoreDumpFile();
	if(!core.empty() && core.string().length() < size_t(boost::size(m_pCrashInfo->coreDumpFile))) {
		util::storeStringTerminated(m_pCrashInfo->coreDumpFile, core.string());
		#if ARX_HAVE_SETRLIMIT
		struct rlimit core_limit;
		core_limit.rlim_cur = RLIM_INFINITY;
		core_limit.rlim_max = RLIM_INFINITY;
		(void)setrlimit(RLIMIT_CORE, &core_limit);
		#endif
	}
	
	return true;
}

CrashHandlerPOSIX::~CrashHandlerPOSIX() {
	m_sInstance = 0;
}

CrashHandlerPOSIX & CrashHandlerPOSIX::getInstance() {
	arx_assert(m_sInstance != 0);
	return *m_sInstance;
}

bool CrashHandlerPOSIX::registerCrashHandlers() {
	
	arx_assert(m_pPreviousCrashHandlers == 0);
	m_pPreviousCrashHandlers = new PlatformCrashHandlers;
	
	// Catch 'bad' signals so we can print some debug output.
	
#ifdef SIGILL
	registerSignalHandler(SIGILL, m_pPreviousCrashHandlers->illHandler);
#endif
	
#ifdef SIGABRT
	registerSignalHandler(SIGABRT, m_pPreviousCrashHandlers->abrtHandler);
#endif
	
#ifdef SIGBUS
	registerSignalHandler(SIGBUS, m_pPreviousCrashHandlers->busHandler);
#endif
	
#ifdef SIGFPE
	registerSignalHandler(SIGFPE, m_pPreviousCrashHandlers->fpeHandler);
#endif
	
#ifdef SIGSEGV
	registerSignalHandler(SIGSEGV, m_pPreviousCrashHandlers->segvHandler);
#endif
	
	// We must also register the main thread crash handlers.
	return registerThreadCrashHandlers();
}

static void removeCrashHandlers(PlatformCrashHandlers * previous) {
	
#ifdef SIGILL
	unregisterSignalHandler(SIGILL, previous->illHandler);
#endif
	
#ifdef SIGABRT
	unregisterSignalHandler(SIGABRT, previous->abrtHandler);
#endif
	
#ifdef SIGBUS
	unregisterSignalHandler(SIGBUS, previous->busHandler);
#endif
	
#ifdef SIGFPE
	unregisterSignalHandler(SIGFPE, previous->fpeHandler);
#endif
	
#ifdef SIGSEGV
	unregisterSignalHandler(SIGSEGV, previous->segvHandler);
#endif
	
}

void CrashHandlerPOSIX::unregisterCrashHandlers() {
	
	unregisterThreadCrashHandlers();
	
	removeCrashHandlers(m_pPreviousCrashHandlers);
	
	delete m_pPreviousCrashHandlers;
	m_pPreviousCrashHandlers = 0;
}


bool CrashHandlerPOSIX::registerThreadCrashHandlers() {
	// All POSIX signals are process wide, so no thread specific actions are needed
	return true;
}

void CrashHandlerPOSIX::unregisterThreadCrashHandlers() {
	// All POSIX signals are process wide, so no thread specific actions are needed
}

void CrashHandlerPOSIX::handleCrash(int signal, void * info, void * context) {
	
	// Remove crash handlers so we don't end in an infinite crash loop
	removeCrashHandlers(m_pPreviousCrashHandlers);
	
	// Run the callbacks
	for(std::vector<CrashHandler::CrashCallback>::iterator it = m_crashCallbacks.begin();
	    it != m_crashCallbacks.end(); ++it) {
		(*it)();
	}
	
	m_pCrashInfo->signal = signal;
	
	#if ARX_HAVE_SIGACTION
	if(info) {
		siginfo_t * siginfo = reinterpret_cast<siginfo_t *>(info);
		m_pCrashInfo->code = siginfo->si_code;
		#if defined(SIGILL) || defined(SIGFPE)
		if(signal == SIGILL || signal == SIGFPE) {
			m_pCrashInfo->address = u64(siginfo->si_addr);
			m_pCrashInfo->hasAddress = true;
		}
		#endif
		#if defined(SIGSEGV) && defined(SIGBUS)
		if(signal == SIGSEGV || signal == SIGBUS) {
			m_pCrashInfo->memory = u64(siginfo->si_addr);
			m_pCrashInfo->hasMemory = true;
		}
		#endif
	}
	#endif
	
	#if ARX_HAVE_SIGACTION && ARX_PLATFORM == ARX_PLATFORM_LINUX
	if(context) {
		ucontext_t * ctx = reinterpret_cast<ucontext_t *>(context);
		#if ARX_ARCH == ARX_ARCH_X86 && defined(REG_EIP)
		m_pCrashInfo->address = ctx->uc_mcontext.gregs[REG_EIP];
		m_pCrashInfo->hasAddress = true;
		m_pCrashInfo->stack = ctx->uc_mcontext.gregs[REG_ESP];
		m_pCrashInfo->hasStack = true;
		m_pCrashInfo->frame = ctx->uc_mcontext.gregs[REG_EBP];
		m_pCrashInfo->hasFrame = true;
		#elif ARX_ARCH == ARX_ARCH_X86_64 && defined(REG_RIP)
		m_pCrashInfo->address = ctx->uc_mcontext.gregs[REG_RIP];
		m_pCrashInfo->hasAddress = true;
		m_pCrashInfo->stack = ctx->uc_mcontext.gregs[REG_RSP];
		m_pCrashInfo->hasStack = true;
		#elif ARX_ARCH == ARX_ARCH_ARM
		m_pCrashInfo->address =  ctx->uc_mcontext.arm_pc;
		m_pCrashInfo->hasAddress = true;
		m_pCrashInfo->stack =  ctx->uc_mcontext.arm_sp;
		m_pCrashInfo->hasStack = true;
		m_pCrashInfo->frame = ctx->uc_mcontext.arm_fp;
		m_pCrashInfo->hasFrame = true;
		#else
		ARX_UNUSED(ctx);
		#endif
	}
	#else
	ARX_UNUSED(context);
	#endif
	
	// Store the backtrace in the shared crash info
	#if ARX_HAVE_BACKTRACE
	backtrace(m_pCrashInfo->backtrace, boost::size(m_pCrashInfo->backtrace));
	#endif
	
	// Change directory core dumps are written to
	if(m_pCrashInfo->crashReportFolder[0] != '\0') {
		#if ARX_HAVE_CHDIR
		if(chdir(m_pCrashInfo->crashReportFolder) == 0) {
			// Shut up GCC, we don't care
		}
		#endif
	}
	
	// Try to spawn a sub-process to process the crash info
	// Using fork() in a signal handler is bad, but we are already crashing anyway
	pid_t processor = fork();
	if(processor > 0) {
		while(true) {
			if(m_pCrashInfo->exitLock.try_wait()) {
				break;
			}
			#if ARX_HAVE_WAITPID
			if(waitpid(processor, NULL, WNOHANG) != 0) {
				break;
			}
			#endif
			#if ARX_HAVE_NANOSLEEP
			timespec t;
			t.tv_sec = 0;
			t.tv_nsec = 100 * 1000;
			nanosleep(&t, NULL);
			#endif
		}
		// Exit if the crash reporter failed
		kill(getpid(), SIGKILL);
		std::abort();
	}
	#ifdef ARX_HAVE_EXECVP
	const char * args[] = { m_executable.string().c_str(), m_arg.c_str(), NULL };
	execvp(m_executable.string().c_str(), const_cast<char **>(args));
	#endif
	
	// Fallback: process the crash info in-process
	processCrash();
	
	std::abort();
}
