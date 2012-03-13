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

#include "platform/crashhandler/CrashHandlerPOSIX.h"

#include "Configure.h"

#if defined(HAVE_BACKTRACE)
#include <execinfo.h>
#endif

#if defined(HAVE_PRCTL)
#include <sys/prctl.h>
#ifndef PR_SET_PTRACER
#define PR_SET_PTRACER 0x59616d61
#endif
#endif

#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "platform/Environment.h"

typedef void (*signal_handler)(int signal);

static void SignalHandler(int signalCode) {
	CrashHandlerPOSIX::getInstance().handleCrash(signalCode);
}

static void SIGFPEHandler(int signalCode, int FPECode) {
	CrashHandlerPOSIX::getInstance().handleCrash(signalCode, FPECode);
}

struct PlatformCrashHandlers {
	signal_handler m_SIGSEGVHandler;    // Illegal storage access handler.
	signal_handler m_SIGILLHandler;     // SIGINT handler.
	signal_handler m_SIGFPEHandler;     // FPE handler.
	signal_handler m_SIGABRTHandler;    // SIGABRT handler.
};

CrashHandlerPOSIX* CrashHandlerPOSIX::m_sInstance = 0;

CrashHandlerPOSIX::CrashHandlerPOSIX() {
	m_sInstance = this;
	m_CrashHandlerApp = "./arxcrashreporter";

#if defined(HAVE_PRCTL)
	// Allow all processes in the same pid namespace to PTRACE this process
	prctl(PR_SET_PTRACER, getpid(), 0, 0, 0);
#endif
}

CrashHandlerPOSIX::~CrashHandlerPOSIX() {
	m_sInstance = 0;
}

CrashHandlerPOSIX& CrashHandlerPOSIX::getInstance() {
	arx_assert(m_sInstance != 0);
	return *m_sInstance;
}

void CrashHandlerPOSIX::fillBasicCrashInfo() {
	CrashHandlerImpl::fillBasicCrashInfo();
	std::string exe = getExecutablePath();
	if(exe.length() < ARRAY_SIZE(m_pCrashInfo->execFullName)) {
		strcpy(m_pCrashInfo->execFullName, exe.c_str());
	} else {
		m_pCrashInfo->execFullName[0] = '\0';
	}
}

bool CrashHandlerPOSIX::registerCrashHandlers() {
	
	arx_assert(m_pPreviousCrashHandlers == 0);
	m_pPreviousCrashHandlers = new PlatformCrashHandlers;
	
	// Catch 'bad' signals so we can print some debug output.
	
#ifdef SIGSEGV
	m_pPreviousCrashHandlers->m_SIGSEGVHandler = signal(SIGSEGV, SignalHandler);
#endif
	
#ifdef SIGILL
	m_pPreviousCrashHandlers->m_SIGILLHandler = signal(SIGILL, SignalHandler);
#endif
	
	// TODO(crash-handler) is the cast OK?
#ifdef SIGFPE
	m_pPreviousCrashHandlers->m_SIGFPEHandler = signal(SIGFPE, signal_handler(SIGFPEHandler));
#endif
	
#ifdef SIGABRT
	m_pPreviousCrashHandlers->m_SIGABRTHandler = signal(SIGABRT, SignalHandler);
#endif
	
	// We must also register the main thread crash handlers.
	return registerThreadCrashHandlers();
}

static void removeCrashHandlers(PlatformCrashHandlers * previous) {
	
#ifdef SIGSEGV
	signal(SIGSEGV, previous->m_SIGSEGVHandler);
#endif
	
#ifdef SIGILL
	signal(SIGILL, previous->m_SIGILLHandler);
#endif
	
#ifdef SIGFPE
	signal(SIGFPE, previous->m_SIGFPEHandler);
#endif
	
#ifdef SIGABRT
	signal(SIGABRT, previous->m_SIGABRTHandler);
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

PlatformCrashHandlers nullHandlers = { 0, 0, 0, 0 };

void CrashHandlerPOSIX::handleCrash(int crashType, int FPECode) {
	
	// Remove crash handlers so we don't end in an infinite crash loop
	removeCrashHandlers(&nullHandlers);
	
	// Run the callbacks
	for(std::vector<CrashHandler::CrashCallback>::iterator it = m_crashCallbacks.begin();
	    it != m_crashCallbacks.end(); ++it) {
		(*it)();
	}
	
	const char* crashSummary;
	switch(crashType) {
		case SIGABRT:     crashSummary = "Abnormal termination"; break;
		case SIGFPE:      crashSummary = "Floating-point error"; break;
		case SIGILL:      crashSummary = "Illegal instruction"; break;
		case SIGSEGV:     crashSummary = "Illegal storage access"; break;
		default:          crashSummary = 0; break;
	}
	
	if(crashSummary != 0) {
		strcpy(m_pCrashInfo->detailedCrashInfo, crashSummary);
	} else {
		sprintf(m_pCrashInfo->detailedCrashInfo, "Received signal #%d", crashType);
	}
	
	if(crashType == SIGFPE) {
		// Append detailed information in case of a FPE exception
		const char* FPEDetailed;
		switch(FPECode) {
			#ifdef FPE_INTDIV
			case FPE_INTDIV: FPEDetailed = ": Integer divide by zero"; break;
			#endif
			
			#ifdef FPE_INTOVF
			case FPE_INTOVF: FPEDetailed = ": Integer overflow"; break;
			#endif
			
			#ifdef FPE_FLTDIV
			case FPE_FLTDIV: FPEDetailed = ": Floating point divide by zero"; break;
			#endif
			
			#ifdef FPE_FLTOVF
			case FPE_FLTOVF: FPEDetailed = ": Floating point overflow"; break;
			#endif
			
			#ifdef FPE_FLTUND
			case FPE_FLTUND: FPEDetailed = ": Floating point underflow"; break;
			#endif
			
			#ifdef FPE_FLTRES
			case FPE_FLTRES: FPEDetailed = ": Floating point inexact result"; break;
			#endif
			
			#ifdef FPE_FLTINV
			case FPE_FLTINV: FPEDetailed = ": Floating point invalid operation"; break;
			#endif
			
			#ifdef FPE_FLTSUB
			case FPE_FLTSUB: FPEDetailed = ": Subscript out of range"; break;
			#endif
		
			default:                  FPEDetailed = "";
		}
		strcat(m_pCrashInfo->detailedCrashInfo, FPEDetailed);
	}
	strcat(m_pCrashInfo->detailedCrashInfo, "\n\n");
	
	// Store the backtrace in the shared crash info
	#ifdef HAVE_BACKTRACE
		backtrace(m_pCrashInfo->backtrace, ARRAY_SIZE(m_pCrashInfo->backtrace));
	#endif
	
	fflush(stdout), fflush(stderr);
	
	// Get current thread id
	m_pCrashInfo->threadId = Thread::getCurrentThreadId();
	
#ifdef HAVE_GETRUSAGE
	m_pCrashInfo->have_rusage = (getrusage(RUSAGE_SELF, &m_pCrashInfo->rusage) == 0);
#else
	m_pCrashInfo->have_rusage = false;
#endif

	if(fork()) {
		while(true) {
			// Busy wait so we don't enter any additional stack frames and keep the backtrace clean.
		}
	} else {
		
		int killer = fork();
		if(!killer) {
			
			// Wait for the CrashReporter signal before exiting.
			m_pCrashInfo->exitLock.wait();
			
			// Kill the original, busy-waiting process.
			kill(m_pCrashInfo->processId, crashType);
			
			exit(1);
			
		} else {
			
			char arguments[256];
			strcpy(arguments, "-crashinfo=");
			strcat(arguments, m_SharedMemoryName.c_str());
			
			// Try a the crash reporter in the same directory as arx or in the current directory.
#ifdef HAVE_EXECL
			execl(m_CrashHandlerApp.c_str(), "arxcrashreporter", arguments, NULL);
#endif
			
			// Try a crash reporter in the system path.
#ifdef HAVE_EXECLP
			execlp("arxcrashreporter", "arxcrashreporter", arguments, NULL);
#endif
			
			// TODO(crash-handler) start fallback in-process crash handler and dump everything to file
			
			m_pCrashInfo->exitLock.post(); // Post signal in case of failure
			
			exit(1);
		}
	}
}
